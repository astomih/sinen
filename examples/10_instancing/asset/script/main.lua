local model = sn.Model()
model:load("DamagedHelmet.glb")
local material = model:get_material()

local transforms = {}
for i = 0, 10 do
  for j = 0, 10 do
    local t = sn.Transform()
    t.position = sn.Vec3(i * 2, 0, j * 2)
    t.rotation = sn.Vec3(90, 0, 180)
    table.insert(transforms, t)
  end
end


local pos = sn.Vec3(-3, 5, -3)
local at = sn.Vec3(15, -5, 15)
local up = sn.Vec3(0, 1, 0)
sn.Graphics.get_camera():lookat(pos, at, up)

function update()
  if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE) then
    sn.Script.load("main", ".")
  end
end

function draw()
  sn.Graphics.draw_model_instanced(model, transforms, material)
end
