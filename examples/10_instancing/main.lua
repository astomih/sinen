local model = sn.Model.new()
model:load("DamagedHelmet.glb")
local transforms = {}
function setup()
    for i = 0, 10 do
        for j = 0, 10 do
            local t = sn.Transform.new()
            t.position = sn.Vec3.new(i * 2, 0, j * 2)
            t.rotation = sn.Vec3.new(90, 0, 180)
            table.insert(transforms, t)
        end
    end

    local pos = sn.Vec3.new(-3, 5, -3)
    local at = sn.Vec3.new(15, -5, 15)
    local up = sn.Vec3.new(0, 1, 0)
    sn.Graphics.getCamera():lookat(pos, at, up)
end

function update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function draw()
    sn.Graphics.drawModelInstanced(model, transforms)
end
