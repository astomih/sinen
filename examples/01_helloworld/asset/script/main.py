import sinen as sn


def update():
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE):
        sn.Script.load("main", ".")


def draw():
    sn.Graphics.draw_text("Hello World!", sn.Vec2(0), sn.Color(1), 32)
