import sinen as sn

texture = sn.Texture()
font = sn.Font()
font.load(16)
font.render_text(texture, "Press SPACE key to play sound", sn.Color(1, 1, 1, 1))
draw2d = sn.Draw2D(texture)
draw2d.scale = texture.size()
sound = sn.Sound()
sound.load("shot.wav")


# Set scale to texture size


def update():
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE):
        sn.Script.load("main", ".")
    if sn.Keyboard.is_pressed(sn.Keyboard.SPACE):
        sound.play()


def draw():
    # Draw texture
    sn.Graphics.draw2d(draw2d)
