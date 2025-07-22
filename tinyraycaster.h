#ifndef TINYRAYCASTER_H
#define TINYRAYCASTER_H

#include <cstdint>  // uint8_t and uint32_t (fixed-width integers for color values)
#include <string>   // std::string
#include <vector>   // std::vector

// Packs 4 bytes (RGBA) into a single 32-bit integer
uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255);

// Unpacks a 32-bit color into its RGBA components
void unpack_color(const uint32_t& color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

// Writes a framebuffer (stored as packed 32-bit color values) to a PPM image file.
// Format used: P6 (binary RGB image format).
void drop_ppm_image(const std::string filename, const std::vector<uint32_t>& image, const size_t h, const size_t w);

#endif // TINYRAYCASTER_H