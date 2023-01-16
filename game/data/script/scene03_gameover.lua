local texture_dead = {}
local font_dead = {}
local drawer_dead = {}
local scene_switcher = require("scene_switcher")()

function Setup()
    texture_dead = texture()
    drawer_dead = draw2d_instanced(texture_dead)
    font_dead = font()
    font_dead:load("x16y32pxGridGazer.ttf", 64)
    font_dead:render_text(texture_dead, "You Dead", color(1, 0.25, 0.25, 1))
    drawer_dead:add(vector2(0, 0), 0, texture_dead:size())
    scene_switcher:setup()
    scene_switcher:start(true, "")
end

local function draw()
    drawer_dead:draw()
end

function Update()
    if scene_switcher.flag then
        scene_switcher:update(draw)
        return
    end
    draw()
    if keyboard:is_key_pressed(keyENTER) then
        scene_switcher:start(false, "main")
    end
end
