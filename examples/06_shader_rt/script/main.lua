local texture = Texture()
texture:Load("logo.png")
local draw2d = Draw2D()
draw2d.material:AppendTexture(texture)

draw2d.scale = texture:Size()

local vertex_shader = Shader()
vertex_shader:CompileAndLoadVertexShader("shader_custom.slang", 1)
local fragment_shader = Shader()
fragment_shader:CompileAndLoadFragmentShader("shader_custom.slang", 0)

local pipeline2d = GraphicsPipeline2D()
pipeline2d:SetVertexShader(vertex_shader)
pipeline2d:SetFragmentShader(fragment_shader)
pipeline2d:Build()

local uniform_data = UniformData()
uniform_data:Add(2.0)
uniform_data:Add(0.5)
uniform_data:Add(0.5)
uniform_data:Add(1.0)


function Update()

end

function Draw()
    Graphics.BindPipeline2D(pipeline2d)
    Graphics.SetUniformData(1, uniform_data)
    -- Draw texture
    draw2d:Draw()
end
