local sn = require("@sinen")

local camera: sn.Camera3D = sn.Camera3D.new()
local models: { sn.Model } = {}
local transforms: { sn.Transform } = {}

local function addModel(model: sn.Model, position: sn.Vec3)
	local transform = sn.Transform.new()
	transform.position = position
	transform.scale = sn.Vec3.new(1)
	table.insert(models, model)
	table.insert(transforms, transform)
end

local function createPyramid(): sn.Model
	local builder = sn.MeshBuilder.new()
	local red = sn.Color.new(0.95, 0.2, 0.15, 1.0)
	local green = sn.Color.new(0.2, 0.8, 0.35, 1.0)
	local blue = sn.Color.new(0.2, 0.45, 1.0, 1.0)
	local yellow = sn.Color.new(1.0, 0.85, 0.25, 1.0)
	local white = sn.Color.new(1.0)

	local a = builder:vertex(sn.Vec3.new(-0.8, -0.6, -0.8), nil, nil, red)
	local b = builder:vertex(sn.Vec3.new(0.8, -0.6, -0.8), nil, nil, green)
	local c = builder:vertex(sn.Vec3.new(0.8, -0.6, 0.8), nil, nil, blue)
	local d = builder:vertex(sn.Vec3.new(-0.8, -0.6, 0.8), nil, nil, yellow)
	local top = builder:vertex(sn.Vec3.new(0, 0.9, 0), nil, nil, white)
	builder:quad(a, b, c, d)
	builder:triangle(a, top, b)
	builder:triangle(b, top, c)
	builder:triangle(c, top, d)
	builder:triangle(d, top, a)
	builder:recalculateNormals()

	return builder:toModel()
end

function setup()
	local sphere = sn.MeshBuilder.new()
	sphere:addSphere(0.85, 18, 36, sn.Color.new(0.95, 0.35, 0.2, 1.0))
	addModel(sphere:toModel(), sn.Vec3.new(-2.2, 0, 0))

	local cylinder = sn.MeshBuilder.new()
	cylinder:addCylinder(0.65, 1.6, 32, sn.Color.new(0.2, 0.65, 1.0, 1.0))
	addModel(cylinder:toModel(), sn.Vec3.new(0, 0, 0))

	addModel(createPyramid(), sn.Vec3.new(2.2, 0, 0))

	camera:lookat(sn.Vec3.new(0, 1.4, 5.2), sn.Vec3.new(0, 0, 0), sn.Vec3.new(0, 1, 0))
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		sn.Script.load("main", ".")
	end

	for _, transform in ipairs(transforms) do
		transform.rotation.y = transform.rotation.y + sn.Time.delta() * 45.0
	end
end

function draw()
	sn.Graphics.begin3D(camera)
	for i, model in ipairs(models) do
		sn.Graphics.drawModel(model, transforms[i])
	end
	sn.Graphics.finish()
end
