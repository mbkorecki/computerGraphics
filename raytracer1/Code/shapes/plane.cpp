#include "plane.h"

#include <cmath>

Hit Plane::intersect(Ray const &ray)
{
    double denom = (point - ray.O).dot(normal);
    double nom = ray.D.dot(normal);

    if (nom == 0) //no intersect or line is on the plane - infinite intersect
		return Hit::NO_HIT();

    double t = denom / nom;
    Vector N = (normal + t * ray.D + ray.O).normalized();

    return Hit(t, N);
}

Plane::Plane(Point const &p, Vector const &n)
: point(p), normal(n)
{}
