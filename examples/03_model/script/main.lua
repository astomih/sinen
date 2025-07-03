local texture = sn.Texture()
texture:FillColor(sn.Color(1, 1, 1, 1))
local model = sn.Model()
model:Load("Suzanne.gltf")
local draw3D = sn.Draw3D(texture)
draw3D.position = sn.Vec3(0)
draw3D.model = model

local pos = sn.Vec3(1, 1, 3)
local at = sn.Vec3(0)
local up = sn.Vec3(0, 1, 0)
sn.Scene.GetCamera():LookAt(pos, at, up)

function Update()
end

function Draw()
    -- Draw texture
    sn.Graphics.Draw3D(draw3D)
end
