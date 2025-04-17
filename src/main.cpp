#include "camera.h"
#include "sphere.h"
#include "hittable.h"
#include "hittable_list.h"
#include "helper.h"
#include "threadpool.h"


int main(){

    hittable_list world;
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1),100 ));

    Camera cam;
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 1920;
    cam.initialize();
    cam.render(world);

    
    return 1;
}

