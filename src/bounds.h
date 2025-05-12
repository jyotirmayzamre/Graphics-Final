#ifndef BOUNDS_H
#define BOUNDS_H

#include "ray.h"
#include "helper.h"

class Bounds {
    public:
        point3 min, max;

        Bounds(const point3& min, const point3& max) : min(min), max(max) {}

        float SurfaceArea() const {
            float x = max.x - min.x;
            float y = max.y - min.y;
            float z = max.z - min.z;
            return 2 * ((x*y) + (x*z) + (y*z));
        }

        int largest() const {
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

        //slab method for intersection with axis aligned bounding box
        bool intersect(const Ray& r, double& tMin, double& tMax) const {
            
            double tmin = (min.x - r.origin().x) / r.direction().x;
            double tmax = (max.x - r.origin().x) / r.direction().x;

            if (tmin > tmax){
                std::swap(tmin, tmax);
            }

            double tymin = (min.y - r.origin().y) / r.direction().y;
            double tymax = (max.y - r.origin().y) / r.direction().y;

            if (tymin > tymax) std::swap(tymin, tymax);

            if ((tmin > tymax) || (tymin > tmax)){
                return false;
            }
            
            if (tymin > tmin) tmin = tymin;
            if (tymax < tmax) tmax = tymax;

            double tzmin = (min.z - r.origin().z) / r.direction().z;
            double tzmax = (max.z - r.origin().z) / r.direction().z;
            if(tzmin > tzmax) std::swap(tzmin, tzmax);

            if ((tmin > tzmax) || (tzmin > tmax)) return false;

            if (tzmin > tmin) tmin = tzmin;
            if (tzmax < tmax) tmax = tzmax;

            tMin = tmin;
            tMax = tmax;
            return true;

        }
};



Bounds Union(const Bounds& b1, const Bounds& b2){
    double newMinX = std::min({b1.min.x, b2.min.x});
    double newMinY = std::min({b1.min.y, b2.min.y});
    double newMinZ = std::min({b1.min.z, b2.min.z});

    double newMaxX = std::max({b1.max.x, b2.max.x});
    double newMaxY = std::max({b1.max.y, b2.max.y});
    double newMaxZ = std::max({b1.max.z, b2.max.z});


    return Bounds(point3(newMinX, newMinY, newMinZ), point3(newMaxX, newMaxY, newMaxZ));

}


#endif