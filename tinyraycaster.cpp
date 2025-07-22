#include <iostream>
#include <fstream>
#include <cassert>

#include "tinyraycaster.h"

uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {
    return (a << 24) + (b << 16) + (g << 8) + r;
}


void unpack_color(const uint32_t& color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    r = (color >>  0) & 0xFF;
    g = (color >>  8) & 0xFF;
    b = (color >> 16) & 0xFF;
    a = (color >> 24) & 0xFF;
}


void drop_ppm_image(const std::string filename, const std::vector<uint32_t>& image, const size_t h, const size_t w) {
    // Ensure the image size matches the expected dimensions
    assert(image.size() == h * w);

    // Open an output file stream in binary mode
    std::ofstream fout(filename, std::ios::binary);

    /*
        Write the PPM header (P6 means binary RGB format)
        Header = "P6\n<width> <height>\n255\n"
            - format identifier
            - image width and height
            - max color value (255 = 8-bit color per channel)
    */
    fout << "P6\n" << w << " " << h << "\n255\n";

    // Loop over every pixel in the framebuffer
    for (size_t i = 0; i < h * w; ++i) {
        uint8_t r, g, b, a;

        // Extract RGBA components from packed 32-bit color
        unpack_color(image[i], r, g, b, a);

        // Write R, G, B values as binary characters to the file (no alpha in PPM)
        // This function is writing binary, not text, so the char casts are important
        fout << static_cast<char>(r)
             << static_cast<char>(g)
             << static_cast<char>(b);
    }

    // Close the file
    fout.close();
}


int main() {
    const size_t win_h = 512;   // image height (pixels)
    const size_t win_w = 512;   // image width  (pixels)

    /*
        1D array that stores the color of every pixel in the image.
        Each pixel's color is encoded as a uint32_t (4 bytes RGBA packed together)

        Conceptually: it's a frame of pixel data stored in memory
        Practically: it's a grid of pixels flattened into a 1D array

        Initialized with 255 for each pixel (red-ish grayish tone)
    */
    std::vector<uint32_t> framebuffer(win_h * win_w, 255);

    for (size_t row = 0; row < win_h; ++row) {  // For each pixel
        for (size_t col = 0; col < win_w; ++col) {
            /*
                Vertical red gradient:
                    - row goes from 0 - (win - 1) (e.g., 0 to 511)
                    - dividing by float(win_h) scales that range [0.0, ~1.0)
                    - multiplying by 255 gives a value in [0, 255)

                So at the top of the image, row = 0 --> red = 0
                At the bottom of the image, row = win_h --> red = ~255
            */
            uint8_t red   = 255 * row / float(win_h);

            /*
                Same logic, but for horizontal green gradient.
                    - left side: col = 0 --> green = 0
                    - right side: col = win_w --> green = ~255
            */
            uint8_t green = 255 * col / float(win_w);

            uint8_t blue  = 0; // No variation in blue--it's fixed at 0

            /*
                Computes the index of the current pixel in the 1D framebuffer
                Then uses pack_color() to convert RGB values into a uint32_t color
            */
            framebuffer[col + row * win_w] = pack_color(red, green, blue);
        }
    }

    // Takes the pixel data from framebuffer and writes it to disk in PPM format
    drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

    return 0;
}