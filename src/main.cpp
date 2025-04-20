#include "camera.h"
#include "sphere.h"
#include "triangle.h"
#include "hittable.h"
#include "hittable_list.h"
#include "helper.h"
#include "threadpool.h"


int main(){

    //make a list of hittable objects
    hittable_list world;
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1),100 ));
    //world.add(make_shared<triangle>(point3(0, 0.5, -0.5), point3(0.5, 0, -0.5), point3(-0.5, 0,-0.5)));
    
   

    Camera cam;
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 1920;
    cam.samples_per_pixel = 4;
    cam.initialize();

    //2d vector for storing colours
    std::vector<std::vector<colour>> image(cam.image_height, std::vector<colour>(cam.image_width));
    auto start = high_resolution_clock::now();

    cam.render(world, image);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    std::clog << "Time taken to render: " << duration.count() << " ms\n";
    cam.writeToFile(image);
    return 1;
}

