#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <cassert>

uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a=255) {
    return (a << 24) + (b << 16) + (g << 8) + r;
}


void unpack_color(const uint32_t& color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    r = (color >>  0) & 0xFF;
    g = (color >>  8) & 0xFF;
    b = (color >> 16) & 0xFF;
    a = (color >> 24) & 0xFF;
}


void drop_ppm_image(const std::string filename, const std::vector<uint32_t>& image, const size_t w, const size_t h) {
    assert(image.size() == w * h);
    std::ofstream fout(filename, std::ios::binary);
    fout << "P6\n" << w << " " << h << "\n255\n";

    // Loop over every pixel in the framebuffer
    for (size_t i = 0; i < w * h; ++i) {
        uint8_t r, g, b, a;

        // Extract RGBA components from packed 32-bit color
        unpack_color(image[i], r, g, b, a);

        // Write R, G, B values as binary characters to the file (no alpha in PPM)
        // This function is writing binary, not text, so the char casts are important
        fout << static_cast<char>(r)
             << static_cast<char>(g)
             << static_cast<char>(b);
    }

    fout.close();
}


void draw_rectangle(std::vector<uint32_t>& image, const size_t image_width, const size_t image_height,
                    const size_t start_col, const size_t start_row,
                    const size_t rect_width, const size_t rect_height,
                    const uint32_t color) {

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
    const size_t win_w = 1024;   // image width  (pixels)
    const size_t win_h = 512;    // image height (pixels)

    /*
        1D array that stores every pixel in the image.

        Conceptually: it's a frame of pixel data stored in memory
        Practically: it's a grid of pixels flattened into a 1D array

        Initialized each pixel to white (blank canvas)
    */
    std::vector<uint32_t> framebuffer(win_w * win_h, pack_color(255, 255, 255));

    /*
        A 16×16 top-down map represented as a flattened 1D string.
        Each character represents a cell in the map.
        ' ' (space) = empty space
        Any other character (e.g., '0', '1', '2', '3') = draw a rectangle (wall or object)

        Stored in row-major order, just like the framebuffer.
    */
    const size_t map_w = 16; // map width
    const size_t map_h = 16; // map height
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

    float player_x = 3.456f; // player x position (map coordinates)
    float player_y = 2.345f; // player y position (map coordinates)
    float player_angle = 1.523f; // player viewing angle in radians (0 = facing right, counter-clockwise)
    const float fov = M_PI / 3.0f; // 60 degree field of view

    const size_t ncolors = 10;
    std::vector<uint32_t> colors(ncolors);
    for (size_t i = 0; i < ncolors; i++) {
        colors[i] = pack_color(rand() % 255, rand() % 255, rand() % 255);
    }

    /*
        Want to overlay the map on top of the gradient background.
        We'll draw a rectangle for each map cell, scaled to the image size
        Calculating the pixel size of each map cell in the image:
            - Since the map is 16×16 and the image is 512×512, 
              each map cell will be 32×32 pixels.
    */
    const size_t rect_w = win_w / (map_w * 2);
    const size_t rect_h = win_h / map_h;


    for (size_t frame = 0; frame < 360; ++frame) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << frame << ".ppm";
        player_angle += 2 * M_PI / 360;

        framebuffer = std::vector<uint32_t>(win_w * win_h, pack_color(255, 255, 255)); // clear the screen

        /*
            Draw the map:
                - Loop through every cell in the map grid.
                - For each non-empty cell in the map grid, draw a filled rectangle (cyan)
                in the framebuffer corresponding to that cell's position in the grid.
        */
        for (size_t row = 0; row < map_h; ++row) {
            for (size_t col = 0; col < map_w; ++col) {
                char cell = map[row * map_w + col];

                // Skip empty space
                if (cell == ' ') continue;

                /*
                    Convert map grid coordinates (col, row) to pixel coordinates (x, y)
                    Say we're drawing cell (5, 2) in map space:
                        - it's the 6th column and 3rd row of the map
                        - Converting to pixel coordinates plug in the values to get (160, 64)
                */
                size_t pixel_x = col * rect_w;
                size_t pixel_y = row * rect_h;

                size_t icolor = map[col + row * map_w] - '0';
                assert(icolor<ncolors);

                // Draw a filled 32x32 cyan rectangle at the pixel location [(160, 64) in my example]
                draw_rectangle(
                    framebuffer,
                    win_w, win_h,
                    pixel_x, pixel_y,
                    rect_w, rect_h,
                    colors[icolor]
                );
            }
        }


        /*
            Draw the visibility cone and "3D" view based on player's viewpoint

            We cast one ray per horizontal screen column (win_w),
            linearly interpolating the angle from [player_angle - fov/2, player_angle + fov/2].
        */
        const float ray_step = 0.01f;
        const float ray_length = 20.0f;

        for (size_t col = 0; col < win_w / 2; ++col) {
            // Compute the current ray angle across the FOV
            float ray_angle = player_angle - fov / 2.0f + fov * (col / float(win_w / 2));

            for (float t = 0.0f; t < ray_length; t += ray_step) {
                float ray_x = player_x + t * std::cos(ray_angle);
                float ray_y = player_y + t * std::sin(ray_angle);

                // Convert map coordinates to pixel coordinates
                size_t pixel_x = static_cast<size_t>(ray_x * rect_w);
                size_t pixel_y = static_cast<size_t>(ray_y * rect_h);

                // Draw a white pixel at this ray position
                framebuffer[pixel_x + pixel_y * win_w] = pack_color(160, 160, 160);

                // If ray hits a wall, draw vertical column to create illusion of 3D
                if (map[int(ray_x) + int(ray_y) * map_w] != ' ') {
                    size_t icolor = map[int(ray_x) + int(ray_y) * map_w] - '0';
                    assert(icolor < ncolors);
                    size_t column_height = win_h / t;
                    draw_rectangle(
                        framebuffer,
                        win_w, win_h,
                        win_w / 2 + col, win_h / 2 - column_height / 2,
                        1, column_height,
                        colors[icolor]
                    );
                    break;
                }
            }
        }
        drop_ppm_image(ss.str(), framebuffer, win_w, win_h);
    }
    return 0;
}