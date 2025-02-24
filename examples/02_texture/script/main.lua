local texture = {}
local draw2d = {}

-- Create a texture
texture = Texture()
texture:load("logo.png")
-- Create a draw2D
draw2d = Draw2D(texture)

-- Set scale to texture size
draw2d.scale = texture:size()

function Update()
end

function Draw()
  -- Draw texture
  draw2d:draw()
end
