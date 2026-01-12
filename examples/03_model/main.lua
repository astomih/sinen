local model = sn.Model.new()
model:load("DamagedHelmet.glb")

local transform = sn.Transform.new()
transform.position = sn.Vec3.new(0, 0, 0)
transform.rotation = sn.Vec3.new(90, 0, 0)
transform.scale = sn.Vec3.new(1, 1, 1)


local pos = sn.Vec3.new(1, 1, 3)
local at = sn.Vec3.new(0)
local up = sn.Vec3.new(0, 1, 0)
function Setup()
    sn.Graphics.getCamera():lookat(pos, at, up)
end

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function Draw()
    sn.Graphics.drawModel(model, transform)
end
