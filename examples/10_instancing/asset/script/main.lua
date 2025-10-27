local model = sn.Model.new()
model:load("DamagedHelmet.glb")
local material = model:getMaterial()

local transforms = {}
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
sn.Graphics.bindPipeline(sn.BuiltinPipelines.get3DInstanced())

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function Draw()
    sn.Graphics.drawModelInstanced(model, transforms, material)
end
