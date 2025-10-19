import sinen as sn
import gc


gc.disable()
boxTex = sn.Texture()
boxTex.fill(sn.Color(1, 1, 1, 1))
boxMat = sn.Material()
boxMat.append_texture(boxTex)

sphereTex = sn.Texture()
sphereTex.fill(sn.Color(1, 0, 0, 1))
sphereMat = sn.Material()
sphereMat.append_texture(sphereTex)


box = sn.Model()
box.load_box()
sphere = sn.Model()
sphere.load("sphere.glb")

boxTransform = sn.Transform()
boxTransform.rotation = sn.Vec3(0, 0, 10)
boxTransform.scale = sn.Vec3(20, 1, 20)
sphereTransform = sn.Transform()
sphereTransform.position = sn.Vec3(0, 2, 0)
sphereTransform.scale = sn.Vec3(0.5)

colliders = list()

floorCollider = sn.Physics.create_box_collider(boxTransform, True)
sphereCollider = sn.Physics.create_sphere_collider(sphereTransform.position, 0.5, False)
sphereCollider.set_linear_velocity(sn.Vec3(0, -5.0, 0))
sn.Physics.add_collider(floorCollider, False)
sn.Physics.add_collider(sphereCollider, True)


sn.Graphics.get_camera().lookat(sn.Vec3(0, 10, 25), sn.Vec3(0, 0, 0), sn.Vec3(0, 1, 0))

sn.Graphics.bind_pipeline(sn.BuiltinPipelines.get_3d())


def update():
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE):
        sn.Script.load("main", ".")
    boxTransform.position = floorCollider.get_position()
    sphereTransform.position = sphereCollider.get_position()

    if sn.Mouse.is_pressed(sn.Mouse.LEFT):
        sc = sn.Physics.create_sphere_collider(sn.Vec3(0, 2, 0), 0.5, False)
        sc.set_linear_velocity(
            sn.Vec3(
                sn.Random.get_float_range(-5, 5),
                sn.Random.get_float_range(-5, 5),
                sn.Random.get_float_range(-5, 5),
            )
        )
        sn.Physics.add_collider(sc, True)
        list.append(colliders, sc)


def draw():
    sn.Graphics.draw_model(sphere, sphereTransform, sphereMat)
    sn.Graphics.draw_model(box, boxTransform, boxMat)
    for c in colliders:
        t = sn.Transform()
        t.position = c.get_position()
        if t.position.y > -10.0:
            t.scale = sn.Vec3(0.5)
            sn.Graphics.draw_model(sphere, t, sphereMat)
        else:
            list.remove(colliders, c)
    gc.collect()
