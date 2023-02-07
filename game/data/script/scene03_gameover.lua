local texture_dead = {}
local font_dead = {}
local drawer_dead = {}
local scene_switcher = require("scene_switcher")()

texture_dead = texture()
drawer_dead = drawui(texture_dead)
font_dead = font()
font_dead:load(DEFAULT_FONT_NAME, 64)
font_dead:render_text(texture_dead, "YOU DIED", color(1, 0.25, 0.25, 1))
drawer_dead.scale = texture_dead:size()
scene_switcher:setup()
scene_switcher:start("")

local function draw()
    drawer_dead:draw()
end

function update()
    if scene_switcher.flag then
        scene_switcher:update(draw)
        return
    end
    draw()
    if mouse.is_pressed(mouse.LEFT) then
        scene_switcher:start("main")
    end
end
