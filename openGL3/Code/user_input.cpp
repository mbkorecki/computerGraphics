#include "mainview.h"

#include <QDebug>

// Record where the mouse enters the window
static int enterPoint[2];

static QVector3D totalTranslation;

// Triggered by pressing a key
void MainView::keyPressEvent(QKeyEvent *ev)
{
    int rotationX = rotation.x();
    while (rotationX < 0)
            rotationX += 360;
    rotationX = rotationX % 360;

    int rotationY = rotation.y();
    while (rotationY < 0)
            rotationY += 360;
    rotationY = rotationY % 360;

    float translateY = 0;
    if (rotationX <= 90)
        translateY = (rotationX / 90.0f) * 0.1f;
    else if (rotationX <= 180)
        translateY = ((rotationX - 180) / 90.0f) * -0.1f;
    else if (rotationX <= 270)
        translateY = ((rotationX - 180) / 90.0f) * -0.1f;
    else
        translateY = ((rotationX - 360) / 90.0f) * 0.1f;

    float translateX = 0;
    if (rotationY <= 90)
        translateX = (rotationY / 90.0f) * -0.1f + fabs(translateY);
    else if (rotationY <= 180)
        translateX = ((rotationY - 180) / 90.0f) * 0.1f  + fabs(translateY);
    else if (rotationY <= 270)
        translateX = ((rotationY - 180) / 90.0f) * 0.1f - fabs(translateY);
    else
        translateX = ((rotationY - 360) / 90.0f) * -0.1f - fabs(translateY);

    float translateZ = 0;
    if ((rotationX >= 270 || rotationX <= 90) && (rotationY >= 270 || rotationY <= 90))
        translateZ = 0.1f - fabs(translateY) - fabs(translateX);
    else
        translateZ = (0.1f - fabs(translateY) - fabs(translateX)) * -1;

    switch(ev->key()) {
    case 'W':
        viewTransform.translate(QVector3D(translateX, translateY, translateZ));
        totalTranslation += QVector3D(translateX, translateY, translateZ);
        break;
    case 'S':
        viewTransform.translate(QVector3D(-translateX, -translateY, -translateZ));
        totalTranslation += QVector3D(-translateX, -translateY, -translateZ);
        break;
    case 'A':
        viewTransform.translate(QVector3D(translateZ, translateY, -translateX));
        totalTranslation += QVector3D(translateZ, translateY, -translateX);
        break;
    case 'D':
        viewTransform.translate(QVector3D(-translateZ, translateY, translateX));
        totalTranslation += QVector3D(-translateZ, translateY, translateX);
        break;
    default:
        // ev->key() is an integer. For alpha numeric characters keys it equivalent with the char value ('A' == 65, '1' == 49)
        // Alternatively, you could use Qt Key enums, see http://doc.qt.io/qt-5/qt.html#Key-enum
        qDebug() << ev->key() << "pressed";
        break;
    }

    // Used to update the screen after changes
    update();
}

// Triggered by releasing a key
void MainView::keyReleaseEvent(QKeyEvent *ev)
{
    switch(ev->key()) {
    case 'A': qDebug() << "A released"; break;
    default:
        qDebug() << ev->key() << "released";
        break;
    }

    update();
}

// Triggered by clicking two subsequent times on any mouse button
// It also fires two mousePress and mouseRelease events!
void MainView::mouseDoubleClickEvent(QMouseEvent *ev)
{
    qDebug() << "Mouse double clicked:" << ev->button();

    update();
}

// Triggered when moving the mouse inside the window (only when the mouse is clicked!)
void MainView::mouseMoveEvent(QMouseEvent *ev)
{
    qDebug() << "x" << ev->x() << "y" << ev->y();

    setRotation(rotation.x() + (enterPoint[1] - ev->y()) / 2, rotation.y() + (enterPoint[0] - ev->x()) / 2, rotation.z());
    enterPoint[0] = ev->x();
    enterPoint[1] = ev->y();

    // Restore translation
    viewTransform.translate(totalTranslation);

    update();
}

// Triggered when pressing any mouse button
void MainView::mousePressEvent(QMouseEvent *ev)
{
    qDebug() << "Mouse button pressed:" << ev->button();

    enterPoint[0] = ev->x();
    enterPoint[1] = ev->y();

    update();
    // Do not remove the line below, clicking must focus on this widget!
    this->setFocus();
}

// Triggered when releasing any mouse button
void MainView::mouseReleaseEvent(QMouseEvent *ev)
{
    qDebug() << "Mouse button released" << ev->button();

    update();
}

// Triggered when clicking scrolling with the scroll wheel on the mouse
void MainView::wheelEvent(QWheelEvent *ev)
{
    // Implement something
    qDebug() << "Mouse wheel:" << ev->delta();

    update();
}
