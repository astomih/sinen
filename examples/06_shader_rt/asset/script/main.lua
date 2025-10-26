local texture = sn.Texture.new()
texture:load("logo.png")
local draw2d = sn.Draw2D.new(texture)

draw2d.scale = texture:size()

local vertex_shader = sn.Shader.new()
vertex_shader:compileLoadVertexShader("shader_custom.slang")
local fragment_shader = sn.Shader.new()
fragment_shader:compileLoadFragmentShader("shader_custom.slang")

local pipeline2d = sn.GraphicsPipeline.new()
pipeline2d:setVertexShader(vertex_shader)
pipeline2d:setFragmentShader(fragment_shader)
pipeline2d:setEnableDepthTest(false)
pipeline2d:build()
sn.Graphics.bindPipeline(pipeline2d)

local uniform_data = sn.UniformData.new()
uniform_data:add(2.0)
uniform_data:add(0.5)
uniform_data:add(0.5)
uniform_data:add(1.0)


function update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function draw()
    sn.Graphics.setUniformData(1, uniform_data)
    -- Draw texture
    sn.Graphics.draw2D(draw2d)
end
