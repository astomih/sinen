import sinen as sn

from gui.button import Button, gm

button = Button()

example_dirs = sn.FileSystem.enumerate_directory(".")
num_examples = 9
offset = num_examples * 16.0


def update():
    gm.update()
    for i in range(num_examples):
        if button.show(
                example_dirs[i], sn.Vec2(0, i * -32.0 + offset), sn.Vec2(300, 32)
        ):
            sn.Script.load("main", example_dirs[i])


def draw():
    sn.Graphics.bind_default_pipeline2d()
    sn.Graphics.bind_default_pipeline3d()
    sn.Graphics.draw_text(
        "Example Launcher", sn.Vec2(0, offset), sn.Color(1, 1, 1, 1), 24
    )
    gm.draw()
