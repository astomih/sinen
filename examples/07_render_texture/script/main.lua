local texture = Texture()
texture:FillColor(Color(1, 1, 1, 1))
local outTexture = Texture()
outTexture:Load("logo.png")
local renderTexture = RenderTexture()
local s = outTexture:Size()
renderTexture:Create(s.x, s.y)

local model = Model()
model:Load("Suzanne.gltf")
local draw3d = Draw3D()
draw3d.material:AppendTexture(texture)
draw3d.position = Vec3(0)
draw3d.model = model

local draw2d = Draw2D()
draw2d.material:AppendTexture(outTexture)
draw2d.scale = outTexture:Size()


local pos = Vec3(1, 1, 3)

function Update()
    Scene.GetCamera():LookAt(pos, Vec3(0), Vec3(0, 1, 0))
end

function Draw()
    Graphics.BeginTarget2D(renderTexture)
    -- Draw texture
    draw3d:Draw()
    Graphics.EndTarget(renderTexture, outTexture)

    -- Draw render texture
    draw2d:Draw()
end
