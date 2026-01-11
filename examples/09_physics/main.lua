local boxTex = sn.Texture.new()
boxTex:fill(sn.Color.new(1, 1, 1, 1))
local boxMat = sn.Material.new()
boxMat:appendTexture(boxTex)

local sphereTex = sn.Texture.new()
sphereTex:fill(sn.Color.new(1, 0, 0, 1))
local sphereMat = sn.Material.new()
sphereMat:appendTexture(sphereTex)

local box = sn.Model.new()
box:loadBox()
local sphere = sn.Model.new()
sphere:load("sphere.glb")

local boxTransform = sn.Transform.new()
boxTransform.rotation = sn.Vec3.new(0, 0, 10)
boxTransform.scale = sn.Vec3.new(20, 1, 20)
local sphereTransform = sn.Transform.new()
sphereTransform.position = sn.Vec3.new(0, 2, 0)
sphereTransform.scale = sn.Vec3.new(0.5)

local colliders = {}

local floorCollider = sn.Physics.createBoxCollider(boxTransform, true)
local sphereCollider = sn.Physics.createSphereCollider(sphereTransform.position, 0.5, false)
sphereCollider:setLinearVelocity(sn.Vec3.new(0, -5.0, 0))
sn.Physics.addCollider(floorCollider, false)
sn.Physics.addCollider(sphereCollider, true)

sn.Graphics.getCamera():lookat(sn.Vec3.new(0, 10, 25), sn.Vec3.new(0, 0, 0), sn.Vec3.new(0, 1, 0))

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    boxTransform.position = floorCollider:getPosition()
    sphereTransform.position = sphereCollider:getPosition()

    if sn.Mouse.isPressed(sn.Mouse.LEFT) then
        local sc = sn.Physics.createSphereCollider(sn.Vec3.new(0, 2, 0), 0.5, false)
        sc:setLinearVelocity(sn.Vec3.new(math.random(-5, 5), math.random(-5, 5), math.random(-5, 5)))
        sn.Physics.addCollider(sc, true)
        table.insert(colliders, sc)
    end
end

function Draw()
    sn.Graphics.drawModel(sphere, sphereTransform, sphereMat)
    sn.Graphics.drawModel(box, boxTransform, boxMat)
    for _, c in ipairs(colliders) do
        local transform = sn.Transform.new()
        transform.position = c:getPosition()
        if transform.position.y < -10.0 then
            table.remove(colliders, _)
        else
            transform.scale = sn.Vec3.new(0.5)
            sn.Graphics.drawModel(sphere, transform, sphereMat)
        end
    end
end
