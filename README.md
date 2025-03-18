# Soft Shadows for Real-time Rendering Applications

Jared Watrous (jwat2002@stanford.edu)

---

This project implements multiple shadow map rendering methods in a real-time render pipeline. Only Windows is supported.

## Build

Rather than building, it may be easier to use the [precompiled release](https://github.com/thetruejard/cs348k_project/releases).

Requires >=C++17, OpenGL 4.3+, MSVC, and the following dependencies:

- [assimp](https://github.com/assimp/assimp)
- [GLEW](https://github.com/nigels-com/glew)
- [GLFW](https://github.com/glfw/glfw)
- [GLM](https://github.com/g-truc/glm)
- [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h) / [stb_image_write.h](https://github.com/nothings/stb/blob/master/stb_image_write.h)

Once all dependencies are installed and visible to the compiler, open `render_engine.sln` in a compatible version of Visual Studio (tested with VS2019 Platform Toolset v142 and Windows SDK 10.0) and build just as any other solution.

## Run

The program must be executed from the `render_engine/` subfolder such that `shaders/` is in its working directory. On Windows, it can typically be executed from the command line with:
```
../x64/Release/render_engine.exe [options]
```
(or the "Debug" equivalent.) If using the precompiled release, the program should be directly executable as `./render_engine.exe`.

The following options are supported (default values can be viewed or changed by modifying `main.cpp`):
- `--scene` (str) path to the scene file to open. `gltf` (not `glb`) is recommended
- `--shadows` (str) force the application to use a specific type of shadow map; one of the following choices:
    - `none`
    - `basic`
    - `pcf`
    - `filteredpcf`
    - `pcss`
    - `raymarch`
    Alternatively, you can suffix the sun lamp names (e.g. `"sun_basic"`) in the scene graph
- `--changerad` (flag only) animate the radius of each sun lamp

## Results

![Sample shadow images](docs/sample_images.png)

See a video overview [here](https://drive.google.com/file/d/1fC4ZWOZHaT7eM9xLLNy3sxiPGZ83Andv/view?usp=sharing)

## Code

This code is based on [a previous project of mine](https://github.com/thetruejard/cs348k_project). See that project's README for more general information.

The shadow mapping code is generally in the following files:
- `main.cpp` (general scene/lights definition)
- `graphics/pipeline/rp_forward_opengl.cpp` (OpenGL scene rendering, shadow map passes, lighting/materials config)
- `shaders/opengl/forward.vert` (vertex shader for most rendering)
- `shaders/opengl/forward.frag` (fragment shader for lighting and **all shadow methods**)
