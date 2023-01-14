local texture_clear = {}
local font_clear = {}
local drawer_clear = {}

function Setup()
    texture_clear = texture()
    drawer_clear = draw2d_instanced(texture_clear)
    font_clear = font()
    font_clear:load("x16y32pxGridGazer.ttf", 64)
    font_clear:render_text(texture_clear, "You Win", color(1, 1, 1, 1))
    drawer_clear:add(vector2(0, 0), 0, texture_clear:size())
end

function Update()
    drawer_clear:draw()
    if keyboard:key_state(keyENTER) == buttonPRESSED then
        change_scene("main")
    end
end
