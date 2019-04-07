#include "sphere.h"

#include <cmath>

using namespace std;

Hit Sphere::intersect(Ray const &ray)
{
    /****************************************************
    * RT1.1: INTERSECTION CALCULATION
    *
    * Given: ray, position, r
    * Sought: intersects? if true: *t
    *
    * Insert calculation of ray/sphere intersection here.
    *
    * You have the sphere's center (C) and radius (r) as well as
    * the ray's origin (ray.O) and direction (ray.D).
    *
    * If the ray does not intersect the sphere, return false.
    * Otherwise, return true and place the distance of the
    * intersection point from the ray origin in *t (see example).
    ****************************************************/

    double a = pow((ray.D).dot(ray.O - position), 2);
    double b = (ray.O - position).dot(ray.O - position) - pow(r, 2);
    
    double expr = a - b;
    if (expr < 0.0)
		return Hit::NO_HIT();
	
	double t1 = -(ray.D).dot(ray.O - position) + sqrt(expr);
	double t2 = -(ray.D).dot(ray.O - position) - sqrt(expr);
	double t = min(t1, t2);
    
    /****************************************************
    * RT1.2: NORMAL CALCULATION
    *
    * Given: t, C, r
    * Sought: N
    *
    * Insert calculation of the sphere's normal at the intersection point.
    ****************************************************/

    Vector N = (ray.at(t) - position).normalized();

    return Hit(t,N);
}

Sphere::Sphere(Point const &pos, double radius)
:
    position(pos),
    r(radius)
{}
