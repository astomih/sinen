local texture = Texture()
texture:FillColor(Color(1, 1, 1, 1))
local model = Model()
model:Load("BrainStem.glb")
local draw3d = Draw3D(texture)
draw3d.position = Vec3(0, 0, 0)
draw3d.model = model

local vertex_shader = Shader()
vertex_shader:CompileAndLoadVertexShader("skinning.slang", 1)
local fragment_shader = Shader()
fragment_shader:CompileAndLoadFragmentShader("skinning.slang", 0)

local pipeline = GraphicsPipeline3D()
pipeline:SetVertexShader(vertex_shader)
pipeline:SetFragmentShader(fragment_shader)
pipeline:SetAnimation(true)
pipeline:Build()


local pos = Vec3(0, -3, 1)
Scene.GetCamera():LookAt(pos, Vec3(0, 0, 1), Vec3(0, 0, 1))

model:Play(0)
function Update()
    model:Update(Scene.DeltaTime())
end

function Draw()
    Graphics.BindPipeline3D(pipeline)
    Graphics.SetUniformData(1, model:GetBoneUniformData())
    -- Draw texture
    draw3d:Draw()
end
