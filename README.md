# tinyraycaster
I don’t know anything about graphics, so I’m starting with this tiny ray caster in C++.

Inspired by early 3D games like *Wolfenstein 3D*, this project helps me explore how ray casting works from scratch. No OpenGL, no fancy libraries—just basic math and pixels.

This is not a finished game—it's a learning tool. I'm focusing on clarity over clever optimizations.

---

## Progress So Far

- **Step 1**: Generate a simple color gradient and save it as a `.ppm` image  
    - Stores pixels in a 1D framebuffer (`std::vector<uint32_t>`)  
    - Uses `pack_color` / `unpack_color` to manage RGBA values  
    - Outputs raw RGB image in PPM (P6) format  