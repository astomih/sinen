--!strict

local sn = require("@sinen")
local test = require("./sinen_test")

local describe = test.describe
local it = test.it
local expect = test.expect

-- Kept uncalled: this verifies the public render-pass surface during Luau
-- analysis without requiring a graphics device in the unit-test process.
local function renderPassApiTypecheck(
	camera2D: sn.Camera2D,
	camera3D: sn.Camera3D,
	target: sn.RenderTexture,
	model: sn.Model,
	texture: sn.Texture,
	style: sn.TextStyle,
	pipeline: sn.GraphicsPipeline,
	transform: sn.Transform,
	textTransform: sn.TextTransform
)
	local pass2D: sn.Render2DPass = sn.Graphics.begin2DPass(camera2D)
	pass2D:drawRect(sn.Rect.new(0, 0, 10, 10), sn.Color.new(1))
	local material: sn.Material = sn.Material.new(pipeline)
	material:setTexture(0, texture)
	material:setUniformBuffer(0, sn.Buffer.new({ 1 }))
	pass2D:drawImage(texture, sn.Rect.new(0, 0, 10, 10), material)
	pass2D:drawText("pass", style, textTransform)
	sn.Graphics.endPass(pass2D)

	local pass3D: sn.Render3DPass = sn.Graphics.begin3DPass(camera3D, target)
	pass3D:drawModel(model, transform, material)
	sn.Graphics.endPass(pass3D)
end

if false then
	renderPassApiTypecheck(
		nil :: any,
		nil :: any,
		nil :: any,
		nil :: any,
		nil :: any,
		nil :: any,
		nil :: any,
		nil :: any,
		nil :: any
	)
end

describe("Sinen Luau API", function()
	it("constructs 2D values", function()
		local center: sn.Vec2 = sn.Vec2.new(320, 240)
		local size: sn.Vec2 = sn.Vec2.new(128, 64)
		local rect: sn.Rect = sn.Rect.new(sn.Pivot.Center, center, size)
		local color: sn.Color = sn.Color.new(1.0, 0.5, 0.25, 1.0)

		expect(rect:center().x):toBeCloseTo(center.x)
		expect(rect:center().y):toBeCloseTo(center.y)
		expect(rect:positionFromPivot(sn.Pivot.Center).x):toBeCloseTo(center.x)
		expect(rect:positionFromPivot(sn.Pivot.Center).y):toBeCloseTo(center.y)
		expect(rect:position().x):toBeCloseTo(rect.x)
		expect(rect:position().y):toBeCloseTo(rect.y)
		expect(rect:size().x):toBeCloseTo(size.x)
		expect(rect:size().y):toBeCloseTo(size.y)
		expect(color.a):toBeCloseTo(1.0)
	end)

	it("constructs 3D mesh data", function()
		local builder: sn.MeshBuilder = sn.MeshBuilder.new()
		local a = builder:vertex(sn.Vec3.new(-0.5, 0, 0), nil, nil, sn.Color.new(1, 0, 0, 1))
		local b = builder:vertex(sn.Vec3.new(0.5, 0, 0), nil, nil, sn.Color.new(0, 1, 0, 1))
		local c = builder:vertex(sn.Vec3.new(0, 1, 0), nil, nil, sn.Color.new(0, 0, 1, 1))

		builder:triangle(a, b, c)
		builder:recalculateNormals()

		expect(builder:vertexCount()):toBe(3)
		expect(builder:indexCount()):toBe(3)
	end)

	it("uses buffers and compute buffers", function()
		local buffer: sn.Buffer = sn.Buffer.new({ 1, 2, 3, 4 })
		local computeBuffer: sn.ComputeBuffer = sn.ComputeBuffer.new(buffer)

		expect(buffer:size()):toBe(16)
		expect(computeBuffer:size()):toBe(16)
	end)

	it("configures pipeline objects", function()
		local graphicsPipeline: sn.GraphicsPipeline = sn.GraphicsPipeline.new()
		graphicsPipeline:setEnableDepthTest(true)
		graphicsPipeline:setEnableInstanced(false)

		local computePipeline: sn.ComputePipeline = sn.ComputePipeline.new()
		computePipeline:setThreadGroupSize(8, 8, 1)

		expect(graphicsPipeline):toBeTruthy()
		expect(computePipeline):toBeTruthy()
	end)

	it("uses timer and transform types", function()
		local timer: sn.Timer = sn.Timer.new()
		local transform: sn.Transform = sn.Transform.new()

		transform.position = sn.Vec3.new(1, 2, 3)
		transform.rotation = sn.Vec3.new(0)
		transform.scale = sn.Vec3.new(1)
		local world: sn.Mat4 = transform:getWorldMatrix()
		local worldCopy: sn.Mat4 = world:copy()
		timer:setTime(1.0)

		expect(transform.position.y):toBeCloseTo(2)
		expect(worldCopy):toBeTruthy()
		expect(timer:isStarted()):toBeFalsy()
	end)

	it("exposes camera matrices", function()
		local camera: sn.Camera3D = sn.Camera3D.new()
		camera:lookat(sn.Vec3.new(0, 0, 5), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))
		camera:perspective(45, 16 / 9, 0.1, 100)

		local view: sn.Mat4 = camera:getView()
		local projection: sn.Mat4 = camera:getProjection()
		local viewProjection: sn.Mat4 = camera:getViewProjection()
		local buffer: sn.Buffer = sn.Buffer.new({ view, projection, viewProjection })

		expect(buffer:size()):toBe(64 * 3)
	end)

	it("casts physics rays for hit scan queries", function()
		local world: sn.World3D = sn.World3D.new()
		local collider = world:newSphereCollider(sn.Vec3.new(0, 0, 5), 1, true)
		world:addCollider(collider, false)

		local ray = sn.Ray.new(sn.Vec3.new(0), sn.Vec3.new(0, 0, 1))
		local hit = world:raycastClosest(ray, 10)

		expect(hit):toBeTruthy()
		if hit then
			expect(hit.collider:isValid()):toBeTruthy()
			expect(hit.distance):toBeCloseTo(4)
		end
	end)
end)

return true
