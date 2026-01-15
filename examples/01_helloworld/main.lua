local font = sn.Font.new()
font:load(32)
function update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function draw()
    sn.Graphics.drawText("Hello World!", font, sn.Vec2.new(0, 0), sn.Color.new(1.0), 32)
end
