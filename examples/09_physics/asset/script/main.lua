local boxTex = sn.Texture()
boxTex:FillColor(sn.Color(1, 1, 1, 1))
local boxMat = sn.Material()
boxMat:AppendTexture(boxTex)

local sphereTex = sn.Texture()
sphereTex:FillColor(sn.Color(1, 0, 0, 1))
local sphereMat = sn.Material()
sphereMat:AppendTexture(sphereTex)



local box = sn.Model()
box:LoadBox()
local sphere = sn.Model()
sphere:Load("sphere.glb")

local boxTransform = sn.Transform()
boxTransform.scale = sn.Vec3(100, 1, 100)
local sphereTransform = sn.Transform()
sphereTransform.position = sn.Vec3(0, 2, 0)
sphereTransform.scale = sn.Vec3(0.5, 0.5, 0.5)

local floorCollider = sn.Physics.CreateBoxCollider(boxTransform, true)
local sphereCollider = sn.Physics.CreateSphereCollider(sphereTransform.position, 0.5, false)
sphereCollider:SetLinearVelocity(sn.Vec3(0, -5.0, 0))
sn.Physics.AddCollider(floorCollider, false)
sn.Physics.AddCollider(sphereCollider, true)


sn.Scene.GetCamera():LookAt(sn.Vec3(0, 2, 2), sn.Vec3(0, 0, 0), sn.Vec3(0, 1, 0))



function Update()
    boxTransform.position = floorCollider:GetPosition()
    sphereTransform.position = sphereCollider:GetPosition()
end

function Draw()
    sn.Graphics.DrawModel(sphere, sphereTransform, sphereMat)
    sn.Graphics.DrawModel(box, boxTransform, boxMat)
end
