# Sinen Project 
Engine, Editor, Game, Tools, and more.
<p align="center"><a href="https://astomih.github.io/sinen"><img src="https://raw.githubusercontent.com/astomih/sinen/main/logo/logo_bg_white.png" width="480" alt="Sinen Logo"></a></p>  

# Engine
Sinen Engine is a media library for C++20 that is being produced mainly for game development.  

## Try on Web
[https://astomih.github.io/sinen/web_demo](https://astomih.github.io/sinen/web_demo/)

## Game demo
[https://astomih.github.io/game-demo/](https://astomih.github.io/game-demo/)

## Feature
- Lua game engine
- Written in C++20
- Using OpenGL/Vulkan APIs
- Cross platform
- Lightweight
- Incorporates a text editor

## Supported platforms
[![Windows(MinGW)](https://github.com/astomih/sinen/actions/workflows/mingw.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/mingw.yml)
[![Linux](https://github.com/astomih/sinen/actions/workflows/linux.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/linux.yml)
[![Emscripten(Web)](https://github.com/astomih/sinen/actions/workflows/emscripten.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/emscripten.yml)
[![Android](https://github.com/astomih/sinen/actions/workflows/android.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/android.yml)

## Documents
[![Documents](https://github.com/astomih/sinen/actions/workflows/documents.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/documents.yml)  
[View Documents](https://astomih.github.io/sinen)  

## Hello world in sinen
``` lua
-- Prepare variables
local hello_texture = {}
local hello_font = {}
local hello_drawer = {}

function Setup()
    -- Create a texture
    hello_texture = texture()
    -- Create a draw2D
    hello_drawer = draw2d(hello_texture)
    -- Create a font
    hello_font = font()
    -- Load a font from file(64px)
    hello_font:load("mplus/mplus-1p-medium.ttf", 64)
    -- Render text to texture
    hello_font:render_text(hello_texture, "Hello Sinen World!",
        color(1, 1, 1, 1))
    -- Set scale to texture size
    hello_drawer.scale = hello_texture:size()
end

function Update()
    -- Draw
    hello_drawer:draw()
end

```
## Lisence
 MIT Lisence

## Dependencies 
- GLEW
- Dear ImGui
- Lua
- Sol2
- mojoAL
- OpenGL 3.3
- rapidjson
- SDL2(image,mixer,net,ttf)
- Vulkan 1.2
- VulkanMemoryAllocator

# Editor
![Editor Sample](https://raw.githubusercontent.com/astomih/sinen/main/docs/image/editor_sample.png)
Editor is a level editor for Engine.  
## Dependencies
- Sinen Engine
- ImGuiColorTextEdit
- imgui_markdown
- ImGuizmo

# Game
I named this game 'DIVE TO SINEN'.  
![Game Sample](https://raw.githubusercontent.com/astomih/sinen/main/docs/image/game_sample.png)

# Tools
## Model Converter
Converts 3D model files to Sinen Engine's model format.