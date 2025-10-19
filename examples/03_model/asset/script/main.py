import sinen as sn

model = sn.Model()
model.load("DamagedHelmet.glb")
material = model.get_material()

transform = sn.Transform()
transform.position = sn.Vec3(0, 0, 0)
transform.rotation = sn.Vec3(90, 0, 0)
transform.scale = sn.Vec3(1, 1, 1)

pos = sn.Vec3(1, 1, 3)
at = sn.Vec3(0)
up = sn.Vec3(0, 1, 0)
sn.Graphics.get_camera().lookat(pos, at, up)
sn.Graphics.bind_pipeline(sn.BuiltinPipelines.get_3d())


def update():
    sn.Logger.info("hello")
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE):
        sn.Script.load("main", ".")


def draw():
    sn.Graphics.draw_model(model, transform, material)
