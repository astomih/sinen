-- Create a texture
local texture = Texture()
texture:Load("logo.png")
-- Create a draw2D
local draw2d = Draw2D()
draw2d.material:AppendTexture(texture)
-- Set scale to texture size
draw2d.scale = texture:Size()

function Update()
end

function Draw()
    -- Draw texture
    draw2d:Draw()
end
