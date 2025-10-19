import sinen as sn

texture = sn.Texture()
draw2d = sn.Draw2D(texture)

vertex_shader = sn.Shader()
vertex_shader.load_vertex_shader("shader_custom.vert.spv", 1)
fragment_shader = sn.Shader()
fragment_shader.load_fragment_shader("shaderAlpha.frag.spv", 0)

pipeline2d = sn.GraphicsPipeline()
pipeline2d.set_vertex_shader(vertex_shader)
pipeline2d.set_fragment_shader(fragment_shader)
pipeline2d.set_enable_depth_test(False)
pipeline2d.build()
sn.Graphics.bind_pipeline(pipeline2d)

uniform_data = sn.UniformData()
uniform_data.add(2.0)
uniform_data.add(0.5)
uniform_data.add(0.5)
uniform_data.add(1.0)

texture.load("logo.png")
draw2d.scale = texture.size()


def update():
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE):
        sn.Script.load("main", ".")


def draw():
    sn.Graphics.set_uniform_data(1, uniform_data)
    # Draw texture
    sn.Graphics.draw2d(draw2d)
