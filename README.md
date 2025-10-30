# Prism Engine
![PrismEditor](/resources/PrismEditorScreenshot.png?raw=true "PrismEditor")
## About
Prism Engine is a custom game engine written in C++ with a custom rendering API layer, currently supporting only DirectX 12.
## Getting Started
#### 1. Install [vcpkg](https://vcpkg.io/) if you don't have it (if you are using Visual Studio you can skip this as it should ask you later to install it for you)
#### 2. Clone the repository
```
git clone https://github.com/Niyoofficial/PrismEngine
```
#### Here you can either continue with the manual setup or use e.g. Visual Studio in CMake mode which will do it for you
#### 3. Run this to generate the project:
```
cmake -B build -S .
```
If this fails it's probably because it can't find your vcpkg toolchain, in that case specify it manually
```
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
```
#### 4. And now to actually compile the project
```
cmake --build build
```
## Features
|Feature                   |Description
|-                         |-
|**Scene System**          |Entity-Component based scene system.
|**Rendering API**         |A low-level rendering API that simplifies the rendering process, similar to DX11 with OpenGL-style resource binding with automatic PSO caching.
|**Editor and UI**         |Basic editor allowing for scene editing implemented using ImGui and ImGuizmo.
|**Shader Hot-Reloading**  |Runtime reloading of shader changes, allowing to instantly see the visual changes.
|**Deferred PBR Renderer** |Render pipeline with PBR, SH-based IBL, and Bloom.
|**Shadows**               |Shadows implemented via shadow maps with PCF filter.
## Features to come
|Feature                   |Description
|-                         |-
|**Emmisive PBR Model**    |Support for physically accurate emmisive materials.
|**3D Physics**            |Physics engine using Jolt Physics.
|**C# Scripting**          |Ability to attach C# scripts to entities.
|**Audio**                 |I don't want to, but I will have to at some point.
|**Asset Manager**         |Asset manager with asset browser.
|**Prefabs**               |Prefabs.
|**CSM**                   |Cascaded shadow maps.
|**Deferred Voxel Shading**|Real-time global illumination using voxel cone tracing.
