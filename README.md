# Sinen Project 
Engine, Editor, Game, Tools, and more.
<p align="center"><a href="https://astomih.github.io/sinen"><img src="https://raw.githubusercontent.com/astomih/sinen/main/logo/logo.png" width="480" alt="Sinen Engine logo"></a></p>  

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
local hello_texture = {}
local hello_font = {}
local hello_drawer = {}

function setup()
	hello_texture = texture()
	hello_drawer = draw2d(hello_texture)
	hello_drawer.scale = vector2(1, 1)
	hello_font = font()
	hello_font:load(DEFAULT_FONT, 128)
	hello_font:render_text(hello_texture, "Hello Sinen World!", color(1, 1, 1, 1))
end

function update() 
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