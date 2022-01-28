# sinen
  sinen is a media library for C++20 that is being produced mainly for game development.  
The library is planned to be used only by me, so I don't think it will have full support and documentation.

## Supported platforms
[![Windows(MinGW)](https://github.com/astomih/sinen/actions/workflows/mingw.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/mingw.yml)
[![Linux](https://github.com/astomih/sinen/actions/workflows/linux.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/linux.yml)
[![Emscripten(Web)](https://github.com/astomih/sinen/actions/workflows/emscripten.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/emscripten.yml)
[![Android](https://github.com/astomih/sinen/actions/workflows/android.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/android.yml)

## Documents
[![doxygen](https://github.com/astomih/sinen/actions/workflows/doxygen.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/doxygen.yml)
[API references](https://astomih.github.io/sinen)  
No tutorials, etc. have been created yet.

## Hello world in sinen
``` c++
#include <Nen/Nen.hpp>

class app : public nen::base_scene {
public:
  app() = default;
  ~app() = default;
  void Setup() override {
    // Set up background color to black.
    GetRenderer()->SetClearColor(nen::palette::Black);

    // Load fonts.
    auto font = std::make_shared<nen::font>();
    font->LoadFromFile("mplus/mplus-1p-medium.ttf", 72);

    // Generate actor and add to scene
    auto actor = this->AddActor<nen::base_actor>();

    // Generate component for actor
    auto text = actor->AddComponent<nen::text_component>();
    text->SetFont(font);
    text->SetString("Hello,World!", nen::palette::White);
    // Register to Renderer
    text->Register();
  }

  void Update(float deltaTime) override {
    // Quit to Q key.
    if (GetInput().Keyboard.GetKeyValue(nen::key_code::Q))
      Quit();
  }
};

int main(int argc, char **argv) {
  nen::Launch(std::make_shared<app>());
  return 0;
}
```
![result](https://github.com/Astomih/sinen/blob/main/example/result.png "result")

## How to build
### Windows
Install msys, CMake and Vulkan. then install 'Using libraries' in pacman.  
Finally, build with CMake.
### WebGL
Install emscripten SDK. and do this.  
 ``` 
 $ emcmake cmake .
 $ emmake make
 ```
 ### Android
 Build with gradle.

## Using libraries
- GLEW
- ImGui
- Lua
- Sol2
- OpenAL 1.1
- rapidJSON
- OpenGL 3.3
- SDL2(image,mixer,net,ttf)
- Vulkan 1.2
- VulkanMemoryAllocator
  
## Lisence
 MIT Lisence