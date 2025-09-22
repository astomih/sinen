local texture = sn.Texture()
local font = sn.Font()
font:Load(16)
font:RenderText(texture, "Press SPACE key to play sound", sn.Color(1, 1, 1, 1))
local draw2d = sn.Draw2D(texture)
draw2d.scale = texture:Size()
local sound = sn.Sound()
sound:Load("shot.wav")



-- Set scale to texture size

function Update()
    if sn.Keyboard.IsPressed(sn.Keyboard.ESCAPE) then
        sn.Script.Load("main", ".")
    end
    if sn.Keyboard.IsPressed(sn.Keyboard.SPACE) then
        sound:Play()
    end
end

function Draw()
    -- Draw texture
    sn.Graphics.Draw2D(draw2d)
end
