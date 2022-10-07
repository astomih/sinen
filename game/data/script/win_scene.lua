local hello_texture = {}
local hello_font = {}
local hello_drawer = {}

function setup()
    hello_texture = texture()
    hello_drawer = draw2d_instanced(hello_texture)
    hello_font = font()
    hello_font:load(DEFAULT_FONT, 64)
    hello_font:render_text(hello_texture, "You Win!!!", color(1, 1, 1, 1))
    hello_drawer:add(vector2(0, 0), 0, hello_texture:size())
end

function update()
    hello_drawer:draw()
    if keyboard:key_state(keySPACE) == buttonPRESSED then
        change_scene("main")
    end
end

