#include "mainview.h"
#include "math.h"

#include <QDateTime>
#include "vertex.h"

static GLuint vbo;
static GLuint vao;

MainView::MainView(QWidget *parent) : QOpenGLWidget(parent)
{
    qDebug() << "MainView constructor";

    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
}

MainView::~MainView()
{
    debugLogger->stopLogging();

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    qDebug() << "MainView destructor";
}

void MainView::initializeGL()
{
    qDebug() << ":: Initializing OpenGL";
    initializeOpenGLFunctions();

    debugLogger = new QOpenGLDebugLogger();
    connect(debugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this, SLOT(onMessageLogged(QOpenGLDebugMessage)), Qt::DirectConnection);

    if (debugLogger->initialize())
    {
        qDebug() << ":: Logging initialized";
        debugLogger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
        debugLogger->enableMessages();
    }

    QString glVersion;
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

    //load shaders and instantiate shader program
    QOpenGLShaderProgram program(nullptr);
    program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");
    program.link();
    program.bind();

    //create triangle vertices
    Vertex triangleVertices[3];
    triangleVertices[0] = Vertex(0, 1, 0, 0, 1);
    triangleVertices[1] = Vertex(-1, -1, 1, 0, 0);
    triangleVertices[2] = Vertex(1, -1, 0, 1, 0);

    //create VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

    //create VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //define data layout
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void*>(nullptr));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(2 * sizeof(float)));
}

void MainView::resizeGL(int newWidth, int newHeight)
{
    Q_UNUSED(newWidth)
    Q_UNUSED(newHeight)
}

void MainView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void MainView::onMessageLogged(QOpenGLDebugMessage Message)
{
    qDebug() << " → Log:" << Message;
}
