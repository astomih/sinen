local texture_dead = {}
local font_dead = {}
local drawer_dead = {}
local scene_switcher = require("scene_switcher")()

texture_dead = Texture()
drawer_dead = Draw2D(texture_dead)
font_dead = Font()
font_dead:load(DEFAULT_FONT_NAME, 64)
font_dead:render_text(texture_dead, "YOU DIED", Color(1, 0.25, 0.25, 1))
drawer_dead.scale = texture_dead:size()
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
        scene_switcher:start("main")
    end
end

function Draw()
    drawer_dead:draw()
    scene_switcher:draw()
    GUI_MANAGER:draw()
end
