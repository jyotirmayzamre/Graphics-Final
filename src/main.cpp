#include "colour.h"
#include "camera.h"

#include <iostream>
#include <stdbool.h>

//sphere intersection code
//need the radius (double), center (point 3), Ray r
//compute the ray-sphere intersection components
//check discriminants (if the ray passes through the sphere, then colour it red)

bool hit_sphere(const Ray& r, const point3& center, double radius){
    vec3 diff = center - r.origin();
    auto a = glm::dot(r.direction(), r.direction());
    auto b = -2.0 * glm::dot(r.direction(), diff);
    auto c = glm::dot(diff, diff) - radius*radius;
    auto disc = b*b - 4*a*c;
    return (disc >= 0);
}


// gradient to get interpolation between blue and white depending on ray's y coordinate
colour ray_colour(const Ray& r){
    if (hit_sphere(r, point3(0, 0, -1), 0.5)){
        return colour(1, 0, 0);
    }
    vec3 unit_direction = glm::normalize(r.direction());
    //scale from [-1, 1] to [0, 1]
    auto a = 0.5*(unit_direction.y + 1.0);
    return double(1.0 - a)*colour(1.0, 1.0, 1.0) + double(a)*colour(0.5, 0.7, 1.0);
}


int main(){

    //setting aspect ratio + image dimensions
    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    int image_height = int (image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    //setting viewport dimensions
    auto distance = 1.0;
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (double(image_width)/image_height);


    //camera class
    Camera camera;
    camera.setCenter(point3(0, 0, 0));
    camera.setViewU(double(viewport_width), double(image_width));
    camera.setViewV(double(viewport_height), double(image_height));
    camera.setLoc(distance);

    point3 center = camera.getCenter();
    vec3 pixel00_loc = camera.getFirstLoc();
    vec3 pixel_delta_u = camera.getDeltaU();
    vec3 pixel_delta_v = camera.getDeltaV();


    //format for ppm file
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = 0; j < image_height; j++){
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++){

            //calculate the pixel center and ray direction
            //from the first pixel location, find the offset using i or j * the offset vectors for the direction and add
            //ray direction: to get vector AB, we do (B-A)
            auto pixel_center = pixel00_loc + (double(i) * pixel_delta_u) + (double(j) * pixel_delta_v);
            auto ray_dir = pixel_center - center;
            Ray r(center, ray_dir);
            colour pixel_colour = ray_colour(r);
            write_colour(std::cout, pixel_colour);
        }
    }

    std::clog << "\rDone.                     \n";
    return 1;
}