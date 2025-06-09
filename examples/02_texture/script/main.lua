-- Create a texture
local texture = Texture()
texture:load("logo.png")
-- Create a draw2D
local draw2d = Draw2D()
draw2d.material:append(texture)
-- Set scale to texture size
draw2d.scale = texture:size()

function update()
end

function draw()
    -- Draw texture
    draw2d:draw()
end
