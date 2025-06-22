local texture = Texture()
local font = Font()
font:Load(16)
font:RenderText(texture, "Press SPACE key to play sound", Color(1, 1, 1, 1))
local draw2d = Draw2D(texture)
draw2d.scale = texture:Size()
local sound = Sound()
sound:Load("shot.wav")



-- Set scale to texture size

function Update()
    if Keyboard.IsPressed(Keyboard.SPACE) then
        sound:Play()
    end
end

function Draw()
    -- Draw texture
    draw2d:Draw()
end
