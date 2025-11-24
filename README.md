# OpenGL-Rasterizer

This project is a GPU implementation of my [Software-Rasterizer](https://github.com/EmilBjorkeng/Software-Rasterizer) using OpenGL. It loads OBJ files and renders them in a 3D environment.
Unlike the Software-Rasterizer, it uses the GLM math library for vectors and matrices instead of a custom implementation.

For more on the origins of this project, see [Software-Rasterizer](https://github.com/EmilBjorkeng/Software-Rasterizer).

## Key Features
- GPU-accelerated rendering with OpenGL
- OBJ and MTL file loader for 3D models and textures
- Modular shader pipeline for flexible rendering
- Lighting support

## Showcase
<img width="1047" height="855" alt="Screenshot 2025-11-24 002603" src="https://github.com/user-attachments/assets/297dcd9c-9dad-4284-a827-fd4751fe1663" />

## How to Run
Clone the repository and run make:
```
git clone https://github.com/EmilBjorkeng/OpenGL-Rasterizer.git
cd OpenGL-Rasterizer
make run
```
Dependencies: OpenGL, GLM, GLFW
