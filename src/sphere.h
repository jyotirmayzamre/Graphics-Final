#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "helper.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <stdlib.h>


class sphere: public hittable {
    public:
        sphere(const point3& center, double radius, shared_ptr<material> mat) : center(center), radius(std::fmax(0, radius)), mat(mat) {}

        //sphere intersection code
        //need the radius (double), center (point 3), Ray r
        //compute the ray-sphere intersection components
        //check discriminant (if the ray passes through the sphere (disc >= 0), then colour it red)
        bool hit(const Ray&r, interval ray_t, hit_record& rec) const override {
            //check for if the center of the sphere is behind the camera 
            //uses squared length for faster computations (avoid square root)
            vec3 diff = center - r.origin();
            auto a = glm::length2(r.direction());
            auto h = glm::dot(r.direction(), diff);
            auto c = glm::length2(diff) - radius*radius;
            auto disc = h*h - a*c;
            
            if (disc < 0){
                return false;
            }

            auto sqrtd = std::sqrt(disc);

            //check if the solutions fall in the range of acceptable t's
            auto root = (h - sqrtd) / a;
            if (!ray_t.surrounds(root)){
                root = (h + sqrtd) / a;
                if (!ray_t.surrounds(root)){
                    return false;
                }
            }

            //set hit record members (compute normalized normal by dividing by radius)
            rec.t = root;
            rec.p = r.eval(root);
            vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.mat = mat;
            return true;
        }

        Bounds3f BoundingBox() const override {
            
            return Bounds3f(point3(0, 0, 0), point3(0, 0, 0)); 
        }

    private:
        point3 center;
        double radius;
        shared_ptr<material> mat;
};

#endif