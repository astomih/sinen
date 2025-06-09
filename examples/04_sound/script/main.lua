local texture = Texture()
local font = Font()
local draw2d = Draw2D()
local hello_sound = Sound()
hello_sound:Load("shot.wav")

-- Create a texture
font:Load(16)
font:RenderText(texture, "Press SPACE key to play sound", Color(1, 1, 1, 1))

draw2d.material:AppendTexture(texture)

-- Set scale to texture size
draw2d.scale = texture:Size()

function Update()
    if Keyboard.IsPressed(Keyboard.SPACE) then
        hello_sound:Play()
    end
end

function Draw()
    -- Draw texture
    draw2d:Draw()
end
