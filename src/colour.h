#ifndef COLOR_H
#define COLOR_H

#include <sstream>

using colour = glm::dvec3;

//takes in reference to output stream and pixel_colour
//const& - passing pixel_colour by reference but promising not to modify it
void write_colour(std::ostringstream& buffer, const colour& pixel_colour){
    auto r = pixel_colour.x;
    auto g = pixel_colour.y;
    auto b = pixel_colour.z;

    int rbyte = int(255.99 * r);

    int gbyte = int(255.99 * g);
    int bbyte = int(255.99 * b);

    //storing the colour values in a buffer and then writing out the output in one go
    buffer << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';

    //out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';

}

#endif