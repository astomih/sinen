local texture = Texture()
texture:fill_color(Color(1, 1, 1, 1))
local model = Model()
model:load("Suzanne.gltf")
draw3d = Draw3D()
draw3d.material:append(texture)
draw3d.position = Vec3(0, 0, 0)
draw3d.model = model


local pos = Vec3(1.0, 1.0, 3.0)
local at = Vec3(0, 0, 0)
local up = Vec3(0, 1, 0)
local speed = 0.1

function update()
    -- # if Keyboard.is_down(Keyboard.W):
    -- #   pos.z -= speed

    -- # if Keyboard.is_down(Keyboard.S):
    -- #   pos.z += speed

    -- # if Keyboard.is_down(Keyboard.A):
    -- #   pos.x -= speed

    -- # if Keyboard.is_down(Keyboard.D):
    -- #   pos.x += speed

    -- # if Keyboard.is_down(Keyboard.LSHIFT):
    -- #   pos.y -= speed

    -- # if Keyboard.is_down(Keyboard.SPACE):
    -- #   pos.y += speed

    scene.camera():lookat(pos, at, up)
end

function draw()
    -- Draw texture
    draw3d:draw()
end
