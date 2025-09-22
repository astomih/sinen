function Update()
    if sn.Keyboard.IsPressed(sn.Keyboard.ESCAPE) then
        sn.Script.Load("main", ".")
    end
end

function Draw()
    sn.Graphics.DrawText("Hello World!", sn.Vec2(0, 0), sn.Color(1, 1, 1, 1), 32)
end
