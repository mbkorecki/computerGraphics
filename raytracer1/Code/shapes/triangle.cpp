#include "triangle.h"

#include <cmath>

//using Möller–Trumbore intersection algorithm
Hit Triangle::intersect(Ray const &ray)
{
    const float EPSILON = 0.0000001;

    Vector edge1, edge2, h, s, q;
    float a,f,u,v;

    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;

    h = ray.D.cross(edge2);
    a = edge1.dot(h);

    if (a > -EPSILON && a < EPSILON)
		return Hit::NO_HIT();    // The ray is parallel to the triangle.

    f = 1.0 / a;
    s = ray.O - vertex0;
    u = f * (s.dot(h));
    if (u < 0.0 || u > 1.0)
		return Hit::NO_HIT();

    q = s.cross(edge1);
    v = f * ray.D.dot(q);
    if (v < 0.0 || u + v > 1.0)
		return Hit::NO_HIT();

    double t = f * edge2.dot(q);
    Vector intersectionPoint;

    if (t > EPSILON) // ray intersection
        intersectionPoint = ray.O + ray.D * t;
    else // line intersection but not a ray intersection.
        return Hit::NO_HIT();

    Vector N = (edge2.cross(edge1) + intersectionPoint).normalized();

    return Hit(t, N);
}

Triangle::Triangle(Point const &v0, Point const &v1, Point const &v2)
: vertex0(v0), vertex1(v1), vertex2(v2)
{}
