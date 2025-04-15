#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <glm/glm.hpp>


using std::make_shared;
using std::shared_ptr;

// Constant
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.141592653589793285;

// Utilities
inline double degrees_to_radians(double degrees){
    return degrees * pi / 180.0;
}

// Common Headers

#include "colour.h"
#include "ray.h"

#endif