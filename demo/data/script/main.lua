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
