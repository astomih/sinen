# Sinen

<p align="center"><a href="https://astomih.github.io/sinen"><img src="https://raw.githubusercontent.com/astomih/sinen/main/docs/logo/logo_bg_white.png" width="480" alt="Sinen Logo"></a></p>  

# Documents

[![Documents](https://github.com/astomih/sinen/actions/workflows/documents.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/documents.yml)  
[https://astomih.github.io/sinen](https://astomih.github.io/sinen)

# Hello World in Sinen

``` lua
-- main.lua
function update()
end

function draw()
    sn.Graphics.drawText("Hello World!", sn.Vec2.new(0, 0), sn.Color.new(1.0), 32)
end
```

![Hello World](https://raw.githubusercontent.com/astomih/sinen/refs/heads/main/examples/screenshot/01.png)

## How to run or asset place
See examples directory.

# How to build from source

## Requirements

- CMake 3.20 or later
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

Run "build/generate_vs2022.bat" and open&build the generated solution file(build/desktop/.).

### Android
Run "build/generate_android.bat", "build_android.bat" in build directory.  
Then open the build/android directory in Android Studio, and build&run.

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