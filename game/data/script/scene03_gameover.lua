local texture_dead = {}
local font_dead = {}
local drawer_dead = {}

function setup()
    texture_dead = texture()
    drawer_dead = draw2d_instanced(texture_dead)
    font_dead = font()
    font_dead:load("x16y32pxGridGazer.ttf", 64)
    font_dead:render_text(texture_dead, "You Dead", color(1, 1, 1, 1))
    drawer_dead:add(vector2(0, 0), 0, texture_dead:size())
end

function update()
    drawer_dead:draw()
    if keyboard:key_state(keySPACE) == buttonPRESSED then
        change_scene("main")
    end
end
