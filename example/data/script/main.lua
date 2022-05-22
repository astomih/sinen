local hello_texture = {}
local hello_font = {}
local hello_drawer = {}

local F3_texture = {}
local F3_drawer = {}
function setup()
    hello_texture = texture()
    F3_texture = texture()
    hello_drawer = draw2d(hello_texture)
    F3_drawer = draw2d(F3_texture)
    hello_font = font()
    hello_font:load(DEFAULT_FONT, 64)
    hello_font:render_text(hello_texture, "Hello Sinen World!",
                           color(1, 1, 1, 1))
    hello_font:render_text(F3_texture, "Press F3 to open the window",
                           color(1, 1, 1, 1))

    hello_drawer.scale = hello_texture:size()
    F3_drawer.scale = F3_texture:size()
    F3_drawer.position = vector2(0, -60)
end

function update()
    hello_drawer:draw()
    F3_drawer:draw()
end
