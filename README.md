# TODO: TITLE

Jared Watrous (jwat2002@stanford.edu)

# Summary

I am going to implement a real-time physically-based 3D render pipeline with emphasis on support for a very large number of light sources. I will focus primarily on the render pipeline itself (e.g. going from geometry and light data to pixels on the screen), and will *not* worry about performance in other parts of the application (e.g. asset loading, event processing, etc.) I will start with the code base in this repository, which is taken from an old project of mine written in C++ and OpenGL that already handles scene graphs and basic asset loading.

## Background

In real-time rendering applications, often the biggest bottleneck in shaders is lighting computations. A naive implementation could iterate over every light source in the pixel shader, but this grows linearly with the number of light sources in the scene, and when the number of light sources exceeds the memory accessible to the GPU, lights may need to be rendered in multiple passes. More sophisticated approaches attempt to mitigate this by removing redundancy and unnecessary lighting computations, such as when light sources are negigible at a pixel due to suffiicent attenuation, or when pixels will be occluded by other geometry later in the render pipeline. This project will target one of two approaches, depending on feasibility determined during implementation:

- Clustered Forward+ Rendering: Based on the original [Forward+](https://takahiroharada.wordpress.com/wp-content/uploads/2015/04/forward_plus.pdf) method and [clustered](https://www.cse.chalmers.se/~uffe/clustered_shading_preprint.pdf) methods, Clusterd Forward+ Rendering subdivides the scene into 3D screen-space tiles ("screen-space" meaning they are aligned to the camera's frustum), and only computes a light source in tiles which the light source significantly influences. To my understanding, methods in this family are typically preferred in modern rendering pipelines,

- [Deferred Rendering](https://www.researchgate.net/profile/Jonathan_Thaler2/publication/323357208_Deferred_Rendering/links/5a8fce31aca272140560aaad/Deferred-Rendering.pdf): Deferred rendering processes occlusions first by rendering the entire scene to a "G-buffer", containing colors/normals/etc. for each pixel, then renders all lights afterwards. Although very effective, this method has high memory overhead, can make antialiasing difficult, and typically has limited (if any) support for transparent objects in the deferred pass.

**In this project,** I will prioritize Clustered Forward+ Rendering, but due to its (geometric) complexity, I *may* fall back to Deferred Rendering if I deem it necessary.

**Extension:** Note that shadows are a separate feature that is somewhat detached from the pipeline architecture defined above. I will work on shadow rendering if I have time, but it is not a priority.
