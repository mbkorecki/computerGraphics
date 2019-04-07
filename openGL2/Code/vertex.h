#ifndef VERTEX_H
#define VERTEX_H

#include <cstdlib>

class Vertex
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat texX = 0;
    GLfloat texY = 0;

public:
    Vertex()
    {

    }

    Vertex(GLfloat i_x, GLfloat i_y, GLfloat i_z, GLfloat i_r, GLfloat i_g, GLfloat i_b)
    : x(i_x), y(i_y), z(i_z), r(i_r), g(i_g), b(i_b)
    {
    }

    Vertex(QVector3D vec)
    : x(vec.x() / 40), y(vec.y() / 40), z(vec.z() / 40), r(rand() % 100 / 100.0), g(rand() % 100 / 100.0), b(rand() % 100 / 100.0)
    {
    }

    Vertex(QVector3D vec, QVector3D norm, QVector2D tex)
    : x(vec.x() * 3), y(vec.y() * 3), z(vec.z() * 3), r(norm.x()), g(norm.y()), b(norm.z()), texX(tex.x()), texY(tex.y())
    {
    }

};

#endif // VERTEX_H
