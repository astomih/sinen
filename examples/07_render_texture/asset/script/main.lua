local texture = sn.Texture()
texture:FillColor(sn.Color(1, 1, 1, 1))
local renderTexture = sn.RenderTexture()
local sx = sn.Scene.Size().x
local sy = sn.Scene.Size().y
renderTexture:Create(sx, sy)

local model = sn.Model()
model:Load("Suzanne.gltf")
local draw3d = sn.Draw3D(texture)
draw3d.position = sn.Vec3(0)
draw3d.model = model

local pos = sn.Vec3(1, 1, 3)

function Update()
    sn.Scene.GetCamera():LookAt(pos, sn.Vec3(0), sn.Vec3(0, 1, 0))
end

function Draw()
    sn.Graphics.SetRenderTarget(renderTexture)
    sn.Graphics.Draw3D(draw3d)
    -- Draw texture
    sn.Graphics.WaitDraw()
    sn.Graphics.DrawImage(sn.Graphics.ReadbackTexture(renderTexture), sn.Rect(0, 0, sx, sy))
end
