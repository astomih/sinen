from sinen import *
texture = Texture()
texture.load("logo.png")
draw2d = Draw2D()
draw2d.material.append(texture)

draw2d.scale = texture.size()

vertex_shader = Shader()
vertex_shader.compile_and_load_vertex_shader("shader_custom.slang", 1)
fragment_shader = Shader()
fragment_shader.compile_and_load_fragment_shader("shader_custom.slang", 0)

pipeline2d = RenderPipeline2D()
pipeline2d.set_vertex_shader(vertex_shader)
pipeline2d.set_fragment_shader(fragment_shader)
pipeline2d.build()

uniform_data = UniformData()
uniform_data.add(2.0)
uniform_data.add(0.5)
uniform_data.add(0.5)
uniform_data.add(1.0)


def update():
  pass

def draw():
  Renderer.begin_pipeline2d(pipeline2d)
  Renderer.set_uniform_data(1, uniform_data)
  # Draw texture
  draw2d.draw()
  Renderer.end_pipeline2d()
