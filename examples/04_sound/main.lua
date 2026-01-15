local sound = sn.Sound.new()
sound:load("shot.wav")

function update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    if sn.Keyboard.isPressed(sn.Keyboard.SPACE) then
        sound:play()
    end
end

function draw()
    sn.Graphics.drawText("Press SPACE key to play sound", FONT, sn.Vec2.new(0), sn.Color.new(1), 16)
end
