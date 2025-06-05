from sinen import *
texture = Texture()
draw2d = Draw2D()
draw2d.material.append(texture)
font = Font()
font.load(96)

def update():
  font.render_text(texture, "Hello Sinen World!", Color(1, 1, 1, 1))
  draw2d.scale = texture.size()

def draw():
  draw2d.draw()
