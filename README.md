# vkTracer
A GPU-based ray tracer inspired by Ray Tracing in One Weekend, implemented with modern C++ and Vulkan compute shaders.
Ray generation, intersection tests, material scattering, and pixel sampling are performed in a Vulkan compute shader, with the resulting image presented through a fullscreen triangle.

![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20MacOS-blue)
![Graphics](https://img.shields.io/badge/API-Vulkan-ac162c)
![License](https://img.shields.io/badge/License-BSD%203--Clause-green)
![Screenshot](assets/demo.gif)

## Features
- GPU ray tracing using Vulkan compute shaders
- Sphere intersection and scene representation
- Lambertian, metallic and dielectric materials
- Multiple ray bounces
- Sampling and anti-aliasing
- Gamma correction
- Interactive camera
- GPU-side random number generation
- Storage-image based rendering
- Vulkan-Hpp RAII API
- Slang shaders
## License
This project is licensed under the BSD 3-Clause License. See the LICENSE file for details.