function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function Draw()
    sn.Graphics.drawText("Hello World!", sn.Vec2.new(0, 0), sn.Color.new(1.0), 32)
end
