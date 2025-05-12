//abstract class that will have a hit function (defined by classes that extend the hittable object)
#ifndef HITTABLE_H
#define HITTABLE_H


#include "helper.h"
#include <stdbool.h>
#include "bounds.h"

class material;

//class to store hit details - t value of ray, normal of surface
class hit_record {
    public:
        point3 p;
        vec3 normal;
        shared_ptr<material> mat;
        double t;
        bool front_face;

        //function for setting the normal vector's direction
        //checks dot product to see if ray is inside the sphere or outside
        void set_face_normal(const Ray&r, const vec3& outward_normal){
            front_face = glm::dot(r.direction(), outward_normal) < 0;
            normal = front_face ? outward_normal : -outward_normal;
        }
};

//virtual lets you override a base class method 
class hittable {
    public:
        //virtual destructor 
        virtual ~hittable() = default;

        virtual bool hit(const Ray& r, interval ray_t, hit_record& rec) const = 0;
        virtual Bounds3f BoundingBox() const = 0;
};


#endif