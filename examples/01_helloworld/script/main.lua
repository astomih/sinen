local texture = {}
local font = {}
local draw2d = {}

-- Create a texture
texture = Texture()
-- Create a draw2D
draw2d = Draw2D(texture)
-- Create a font
font = Font()
-- Load a font from file(96px)
font:load("mplus/mplus-1p-medium.ttf", 96)

function Update()
  -- Render text to texture with blinking
  font:render_text(texture, "Hello Sinen World!",
    Color(1, 1, 1, periodic.sin0_1(1.0, time.seconds())))
  -- Set scale to texture size
  draw2d.scale = texture:size()
end

function Draw()
  -- Draw texture
  draw2d:draw()
end
