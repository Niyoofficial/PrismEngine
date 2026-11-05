# Prism Engine

![PrismEditor](/resources/PrismEditorScreenshot.png?raw=true "PrismEditor")

## About

Prism Engine is a custom game engine written in C++ with a custom rendering API layer with support for DirectX 12 and
Vulkan.

## Getting Started

#### 1. Install [vcpkg](https://vcpkg.io/) if you don't have it

#### 2. Make sure you have an environment variable VCPKG_ROOT set to the vcpkg main folder

#### 3. Clone the repository

```
git clone https://github.com/Niyoofficial/PrismEngine
```

#### Here you can either continue with the manual setup or use e.g. Visual Studio in CMake mode, which will do it for you

#### 4. Configure the project

Choose the rendering backend you want to use:

| Configure preset           | Rendering backend |
|----------------------------|-------------------|
| `vcpkg-visual-2026-dx12`   | DirectX 12        |
| `vcpkg-visual-2026-vulkan` | Vulkan            |

Configure the project using one of the following commands:

```bash
cmake --preset vcpkg-visual-2026-dx12
```

or

```bash
cmake --preset vcpkg-visual-2026-vulkan
```

#### 5. Build the project

The following build presets are available:

| Configuration | DirectX 12          | Vulkan           |
|---------------|---------------------|------------------|
| Debug         | `Debug Directx12`   | `Debug Vulkan`   |
| Profile       | `Profile Directx12` | `Profile Vulkan` |
| Release       | `Release Directx12` | `Release Vulkan` |

Build the project using the desired preset, for example:

```bash
cmake --build --preset "Profile Directx12"
```

or

```bash
cmake --build --preset "Profile Vulkan"
```

#### 6. Go to the `Sandbox` folder and run the
`RunSandbox-<config>.bat` - this will run the executable with the proper working directory

## Additional requirements

#### Vulkan Requirements

The Vulkan backend requires a GPU and driver supporting Vulkan 1.3 and the following device extensions:

| Extension                        |
|----------------------------------|
| `VK_KHR_swapchain`               |
| `VK_EXT_mutable_descriptor_type` |
| `VK_GOOGLE_user_type`            |
| `VK_GOOGLE_hlsl_functionality1`  |

The Vulkan SDK must be installed and available to the build environment. The VK_LAYER_KHRONOS_validation validation
layer is also configured by the engine for Vulkan validation.

## Features

| Feature                   | Description                                                                                                                                     
|---------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------
| **Scene System**          | Entity-Component based scene system.                                                                                                            
| **Rendering API**         | A low-level rendering API that simplifies the rendering process, similar to DX11 with OpenGL-style resource binding with automatic PSO caching. 
| **Editor and UI**         | Basic editor allowing for scene editing implemented using ImGui and ImGuizmo.                                                                   
| **Shader Hot-Reloading**  | Runtime reloading of shader changes, allowing to instantly see the visual changes.                                                              
| **Deferred PBR Renderer** | Render pipeline with PBR, SH-based IBL, and Bloom.                                                                                              
| **Shadows**               | Shadows implemented via shadow maps with PCF filter.                                                                                            

## Features to come

| Feature                    | Description                                             
|----------------------------|---------------------------------------------------------
| **Emmisive PBR Model**     | Support for physically accurate emmisive materials.     
| **3D Physics**             | Physics engine using Jolt Physics.                      
| **C# Scripting**           | Ability to attach C# scripts to entities.               
| **Audio**                  | I don't want to, but I will have to at some point.      
| **Asset Manager**          | Asset manager with asset browser.                       
| **Prefabs**                | Prefabs.                                                
| **CSM**                    | Cascaded shadow maps.                                   
| **Deferred Voxel Shading** | Real-time global illumination using voxel cone tracing. 
