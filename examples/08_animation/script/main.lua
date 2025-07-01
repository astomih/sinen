local texture = sn.Texture()
texture:FillColor(sn.Color(1, 1, 1, 1))
local model = sn.Model()
model:Load("BrainStem.glb")
local draw3d = sn.Draw3D(texture)
draw3d.position = sn.Vec3(0, 0, 0)
draw3d.model = model

local vertex_shader = sn.Shader()
vertex_shader:CompileAndLoadVertexShader("skinning.slang", 1)
local fragment_shader = sn.Shader()
fragment_shader:CompileAndLoadFragmentShader("skinning.slang", 0)

local pipeline = sn.GraphicsPipeline3D()
pipeline:SetVertexShader(vertex_shader)
pipeline:SetFragmentShader(fragment_shader)
pipeline:SetAnimation(true)
pipeline:Build()


local pos = sn.Vec3(0, -3, 1)
sn.Scene.GetCamera():LookAt(pos, sn.Vec3(0, 0, 1), sn.Vec3(0, 0, 1))

model:Play(0)
function Update()
    model:Update(sn.Scene.DeltaTime())
end

function Draw()
    sn.Graphics.BindPipeline3D(pipeline)
    sn.Graphics.SetUniformData(1, model:GetBoneUniformData())
    -- Draw texture
    draw3d:Draw()
end
