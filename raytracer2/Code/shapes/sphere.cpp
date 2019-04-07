#include "sphere.h"
#include "solvers.h"

#include <cmath>
#include <iostream>

using namespace std;

Color Sphere::textureColorAt(Point point, bool shouldRotate)
{
  Vector N = (point - position).normalized();

  if (shouldRotate)
    N = rotate(N);

  double PI = acos(-1);

  double u = atan2(-N.y, -N.x) / (2 * PI) + 0.5;
  double v = 0.5 - asin(N.z) / PI;

  Color color = material.texture.colorAt(u, v);

  return color;
}

Vector Sphere::rotate(Vector normal)
{
  double radAngle = angle * (acos(-1) / 180);

  Vector rotatedNormal = normal * cos(radAngle) + ((rotation.normalized()).cross(normal)) * sin(radAngle) +
          rotation.normalized() * (rotation.normalized()).dot(normal) * (1 - cos(radAngle));

  return rotatedNormal.normalized();
}


Hit Sphere::intersect(Ray const &ray)
{
    // Sphere formula: ||x - position||^2 = r^2
    // Line formula:   x = ray.O + t * ray.D

    Vector L = ray.O - position;
    double a = ray.D.dot(ray.D);
    double b = 2 * ray.D.dot(L);
    double c = L.dot(L) - r * r;

    double t0;
    double t1;
    if (!Solvers::quadratic(a, b, c, t0, t1))
        return Hit::NO_HIT();

    // t0 is closest hit
    if (t0 < 0)  // check if it is not behind the camera
    {
        t0 = t1;    // try t1
        if (t0 < 0) // both behind the camera
            return Hit::NO_HIT();
    }

    // calculate normal
    Point hit = ray.at(t0);
    Vector N = (hit - position).normalized();

    // determine orientation of the normal
    if (N.dot(ray.D) > 0)
        N = -N;

    return Hit(t0, N);
}

Sphere::Sphere(Point const &pos, double radius, Vector rot, int ang)
:
    position(pos),
    r(radius),
    rotation(rot),
    angle(ang)
{}
