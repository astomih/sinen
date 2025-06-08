-- Create a texture
local texture = Texture()
-- Create a draw2D
local draw2d = Draw2D()
draw2d.material:append(texture)
-- Create a font
local font = Font()
-- Load a font (96px)
font:load(96)

function update()
    -- Render text to texture
    font:render_text(texture, "Hello Sinen World!",
        Color(1, 1, 1, 1))
    -- Set scale to texture size
    draw2d.scale = texture:size()
end

function draw()
    -- Draw texture
    draw2d:draw()
end
