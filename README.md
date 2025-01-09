# Sinen Engine 
<p align="center"><a href="https://astomih.github.io/sinen"><img src="https://raw.githubusercontent.com/astomih/sinen/main/docs/logo/logo_bg_white.png" width="480" alt="Sinen Logo"></a></p>  

# Summary
Sinen is a toolkit for creative coding.  

[![CI](https://github.com/astomih/sinen/actions/workflows/CI.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/CI.yml)  
# Feature
- Intuitive writing in the Lua programming language
- Sinen is Written in C++
- Using Vulkan API
- Lightweight

# Documents
[![Documents](https://github.com/astomih/sinen/actions/workflows/documents.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/documents.yml)  
[https://astomih.github.io/sinen](https://astomih.github.io/sinen)  

# Hello world in Sinen as a library
``` lua
local hello_texture = {}
local hello_font = {}
local hello_drawer = {}

-- Create a texture
hello_texture = texture()
-- Create a draw2D
hello_drawer = draw2d(hello_texture)
-- Create a font
hello_font = font()
-- Load a font from file(96px)
hello_font:load("mplus/mplus-1p-medium.ttf", 96)
-- Render text to texture
hello_font:render_text(hello_texture, "Hello Sinen World!",
    color(1, 1, 1, 1))
-- Set scale to texture size
hello_drawer.scale = hello_texture:size()

function update()
    -- Draw
    hello_drawer:draw()
end

```
![Hello World](https://raw.githubusercontent.com/astomih/sinen/main/docs/image/hello_world.png)
# Lisence
 MIT Lisence

# Dependencies 
## Sinen Core
- Dear ImGui
- Lua
- Sol2
- mojoAL
- rapidjson
- SDL2(image,mixer,ttf)
- Vulkan 1.1
- VulkanMemoryAllocator
## Sinen Editor
- ImGuiColorTextEdit
- ImGuizmo
- imgui-filebrowser
- imgui_markdown
- Sinen Core

# How to build
## Requirements
- CMake 3.7 or later
- Ninja
- C++20 compiler
- Vulkan SDK
## Build commands
``` bash
git clone https://github.com/astomih/sinen.git
cd sinen
git submodule update --init --recursive
```
### Windows
#### Visual Studio 2022
Run "build/generate_vs2022.bat" and open&build the generated solution file(build/msvc/.).
### MacOS
WIP

### Linux
``` bash
cmake -B build/linux -DEXTERNAL_ALL_BUILD=ON -DCMAKE_BUILD_TYPE=Release
cd build && ninja
```


# Editor(Experimental)
Editor is a tool for creating and editing Scene files.
![Editor Sample](https://raw.githubusercontent.com/astomih/sinen/main/docs/image/editor_sample.png)  
