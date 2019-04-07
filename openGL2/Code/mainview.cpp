#include "mainview.h"
#include "model.h"
#include "vertex.h"

#include <math.h>
#include <QDateTime>

static MainView::ShadingMode shadingMode;

/**
 * @brief MainView::MainView
 *
 * Constructor of MainView
 *
 * @param parent
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent)
{
    qDebug() << "MainView constructor";

    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
}

/**
 * @brief MainView::~MainView
 *
 * Destructor of MainView
 * This is the last function called, before exit of the program
 * Use this to clean up your variables, buffers etc.
 *
 */
MainView::~MainView()
{
    makeCurrent();
    debugLogger->stopLogging();
    delete debugLogger;

    qDebug() << "MainView destructor";
    glDeleteTextures(1, &texture);
    destroyModelBuffers();
    doneCurrent();
}

// --- OpenGL initialization

/**
 * @brief MainView::initializeGL
 *
 * Called upon OpenGL initialization
 * Attaches a debugger and calls other init functions
 */
void MainView::initializeGL()
{
    qDebug() << ":: Initializing OpenGL";
    initializeOpenGLFunctions();

    debugLogger = new QOpenGLDebugLogger();
    connect(debugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)),
             this, SLOT(onMessageLogged(QOpenGLDebugMessage)), Qt::DirectConnection);

    if (debugLogger->initialize())
    {
        qDebug() << ":: Logging initialized";
        debugLogger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
        debugLogger->enableMessages();
    }

    QString glVersion;
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0, 1.0, 0.0, 1.0);

    createShaderProgram();
    loadMesh();
    loadTexture();

    // Initialize tranformations
    updateProjectionTransform();
    updateModelTransforms();
}

void MainView::createShaderProgram()
{
    // Create shader programs
    shaderProgramPhong.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader_phong.glsl");
    shaderProgramPhong.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader_phong.glsl");
    shaderProgramPhong.link();

    shaderProgramNormal.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader_normal.glsl");
    shaderProgramNormal.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader_normal.glsl");
    shaderProgramNormal.link();

    shaderProgramGouraud.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader_gouraud.glsl");
    shaderProgramGouraud.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader_gouraud.glsl");
    shaderProgramGouraud.link();
}

void MainView::loadMesh()
{
    Model model(":/models/cat.obj");
    QVector<QVector3D> vertexCoords = model.getVertices();
    QVector<QVector3D> normalCoords = model.getNormals();
    QVector<QVector2D> textureCoords = model.getTextureCoords();

    meshSize = vertexCoords.size();

    std::vector<Vertex> meshData;
    for (int i = 0; i < meshSize; i++)
        meshData.emplace_back(Vertex(vertexCoords[i], normalCoords[i], textureCoords[i]));

    // Generate VAO
    glGenVertexArrays(1, &meshVAO);
    glBindVertexArray(meshVAO);

    // Generate VBO
    glGenBuffers(1, &meshVBO);
    glBindBuffer(GL_ARRAY_BUFFER, meshVBO);

    // Write the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(Vertex), &meshData.front(), GL_STATIC_DRAW);

    // Set vertex coordinates to location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(0);

    // Set colour coordinates to location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set texture coordinates to location 2
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void MainView::loadTexture()
{
    QImage image{":/textures/cat_diff.png"};
    QVector<quint8> byteImage = imageToBytes(image);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, byteImage.data());
}

// --- OpenGL drawing

/**
 * @brief MainView::paintGL
 *
 * Actual function used for drawing to the screen
 *
 */
void MainView::paintGL()
{
    QMatrix3x3 normalTransform  = meshTransform.normalMatrix();;

    // Clear the screen before rendering
    glClearColor(0.2f, 0.5f, 0.7f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch (shadingMode)
    {
        case PHONG:
            uniformModel = shaderProgramPhong.uniformLocation("modelTransform");
            uniformProjection = shaderProgramPhong.uniformLocation("projectionTransform");
            uniformNormal = shaderProgramPhong.uniformLocation("normalTransform");
            uniformLight = shaderProgramPhong.uniformLocation("lightPosition");
            uniformMaterial = shaderProgramPhong.uniformLocation("material");
            uniformTexColor = shaderProgramPhong.uniformLocation("texColor");
            shaderProgramPhong.bind();
            shaderProgramPhong.setUniformValue(uniformModel, meshTransform);
            shaderProgramPhong.setUniformValue(uniformProjection, projectionTransform);
            shaderProgramPhong.setUniformValue(uniformNormal, normalTransform);
            shaderProgramPhong.setUniformValue(uniformLight, QVector3D(50, 50, 50));
            shaderProgramPhong.setUniformValue(uniformMaterial, QVector4D(0.2f, 0.7f, 0.5f, 4));
            break;
        case NORMAL:
            uniformModel = shaderProgramNormal.uniformLocation("modelTransform");
            uniformProjection = shaderProgramNormal.uniformLocation("projectionTransform");
            uniformNormal = shaderProgramNormal.uniformLocation("normalTransform");
            shaderProgramNormal.bind();
            shaderProgramNormal.setUniformValue(uniformModel, meshTransform);
            shaderProgramNormal.setUniformValue(uniformProjection, projectionTransform);
            shaderProgramNormal.setUniformValue(uniformNormal, normalTransform);
            break;
        case GOURAUD:
            uniformModel = shaderProgramGouraud.uniformLocation("modelTransform");
            uniformProjection = shaderProgramGouraud.uniformLocation("projectionTransform");
            uniformNormal = shaderProgramGouraud.uniformLocation("normalTransform");
            uniformLight = shaderProgramGouraud.uniformLocation("lightPosition");
            uniformMaterial = shaderProgramGouraud.uniformLocation("material");
            uniformTexColor = shaderProgramGouraud.uniformLocation("texColor");
            shaderProgramGouraud.bind();
            shaderProgramGouraud.setUniformValue(uniformModel, meshTransform);
            shaderProgramGouraud.setUniformValue(uniformProjection, projectionTransform);
            shaderProgramGouraud.setUniformValue(uniformNormal, normalTransform);
            shaderProgramGouraud.setUniformValue(uniformLight, QVector3D(50, 50, 50));
            shaderProgramGouraud.setUniformValue(uniformMaterial, QVector4D(0.2f, 0.7f, 0.5f, 4));
            break;
    }

    glUniform1i(uniformTexColor, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawArrays(GL_TRIANGLES, 0, meshSize);

    switch (shadingMode)
    {
    case PHONG:
        shaderProgramPhong.release();
        break;
    case NORMAL:
        shaderProgramNormal.release();
        break;
    case GOURAUD:
        shaderProgramGouraud.release();
        break;
    }
}

/**
 * @brief MainView::resizeGL
 *
 * Called upon resizing of the screen
 *
 * @param newWidth
 * @param newHeight
 */
void MainView::resizeGL(int newWidth, int newHeight)
{
    Q_UNUSED(newWidth)
    Q_UNUSED(newHeight)
    updateProjectionTransform();
}

void MainView::updateProjectionTransform()
{
    float aspect_ratio = static_cast<float>(width()) / static_cast<float>(height());
    projectionTransform.setToIdentity();
    projectionTransform.perspective(60, aspect_ratio, 0.2f, 20);
}

void MainView::updateModelTransforms()
{
    meshTransform.setToIdentity();
    meshTransform.translate(0.0f, 0.0f, -5.0f);
    meshTransform.scale(scale);
    meshTransform.rotate(QQuaternion::fromEulerAngles(rotation));

    update();
}

// --- OpenGL cleanup helpers

void MainView::destroyModelBuffers()
{
    glDeleteBuffers(1, &meshVBO);
    glDeleteVertexArrays(1, &meshVAO);
}

// --- Public interface

void MainView::setRotation(int rotateX, int rotateY, int rotateZ)
{
    rotation =
    {
        static_cast<float>(rotateX),
        static_cast<float>(rotateY),
        static_cast<float>(rotateZ)
    };
    updateModelTransforms();
}

void MainView::setScale(int newScale)
{
    scale = static_cast<float>(newScale) / 100.0f;
    updateModelTransforms();
}

void MainView::setShadingMode(ShadingMode shading)
{
    shadingMode = shading;
}

// --- Private helpers

/**
 * @brief MainView::onMessageLogged
 *
 * OpenGL logging function, do not change
 *
 * @param Message
 */
void MainView::onMessageLogged(QOpenGLDebugMessage Message)
{
    qDebug() << " → Log:" << Message;
}

QVector<quint8> MainView::imageToBytes(QImage image) {
    // needed since (0,0) is bottom left in OpenGL
    QImage im = image.mirrored();
    QVector<quint8> pixelData;
    pixelData.reserve(im.width()*im.height()*4);

    for (int i = 0; i != im.height(); ++i) {
        for (int j = 0; j != im.width(); ++j) {
            QRgb pixel = im.pixel(j,i);

            // pixel is of format #AARRGGBB (in hexadecimal notation)
            // so with bitshifting and binary AND you can get
            // the values of the different components
            quint8 r = (quint8)((pixel >> 16) & 0xFF); // Red component
            quint8 g = (quint8)((pixel >> 8) & 0xFF); // Green component
            quint8 b = (quint8)(pixel & 0xFF); // Blue component
            quint8 a = (quint8)((pixel >> 24) & 0xFF); // Alpha component

            // Add them to the Vector
            pixelData.append(r);
            pixelData.append(g);
            pixelData.append(b);
            pixelData.append(a);
        }
    }
    return pixelData;
}
