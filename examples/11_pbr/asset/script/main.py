import sinen as sn

model = sn.Model()
model.load("DamagedHelmet.glb")
material = model.get_material()


transform = sn.Transform()
transform.position = sn.Vec3(0, 0, 0)
transform.rotation = sn.Vec3(90, 0, 0)
transform.scale = sn.Vec3(1, 1, 1)


pos = sn.Vec3(0.7, 0.7, 2.1)
at = sn.Vec3(0)
up = sn.Vec3(0, 1, 0)
sn.Graphics.get_camera().lookat(pos, at, up)

vertex_shader = sn.Shader()
vertex_shader.compile_and_load_vertex_shader("shader_custom.slang")
fragment_shader = sn.Shader()
fragment_shader.compile_and_load_fragment_shader("shader_custom.slang")
pipeline3d = sn.GraphicsPipeline3D()
pipeline3d.set_vertex_shader(vertex_shader)
pipeline3d.set_fragment_shader(fragment_shader)
pipeline3d.set_enable_tangent(True)
pipeline3d.build()

light_pos = sn.Vec3(2, 0, 0)
light_intensity = 5.0

uniform_data = sn.UniformData()
uniform_data.add(pos.x)
uniform_data.add(pos.y)
uniform_data.add(pos.z)
uniform_data.add(light_pos.x)
uniform_data.add(light_pos.y)
uniform_data.add(light_pos.z)
uniform_data.add(light_intensity)


def update():
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE):
        sn.Script.load("main", ".")
    transform.rotation.z = transform.rotation.z + sn.Time.delta() * 10
    if sn.Keyboard.is_down(sn.Keyboard.LEFT):
        light_pos.x = light_pos.x - sn.Time.delta() * 5
    if sn.Keyboard.is_down(sn.Keyboard.RIGHT):
        light_pos.x = light_pos.x + sn.Time.delta() * 5
    uniform_data.change(light_pos.x, 3)
    if sn.Keyboard.is_down(sn.Keyboard.UP):
        light_pos.y = light_pos.y + sn.Time.delta() * 5
    if sn.Keyboard.is_down(sn.Keyboard.DOWN):
        light_pos.y = light_pos.y - sn.Time.delta() * 5
    uniform_data.change(light_pos.y, 4)


def draw():
    sn.Graphics.bind_pipeline3d(pipeline3d)
    sn.Graphics.set_uniform_data(1, uniform_data)
    sn.Graphics.draw_model(model, transform, material)
