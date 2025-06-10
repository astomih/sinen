# Sinen

<p align="center"><a href="https://astomih.github.io/sinen"><img src="https://raw.githubusercontent.com/astomih/sinen/main/docs/logo/logo_bg_white.png" width="480" alt="Sinen Logo"></a></p>  

# Documents

[![Documents](https://github.com/astomih/sinen/actions/workflows/documents.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/documents.yml)  
[https://astomih.github.io/sinen](https://astomih.github.io/sinen)


# Hello World in Sinen
## Lua
``` lua
-- Create a texture
local texture = Texture()
-- Create a draw2D
local draw2d = Draw2D()
draw2d.material:AppendTexture(texture)
-- Create a font
local font = Font()
-- Load a default font (96px)
font:Load(96)

function Update()
    -- Render text to texture
    font:RenderText(texture, "Hello World!", Color(1, 1, 1, 1))
    -- Set scale to texture size
    draw2d.scale = texture:Size()
end

function Draw()
    -- Draw texture
    draw2d:Draw()
end

![Hello World](https://raw.githubusercontent.com/astomih/sinen/refs/heads/main/examples/screenshot/01.png)

# How to build from source

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

# Lisence
MIT Lisence

# Community
[Discord](https://discord.gg/7U7VDwWjfy)