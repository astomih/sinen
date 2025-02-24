local texture = {}
local font = {}
local draw2d = {}

local hello_sound = Sound()
hello_sound:load("shot.wav")

-- Create a texture
texture = Texture()

font = Font()
font:load("mplus/mplus-1p-medium.ttf", 16)
font:render_text(texture, "Press SPACE key to play sound", Color(1, 1, 1, 1))

-- Create a draw2D
draw2d = Draw2D(texture)

-- Set scale to texture size
draw2d.scale = texture:size()

function Update()
  if keyboard.is_pressed(keyboard.SPACE) then
    hello_sound:play()
  end
end

function Draw()
  -- Draw texture
  draw2d:draw()
end
