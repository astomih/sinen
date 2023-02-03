-- Prepare variables
local hello_texture = {}
local hello_font = {}
local hello_drawer = {}

-- Create a texture
hello_texture = texture()
-- Create a draw2D
hello_drawer = draw2d(hello_texture)
-- Create a font
hello_font = font()
-- Load a font from file(96px)
hello_font:load("mplus/mplus-1p-medium.ttf", 96)
-- Render text to texture
hello_font:render_text(hello_texture, "Hello Sinen World!",
    color(1, 1, 1, 1))
-- Set scale to texture size
hello_drawer.scale = hello_texture:size()

function update()
    -- Draw
    hello_drawer:draw()
end
