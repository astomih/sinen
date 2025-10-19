import sinen as sn
import math

model = sn.Model()
model.load("DamagedHelmet.glb")
material = model.get_material()

transforms = []
for i in range(10):
    for j in range(10):
        t = sn.Transform()
        t.position = sn.Vec3(i * 2, 0, j * 2)
        transforms.append(t)


pos = sn.Vec3(-3, 5, -3)
at = sn.Vec3(15, -5, 15)
up = sn.Vec3(0, 1, 0)
sn.Graphics.get_camera().lookat(pos, at, up)

sn.Graphics.bind_pipeline(sn.BuiltinPipelines.get_3d())


def update():
    sn.Logger.info("hello")
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE):
        sn.Script.load("main", ".")


def draw():
    sn.Graphics.draw_model_instanced(model, transforms, material)
