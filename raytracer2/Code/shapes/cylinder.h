#ifndef CYLINDER_H_
#define CYLINDER_H_

#include "../object.h"

class Cylinder: public Object
{
    public:
        Cylinder(Point const &p, double r, double h);

        virtual Hit intersect(Ray const &ray);
        virtual Color textureColorAt(Point N, bool rotate){ return Color(); };
        virtual bool isRotated() { return false; };
        virtual Vector rotate(Point point) { return Vector(); };

        Point const center;
        double const radius;
        double const height;
};

#endif
