local texture = Texture()
texture:FillColor(Color(1, 1, 1, 1))
local model = Model()
model:Load("Suzanne.gltf")
local draw3d = Draw3D(texture)
Logger.Info("AA")
draw3d.position = Vec3(0)
draw3d.model = model

local pos = Vec3(1, 1, 3)
local at = Vec3(0)
local up = Vec3(0, 1, 0)
Scene.GetCamera():LookAt(pos, at, up)

function Update()
end

function Draw()
    -- Draw texture
    draw3d:Draw()
end
