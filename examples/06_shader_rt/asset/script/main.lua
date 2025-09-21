local texture = sn.Texture()
texture:Load("logo.png")
local draw2d = sn.Draw2D(texture)

draw2d.scale = texture:Size()

local vertex_shader = sn.Shader()
vertex_shader:CompileAndLoadVertexShader("shader_custom.slang", 1)
local fragment_shader = sn.Shader()
fragment_shader:CompileAndLoadFragmentShader("shader_custom.slang", 0)

local pipeline2d = sn.GraphicsPipeline2D()
pipeline2d:SetVertexShader(vertex_shader)
pipeline2d:SetFragmentShader(fragment_shader)
pipeline2d:Build()

local uniform_data = sn.UniformData()
uniform_data:Add(2.0)
uniform_data:Add(0.5)
uniform_data:Add(0.5)
uniform_data:Add(1.0)


function Update()
    if sn.Keyboard.IsPressed(sn.Keyboard.ESCAPE) then
        sn.Scene.Change("main", ".")
    end
end

function Draw()
    sn.Graphics.BindPipeline2D(pipeline2d)
    sn.Graphics.SetUniformData(1, uniform_data)
    -- Draw texture
    sn.Graphics.Draw2D(draw2d)
end
