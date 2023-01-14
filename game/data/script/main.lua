require("global")
local texture_title = {}
local texture_press = {}
local font_title = {}
local font_press = {}
local drawer_title = {}
local drawer_press = {}
local menu = require("gui/menu")
local menu_object = menu()
function Setup()
    SCORE = 0
    NOW_STAGE = 1
    menu_object:setup()
    texture_title = texture()
    drawer_title = draw2d(texture_title)
    font_title = font()
    font_title:load("x16y32pxGridGazer.ttf", 64)
    font_title:render_text(texture_title, "DIVE TO SINEN", color(1, 1, 1, 1))
    drawer_title.scale = texture_title:size()

    texture_press = texture()
    drawer_press = draw2d(texture_press)
    font_press = font()
    font_press:load("x16y32pxGridGazer.ttf", 64)
    font_press:render_text(texture_press, "PRESS ENTER", color(1, 1, 1, 1))
    drawer_press.scale = texture_press:size()
    drawer_press.position = vector2(0, -drawer_title.scale.y * 1.5)
    local skybox_tex = texture()
    skybox_tex:fill_color(color(0.2, 0.2, 0.2, 1))
    set_skybox_texture(skybox_tex)
end

function Update()
    drawer_title:draw()
    drawer_press:draw()
    menu_object:update()
    menu_object:draw()
    if menu_object.hide then
        if keyboard:key_state(keyENTER) == buttonPRESSED then
            change_scene("scene01_stage")
        end
    end
end
