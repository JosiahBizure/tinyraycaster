# tinyraycaster
I didn’t know anything about graphics when I started this project, so I’m building a tiny ray caster in C++.

Inspired by early 3D games like *Wolfenstein 3D*, this project helps me explore how ray casting works from scratch. No OpenGL, no fancy libraries, just basic math and pixels.

This is not a finished game, it's a learning tool. I'm focusing on clarity over clever optimizations.

---

## Progress So Far

- **Step 1**: Generate a simple color gradient and save it as a `.ppm` image  
    - Stores pixels in a 1D framebuffer (`std::vector<uint32_t>`)  
    - Uses `pack_color` / `unpack_color` to manage RGBA values  
    - Outputs raw RGB image in PPM (P6) format

- **Step 2**: Draw a top-down map as a grid of rectangles  
    - Represents a 16×16 map using a flat string  
    - Each map cell corresponds to a 32×32 block in the image  
    - Non-space characters are drawn as cyan rectangles over the gradient  
    - Uses `draw_rectangle()` helper to fill pixel regions based on map layout

- **Step 3**: Draw the player’s position on the map  
    - Adds floating-point `(x, y)` coordinates to represent the player in map space  
    - Converts player position to pixel space and draws a small 5×5 white square  
    - Independent of map content; no collision or orientation yet

- **Step 4**: Cast a simple ray from the player's viewpoint  
    - Adds `player_angle` to define viewing direction (in radians)  
    - Steps along a ray using cosine and sine to compute (x, y) positions  
    - Stops when a wall is hit; draws each ray point in white