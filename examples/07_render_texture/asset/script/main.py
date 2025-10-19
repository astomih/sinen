import sinen as sn

texture = sn.Texture()
texture.fill(sn.Color(1))
renderTexture = sn.RenderTexture()
out = sn.Texture()
out.load("logo.png")
sx = out.size().x
sy = out.size().y
renderTexture.create(int(sx), int(sy))

model = sn.Model()
model.load("Suzanne.gltf")
draw3d = sn.Draw3D(texture)
draw3d.position = sn.Vec3(0)
draw3d.model = model

pos = sn.Vec3(1, 1, 3)


def update():
    pass
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE):
        sn.Script.load("main", ".")
    sn.Graphics.get_camera().lookat(pos, sn.Vec3(0), sn.Vec3(0, 1, 0))


def draw():
    sn.Graphics.set_render_target(renderTexture)
    sn.Graphics.bind_pipeline(sn.BuiltinPipelines.get_3d())
    sn.Graphics.draw3d(draw3d)
    # Draw texture
    sn.Graphics.flush()
    sn.Graphics.readback_texture(renderTexture, out)
    sn.Graphics.bind_pipeline(sn.BuiltinPipelines.get_2d())
    sn.Graphics.draw_image(out, sn.Rect(0, 0, sx, sy))
