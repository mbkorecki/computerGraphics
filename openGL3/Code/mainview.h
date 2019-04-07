#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "model.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <QVector3D>
#include <QImage>
#include <QVector>
#include <memory>
#include <QMatrix4x4>
#include <vector>
#include <cmath>

struct object
{
  QVector3D position;
  QVector3D rotationAxis;
  double speed;
  
  GLuint vaoID;
  GLuint vboID;

  GLuint numVertices;
  GLuint textureID;

  QMatrix3x3 meshNormalTransform;
  QMatrix4x4 meshTransform;
};

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

    QOpenGLDebugLogger *debugLogger;
    QTimer timer; // timer used for animation

    QOpenGLShaderProgram normalShaderProgram,
                         gouraudShaderProgram,
                         phongShaderProgram,
                         waveShaderProgram;

    // Uniforms for the normal shader.
    GLint uniformModelTransformNormal;
    GLint uniformProjectionTransformNormal;
    GLint uniformNormalTransformNormal;

    // Uniforms for the gouraud shader.
    GLint uniformModelTransformGouraud;
    GLint uniformProjectionTransformGouraud;
    GLint uniformNormalTransformGouraud;

    GLint uniformMaterialGouraud;
    GLint uniformLightPositionGouraud;
    GLint uniformLightColourGouraud;

    GLint uniformTextureSamplerGouraud;

    // Uniforms for the phong shader.
    GLint uniformModelTransformPhong;
    GLint uniformProjectionTransformPhong;
    GLint uniformNormalTransformPhong;

    GLint uniformMaterialPhong;
    GLint uniformLightPositionPhong;
    GLint uniformLightColourPhong;

    GLint uniformTextureSamplerPhong;

    GLint uniformViewTransformNormal;
    GLint uniformViewTransformPhong;
    GLint uniformViewTransformGouraud;

    GLint uniformModelTransformWave;
    GLint uniformProjectionTransformWave;
    GLint uniformNormalTransformWave;
    GLint uniformViewTransformWave;
    GLint uniformAmplitudeWave;
    GLint uniformFrequencyWave;
    GLint uniformPhaseWave;
    GLint uniformMaterialWave;
    GLint uniformLightPositionWave;
    GLint uniformLightColourWave;
    GLint uniformTextureSamplerWave;
    GLint uniformColorWave;
    GLint uniformTimeWave;


    std::vector<object> objects;
    object wave;

    // Transforms
    QMatrix4x4 projectionTransform;
    QMatrix4x4 viewTransform;

    float scale = 1.f;

    QVector3D rotation;

    // Phong model constants.
    QVector4D material = {0.5, 0.5, 1, 5};
    QVector3D lightPosition = {1, 100, 1};
    QVector3D lightColour = {1, 1, 1};

public:
    enum ShadingMode : GLuint
    {
        PHONG = 0, NORMAL, GOURAUD
    };

    MainView(QWidget *parent = 0);
    ~MainView();

    // Functions for widget input events
    void setRotation(int rotateX, int rotateY, int rotateZ);
    void setScale(int scale);
    void setShadingMode(ShadingMode shading);

protected:
    void initializeGL();
    void resizeGL(int newWidth, int newHeight);
    void paintGL();

    // Functions for keyboard input events
    void keyPressEvent(QKeyEvent *ev);
    void keyReleaseEvent(QKeyEvent *ev);

    // Function for mouse input events
    void mouseDoubleClickEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);

private slots:
    void onMessageLogged( QOpenGLDebugMessage Message );

private:
    void createShaderProgram();
    void loadMesh(GLuint&, GLuint&, GLuint&, QString);

    // Loads texture data into the buffer of texturePtr.
    void loadTextures();
    void loadTexture(QString file, GLuint texturePtr);

    void destroyModelBuffers();

    void updateProjectionTransform();
    void updateModelTransforms();
    void updateViewTransforms();

    void updateNormalUniforms();
    void updateGouraudUniforms();
    void updatePhongUniforms();
    void updateIndividualUniforms(object &model);
    // Useful utility method to convert image to bytes.
    QVector<quint8> imageToBytes(QImage image);

    // The current shader to use.
    ShadingMode currentShader = PHONG;
};

#endif // MAINVIEW_H
