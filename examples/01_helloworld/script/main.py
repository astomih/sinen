import sinen as sn

texture = sn.Texture()
draw2d = sn.Draw2D(texture)
font = sn.Font()
font.load(96)

font.render_text(texture, "Hello Sinen World!", sn.Color(1, 1, 1, 1))
draw2d.scale = texture.size()


def update():
    pass


def draw():
    sn.Graphics.draw2d(draw2d)
