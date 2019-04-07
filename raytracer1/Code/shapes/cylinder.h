#ifndef CYLINDER_H_
#define CYLINDER_H_

#include "../object.h"

class Cylinder: public Object
{
    public:
        Cylinder(Point const &p, double r, double h);

        virtual Hit intersect(Ray const &ray);

        Point const center;
        double const radius;
        double const height;
};

#endif
