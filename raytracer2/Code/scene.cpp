#include "scene.h"

#include "hit.h"
#include "image.h"
#include "material.h"
#include "ray.h"

#include <cmath>
#include <limits>
#include <iostream>

using namespace std;


Color Scene::reflectRay(int depth, Hit min_hit, Ray ray, ObjectPtr obj)
{
  Material &material = obj->material;
  Point hit = ray.at(min_hit.t - 0.0000000001);             //the hit point
  Vector N = min_hit.N;

  Vector R = 2 * (N.dot(-ray.D)) * N + ray.D;
  Vector V = -ray.D;
  R.normalize();
  Ray reflectedRay{hit, R};

  Hit min_reflectedHit(numeric_limits<double>::infinity(), Vector());
  ObjectPtr refObj = nullptr;
  for (unsigned idx = 0; idx != objects.size(); ++idx)
  {
    Hit hit2(objects[idx]->intersect(reflectedRay));
    if (hit2.t < min_reflectedHit.t && objects[idx] != obj)
    {
      min_reflectedHit = hit2;
      refObj = objects[idx];
    }
  }
  if (refObj != nullptr)
  {
    Point reflectedHit = reflectedRay.at(min_reflectedHit.t);
    Light reflectedLight(reflectedHit, trace(reflectedRay, depth + 1) * material.ks);
    Vector L = (reflectedLight.position - hit).normalized();
    R = 2 * (N.dot(L)) * N - L;

    return Color(pow(fmax(0 ,V.dot(R)), material.n) * reflectedLight.color * material.ks);
  }
  return Color(0, 0, 0);
}

Color Scene::trace(Ray const &ray, int depth)
{
	// Find hit object and distance
	Hit min_hit(numeric_limits<double>::infinity(), Vector());
	ObjectPtr obj = nullptr;
	for (unsigned idx = 0; idx != objects.size(); ++idx)
	{
		Hit hit(objects[idx]->intersect(ray));
		if (hit.t < min_hit.t)
		{
			min_hit = hit;
			obj = objects[idx];
		}
	}
	// No hit? Return background color.
	if (!obj) return Color(0.0, 0.0, 0.0);

  Material &material = obj->material;         //the hit objects material
  Point hit;       //the hit point
  Vector V = -ray.D;
  Vector R;
  Vector N = min_hit.N;
  hit = ray.at(min_hit.t - 0.0000000001);          //the hit point


  if (material.isTextured())
  {
    material.color = obj->textureColorAt(hit, obj->isRotated());
  }

  Color Ia = material.color * material.ka;
  Color Id(0, 0, 0);
	Color Is(0, 0, 0);

	for (auto const light : lights)
	{
		//shadows calculations:
		if (shadows)
		{
			Ray lightRay(light->position, -(light->position - hit).normalized());

			Hit min_hit2(numeric_limits<double>::infinity(), Vector());
			ObjectPtr blockingObj = nullptr;
			for (unsigned idx = 0; idx != objects.size(); ++idx)
			{
				Hit checkHit(objects[idx]->intersect(lightRay));
				if (checkHit.t < min_hit2.t)
				{
					min_hit2 = checkHit;
					blockingObj = objects[idx];
				}
			}
			if (blockingObj == obj)
			{
				R = 2 * (N).dot((light->position - hit).normalized()) * N - (light->position - hit).normalized();
				Id += fmax(0, ((light->position - hit).normalized()).dot(N.normalized())) * material.color * light->color * material.kd;
				Is += pow(fmax(0, R.dot(V)), material.n) * light->color * material.ks;

        if (depth < recursionDepth)
        {
            Is += reflectRay(depth, min_hit, ray, obj);
        }
			}
		}
		else
		{
      R = 2 * (N).dot((light->position - hit).normalized()) * N - (light->position - hit).normalized();
			Id += fmax(0, ((light->position - hit).normalized()).dot(N.normalized())) * material.color * light->color * material.kd;
			Is += pow(fmax(0, R.dot(V)), material.n) * light->color * material.ks;
      if (depth < recursionDepth)
      {
          Is += reflectRay(depth, min_hit, ray, obj);
      }
		}
	}

	Color color = Ia + Id + Is;

	return color;
}

void Scene::render(Image &img)
{
	unsigned w = img.width();
	unsigned h = img.height();
  Color col{};
  for (float i = 0.5 / samplingFactor; i < h; i += 1.0 / samplingFactor)
  {
    for (float j = 0.5 / samplingFactor; j < w; j += 1.0 / samplingFactor)
    {
      Point pixel(i + 0.5, (h - 1 - j) + 0.5, 0);
      Ray ray(eye, (pixel - eye).normalized());
      col = trace(ray, 0);
      col.clamp();
      img((int)i, (int)j) += col / (samplingFactor * samplingFactor);
    }
  }
}

// --- Misc functions ----------------------------------------------------------

void Scene::addObject(ObjectPtr obj)
{
	objects.push_back(obj);
}

void Scene::addLight(Light const &light)
{
	lights.push_back(LightPtr(new Light(light)));
}

void Scene::setEye(Triple const &position)
{
	eye = position;
}

unsigned Scene::getNumObject()
{
	return objects.size();
}

unsigned Scene::getNumLights()
{
	return lights.size();
}
