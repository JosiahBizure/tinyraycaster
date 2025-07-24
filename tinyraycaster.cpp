#include <iostream>
#include <fstream>
#include <cassert>

#include "tinyraycaster.h"

uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
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


/*
    Draws a filled rectangle on the image buffer, starting at position (start_col, start_row)
    and extending the width rect_width pixels to the right and rect_height pixels downward.
    Color is packed RGBA (as a uint32_t)
*/
void draw_rectangle(std::vector<uint32_t>& image, const size_t image_height, const size_t image_width,
                    const size_t start_col, const size_t start_row,
                    const size_t rect_height, const size_t rect_width,
                    const uint32_t color) {
    // Ensure the image has the correct total size
    assert(image.size() == image_width * image_height);

    // Loop over every pixel in the rectangle's area
    for (size_t col_offset = 0; col_offset < rect_width; ++col_offset) {
        for (size_t row_offset = 0; row_offset < rect_height; ++row_offset) {
            size_t col = start_col + col_offset; // actual column position in image
            size_t row = start_row + row_offset; // actual row position in image

            // Make sure we don’t go out of bounds
            assert(col < image_width && row < image_height);

            // Convert 2D (row, col) to 1D index and set the pixel color
            image[col + row * image_width] = color;
        }
    }
}


int main() {
    const size_t win_h = 512;   // image height (pixels)
    const size_t win_w = 512;   // image width  (pixels)

    /*
        1D array that stores every pixel in the image.
        Each pixel's color is encoded as a uint32_t (4 bytes RGBA packed together)

        Conceptually: it's a frame of pixel data stored in memory
        Practically: it's a grid of pixels flattened into a 1D array

        Initialized with 255 for each pixel (red-ish grayish tone)
        It's basically a blank canvas at this point
    */
    std::vector<uint32_t> framebuffer(win_h * win_w, 255);

    /*
        A 16×16 top-down map represented as a flattened 1D string.
        Each character represents a cell in the map.
        ' ' (space) = empty space
        Any other character (e.g., '0', '1', '2', '3') = draw a rectangle (wall or object)

        Stored in row-major order, just like the framebuffer.
    */
    const size_t map_h = 16; // map height
    const size_t map_w = 16; // map width
    const char map[] = "0000222222220000"\
                       "1              0"\
                       "1      11111   0"\
                       "1     0        0"\
                       "0     0  1110000"\
                       "0     3        0"\
                       "0   10000      0"\
                       "0   0   11100  0"\
                       "0   0   0      0"\
                       "0   0   1  00000"\
                       "0       1      0"\
                       "2       1      0"\
                       "0       0      0"\
                       "0 0000000      0"\
                       "0              0"\
                       "0002222222200000"; // our game map

    assert(sizeof(map) == map_h * map_w + 1); // +1 for the null terminated string

    float player_x = 3.456; // player x position
    float player_y = 2.345; // player y position

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

    /*
        Want to overlay the map on top of the gradient background.
        We'll draw a rectangle for each map cell, scaled to the image size
        Calculating the pixel size of each map cell in the image:
            - Since the map is 16×16 and the image is 512×512, 
              each map cell will be 32×32 pixels.
    */
    const size_t rect_w = win_w / map_w;
    const size_t rect_h = win_h / map_h;

    /*
        Loop through every cell in the map grid.
        For each non-empty cell, draw a filled rectangle (cyan) in the framebuffer
        corresponding to that cell's position in the grid.
    */
    for (size_t row = 0; row < map_h; ++row) {
        for (size_t col = 0; col < map_w; ++col) {
            char cell = map[row * map_w + col];

            // Skip drawing if the cell is a space (empty)
            if (cell == ' ') continue;

            /*
                Convert map grid coordinates (col, row) to pixel coordinates (x, y)
                Say we're drawing cell (5, 2) in map space:
                    - it's the 6th column and 3rd row of the map
                    - Converted to pixel coordinates plug in the values to get (160, 64)
            */
            size_t pixel_x = col * rect_w;
            size_t pixel_y = row * rect_h;

            // Draw a filled 32x32 cyan rectangle at the pixel location [(160, 64) in my example]
            draw_rectangle(
                framebuffer,
                win_w, win_h,
                pixel_x, pixel_y,
                rect_w, rect_h,
                pack_color(0, 255, 255) // cyan
            );
        }
    }

    /*
        Draw the player as a small white square on the map.

        The player’s position is defined in map space (floating-point grid coords),
        not pixel space, so we convert it to image pixel space by scaling with rect_w and rect_h.

        The marker is always a fixed 5×5 square regardless of zoom or cell size.
    */
    draw_rectangle(
                framebuffer,
                win_w, win_h,
                player_x * rect_w, player_y * rect_h,
                5, 5,
                pack_color(255, 255, 255) // white
            );

    // Takes the pixel data from framebuffer and writes it to disk in PPM format
    drop_ppm_image("./out.ppm", framebuffer, win_h, win_w);

    return 0;
}