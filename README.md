[![MinGW](https://github.com/astomih/sinen/actions/workflows/mingw.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/mingw.yml)
[![Linux](https://github.com/astomih/sinen/actions/workflows/linux.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/linux.yml)
[![Emscripten](https://github.com/astomih/sinen/actions/workflows/emscripten.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/emscripten.yml)
[![Android](https://github.com/astomih/sinen/actions/workflows/android.yml/badge.svg)](https://github.com/astomih/sinen/actions/workflows/android.yml)
# sinen
  sinen is a media library for C++20 that is being produced mainly for game development.  
The library is planned to be used only by me, so I don't think it will have full support and documentation.

## Supported platforms
    Windows/Linux/WebGL/Android(experimental)

## Hello world in sinen
``` c++
#include <Nen/Nen.hpp>

/**
* Main class has already been defined.
*/
void Main::Setup()
{
    //Set up background color to black.
    GetRenderer()->SetClearColor(nen::palette::Black);

    //Load fonts.
    auto f = std::make_shared<nen::font>();
    f->LoadFromFile("Assets/Font/mplus/mplus-1p-medium.ttf", 72);

    //Generate actor and add to scene
    auto actor = this->AddActor<nen::base_actor>();

    //Generate component for actor 
    auto text = actor->AddComponent<nen::text_component>();
    text->SetFont(font);
    text->SetString("Hello,World!", nen::palette::White);
}

void Main::Update(float deltaTime)
{
    //Quit to Q key.
    if (GetInput().Keyboard.GetKeyValue(nen::key_code::Q))
        Quit();

    /**
     * The drawing process is done behind the scenes.
    */
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

## Documents
[APIリファレンス](https://astomih.github.io/sinen)  
No tutorials, etc. have been created yet.

## Using libraries
- GLEW
- ImGui
- Lua
- Sol2
- OpenAL 1.1
- rapidJSON
- SDL2
- SDL2_image
- SDL2_mixer
- SDL2_net
- SDL2_ttf
  
## Lisence
 MIT Lisence