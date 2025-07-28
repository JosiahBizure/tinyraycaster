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

- **Step 5**: Render a visibility cone using many rays  
    - Adds `fov` (field of view) and casts one ray per horizontal screen column  
    - Each ray marches forward until it hits a wall, drawing a white pixel trail  
    - The result is a full visibility cone projected from the player’s perspective  

- **Step 6**: Split Screen — 2D Map + 3D Projection
    - The **left half** (512px) shows a top-down map view of the player and walls.
    - The **right half** (512px) contains a basic raycasted projection to simulate a 3D environment.
    - Each ray marches forward from the player's position until it hits a wall, where we:
        - Draw a gray dot on the 2D map.
        - Render a vertical wall slice in the 3D view (column height inversely proportional to distance).
    - This simulates a basic Wolfenstein-style visibility cone and perspective rendering.

- **Step 7**: First animation — rotating the player’s view  
    - Loops over **360 frames**, incrementing the player’s viewing angle each time  
    - Renders a full circular sweep from the player’s perspective, saving each frame as a `.ppm` image  
    - Assigns a **random color to each wall type** (`'0'`, `'1'`, etc.) for easy visual differentiation  
    - Each frame includes both the **2D map** and the **3D projection**, as in Step 6  
    - Resulting images can be compiled into a `.gif` or `.mp4` using external tools
