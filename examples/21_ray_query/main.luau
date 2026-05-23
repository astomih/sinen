local sn = require("@sinen")

local width = 384
local height = 288
local threadGroupSize = 8

local font: sn.Font = sn.Font.new()
font:load(22)

local shader: sn.Shader = sn.Shader.new()
local pipeline: sn.ComputePipeline = sn.ComputePipeline.new()
local outputTexture: sn.Texture = sn.Texture.new(width, height)
local outputBuffer: sn.ComputeBuffer = sn.ComputeBuffer.new(width * height * 4)

local vertexData = sn.Buffer.new({
	sn.Vec3.new(-0.7, -0.6, 0.0),
	sn.Vec3.new(0.7, -0.6, 0.0),
	sn.Vec3.new(0.0, 0.65, 0.0),
})
local vertexBuffer = sn.ComputeBuffer.new(vertexData)

local blas: sn.RaytracingAccelerationStructure? = nil
local tlas: sn.RaytracingAccelerationStructure? = nil
local built = false
local rendered = false
local supported = sn.Raytracing.isRayQuerySupported()
local status = if supported
	then "Compiling ray query compute shader..."
	else "Ray query is not supported by this backend/device"

if supported then
	shader:compileAndLoad("ray_query.slang", sn.ShaderStage.Compute)
end

local function buildAccelerationStructures()
	if blas == nil then
		blas = sn.Raytracing.createBottomLevel({
			{
				vertexBuffer = vertexBuffer,
				vertexStride = 12,
				vertexCount = 3,
			} :: sn.RaytracingTriangleGeometry,
		})
	end

	if tlas == nil and blas ~= nil then
		tlas = sn.Raytracing.createTopLevel({
			{
				accelerationStructure = blas,
				mask = 0xff,
			},
		})
	end
end

local function render()
	buildAccelerationStructures()

	sn.Compute.setComputePipeline(pipeline)
	sn.Compute.setStorageBuffer(0, outputBuffer)
	if tlas ~= nil then
		sn.Raytracing.setComputeAccelerationStructure(0, tlas)
	end
	sn.Compute.dispatch(width / threadGroupSize, height / threadGroupSize, 1)

	local pixels = outputBuffer:download()
	if outputTexture:loadPixels(pixels, width, height, "rgba8") then
		status = "Ray query triangle from a compute shader"
	else
		status = "Ray query finished, but texture upload failed"
	end
	rendered = true
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("main", ".")
	end

	if not supported then
		return
	end

	if not built and shader:isReady() then
		pipeline:setShader(shader)
		pipeline:setThreadGroupSize(threadGroupSize, threadGroupSize, 1)
		pipeline:build()
		built = true
		status = "Dispatching ray query compute shader..."
	end

	if built and not rendered then
		render()
	end
end

function draw()
	local center = sn.Window.center()
	local imageRect = sn.Rect.new(sn.Pivot.Center, center, sn.Vec2.new(width, height))

	if rendered then
		sn.Graphics.drawImage(outputTexture, imageRect)
	end

	local textPosition = sn.Vec2.new(center.x - 300, imageRect.y + imageRect.height + 24)
	sn.Graphics.drawText(status, font, textPosition, sn.Color.new(1.0), 22)
end
