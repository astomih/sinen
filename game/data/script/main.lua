require("global")
local texture_title = {}
local texture_press = {}
local font_title = {}
local font_press = {}
local drawer_title = {}
local drawer_press = {}
local menu = require("gui/menu")
local menu_object = menu()
local scene_switcher = require("scene_switcher")()
local button = require("gui/button")()

scene.resize(vector2(1280, 720))
window.rename("DIVE TO SINEN")
SCORE = 0
NOW_STAGE = 1
menu_object:setup()
texture_title = texture()
drawer_title = drawui(texture_title)
font_title = font()
font_title:load(DEFAULT_FONT_NAME, 64)
font_title:render_text(texture_title, "DIVE TO SINEN", color(1, 1, 1, 0.9))
drawer_title.scale = texture_title:size()

texture_press = texture()
drawer_press = drawui(texture_press)
font_press = font()
font_press:load(DEFAULT_FONT_NAME, 32)
font_press:render_text(texture_press, "CLICK TO START", color(1, 1, 1, 0.9))
drawer_press.scale = texture_press:size()
drawer_press.position = vector2(0, -drawer_title.scale.y * 3.0)
local skybox_tex = texture()
skybox_tex:fill_color(color(0.2, 0.2, 0.2, 1))
set_skybox_texture(skybox_tex)
scene_switcher:setup()
scene_switcher:start("")
renderer.at_render_texture_user_data(0, 0.0)


local draw = function()
end


function update()
    if scene_switcher.flag then
        scene_switcher:update(draw)
        return
    end

    menu_object:update()
    font_press:render_text(texture_press, "CLICK TO START", color(1, 1, 1, dts.sin_0_1(2.0)))
    draw()
    if menu_object.hide then
        if mouse.is_pressed(mouse.LEFT) then
            scene_switcher:start("scene00_base")
        end
    end
end

draw = function()
    drawer_title:draw()
    drawer_press:draw()
    menu_object:draw()

    GUI_MANAGER:update()
end
