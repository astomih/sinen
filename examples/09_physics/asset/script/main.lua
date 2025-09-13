local boxTex = sn.Texture()
boxTex:FillColor(sn.Color(1, 1, 1, 1))
local boxMat = sn.Material()
boxMat:AppendTexture(boxTex)

local sphereTex = sn.Texture()
sphereTex:FillColor(sn.Color(1, 0, 0, 1))
local sphereMat = sn.Material()
sphereMat:AppendTexture(sphereTex)


local floorCollider = sn.Physics.CreateBoxCollider()
local sphereCollider = sn.Physics.CreateSphereCollider()

local box = sn.Model()
box:LoadBox()
local sphere = sn.Model()
sphere:Load("sphere.glb")

local boxTransform = sn.Transform()
boxTransform.scale = sn.Vec3(100, 1, 100)
local sphereTransform = sn.Transform()
sphereTransform.scale = sn.Vec3(0.5, 0.5, 0.5)


sn.Scene.GetCamera():LookAt(sn.Vec3(0, 5, 10), sn.Vec3(0, 0, 0), sn.Vec3(0, 1, 0))



function Update()
  boxTransform.position = floorCollider:GetPosition()
  sphereTransform.position = sphereCollider:GetPosition()
end

function Draw()
  sn.Graphics.DrawModel(sphere, sphereTransform, sphereMat)
  sn.Graphics.DrawModel(box, boxTransform, boxMat)
end
