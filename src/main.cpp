#include "camera.h"
#include "sphere.h"
#include "hittable.h"
#include "hittable_list.h"
#include "helper.h"
#include <stdbool.h>
#include <chrono>
#include <algorithm>
#include <execution>

using namespace std::chrono;




// gradient to get interpolation between blue and white depending on ray's y coordinate
//if sphere is hit, then shade based on normal vector's components
colour ray_colour(const Ray& r, const hittable& world){
    hit_record rec;
    if (world.hit(r, 0, infinity, rec)){
        return 0.5 * (rec.normal + colour(1, 1, 1));
    }
    
    vec3 unit_direction = glm::normalize(r.direction());
    //scale from [-1, 1] to [0, 1]
    auto a = 0.5*(unit_direction.y + 1.0);
    return double(1.0 - a)*colour(1.0, 1.0, 1.0) + double(a)*colour(0.5, 0.7, 1.0);
}


int main(){

    //setting aspect ratio + image dimensions
    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 1920;
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

    hittable_list world;
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1),100 ));


    //multithreading
    std::vector<double> horizontalIter, verticalIter;
    horizontalIter.resize(image_width);
    verticalIter.resize(image_height);

    for (double i = 0; i < image_width; i++){horizontalIter.push_back(i);}
    for (double i = 0; i < image_height; i++){verticalIter.push_back(i);}




    //format for ppm file
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    auto start  = high_resolution_clock::now();

    std::vector<std::vector<colour>> image(image_height, std::vector<colour>(image_width));
    #define MT 1
    #if MT
        std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(),
            [&](double y) 
            {
                std::for_each(std::execution::par, horizontalIter.begin(), horizontalIter.end(), 
            [&, y](double x) {
                auto pixel_center = pixel00_loc + (double(x) * pixel_delta_u) + (double(y) * pixel_delta_v);
                auto ray_dir = pixel_center - center;
                Ray r(center, ray_dir);
                colour pixel_colour = ray_colour(r, world);
                image[y][x] = pixel_colour;
            });
            });
    #else 
        for (int j = 0; j < image_height; j++){
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++){

                //calculate the pixel center and ray direction
                //from the first pixel location, find the offset using i or j * the offset vectors for the direction and add
                //ray direction: to get vector AB, we do (B-A)
                auto pixel_center = pixel00_loc + (double(i) * pixel_delta_u) + (double(j) * pixel_delta_v);
                auto ray_dir = pixel_center - center;
                Ray r(center, ray_dir);
                colour pixel_colour = ray_colour(r, world);
                write_colour(std::cout, pixel_colour);
            }
        }
    #endif
    
    auto stop = high_resolution_clock::now();
    
    #if MT
        for(int j = 0; j < image_height; j++){
            for (int i = 0; i < image_width; i++){
                write_colour(std::cout, image[j][i]);
            }
        }
    #else
    #endif

    auto duration = duration_cast<milliseconds>(stop - start);
    std::clog << "\nRender Time: " << duration.count() << " ms\n";
    return 1;
}

/*
Implementing multithreading for ray tracer
use 8 threads where each thread renders a single pixel
start off with multithreading using CPU
move to laptop GPU which has 387 cores...

*/