local boxTex = sn.Texture()
boxTex:fill(sn.Color(1, 1, 1, 1))
local boxMat = sn.Material()
boxMat:append_texture(boxTex)

local sphereTex = sn.Texture()
sphereTex:fill(sn.Color(1, 0, 0, 1))
local sphereMat = sn.Material()
sphereMat:append_texture(sphereTex)



local box = sn.Model()
box:load_box()
local sphere = sn.Model()
sphere:load("sphere.glb")

local boxTransform = sn.Transform()
boxTransform.rotation = sn.Vec3(0, 0, 10)
boxTransform.scale = sn.Vec3(20, 1, 20)
local sphereTransform = sn.Transform()
sphereTransform.position = sn.Vec3(0, 2, 0)
sphereTransform.scale = sn.Vec3(0.5)

local colliders = {}

local floorCollider = sn.Physics.create_box_collider(boxTransform, true)
local sphereCollider = sn.Physics.create_sphere_collider(sphereTransform.position, 0.5, false)
sphereCollider:set_linear_velocity(sn.Vec3(0, -5.0, 0))
sn.Physics.add_collider(floorCollider, false)
sn.Physics.add_collider(sphereCollider, true)


sn.Graphics.get_camera():lookat(sn.Vec3(0, 10, 25), sn.Vec3(0, 0, 0), sn.Vec3(0, 1, 0))



function update()
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    boxTransform.position = floorCollider:get_position()
    sphereTransform.position = sphereCollider:get_position()

    if sn.Mouse.is_pressed(sn.Mouse.LEFT) then
        local sc = sn.Physics.create_sphere_collider(sn.Vec3(0, 2, 0), 0.5, false)
        sc:set_linear_velocity(sn.Vec3(math.random(-5, 5), math.random(-5, 5), math.random(-5, 5)))
        sn.Physics.add_collider(sc, true)
        table.insert(colliders, sc)
    end
end

function draw()
    sn.Graphics.draw_model(sphere, sphereTransform, sphereMat)
    sn.Graphics.draw_model(box, boxTransform, boxMat)
    for _, c in ipairs(colliders) do
        local transform = sn.Transform()
        transform.position = c:get_position()
        if transform.position.y < -10.0 then
            table.remove(colliders, _)
        else
            transform.scale = sn.Vec3(0.5)
            sn.Graphics.draw_model(sphere, transform, sphereMat)
        end
    end
end
