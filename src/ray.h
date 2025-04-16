#ifndef RAY_H
#define RAY_H

#include "helper.h"

#include <glm/glm.hpp>

using point3 = glm::dvec3;
using vec3 = glm::dvec3;

/*
Ray class
Members: glm::vec3 origin, glm::vec3 direction
Methods:

eval() - Evaluates the ray at given time t

*/
class Ray {
    public:

        Ray() {};

        //constructor function
        Ray(const point3& origin, const vec3& direction) : orig(origin), dir(direction) {};

        //getters for members - read only reference, doesn't modify the object
        const point3& origin() const {
            return orig;
        }

        const vec3& direction() const {
            return dir;
        }

        //const after the argument means that the object it is called on isn't modified
        point3 eval(double t) const {
            return orig + t*dir;
        }


    private:
        point3 orig;
        vec3 dir;

};



#endif