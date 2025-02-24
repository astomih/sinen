local hello_texture = {}
local hello_font = {}
local hello_drawer = {}

local hello_sound = sound()
hello_sound:load("shot.wav")

-- Create a texture
hello_texture = texture()

hello_font = font()
hello_font:load("mplus/mplus-1p-medium.ttf", 16)
hello_font:render_text(hello_texture, "Press SPACE key to play sound", color(1, 1, 1, 1))

-- Create a draw2D
hello_drawer = draw2d(hello_texture)

-- Set scale to texture size
hello_drawer.scale = hello_texture:size()

function Update()
  if keyboard.is_pressed(keyboard.SPACE) then
    hello_sound:play()
  end
end

function Draw()
  -- Draw texture
  hello_drawer:draw()
end
