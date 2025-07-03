-- Create a texture
local texture = sn.Texture()
texture:Load("logo.png")
-- Create a draw2D
local draw2d = sn.Draw2D(texture)
-- Set scale to texture size
draw2d.scale = texture:Size()

function Update()
end

function Draw()
    -- Draw texture
    sn.Graphics.Draw2D(draw2d)
end
