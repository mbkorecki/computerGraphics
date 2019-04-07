#include "mainview.h"
#include "math.h"

#include <QDateTime>
#include <vector>

#include "vertex.h"

static GLuint vboCube;
static GLuint vaoCube;
static GLuint iboCube;

static GLuint vboPyramid;
static GLuint vaoPyramid;
static GLuint iboPyramid;

static GLuint vboModel;
static GLuint vaoModel;

//rotation
static QMatrix4x4 totalRotation;
static int previousRotX;
static int previousRotY;
static int previousRotZ;
//scaling
static QMatrix4x4 previousScaling;

/**
 * @brief MainView::MainView
 *
 * Constructor of MainView
 *
 * @param parent
 */
MainView::MainView(QWidget *parent)
: QOpenGLWidget(parent), externalModel(":/models/sphere.obj")
{
    qDebug() << "MainView constructor";

    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));

    cubeModelTransform.setToIdentity();
    cubeModelTransform.translate(2.0f, 0.0f, -6.0f);

    pyramidModelTransform.setToIdentity();
    pyramidModelTransform.translate(-2.0f, 0.0f, -6.0f);

    externalModelTransform.setToIdentity();
    externalModelTransform.translate(0.0f, 0.0f, -10.0f);
}

/**
 * @brief MainView::~MainView
 *
 * Destructor of MainView
 * This is the last function called, before exit of the program
 * Use this to clean up your variables, buffers etc.
 *
 */
MainView::~MainView() {
    debugLogger->stopLogging();

    glDeleteBuffers(1, &vboCube);
    glDeleteBuffers(1, &iboCube);
    glDeleteVertexArrays(1, &vaoCube);

    glDeleteBuffers(1, &vboPyramid);
    glDeleteBuffers(1, &iboPyramid);
    glDeleteVertexArrays(1, &vaoPyramid);

    glDeleteBuffers(1, &vboModel);
    glDeleteVertexArrays(1, &vaoModel);

    qDebug() << "MainView destructor";
}

// --- OpenGL initialization

/**
 * @brief MainView::initializeGL
 *
 * Called upon OpenGL initialization
 * Attaches a debugger and calls other init functions
 */
void MainView::initializeGL() {
    qDebug() << ":: Initializing OpenGL";
    initializeOpenGLFunctions();

    debugLogger = new QOpenGLDebugLogger();
    connect( debugLogger, SIGNAL( messageLogged( QOpenGLDebugMessage ) ),
             this, SLOT( onMessageLogged( QOpenGLDebugMessage ) ), Qt::DirectConnection );

    if ( debugLogger->initialize() ) {
        qDebug() << ":: Logging initialized";
        debugLogger->startLogging( QOpenGLDebugLogger::SynchronousLogging );
        debugLogger->enableMessages();
    }

    QString glVersion;
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable backface culling
    glEnable(GL_CULL_FACE);

    // Default is GL_LESS
    glDepthFunc(GL_LEQUAL);

    // Set the color of the screen to be black on clear (new frame)
    glClearColor(0.2f, 0.5f, 0.7f, 0.0f);

    createShaderProgram();

    //create triangle vertices
    Vertex cubeVertices[8];
    cubeVertices[0] = Vertex(-1, -1, 1, 0, 1, 0);
    cubeVertices[1] = Vertex(1, -1, 1, 0, 0, 1);
    cubeVertices[2] = Vertex(1, 1, 1, 1, 0, 0);

    cubeVertices[3] = Vertex(-1, 1, 1, 0, 0, 1);

    cubeVertices[4] = Vertex(-1, -1, -1, 1, 0, 0);
    cubeVertices[5] = Vertex(1, -1, -1, 0, 0, 1);
    cubeVertices[6] = Vertex(1, 1, -1, 0, 1, 0);

    cubeVertices[7] = Vertex(-1, 1, -1, 1, 0, 0);


    GLushort cubeElements[36] =
    {
      // front
  		0, 1, 2,
  		2, 3, 0,
  		// right
  		1, 5, 6,
  		6, 2, 1,
  		// back
  		7, 6, 5,
  		5, 4, 7,
  		// left
  		4, 0, 3,
  		3, 7, 4,
  		// bottom
  		4, 5, 1,
  		1, 0, 4,
  		// top
  		3, 2, 6,
  		6, 7, 3
    };

    Vertex pyramidVertices[5];
    pyramidVertices[0] = Vertex(-1, -1, 1, 0, 1, 0);
    pyramidVertices[1] = Vertex(1, -1, 1, 0, 1, 0);
    pyramidVertices[2] = Vertex(1, -1, -1, 0, 0, 1);
    pyramidVertices[3] = Vertex(0, 1, 0, 0, 0, 1);
    pyramidVertices[4] = Vertex(-1, -1, -1, 1, 0, 0);

    GLushort pyramidElements[18] =
    {
      // bottom
      4, 2, 1,
      1, 0, 4,
      //sides
      0, 1, 3,
      0, 3, 4,
      1, 2, 3,
      2, 4, 3
    };
    //create VBO
    glGenBuffers(1, &vboCube);
    glBindBuffer(GL_ARRAY_BUFFER, vboCube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &iboCube);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCube);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeElements), cubeElements, GL_STATIC_DRAW);
    //create VAO
    glGenVertexArrays(1, &vaoCube);
    glBindVertexArray(vaoCube);

    //define data layout
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void*>(nullptr));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));

    glGenBuffers(1, &vboPyramid);
    glBindBuffer(GL_ARRAY_BUFFER, vboPyramid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &iboPyramid);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboPyramid);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidElements), pyramidElements, GL_STATIC_DRAW);
    //create VAO
    glGenVertexArrays(1, &vaoPyramid);
    glBindVertexArray(vaoPyramid);

    //define data layout
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void*>(nullptr));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));

    //external model pipeline
    QVector<QVector3D> tempModelVertices = externalModel.getVertices();
    std::vector<Vertex> modelVertices;

    for (int i = 0; i < tempModelVertices.size(); i++)
      modelVertices.emplace_back(Vertex(tempModelVertices[i]));

    glGenBuffers(1, &vboModel);
    glBindBuffer(GL_ARRAY_BUFFER, vboModel);
    glBufferData(GL_ARRAY_BUFFER, modelVertices.size() * sizeof(Vertex), &modelVertices.front(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &vaoModel);
    glBindVertexArray(vaoModel);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void*>(nullptr));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));

}

void MainView::createShaderProgram()
{
    // Create shader program
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader.glsl");
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader.glsl");
    shaderProgram.link();

    uniformModelCube = shaderProgram.uniformLocation("modelTransform");
    uniformProjection = shaderProgram.uniformLocation("projectionTransform");
}

// --- OpenGL drawing

/**
 * @brief MainView::paintGL
 *
 * Actual function used for drawing to the screen
 *
 */
void MainView::paintGL() {
    // Clear the screen before rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderProgram.bind();

    shaderProgram.setUniformValue(uniformProjection, projectionTransform);
    // Draw here
    glBindVertexArray(vaoCube);

    glBindBuffer(GL_ARRAY_BUFFER, vboCube);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCube);
    shaderProgram.setUniformValue(uniformModelCube, cubeModelTransform);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(vaoPyramid);

    glBindBuffer(GL_ARRAY_BUFFER, vboPyramid);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboPyramid);
    shaderProgram.setUniformValue(uniformModelCube, pyramidModelTransform);

    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(vaoModel);
    glBindBuffer(GL_ARRAY_BUFFER, vboModel);
    shaderProgram.setUniformValue(uniformModelCube, externalModelTransform);

    glDrawArrays(GL_TRIANGLES, 0, externalModel.getVertices().size());

    shaderProgram.release();
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
    // TODO: Update projection to fit the new aspect ratio

    projectionTransform.setToIdentity();
    projectionTransform.perspective(60.0f, GLfloat(newWidth) / newHeight, 0.01f, 100.0f);
    projectionTransform.lookAt(QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 0.0, -4.0), QVector3D(0.0, 1.0, 0.0));
}

// --- Public interface

void MainView::setRotation(int rotateX, int rotateY, int rotateZ)
{
    qDebug() << "Rotation changed to (" << rotateX << "," << rotateY << "," << rotateZ << ")";

    if (rotateX == 0 && rotateY == 0 && rotateZ == 0)
    {
        cubeModelTransform *= totalRotation.inverted();
        pyramidModelTransform *= totalRotation.inverted();
        externalModelTransform *= totalRotation.inverted();
        totalRotation.setToIdentity();
        previousRotX = 0;
        previousRotY = 0;
        previousRotZ = 0;
    }
    else
    {
        QMatrix4x4 rotationMatrix;
        rotationMatrix.rotate(rotateX - previousRotX, QVector3D(1, 0, 0));
        rotationMatrix.rotate(rotateY - previousRotY, QVector3D(0, 1, 0));
        rotationMatrix.rotate(rotateZ - previousRotZ, QVector3D(0, 0, 1));
        totalRotation *= rotationMatrix;
        previousRotX = rotateX;
        previousRotY = rotateY;
        previousRotZ = rotateZ;

        cubeModelTransform *= rotationMatrix;
        pyramidModelTransform *= rotationMatrix;
        externalModelTransform *= rotationMatrix;
    }

    update();
}

void MainView::setScale(int scale)
{
    qDebug() << "Scale changed to " << scale;

    cubeModelTransform *= previousScaling.inverted();
    pyramidModelTransform *= previousScaling.inverted();
    externalModelTransform *= previousScaling.inverted();

    QMatrix4x4 scalingMatrix;
    scalingMatrix.scale(scale / 100.0f);
    previousScaling = scalingMatrix;

    cubeModelTransform *= scalingMatrix;
    pyramidModelTransform *= scalingMatrix;
    externalModelTransform *= scalingMatrix;

    update();
}

void MainView::setShadingMode(ShadingMode shading)
{
    qDebug() << "Changed shading to" << shading;
    Q_UNIMPLEMENTED();
}

// --- Private helpers

/**
 * @brief MainView::onMessageLogged
 *
 * OpenGL logging function, do not change
 *
 * @param Message
 */
void MainView::onMessageLogged( QOpenGLDebugMessage Message ) {
    qDebug() << " → Log:" << Message;
}
