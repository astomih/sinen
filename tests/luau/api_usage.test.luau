--!strict

local sn = require("@sinen")
local test = require("./sinen_test")

local describe = test.describe
local it = test.it
local expect = test.expect

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
		timer:setTime(1.0)

		expect(transform.position.y):toBeCloseTo(2)
		expect(timer:isStarted()):toBeFalsy()
	end)
end)

return true
