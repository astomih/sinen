local texture_clear = {}
local font_clear = {}
local drawer_clear = {}

function setup()
    texture_clear = texture()
    drawer_clear = draw2d_instanced(texture_clear)
    font_clear = font()
    font_clear:load(DEFAULT_FONT, 64)
    font_clear:render_text(texture_clear, "You Win", color(1, 1, 1, 1))
    drawer_clear:add(vector2(0, 0), 0, texture_clear:size())
end

function update()
    drawer_clear:draw()
    if keyboard:key_state(keySPACE) == buttonPRESSED then
        change_scene("main")
    end
end
