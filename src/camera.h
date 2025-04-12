#ifndef CAMERA_H
#define CAMERA_H

#include "ray.h"

/*
Camera class

Members: 


*/

class Camera {
    public:
        Camera() {};

        //set center of the camera
        void setCenter(const point3 camcenter) {
            center = camcenter;
        }

        //vectors across horizontal + vertical viewport edges
        void setViewU(const double& viewport_width, const double& image_width){
            viewport_u = vec3(viewport_width, 0, 0);
            pixel_delta_u = viewport_u / image_width;
        }

        //horizontal + vertical delta vectors
        void setViewV(const double& viewport_height, const double& image_height){
            viewport_v = vec3(0, -viewport_height, 0);
            pixel_delta_v = viewport_v / image_height;
        }

        /*
        Finding the location of the upper left pixel
        Find the upper left corner of the viewport by subtract focal length from the camera center to get viewport center
        Subtract half of horizontal + vertical vectors to move left and up
        */
        void setLoc(const double& distance){
            pixel00_loc = center - vec3(0, 0, distance) - (viewport_u + viewport_v) / 2.0 + 0.5*(pixel_delta_u + pixel_delta_v);
        }

        const point3& getCenter() const {
            return center;
        }

        const vec3& getDeltaU() const {
            return pixel_delta_u;
        }


        const vec3& getDeltaV() const {
            return pixel_delta_v;
        }


        const vec3& getFirstLoc() const {
            return pixel00_loc;
        }

        
    private:
        point3 center = point3(0, 0, 0);
        vec3 viewport_u;
        vec3 viewport_v;        
        vec3 pixel_delta_u;
        vec3 pixel_delta_v;
        vec3 pixel00_loc;

};

#endif

