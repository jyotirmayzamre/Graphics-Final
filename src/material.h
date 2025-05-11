#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"

/*
Abstract class for materials that an object/primitive will refer to
*/
class material {
    public:

        //virtual destructor with default behaviour
        virtual ~material() = default;

        //function to get scattering of ray
        virtual bool scatter(const Ray& r, const hit_record& rec, colour& attenuation, Ray& scattered) const {
            return false;
        }
};


/*
Randomly scatter light with some attenuation
*/
class lambertian : public material {
    public:
        lambertian(const colour& albedo) : albedo(albedo) {}

        bool scatter(const Ray& r, const hit_record& rec, colour& attentuation, Ray& scattered) const override {
            auto dir = rec.normal + rand_unit_vector();

            //method to avoid 0 direction
            if (near_zero(dir)){
                dir = rec.normal;
            }

            scattered = Ray(rec.p, dir);
            attentuation = albedo;
            return true;
        }

    private:
        colour albedo;
};

class metal: public material {
    public:
        metal(const colour& albedo) : albedo(albedo) {}
        bool scatter(const Ray& r, const hit_record& rec, colour& attentuation, Ray& scattered) const override {
            
            vec3 reflected = reflect(r.direction(), rec.normal);

            scattered = Ray(rec.p, reflected);
            attentuation = albedo;
            return true;
        }

    private:
        colour albedo;
        
};




#endif