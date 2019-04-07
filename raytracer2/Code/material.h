#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "triple.h"
#include "image.h"

class Material
{
    public:
        Color color;        // base color
        Image texture;        // base texture
        bool textured = false;
        double ka;          // ambient intensity
        double kd;          // diffuse intensity
        double ks;          // specular intensity
        double n;           // exponent for specular highlight size

        Material() = default;

        Material(Color const &color, double ka, double kd, double ks, double n)
        :
            color(color),
            ka(ka),
            kd(kd),
            ks(ks),
            n(n)
        {}
        Material(std::string const &filename, double ka, double kd, double ks, double n)
        :
            color(Color()),
            texture(filename),
            textured(true),
            ka(ka),
            kd(kd),
            ks(ks),
            n(n)
        {}
        bool isTextured() { return textured; };
};

#endif
