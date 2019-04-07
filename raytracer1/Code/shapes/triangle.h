#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "../object.h"

class Triangle: public Object
{
    public:
        Triangle(Point const &v0, Point const &v1, Point const &v2);

        virtual Hit intersect(Ray const &ray);

        Point const vertex0;
        Point const vertex1;
        Point const vertex2;
};

#endif
