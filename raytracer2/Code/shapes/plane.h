#ifndef PLANE_H_
#define PLANE_H_

#include "../object.h"

class Plane: public Object
{
    public:
        Plane(Point const &p, Vector const &n);

        virtual Hit intersect(Ray const &ray);
        virtual Color textureColorAt(Point N, bool rotate){ return Color(); };
        virtual bool isRotated() { return false; };
        virtual Vector rotate(Point point) { return Vector(); };

        Point const point;
        Vector const normal;
};

#endif
