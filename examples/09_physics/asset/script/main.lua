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
boxTransform.rotation = sn.Vec3(0, 0, 10)
boxTransform.scale = sn.Vec3(20, 1, 20)
local sphereTransform = sn.Transform()
sphereTransform.position = sn.Vec3(0, 2, 0)
sphereTransform.scale = sn.Vec3(0.5)

local colliders = {}

local floorCollider = sn.Physics.CreateBoxCollider(boxTransform, true)
local sphereCollider = sn.Physics.CreateSphereCollider(sphereTransform.position, 0.5, false)
sphereCollider:SetLinearVelocity(sn.Vec3(0, -5.0, 0))
sn.Physics.AddCollider(floorCollider, false)
sn.Physics.AddCollider(sphereCollider, true)


sn.Scene.GetCamera():LookAt(sn.Vec3(0, 10, 25), sn.Vec3(0, 0, 0), sn.Vec3(0, 1, 0))



function Update()
    if sn.Keyboard.IsPressed(sn.Keyboard.ESCAPE) then
        sn.Scene.Change("main", ".")
    end
    boxTransform.position = floorCollider:GetPosition()
    sphereTransform.position = sphereCollider:GetPosition()

    if sn.Mouse.IsPressed(sn.Mouse.LEFT) then
        local sc = sn.Physics.CreateSphereCollider(sn.Vec3(0, 2, 0), 0.5, false)
        sc:SetLinearVelocity(sn.Vec3(math.random(-5, 5), math.random(-5, 5), math.random(-5, 5)))
        sn.Physics.AddCollider(sc, true)
        table.insert(colliders, sc)
    end
end

function Draw()
    sn.Graphics.DrawModel(sphere, sphereTransform, sphereMat)
    sn.Graphics.DrawModel(box, boxTransform, boxMat)
    for _, c in ipairs(colliders) do
        local transform = sn.Transform()
        transform.position = c:GetPosition()
        if transform.position.y < -10.0 then
            table.remove(colliders, _)
        else
            transform.scale = sn.Vec3(0.5)
            sn.Graphics.DrawModel(sphere, transform, sphereMat)
        end
    end
end
