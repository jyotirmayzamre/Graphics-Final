#include "camera.h"
#include "sphere.h"
#include "triangle.h"
#include "hittable.h"
#include "hittable_list.h"
#include "helper.h"
#include "threadpool.h"
#include "material.h"


inline void create_mesh(const char* file, hittable_list& world){
    std::ifstream mesh(file);
    auto no_material = make_shared<absorbing>();

    std::string line;
    while(std::getline(mesh, line)){
        if(line.empty())
            continue;
        double x1, y1, z1, x2, y2, z2, x3, y3, z3;
        std::istringstream iss(line.substr(0));
        iss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3;
        world.add(make_shared<triangle>(point3(x1, y1, z1), point3(x2, y2, z2), point3(x3, y3, z3), no_material));
    }
}

int main(){

    #define parse 0

    #if parse == 1
        parse_obj("assets/dragon_vertices.obj", "assets/dragon_faces.obj");
    #endif
    
    //make a list of hittable objects
    hittable_list world;
    create_mesh("assets/dragon.txt", world);

    // world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    // world.add(make_shared<sphere>(point3(0, -100.5, -1),100 ));

    // auto material_ground = make_shared<lambertian>(colour(0.8, 0.8, 0.0));
    // auto material_center = make_shared<lambertian>(colour(0.1, 0.2, 0.5));
    // auto material_left   = make_shared<metal>(colour(0.8, 0.8, 0.8));
    // auto material_right  = make_shared<metal>(colour(0.8, 0.6, 0.2));

    // world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    // world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.2),   0.5, material_center));
    // world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
    // world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));
    


    Camera cam;
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 100;
    cam.max_depth = 1;
    cam.samples_per_pixel = 1;
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

