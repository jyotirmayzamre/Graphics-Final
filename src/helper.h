#ifndef HELPER_H
#define HELPER_H

#include <cmath>
#include <iostream>
#include <limits>
#include <stdbool.h>
#include <chrono>
#include <algorithm>
#include <execution>
#include <thread>
#include <functional>
#include <memory>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>




using std::make_shared;
using std::shared_ptr;


// Constant
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.141592653589793285;

// Utilities
inline double degrees_to_radians(double degrees){
    return degrees * pi / 180.0;
}

inline double random_double(double min, double max) {
    return min + (max-min)*(std::rand() / (RAND_MAX +  1.0));
}



#include "colour.h"
#include "ray.h"
#include "interval.h"

inline vec3 rand_unit_vector() {
    while (true) {
        auto p = vec3(random_double(-1,1), random_double(-1,1), random_double(-1,1));
        auto lensq = glm::length2(p);
        if (1e-160 < lensq && lensq <= 1)
            return p / sqrt(lensq);
    }
}

inline bool near_zero(vec3 ray){
    auto s = 1e-8;
    return (std::fabs(ray.x) < s) && (std::fabs(ray.y) < s) && (std::fabs(ray.z) < s);
}

inline vec3 reflect(const vec3& v, const vec3& n){
    return v - 2*glm::dot(v, n)*n;
}

double getCoord(const point3& p, int axis){
    switch(axis){
        case 0: return p.x;
        case 1: return p.y;
        case 2: return p.z;
        default: throw std::runtime_error("Invalid axis");
    }
}


inline void parse_obj(const char* file_vert, const char* file_faces){
    std::vector<point3> vertices;
    
    std::ifstream file(file_vert);

    std::string line;
    while(std::getline(file, line)){
        if(line.empty())
            continue;

        double x, y, z;
        std::istringstream iss(line.substr(1));
        iss >> x >> y >> z;
        vertices.push_back(point3(x, y, z));
    }
    file.close();



    std::ifstream file2(file_faces);
    std::string line2;

    std::ostringstream buffer;
    while(std::getline(file2, line2)){
        if(line2.empty())
            continue;
        int v1, v2, v3;
        std::istringstream iss(line2.substr(1));
        iss >> v1 >> v2 >> v3;
        buffer << vertices[v1-1].x << ' ' <<  vertices[v1-1].y << ' ' << vertices[v1-1].z << ' ' << vertices[v2-1].x << ' ' <<  vertices[v2-1].y << ' ' << vertices[v2-1].z << ' ' << vertices[v3-1].x << ' ' <<  vertices[v3-1].y << ' ' << vertices[v3-1].z << '\n';
    }
    file2.close();
    
    std::ofstream outFile("assets/dragon.txt");
    outFile << buffer.str();
    
}


#endif