local sn = require("@sinen")

local width = 384
local height = 288

local font: sn.Font = sn.Font.new()
font:load(22)

local outputTexture: sn.Texture = sn.Texture.new(width, height)
local outputBuffer: sn.ComputeBuffer = sn.ComputeBuffer.new(width * height * 4)

local raygen: sn.Shader = sn.Shader.new()
local miss: sn.Shader = sn.Shader.new()
local closestHit: sn.Shader = sn.Shader.new()
raygen:compileAndLoad("raytrace.slang", sn.ShaderStage.RayGeneration)
miss:compileAndLoad("raytrace.slang", sn.ShaderStage.Miss)
closestHit:compileAndLoad("raytrace.slang", sn.ShaderStage.ClosestHit)

local pipeline = sn.Raytracing.Pipeline.new()
pipeline:addShader(raygen, "RayGenMain")
pipeline:addShader(miss, "MissMain")
pipeline:addShader(closestHit, "ClosestHitMain")
pipeline:addHitGroup("TriangleHitGroup", 2)
pipeline:setMaxPayloadSize(16)
pipeline:setMaxAttributeSize(8)
pipeline:setMaxRecursionDepth(1)
pipeline:build()

local vertexData = sn.Buffer.new({
	sn.Vec3.new(-0.7, -0.6, 0.0),
	sn.Vec3.new(0.7, -0.6, 0.0),
	sn.Vec3.new(0.0, 0.65, 0.0),
})
local vertexBuffer = sn.ComputeBuffer.new(vertexData)

local blas: sn.RaytracingAccelerationStructure? = nil
local tlas: sn.RaytracingAccelerationStructure? = nil
local shaderTable: sn.ComputeBuffer? = nil
local rendered = false
local status = "Checking ray tracing support..."

local function render()
	if not sn.Raytracing.isSupported() then
		status = "Ray tracing is not supported by this backend/device"
		return
	end

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

	if shaderTable == nil then
		local handles = pipeline:getShaderGroupHandles(0, 3)
		shaderTable = sn.ComputeBuffer.new(handles)
	end

	local handleSize = pipeline:getShaderGroupHandleSize()
	if tlas ~= nil then
		sn.Raytracing.dispatch(pipeline, {
			shaderTable = shaderTable :: sn.ComputeBuffer,
			width = width,
			height = height,
			accelerationStructures = { tlas },
			storageBuffers = { outputBuffer },
			rayGeneration = { offset = 0, size = handleSize, stride = handleSize },
			miss = { offset = handleSize, size = handleSize, stride = handleSize },
			hit = { offset = handleSize * 2, size = handleSize, stride = handleSize },
		})
	end

	local pixels = outputBuffer:download()
	if outputTexture:loadPixels(pixels, width, height, "rgba8") then
		status = "Ray traced triangle"
	else
		status = "Ray tracing finished, but texture upload failed"
	end
	rendered = true
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("main", ".")
	end

	if not rendered and pipeline:isReady() then
		status = "Dispatching rays..."
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
