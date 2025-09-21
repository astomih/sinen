local texture = sn.Texture()
texture:FillColor(sn.Color(1, 1, 1, 1))
local material = sn.Material()
material:AppendTexture(texture)
local model = sn.Model()
model:Load("Suzanne.gltf")

local transform = sn.Transform()
transform.position = sn.Vec3(0, 0, 0)
transform.scale = sn.Vec3(1, 1, 1)


local pos = sn.Vec3(1, 1, 3)
local at = sn.Vec3(0)
local up = sn.Vec3(0, 1, 0)
sn.Scene.GetCamera():LookAt(pos, at, up)

function Update()
    if sn.Keyboard.IsPressed(sn.Keyboard.ESCAPE) then
        sn.Scene.Change("main", ".")
    end
end

function Draw()
    sn.Graphics.DrawModel(model, transform, material)
end
