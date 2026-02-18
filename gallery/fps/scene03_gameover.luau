local scene_switcher = require("scene_switcher").new()
scene_switcher:start("")
sn.Mouse.hideCursor(false)
sn.Mouse.setRelative(false)

function update()
    GUI_MANAGER:update()
    if scene_switcher.flag then
        scene_switcher:update()
        return
    end
    if sn.Mouse.isPressed(sn.Mouse.LEFT) then
        scene_switcher:start("main")
    end
end

function draw()
    sn.Graphics.drawText("GAME OVER", FONT_DEFAULT64, sn.Vec2.new(0), sn.Color.new(1, 0.25, 0.25, 1), 64)
    scene_switcher:draw()
    GUI_MANAGER:draw()
end
