function update()
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function draw()
    sn.Graphics.draw_text("Hello World!", sn.Vec2(0, 0), sn.Color(1, 1, 1, 1), 32)
end
