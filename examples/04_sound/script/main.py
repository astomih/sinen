from sinen import *
texture = Texture()
font = Font()
draw2d = Draw2D()
hello_sound = Sound()
hello_sound.load("shot.wav")

# Create a texture
font.load(16)
font.render_text(texture, "Press SPACE key to play sound", Color(1, 1, 1, 1))

# Create a draw2D
draw2d = Draw2D(texture)

# Set scale to texture size
draw2d.scale = texture.size()

def update():
  if Keyboard.is_pressed(Keyboard.SPACE):
    hello_sound.play()

def draw():
  # Draw texture
  draw2d.draw()
