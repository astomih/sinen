# Sinen

<p align="center"><a href="https://astomih.github.io/sinen"><img src="https://raw.githubusercontent.com/astomih/sinen/main/docs/logo/logo_bg_white.png" width="480" alt="Sinen Logo"></a></p>  

# Summary

Sinen is a toolkit for creative coding.

# Feature

- Intuitive writing in the Python
- Sinen is Written in C++
- Using Vulkan API
- Lightweight

# Documents

[![Documents](https://github.com/astomih/sinen/actions/workflows/documents.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/documents.yml)  
[https://astomih.github.io/sinen](https://astomih.github.io/sinen)

# Hello world in Sinen as a library

``` python
from sinen import *
texture = Texture()
draw2d = Draw2D(texture)
font = Font()
font.load("mplus/mplus-1p-medium.ttf", 96)

def update():
  font.render_text(texture, "Hello Sinen World!", Color(1, 1, 1, 1))
  draw2d.scale = texture.size()

def draw():
  draw2d.draw()
```

![Hello World](https://raw.githubusercontent.com/astomih/sinen/refs/heads/main/examples/screenshot/01.png)

# Lisence

MIT Lisence

# Dependencies

## Sinen Core

- Dear ImGui
- mojoAL
- rapidjson
- ImGui
- SDL3(image,mixer,ttf)
- Paranoixa

## Script

- PocketPy

# How to build

## Requirements

- CMake 3.20 or later
- Ninja
- C++23 compiler
- Vulkan SDK(In MacOS, MoltenVK)
- [yasm](https://github.com/yasm/yasm)(for SDL3_Image , add to PATH)

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

``` bash
cmake -B build/macos -DEXTERNAL_ALL_BUILD=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/macos
```

### Linux

``` bash
cmake -B build/linux -DEXTERNAL_ALL_BUILD=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux
```