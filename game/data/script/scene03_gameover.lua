local text_window = require "text_window"
local text_window_object = text_window()
local menu = require("menu")
local menu_object = menu()
local hello_texture = {}
local hello_font = {}
local hello_drawer = {}

function setup()
    local skybox_tex = texture()
    skybox_tex:fill_color(color(0, 0, 0, 1))
    set_skybox_texture(skybox_tex)
    hello_texture = texture()
    hello_drawer = draw2d_instanced(hello_texture)
    hello_font = font()
    hello_font:load(DEFAULT_FONT, 64)
    hello_font:render_text(hello_texture, "You Dead", color(1, 1, 1, 1))
    hello_drawer:add(vector2(0, 0), 0, hello_texture:size())
    menu_object:setup()
    text_window_object:setup()

    text_window_object.texts = {
        "夢の中で、"
    }
end

function update()
    menu_object:update()
    if menu_object.hide then
        if (keyboard:key_state(keySPACE) == buttonPRESSED or
            mouse:button_state(mouseLEFT) == buttonPRESSED) and
            text_window_object.is_draw_all_texts then
            change_scene("main")
        end
        text_window_object:update()
    end
    text_window_object:draw()
    hello_drawer:draw()
    menu_object:draw()
end
