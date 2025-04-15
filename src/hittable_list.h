#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "helper.h"
#include "hittable.h"
#include <vector>

using std::make_shared;
using std::shared_ptr;

class hittable_list : public hittable {
    public:
        //objects holds shared pointers to hittable objects
        //so using shared pointers allows other pieces of code to access the object without memory related issues
        //no need to cleanup the object yourself
        std::vector<shared_ptr<hittable>> objects;

        hittable_list() {}
        hittable_list(shared_ptr<hittable> object) { add(object); }

        void clear() { objects.clear(); }

        //push back = append
        void add(shared_ptr<hittable> object) {
            objects.push_back(object);
        }

        bool hit(const Ray&r, double ray_tmin, double ray_tmax, hit_record& rec) const override {
            hit_record temp_rec;
            bool hit_anything = false;

            auto closest = ray_tmax;

            //loop over all objects in hittable list
            //if any object is hit, update hit records
            //new t will always be smaller than closest and keep passing it as ray_tmax
            for (const auto& object : objects) {
                if (object->hit(r, ray_tmin, closest, temp_rec)){
                    hit_anything = true;
                    closest = temp_rec.t;
                    rec = temp_rec;
                }
            }
            return hit_anything;
        }
};

#endif