import sinen
texture = sinen.Texture()
draw2d = sinen.Draw2D(texture)
font = sinen.Font()
font.load("mplus/mplus-1p-medium.ttf", 96)

def update():
  font.render_text(texture, "Hello Sinen World!", sinen.Color(1, 1, 1, 1))
  draw2d.scale = texture.size()

def draw():
  draw2d.draw()
