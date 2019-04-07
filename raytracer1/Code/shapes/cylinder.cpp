#include "cylinder.h"
#include "plane.h"

#include <cmath>

Hit Cylinder::intersect(Ray const &ray)
{
  	Point rayT(ray.O.x-center.x, ray.O.y-center.y, ray.O.z-center.z);

    //check if we intersect the core
	double EPSILON = 0.0000001;
    double a = pow(ray.D.x, 2) + pow(ray.D.z, 2);
    double b = rayT.x * ray.D.x + ray.D.z * rayT.z;
    double c = pow(rayT.x, 2) + pow(rayT.z, 2) - pow(radius, 2);

    if ((b * b - a * c) < EPSILON)
		return Hit::NO_HIT();

    double t0 = (-b + sqrt(b * b  - a * c)) / a;
    double t1 = (-b - sqrt(b * b  - a * c)) / a;

    double t = fmin(t0, t1);

    if (t <= EPSILON)
		return Hit::NO_HIT();

    double y = rayT.y + t * ray.D.y;

    Point intersectionPoint = t * ray.D + ray.O;
    Vector N = (intersectionPoint - Point(center.x, intersectionPoint.y, center.z)).normalized();


  	// check if we intersect one of the bases
    if (y < -EPSILON)
    {
		Plane base(center, Vector(0, 1, 0));
		Hit intersect = base.intersect(ray);
		if (intersect.t < EPSILON)
			return Hit::NO_HIT();

		t = intersect.t;
		intersectionPoint = intersect.t * ray.D + ray.O;

		if (sqrt(pow(intersectionPoint.x - center.x, 2) + pow(intersectionPoint.z - center.z, 2)) > radius)
			return Hit::NO_HIT();
		else
			N = Point(0, -1, 0);
    }
    if (y > height + EPSILON)
    {
		Plane base(Point(center.x, center.y + height, center.z), Vector(0, -1, 0));
		Hit intersect = base.intersect(ray);
		if (intersect.t < EPSILON)
			return Hit::NO_HIT();

		t = intersect.t;
		intersectionPoint = intersect.t * ray.D + ray.O;

		if (sqrt(pow(intersectionPoint.x - center.x, 2) + pow(intersectionPoint.z - center.z, 2)) > radius)
			return Hit::NO_HIT();
		else
			N = Point(0, 1, 0);
    }

    return Hit(t, N);
}

Cylinder::Cylinder(Point const &p, double r, double h)
: center(p), radius(r), height(h)
{}
