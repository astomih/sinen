import sinen as sn
texture = sn.Texture()
draw2d = sn.Draw2D(texture)
font = sn.Font()
font.load("mplus/mplus-1p-medium.ttf", 96)

def update():
  font.render_text(texture, "Hello Sinen World!", sn.Color(1, 1, 1, 1))
  draw2d.scale = texture.size()

def draw():
  draw2d.draw()
