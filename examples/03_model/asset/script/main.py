import sinen as sn

texture = sn.Texture()
texture.fill(sn.Color(1, 1, 1, 1))
material = sn.Material()
material.append_texture(texture)
model = sn.Model()
model.load("Suzanne.gltf")

transform = sn.Transform()
transform.position = sn.Vec3(0, 0, 0)
transform.scale = sn.Vec3(1, 1, 1)


pos = sn.Vec3(1, 1, 3)
at = sn.Vec3(0)
up = sn.Vec3(0, 1, 0)
sn.Graphics.get_camera().lookat(pos, at, up)


def update():
    sn.Logger.info("hello")
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE):
        sn.Script.load("main", ".")


def draw():
    sn.Graphics.draw_model(model, transform, material)
