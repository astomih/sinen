local texture = Texture()
local draw2d = Draw2D()
draw2d.material:append(texture)

local vertex_shader = Shader()
vertex_shader:load_vertex_shader("shader_custom.vert.spv", 1)
local fragment_shader = Shader()
fragment_shader:load_fragment_shader("shaderAlpha.frag.spv", 0)

local pipeline2d = GraphicsPipeline2D()
pipeline2d:set_vertex_shader(vertex_shader)
pipeline2d:set_fragment_shader(fragment_shader)
pipeline2d:build()

local uniform_data = UniformData()
uniform_data:add(2.0)
uniform_data:add(0.5)
uniform_data:add(0.5)
uniform_data:add(1.0)

texture:load("logo.png")
draw2d.scale = texture:size()

function update()
end

function draw()
    Graphics.bind_pipeline2d(pipeline2d)
    Graphics.set_uniform_data(1, uniform_data)
    -- Draw texture
    draw2d:draw()
    Graphics.bind_default_pipeline2d()
end
