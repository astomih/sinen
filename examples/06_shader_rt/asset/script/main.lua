local texture = sn.Texture()
texture:load("logo.png")
local draw2d = sn.Draw2D(texture)

draw2d.scale = texture:size()

local vertex_shader = sn.Shader()
vertex_shader:compile_load_vertex_shader("shader_custom.slang", 1)
local fragment_shader = sn.Shader()
fragment_shader:compile_load_fragment_shader("shader_custom.slang", 0)

local pipeline2d = sn.GraphicsPipeline2D()
pipeline2d:set_vertex_shader(vertex_shader)
pipeline2d:set_fragment_shader(fragment_shader)
pipeline2d:build()

local uniform_data = sn.UniformData()
uniform_data:add(2.0)
uniform_data:add(0.5)
uniform_data:add(0.5)
uniform_data:add(1.0)


function update()
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function draw()
    sn.Graphics.bind_pipeline2d(pipeline2d)
    sn.Graphics.set_uniform_data(1, uniform_data)
    -- Draw texture
    sn.Graphics.draw2d(draw2d)
end
