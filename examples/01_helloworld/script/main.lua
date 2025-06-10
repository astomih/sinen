-- Create a texture
local texture = Texture()
-- Create a draw2D
local draw2d = Draw2D()
draw2d.material:AppendTexture(texture)
-- Create a font
local font = Font()
-- Load a default font (96px)
font:Load(96)
-- If you want to use a custom font, you can load it like this:
-- font:LoadFromFile("path/to/your/font.ttf", 96)

function Update()
    -- Render text to texture
    font:RenderText(texture, "Hello World!", Color(1, 1, 1, 1))
    -- Set scale to texture size
    draw2d.scale = texture:Size()
end

function Draw()
    -- Draw texture
    draw2d:Draw()
end
