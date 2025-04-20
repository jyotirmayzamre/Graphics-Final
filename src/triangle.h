#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "helper.h"


class triangle: public hittable {
    public:

        triangle(const point3& t1, const point3& t2, const point3& t3) : t1(t1), t2(t2), t3(t3) {}

        bool hit(const Ray& r, interval ray_t, hit_record& rec) const override {

            //calculate barycentric coordinates and check conditions
            auto a = t1.x - t2.x;
            auto b = t1.y - t2.y;
            auto c = t1.z - t2.z;
            auto d = t1.x - t3.x;
            auto e = t1.y - t3.y;
            auto f = t1.z - t3.z;
            auto g = r.direction().x;
            auto h = r.direction().y;
            auto i = r.direction().z;
            auto j = t1.x - r.origin().x;
            auto k = t1.y - r.origin().y;
            auto l = t1.z - r.origin().z;

            //precompute common terms
            auto c1 = e*i - h*f;
            auto c2 = g*f - d*i;
            auto c3 = d*h - e*g;
            auto c4 = a*k - j*b;
            auto c5 = j*c - a*l;
            auto c6 = b*l - k*c;

            auto M = 1 / (a*c1 + b*c2 + c*c3);
            
            auto t = M * (-f * c4 + e*c5 + d*c6);

            if(!ray_t.surrounds(t)){return false;}

            auto gamma = M * (i*c4 + h*c5 + g*c6);
            if(gamma < 0 || gamma > 1){return false;}

            auto beta = M * (j*c1 + k*c2 + l*c3);
            if(beta < 0 || beta > 1 - gamma){return false;}

            //store hit record details
            rec.t = t;
            rec.p = r.eval(t);
            vec3 normal = glm::normalize(glm::cross(t2-t1, t3-t1));
            rec.set_face_normal(r, normal);
            return true;
        }


    private:
        point3 t1;
        point3 t2;
        point3 t3;
};


#endif