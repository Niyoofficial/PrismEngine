# Prism Engine
![PrismEditor](/resources/PrismEditorScreenshot.png?raw=true "PrismEditor")
## About
Prism Engine is a custom game engine written in C++ with a custom rendering API layer, currently supporting only DirectX 12.
## Getting Started
#### 1. Install [vcpkg](https://vcpkg.io/) if you don't have it
#### 2. Make sure you have an environment variable VCPKG_ROOT set to the vcpkg main folder
#### 3. Clone the repository
```
git clone https://github.com/Niyoofficial/PrismEngine
```
#### Here you can either continue with the manual setup or use e.g. Visual Studio in CMake mode, which will do it for you
#### 4. Run this to generate the project:
```
cmake -B <output_folder>
```
#### 5. And now to actually compile the project
```
cmake --build <output_folder> --preset=Profile
```
There are 3 presets available: `Debug`, `Profile` and `Release` which you can use in the above function
#### 6. Go to the `Sandbox` folder and run the `RunSandbox-<config>.bat` - this will run the executable with the proper working directory
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
