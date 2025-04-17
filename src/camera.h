#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "threadpool.h"

using namespace std::chrono;

/*
Camera class

Contains all viewport, image related parameters
Contains methods for intializing all the parameters
Contains render method (thread pool, normal multithreading, normal)
Contains ray_colour (method to compute colour given the pixel ray and hittable objects list)
*/


class Camera {
    public:

        //aspect ratio + image width + samples_per_pixel (antialiasing)
        double aspect_ratio;
        int image_width;
        int samples_per_pixel;
        int image_height;

        
        //function to initialize all the needed parameters
        void initialize() {

            //image height
            image_height = int (image_width / aspect_ratio);
            image_height = (image_height < 1) ? 1 : image_height;

            //camera center
            center = point3(0, 0, 0);

            //viewport dimensions
            auto distance = 1.0;
            auto viewport_height = 2.0;
            auto viewport_width = viewport_height * (double(image_width)/image_height);

            //vectors across horizontal + vertical viewport edges
            auto viewport_u = vec3(viewport_width, 0, 0);
            auto viewport_v = vec3(0, -viewport_height, 0);

            pixel_delta_u = viewport_u / double(image_width);
            pixel_delta_v = viewport_v / double(image_height);

            pixel00_loc = center - vec3(0, 0, distance) - (viewport_u + viewport_v) / 2.0 + 0.5*(pixel_delta_u + pixel_delta_v);

            sample_scale = 1.0 / samples_per_pixel;
        }


        void render(const hittable& world, std::vector<std::vector<colour>>& image){

            //format for ppm file
            std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

            //2d vector for storing pixel values of the image
        

            #define MT 2
            //multithreaded approach
            //essentially compute rows in parallel
            //obtained a speedup from 6 seconds to 1.7 seconds
            //next task: learn about threading properly and implement a threadpool to avoid compute for thread joining
            auto start  = high_resolution_clock::now();
            #if MT == 2
                ThreadPool pool(std::thread::hardware_concurrency());
                for (int j = 0; j < image_height; j++){
                    pool.enqueue([=, &image, &world]{
                        for (int i = 0; i < image_width; i++){
                            colour pixel_colour(0, 0, 0);
                            for (int s = 0; s < samples_per_pixel; s++){
                                Ray r = getRay(i, j);
                                pixel_colour += ray_colour(r, world);
                            }
                            image[j][i] = sample_scale * pixel_colour;
                        }
                    });
                }

                //initial approach for multithreading using std::for_each

            #elif MT == 1
                //multithreading iters
                std::vector<double> horizontalIter, verticalIter;
                horizontalIter.resize(image_width);
                verticalIter.resize(image_height);

                for (double i = 0; i < image_width; i++){horizontalIter.push_back(i);}
                for (double i = 0; i < image_height; i++){verticalIter.push_back(i);}
                std::for_each(std::execution::par_unseq, verticalIter.begin(), verticalIter.end(), [&](double y) {
                    for (int i = 0; i < image_width; i++){
                        auto pixel_center = pixel00_loc + (double(i) * pixel_delta_u) + (double(y) * pixel_delta_v);
                        auto ray_dir = pixel_center - center;
                        Ray r(center, ray_dir);
                        colour pixel_colour = ray_colour(r, world);
                        image[y][i] = pixel_colour;
                    }
                });

            

            //normal approach
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
                        image[j][i] = pixel_colour;
                    }
                }
            #endif
            
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            std::clog << "\nTime taken to render: " << duration.count() << " ms\n";
        }


        //function for writing colours to file
        //separated it out of render function to give thread pool enough time to join all the threads
        //destructor is called when the thread pool goes out of scope (which joins all the threads)
        void writeToFile(const std::vector<std::vector<colour>>& image) {

            auto start = high_resolution_clock::now();

            //buffer for storing pixel files to be flushed to std::cout
            std::ostringstream buffer;

            //reduced the time taken to write to file by storing pixel values in a buffer
            //flushing the buffer to std::cout once all the values are written
            //note: only written for multithreading approach (since for normal nested loop, we can write as the pixel is being processed)
            for(int j = 0; j < image_height; j++){
                for (int i = 0; i < image_width; i++){
                    write_colour(buffer, image[j][i]);
                }
            }

            std::cout << buffer.str();

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            std::clog << "\nTime taken to write to file: " << duration.count() << " ms\n";
        }
        

    private:
        point3 center;
        vec3 pixel_delta_u;
        vec3 pixel_delta_v;
        point3 pixel00_loc;
        double sample_scale;



        //function to return the ray from the camera to the pixel
        //calculate an offset between 0 and 1 and subtract 0.5 (because we are already at the center of the pixel)
        //add the offsets to i and j to get samples within the pixel square
        Ray getRay(int i, int j) const{
            auto offset = bound();
            auto sample = pixel00_loc + ((double(i) + offset.x) * pixel_delta_u) + ((double(j) + offset.y) * pixel_delta_v);
            auto ray_dir = sample - center;
            return Ray(center, ray_dir);
        }

        // gradient to get interpolation between blue and white depending on ray's y coordinate
        //if sphere is hit, then shade based on normal vector's components
        colour ray_colour(const Ray& r, const hittable& world) const{
            hit_record rec;
            if (world.hit(r, interval(0, infinity), rec)){
                return 0.5 * (rec.normal + colour(1, 1, 1));
            }
            
            vec3 unit_direction = glm::normalize(r.direction());
            //scale from [-1, 1] to [0, 1]
            auto a = 0.5*(unit_direction.y + 1.0);
            return double(1.0 - a)*colour(1.0, 1.0, 1.0) + double(a)*colour(0.5, 0.7, 1.0);
        }

        vec3 bound() const {
            return vec3(random_double() - 0.5, random_double() - 0.5, 0);
        }
};


#endif

