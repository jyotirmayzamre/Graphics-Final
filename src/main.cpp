#include "camera.h"
#include "sphere.h"
#include "triangle.h"
#include "hittable.h"
#include "hittable_list.h"
#include "helper.h"
#include "threadpool.h"


inline void create_mesh(const char* file, hittable_list& world){
    std::ifstream mesh(file);

    std::string line;
    while(std::getline(mesh, line)){
        if(line.empty())
            continue;
        double x1, y1, z1, x2, y2, z2, x3, y3, z3;
        std::istringstream iss(line.substr(0));
        iss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3;
        world.add(make_shared<triangle>(point3(x1, y1, z1), point3(x2, y2, z2), point3(x3, y3, z3)));
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
    


    Camera cam;
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 250;
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

