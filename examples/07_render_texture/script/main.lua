local texture = sn.Texture()
texture:FillColor(sn.Color(1, 1, 1, 1))
local outTexture = sn.Texture()
outTexture:Load("logo.png")
local renderTexture = sn.RenderTexture()
local s = outTexture:Size()
renderTexture:Create(s.x, s.y)

local model = sn.Model()
model:Load("Suzanne.gltf")
local draw3d = sn.Draw3D(texture)
draw3d.position = sn.Vec3(0)
draw3d.model = model

local draw2d = sn.Draw2D(outTexture)
draw2d.scale = outTexture:Size()

local pos = sn.Vec3(1, 1, 3)

function Update()
    sn.Scene.GetCamera():LookAt(pos, sn.Vec3(0), sn.Vec3(0, 1, 0))
end

function Draw()
    sn.Graphics.BeginTarget2D(renderTexture)
    -- Draw texture
    draw3d:Draw()
    sn.Graphics.EndTarget(renderTexture, outTexture)

    -- Draw render texture
    draw2d:Draw()
end
