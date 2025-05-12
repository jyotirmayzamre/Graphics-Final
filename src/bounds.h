#ifndef BOUNDS_H
#define BOUNDS_H

#include "ray.h"

class Bounds3f {
    public:
        point3 min, max;

        Bounds3f(const point3& min, const point3& max) : min(min), max(max) {}

        float SurfaceArea() const {
            float x = max.x - min.x;
            float y = max.y - min.y;
            float z = max.z - min.z;
            return 2 * ((x*y) + (x*z) + (y*z));
        }

        int maximumExtent() const {
            float x = max.x - min.x;
            float y = max.y - min.y;
            float z = max.z - min.z;
            float maxEx = std::max({x, y, z});
            if (x == maxEx){
                return 0;
            } else if (y == maxEx){
                return 1;
            } else {
                return 2;
            }
        }
};



Bounds3f Union(const Bounds3f& b1, const Bounds3f& b2){
    double newMinX = std::min({b1.min.x, b2.min.x});
    double newMinY = std::min({b1.min.y, b2.min.y});
    double newMinZ = std::min({b1.min.z, b2.min.z});

    double newMaxX = std::max({b1.min.x, b2.min.x});
    double newMaxY = std::max({b1.min.y, b2.min.y});
    double newMaxZ = std::max({b1.min.z, b2.min.z});


    return Bounds3f(point3(newMinX, newMinY, newMinZ), point3(newMaxX, newMaxY, newMaxZ));

}


#endif