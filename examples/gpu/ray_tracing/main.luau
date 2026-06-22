local sn = require("@sinen")

local width = 384
local height = 288

local font: sn.Font = sn.Font.new(22)

local outputTexture: sn.Texture = sn.Texture.new(width, height)
local outputBuffer: sn.ComputeBuffer = sn.ComputeBuffer.new(width * height * 4)
local supported = sn.Raytracing.isDeviceSupported()
local status = if supported
	then "Compiling ray tracing shaders..."
	else "Ray tracing is not supported by this backend/device"

local raygen: sn.Shader = sn.Shader.compileAndLoad("raytrace.slang", sn.ShaderStage.RayGeneration)
local miss: sn.Shader = sn.Shader.compileAndLoad("raytrace.slang", sn.ShaderStage.Miss)
local closestHit: sn.Shader = sn.Shader.compileAndLoad("raytrace.slang", sn.ShaderStage.ClosestHit)
local pipeline: sn.RaytracingPipeline? = nil
if supported then

	local p = sn.Raytracing.Pipeline.new()
	p:addShader(raygen, "RayGenMain")
	p:addShader(miss, "MissMain")
	p:addShader(closestHit, "ClosestHitMain")
	p:addHitGroup("TriangleHitGroup", 2)
	p:setMaxPayloadSize(16)
	p:setMaxAttributeSize(8)
	p:setMaxRecursionDepth(1)
	p:build()
	pipeline = p
end

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

local function render()
	if not supported or pipeline == nil then
		status = "Ray tracing is not supported by this backend/device"
		return
	end
	local p = pipeline :: sn.RaytracingPipeline

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
		local handles = p:getShaderGroupHandles(0, 3)
		shaderTable = sn.ComputeBuffer.new(handles)
	end

	local handleSize = p:getShaderGroupHandleSize()
	if tlas ~= nil then
		sn.Raytracing.dispatch(p, {
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
	if outputTexture:updatePixels(pixels, width, height, "rgba8") then
		status = "Ray traced triangle"
	else
		status = "Ray tracing finished, but texture upload failed"
	end
	rendered = true
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end

	if pipeline == nil or not supported then
		return
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
	sn.Graphics.drawText(
		status,
		sn.TextStyle.new(font, sn.Color.new(1.0), 22),
		sn.TextTransform.new(textPosition)
	)
end
