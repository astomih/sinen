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
local texture = {}
local font = {}
local draw2d = {}

-- Create a texture
texture = Texture()
-- Create a draw2D
draw2d = Draw2D(texture)
-- Create a font
font = Font()
-- Load a font from file(96px)
font:load("mplus/mplus-1p-medium.ttf", 96)

function Update()
  -- Render text to texture with blinking
  font:render_text(texture, "Hello Sinen World!",
    Color(1, 1, 1, periodic.sin0_1(1.0, time.seconds())))
  -- Set scale to texture size
  draw2d.scale = texture:size()
end

function Draw()
  -- Draw texture
  draw2d:draw()
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
- ImGui
- SDL3(image,mixer,ttf)
- Paranoixa
## Sinen Editor
- ImGuiColorTextEdit
- ImGuizmo
- imgui-filebrowser
- imgui_markdown
- Sinen Core

# How to build
## Requirements
- CMake 3.20 or later
- Ninja
- C++23 compiler
- Vulkan SDK
- [yasm](https://github.com/yasm/yasm)(for SDL3_Image )
## Build commands
``` bash
git clone https://github.com/astomih/sinen.git
cd sinen
git submodule update --init --recursive
```
### Windows
#### Visual Studio 2022
Run "build/generate_vs2022.bat" and open&build the generated solution file(build/vs2022/.).
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
