# Tutorial
## Introduction
### What is Sinen?
Sinen is a library for game development.  
The goal is to be able to write in Lua intuitively.  
I have prepared a web version [here](https://astomih.github.io/sinen/web_demo) for you to try it out.  
### Target Platforms
- Windows
- Linux
## Hello World in Sinen
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
