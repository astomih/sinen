local boxTex = sn.Texture.new()
boxTex:fill(sn.Color.new(1, 1, 1, 1))

local sphereTex = sn.Texture.new()
sphereTex:fill(sn.Color.new(1, 0, 0, 1))

local box = sn.Model.new()
box:loadBox()
box:setTexture(sn.TextureKey.BaseColor, boxTex)

local sphere = sn.Model.new()
sphere:load("sphere.glb")
sphere:setTexture(sn.TextureKey.BaseColor, sphereTex)

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

function update()
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

function draw()
    sn.Graphics.drawModel(box, boxTransform)

    sn.Graphics.drawModel(sphere, sphereTransform)
    for _, c in ipairs(colliders) do
        local transform = sn.Transform.new()
        transform.position = c:getPosition()
        if transform.position.y < -10.0 then
            table.remove(colliders, _)
        else
            transform.scale = sn.Vec3.new(0.5)
            sn.Graphics.drawModel(sphere, transform)
        end
    end
end
