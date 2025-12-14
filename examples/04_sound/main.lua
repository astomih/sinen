local sound = sn.Sound.new()
sound:load("shot.wav")

-- Set scale to texture size

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    if sn.Keyboard.isPressed(sn.Keyboard.SPACE) then
        sound:play()
    end
end

function Draw()
    sn.Graphics.drawText("Press SPACE key to play sound", FONT, sn.Vec2.new(0), sn.Color.new(1), 16)
end
