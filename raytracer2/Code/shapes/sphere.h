#ifndef SPHERE_H_
#define SPHERE_H_

#include "../object.h"

class Sphere: public Object
{
    public:
        Sphere(Point const &pos, double radius, Vector rotation, int angle);

        virtual Hit intersect(Ray const &ray);
        virtual Color textureColorAt(Point N, bool rotate);

        virtual bool isRotated() { return (angle != -1); };
        virtual Vector rotate(Point point);

        Point const position;
        double const r;
        Vector rotation;
        int angle;
};

#endif
