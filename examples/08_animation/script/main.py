from sinen import *
texture = Texture()
texture.fill_color(Color(1, 1, 1, 1))
model = Model()
model.load("BrainStem.glb")
draw3d = Draw3D()
draw3d.material.append(texture)
draw3d.position = Vec3(0, 0, 0)
draw3d.model = model

vertex_shader = Shader()
vertex_shader.compile_and_load_vertex_shader("skinning.slang", 1)
fragment_shader = Shader()
fragment_shader.compile_and_load_fragment_shader("skinning.slang", 0)

pipeline = RenderPipeline3D()
pipeline.set_vertex_shader(vertex_shader)
pipeline.set_fragment_shader(fragment_shader)
pipeline.set_animation(True)
pipeline.build()


pos = Vec3(0, -3, 1)
Scene.camera().lookat(pos, Vec3(0, 0, 1), Vec3(0, 0, 1))

model.play(0)
def update():
  model.update(Scene.delta_time())

def draw():
  Renderer.begin_pipeline3d(pipeline)
  Renderer.set_uniform_data(1, model.bone_uniform_data())
  # Draw texture
  draw3d.draw()
  Renderer.end_pipeline3d()

