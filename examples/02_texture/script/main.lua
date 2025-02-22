local hello_texture = {}
local hello_font = {}
local hello_drawer = {}

-- Create a texture
hello_texture = texture()
hello_texture:load("logo.png")
-- Create a draw2D
hello_drawer = draw2d(hello_texture)

-- Set scale to texture size
hello_drawer.scale = hello_texture:size()

function Update()
end

function Draw()
  -- Draw texture
  hello_drawer:draw()
end
