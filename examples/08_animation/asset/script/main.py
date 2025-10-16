import sinen as sn

texture = sn.Texture()
texture.fill(sn.Color(1, 1, 1, 1))
model = sn.Model()
model.load("BrainStem.glb")

vertex_shader = sn.Shader()
vertex_shader.compile_and_load_vertex_shader("skinning.slang")
fragment_shader = sn.Shader()
fragment_shader.compile_and_load_fragment_shader("skinning.slang")

pipeline = sn.GraphicsPipeline3D()
pipeline.set_vertex_shader(vertex_shader)
pipeline.set_fragment_shader(fragment_shader)
pipeline.set_animation(True)
pipeline.build()


sn.Graphics.get_camera().lookat(sn.Vec3(0, -3, 1), sn.Vec3(0, 0, 1), sn.Vec3(0, 0, 1))

model.play(0)


transform = sn.Transform()
material = sn.Material()
material.append_texture(texture)


def update():
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE):
        sn.Script.load("main", ".")
    model.update(sn.Time.delta())


def draw():
    sn.Graphics.bind_pipeline3d(pipeline)
    sn.Graphics.set_uniform_data(1, model.get_bone_uniform_data())
    # Draw texture
    sn.Graphics.draw_model(model, transform, material)
