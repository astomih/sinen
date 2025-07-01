-- Create a texture
local texture = sn.Texture()
-- Create a draw2D
local draw2d = sn.Draw2D(texture)
-- Create a font
local font = sn.Font()
-- Load a default font (96px)
font:Load(96)
-- If you want to use a custom font, you can load it like this:
-- font:LoadFromFile("path/to/your/font.ttf", 96)

function Update()
    -- Render text to texture
    font:RenderText(texture, "Hello World!", sn.Color(1, 1, 1, 1))
    -- Set scale to texture size
    draw2d.scale = texture:Size()
end

function Draw()
    -- Draw texture
    draw2d:Draw()
end
