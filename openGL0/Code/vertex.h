#ifndef VERTEX_H
#define VERTEX_H

class Vertex
{
    GLfloat x;
    GLfloat y;
    GLfloat r;
    GLfloat g;
    GLfloat b;

public:
    Vertex()
    {

    }

    Vertex(GLfloat i_x, GLfloat i_y, GLfloat i_r, GLfloat i_g, GLfloat i_b)
    : x(i_x), y(i_y), r(i_r), g(i_g), b(i_b)
    {
    }
};

#endif // VERTEX_H
