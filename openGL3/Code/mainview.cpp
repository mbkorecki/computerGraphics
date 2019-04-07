#include "mainview.h"
#include "model.h"
#include "vertex.h"

#include <math.h>
#include <QDateTime>

/**
 * @brief MainView::MainView
 *
 * Constructor of MainView
 *
 * @param parent
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
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
MainView::~MainView() {
    debugLogger->stopLogging();

    qDebug() << "MainView destructor";

    for (object &model : objects)
    {
      glDeleteTextures(1, &model.textureID);

    }
    destroyModelBuffers();
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

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0, 1.0, 0.0, 1.0);


    object cat;
    object cube;
    object sphere;

    createShaderProgram();
    loadMesh(cat.numVertices, cat.vboID, cat.vaoID, ":/models/cat.obj");
    loadMesh(cube.numVertices, cube.vboID, cube.vaoID, ":/models/cube.obj");
    loadMesh(sphere.numVertices, sphere.vboID, sphere.vaoID, ":/models/sphere.obj");
    loadMesh(wave.numVertices, wave.vboID, wave.vaoID, ":/models/grid.obj");

    objects.reserve(4);
    cat.speed = 1;
    cat.position = QVector3D(2, 0, -4);
    cat.rotationAxis = QVector3D(0, 1, 0);
    objects.push_back(cat);

    cube.speed = 2;
    cube.position = QVector3D(-2, 0, -4);
    cube.rotationAxis = QVector3D(1, 0, 0);
    objects.push_back(cube);

    cat.speed = 3;
    cat.position = QVector3D(2, 1, -8);
    cat.rotationAxis = QVector3D(0, 1, 0);
    objects.push_back(cat);

    sphere.speed = 4;
    sphere.position = QVector3D(0, 0, -8);
    sphere.rotationAxis = QVector3D(1, 1, 0);
    objects.push_back(sphere);

    wave.speed = 0;
    wave.position = QVector3D(0, 0, -4);
    wave.rotationAxis = QVector3D(1, 0, 0);

    wave.meshTransform.setToIdentity();
    wave.meshTransform.translate(wave.position);
    wave.meshTransform.rotate(50, QVector3D(1, 0, 0));
    // wave.meshTransform.rotate(wave.speed, wave.rotationAxis);

    loadTextures();

    viewTransform.setToIdentity();
    updateViewTransforms();
    // Initialize transformations
    updateProjectionTransform();
    updateModelTransforms();

    timer.start(1000.0 / 60.0);
}

void MainView::createShaderProgram()
{
    waveShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader_wave.glsl");
    waveShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader_wave.glsl");
    waveShaderProgram.link();
    // Create Normal Shader program
    normalShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader_normal.glsl");
    normalShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader_normal.glsl");
    normalShaderProgram.link();

    // Create Gouraud Shader program
    gouraudShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader_gouraud.glsl");
    gouraudShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader_gouraud.glsl");
    gouraudShaderProgram.link();

    // Create Phong Shader program
    phongShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                           ":/shaders/vertshader_phong.glsl");
    phongShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                           ":/shaders/fragshader_phong.glsl");
    phongShaderProgram.link();


    uniformModelTransformWave  = waveShaderProgram.uniformLocation("modelTransform");
    uniformProjectionTransformWave = waveShaderProgram.uniformLocation("projectionTransform");
    uniformNormalTransformWave     = waveShaderProgram.uniformLocation("normalTransform");
    uniformViewTransformWave = waveShaderProgram.uniformLocation("viewTransform");
    uniformAmplitudeWave = waveShaderProgram.uniformLocation("amplitude");
    uniformFrequencyWave = waveShaderProgram.uniformLocation("frequency");
    uniformPhaseWave = waveShaderProgram.uniformLocation("phase");
    uniformMaterialWave            = waveShaderProgram.uniformLocation("material");
    uniformLightPositionWave       = waveShaderProgram.uniformLocation("lightPosition");
    uniformLightColourWave          = waveShaderProgram.uniformLocation("lightColour");
    uniformTextureSamplerWave      = waveShaderProgram.uniformLocation("textureSampler");
    uniformColorWave      = waveShaderProgram.uniformLocation("waveColor");
    uniformTimeWave = waveShaderProgram.uniformLocation("waveTime");

    // Get the uniforms for the normal shader.
    uniformModelTransformNormal  = normalShaderProgram.uniformLocation("modelTransform");
    uniformProjectionTransformNormal = normalShaderProgram.uniformLocation("projectionTransform");
    uniformNormalTransformNormal     = normalShaderProgram.uniformLocation("normalTransform");
    uniformViewTransformNormal = normalShaderProgram.uniformLocation("viewTransform");

    // Get the uniforms for the gouraud shader.
    uniformViewTransformGouraud = gouraudShaderProgram.uniformLocation("viewTransform");
    uniformModelTransformGouraud  = gouraudShaderProgram.uniformLocation("modelTransform");
    uniformProjectionTransformGouraud = gouraudShaderProgram.uniformLocation("projectionTransform");
    uniformNormalTransformGouraud     = gouraudShaderProgram.uniformLocation("normalTransform");
    uniformMaterialGouraud            = gouraudShaderProgram.uniformLocation("material");
    uniformLightPositionGouraud       = gouraudShaderProgram.uniformLocation("lightPosition");
    uniformLightColourGouraud         = gouraudShaderProgram.uniformLocation("lightColour");
    uniformTextureSamplerGouraud      = gouraudShaderProgram.uniformLocation("textureSampler");

    // Get the uniforms for the phong shader.
    uniformViewTransformPhong = phongShaderProgram.uniformLocation("viewTransform");
    uniformModelTransformPhong  = phongShaderProgram.uniformLocation("modelTransform");
    uniformProjectionTransformPhong = phongShaderProgram.uniformLocation("projectionTransform");
    uniformNormalTransformPhong     = phongShaderProgram.uniformLocation("normalTransform");
    uniformMaterialPhong            = phongShaderProgram.uniformLocation("material");
    uniformLightPositionPhong       = phongShaderProgram.uniformLocation("lightPosition");
    uniformLightColourPhong         = phongShaderProgram.uniformLocation("lightColour");
    uniformTextureSamplerPhong      = phongShaderProgram.uniformLocation("textureSampler");
}

void MainView::loadMesh(GLuint &meshSize, GLuint &meshVBO, GLuint &meshVAO, QString source)
{
    Model model(source);
    model.unitize();
    QVector<float> meshData = model.getVNTInterleaved();

    meshSize = model.getVertices().size();

    // Generate VAO
    glGenVertexArrays(1, &meshVAO);
    glBindVertexArray(meshVAO);

    // Generate VBO
    glGenBuffers(1, &meshVBO);
    glBindBuffer(GL_ARRAY_BUFFER, meshVBO);

    // Write the data to the buffer
    glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(float), meshData.data(), GL_STATIC_DRAW);

    // Set vertex coordinates to location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Set vertex normals to location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set vertex texture coordinates to location 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void MainView::loadTextures()
{
  glGenTextures(1, &objects[0].textureID);
  loadTexture(":/textures/cat_diff.png", objects[0].textureID);
  glGenTextures(1, &objects[1].textureID);
  loadTexture(":/textures/jupitermap.png", objects[1].textureID);
  glGenTextures(1, &objects[2].textureID);
  loadTexture(":/textures/cat_norm.png", objects[2].textureID);
  glGenTextures(1, &objects[3].textureID);
  loadTexture(":/textures/neptunemap.png", objects[3].textureID);
}

void MainView::loadTexture(QString file, GLuint texturePtr)
{
    // Set texture parameters.
    glBindTexture(GL_TEXTURE_2D, texturePtr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Push image data to texture.
    QImage image(file);
    QVector<quint8> imageData = imageToBytes(image);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
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
    glClearColor(0.2f, 0.5f, 0.7f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Choose the selected shader.
    QOpenGLShaderProgram *shaderProgram;

    switch (currentShader) {
    case NORMAL:
        shaderProgram = &normalShaderProgram;
        shaderProgram->bind();
        updateNormalUniforms();
        break;
    case GOURAUD:
        shaderProgram = &gouraudShaderProgram;
        shaderProgram->bind();
        updateGouraudUniforms();
        break;
    case PHONG:
        shaderProgram = &phongShaderProgram;
        shaderProgram->bind();
        updatePhongUniforms();
        break;
    }

    glActiveTexture(GL_TEXTURE0);
    for (object &model : objects)
    {
      glBindTexture(GL_TEXTURE_2D, model.textureID);
      glBindVertexArray(model.vaoID);
      updateIndividualUniforms(model);
      glDrawArrays(GL_TRIANGLES, 0, model.numVertices);
    }
    shaderProgram->release();

    shaderProgram = &waveShaderProgram;
    shaderProgram->bind();
    glUniformMatrix4fv(uniformProjectionTransformWave, 1, GL_FALSE, projectionTransform.data());
    glUniformMatrix4fv(uniformViewTransformWave, 1, GL_FALSE, viewTransform.data());

    GLfloat amplitude[4] = {0.04f, 0.023f, 0.05f, 0.03f};
    GLfloat frequency[4] = {2.3f, 3.5f, 7.0f, 5.0f};
    GLfloat phase[4] = {0.2f, 0.5f, 1.0f, 0.0f};
    wave.speed += 1 / 100.0;

    glUniform1f(uniformTimeWave, wave.speed);
    glUniform1fv(uniformAmplitudeWave, 4, amplitude);
    glUniform1fv(uniformFrequencyWave, 4, frequency);
    glUniform1fv(uniformPhaseWave, 4, phase);

    glBindVertexArray(wave.vaoID);

    // wave.meshTransform.rotate(wave.speed, wave.rotationAxis);
    wave.meshNormalTransform = wave.meshTransform.normalMatrix();

    glUniformMatrix4fv(uniformModelTransformWave, 1, GL_FALSE, wave.meshTransform.data());
    glUniformMatrix3fv(uniformNormalTransformWave, 1, GL_FALSE, wave.meshNormalTransform.data());
    QVector4D waveMaterial = {0.2f,0.6f,0.5f,1};
    glUniform4fv(uniformMaterialWave, 1, &waveMaterial[0]);

    QVector3D wavecolor = {0.7, 0.7, 1};
    glUniform3fv(uniformColorWave, 1, &wavecolor[0]);
    glUniform3fv(uniformLightPositionWave, 1, &lightPosition[0]);
    glUniform3fv(uniformLightColourWave, 1, &lightColour[0]);
    glUniform1i(uniformTextureSamplerWave, 0);

    glDrawArrays(GL_TRIANGLES, 0, wave.numVertices);

    // Set the texture and draw the mesh.

    shaderProgram->release();
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

void MainView::updateIndividualUniforms(object &model)
{
  model.meshTransform.rotate(model.speed, model.rotationAxis);
  model.meshNormalTransform = model.meshTransform.normalMatrix();

  switch (currentShader) {
  case NORMAL:
      glUniformMatrix4fv(uniformModelTransformNormal, 1, GL_FALSE, model.meshTransform.data());
      glUniformMatrix3fv(uniformNormalTransformNormal, 1, GL_FALSE, model.meshNormalTransform.data());
      break;
  case GOURAUD:
      glUniformMatrix4fv(uniformModelTransformGouraud, 1, GL_FALSE, model.meshTransform.data());
      glUniformMatrix3fv(uniformNormalTransformGouraud, 1, GL_FALSE, model.meshNormalTransform.data());
      glUniform4fv(uniformMaterialGouraud, 1, &material[0]);
      break;
  case PHONG:
      glUniformMatrix4fv(uniformModelTransformPhong, 1, GL_FALSE, model.meshTransform.data());
      glUniformMatrix3fv(uniformNormalTransformPhong, 1, GL_FALSE, model.meshNormalTransform.data());
      glUniform4fv(uniformMaterialPhong, 1, &material[0]);
      break;
  }
}

void MainView::updateNormalUniforms()
{
    glUniformMatrix4fv(uniformProjectionTransformNormal, 1, GL_FALSE, projectionTransform.data());
    glUniformMatrix4fv(uniformViewTransformNormal, 1, GL_FALSE, viewTransform.data());

}

void MainView::updateGouraudUniforms()
{
    glUniformMatrix4fv(uniformProjectionTransformGouraud, 1, GL_FALSE, projectionTransform.data());
    glUniformMatrix4fv(uniformViewTransformGouraud, 1, GL_FALSE, viewTransform.data());
    glUniform3fv(uniformLightPositionGouraud, 1, &lightPosition[0]);
    glUniform3fv(uniformLightColourGouraud, 1, &lightColour[0]);

    glUniform1i(uniformTextureSamplerGouraud, 0); // Redundant now, but useful when you have multiple textures.
}

void MainView::updatePhongUniforms()
{
    glUniformMatrix4fv(uniformProjectionTransformPhong, 1, GL_FALSE, projectionTransform.data());
    glUniformMatrix4fv(uniformViewTransformPhong, 1, GL_FALSE, viewTransform.data());
    glUniform3fv(uniformLightPositionPhong, 1, &lightPosition[0]);
    glUniform3fv(uniformLightColourPhong, 1, &lightColour[0]);

    glUniform1i(uniformTextureSamplerPhong, 0);
}

void MainView::updateProjectionTransform()
{
    float aspect_ratio = static_cast<float>(width()) / static_cast<float>(height());
    projectionTransform.setToIdentity();
    projectionTransform.perspective(60, aspect_ratio, 0.2, 20);
}

void MainView::updateModelTransforms()
{
    for (object &model : objects)
    {
      model.meshTransform.setToIdentity();
      model.meshTransform.translate(model.position);
      model.meshTransform.scale(scale);
      // model.meshTransform.rotate(QQuaternion::fromEulerAngles(rotation));
      model.meshNormalTransform = model.meshTransform.normalMatrix();
    }
    update();
}

void MainView::updateViewTransforms()
{
    viewTransform.setToIdentity();
    viewTransform.rotate(QQuaternion::fromEulerAngles(rotation));
}

// --- OpenGL cleanup helpers

void MainView::destroyModelBuffers()
{
    for (object &model : objects)
    {
        glDeleteBuffers(1, &model.vboID);
        glDeleteVertexArrays(1, &model.vaoID);
    }
    glDeleteBuffers(1, &wave.vboID);
    glDeleteVertexArrays(1, &wave.vaoID);
}

// --- Public interface

void MainView::setRotation(int rotateX, int rotateY, int rotateZ)
{
    rotation = { static_cast<float>(rotateX), static_cast<float>(rotateY), static_cast<float>(rotateZ) };
    updateViewTransforms();
}

void MainView::setScale(int newScale)
{
    scale = static_cast<float>(newScale) / 100.f;
    updateModelTransforms();
}

void MainView::setShadingMode(ShadingMode shading)
{
    qDebug() << "Changed shading to" << shading;
    currentShader = shading;
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
