local texture = {}
local draw2d = {}

local vertex_shader = Shader()
vertex_shader:load_vertex_shader("shader_custom.vert.spv", 1)
local fragment_shader = Shader()
fragment_shader:load_fragment_shader("shaderAlpha.frag.spv", 0)

local pipeline2d = RenderPipeline2D()
pipeline2d:set_vertex_shader(vertex_shader)
pipeline2d:set_fragment_shader(fragment_shader)
pipeline2d:build()

local uniform_data = UniformData()
uniform_data:add(2.0)
uniform_data:add(0.5)
uniform_data:add(0.5)
uniform_data:add(1.0)

texture = Texture()
texture:load("logo.png")
draw2d = Draw2D(texture)

draw2d.scale = texture:size()

function Update()
end

function Draw()
  renderer.begin_pipeline2d(pipeline2d)
  renderer.set_uniform_data(1, uniform_data)
  -- Draw texture
  draw2d:draw()
  renderer.end_pipeline2d()
end
