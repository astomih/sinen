local texture_clear = {}
local font_clear = {}
local drawer_clear = {}
local scene_switcher = require("scene_switcher")()
NOW_STAGE = 1
texture_clear = Texture()
drawer_clear = Draw2D(texture_clear)
font_clear = Font()
font_clear:load(DEFAULT_FONT_NAME, 64)
font_clear:render_text(texture_clear, "STAGE CLEAR", Color(1, 1, 1, 1))
drawer_clear.scale = texture_clear:size()
scene_switcher:setup()
scene_switcher:start("")

function Update()
    GUI_MANAGER:update()
    mouse.hide_cursor(false)
    if scene_switcher.flag then
        scene_switcher:update()
        return
    end
    if mouse.is_pressed(mouse.LEFT) then
        scene_switcher:start("scene00_base")
    end
end

function Draw()
    drawer_clear:draw()
    scene_switcher:draw()
    GUI_MANAGER:draw()
end
