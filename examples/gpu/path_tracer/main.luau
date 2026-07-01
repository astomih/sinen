local sn = require("@sinen")

local width = 512
local height = 200

local font: sn.Font = sn.Font.new(22)
local outputTexture: sn.Texture = sn.Texture.new(width, height)
local outputBuffer: sn.ComputeBuffer = sn.ComputeBuffer.new(width * height * 4)

local supported = sn.Raytracing.isDeviceSupported()
local status = if supported
	then "Building path tracing scene..."
	else "Ray tracing is not supported by this backend/device"

local raygen: sn.Shader? = nil
local miss: sn.Shader? = nil
local closestHit: sn.Shader? = nil
local pipeline: sn.RaytracingPipeline? = nil

if supported then
	raygen = sn.Shader.compileAndLoad("path_tracer.slang", sn.ShaderStage.RayGeneration)
	miss = sn.Shader.compileAndLoad("path_tracer.slang", sn.ShaderStage.Miss)
	closestHit = sn.Shader.compileAndLoad("path_tracer.slang", sn.ShaderStage.ClosestHit)

	local p = sn.Raytracing.Pipeline.new()
	p:addShader(raygen :: sn.Shader, "RayGenMain")
	p:addShader(miss :: sn.Shader, "MissMain")
	p:addShader(closestHit :: sn.Shader, "ClosestHitMain")
	p:addHitGroup("TriangleHitGroup", 2)
	p:setMaxPayloadSize(64)
	p:setMaxAttributeSize(8)
	p:setMaxRecursionDepth(3)
	p:build()
	pipeline = p
end

local vertices = {}
local sceneFloats = { 0.0, 0.0, 1.93, 0.15 }

local function v(x: number, y: number, z: number): sn.Vec3
	return sn.Vec3.new(x, y, z)
end

local function pushTriangleData(a: sn.Vec3, b: sn.Vec3, c: sn.Vec3, material: number)
	table.insert(sceneFloats, a.x)
	table.insert(sceneFloats, a.y)
	table.insert(sceneFloats, a.z)
	table.insert(sceneFloats, material)
	table.insert(sceneFloats, b.x)
	table.insert(sceneFloats, b.y)
	table.insert(sceneFloats, b.z)
	table.insert(sceneFloats, 0.0)
	table.insert(sceneFloats, c.x)
	table.insert(sceneFloats, c.y)
	table.insert(sceneFloats, c.z)
	table.insert(sceneFloats, 0.0)
end

local function addTri(a: sn.Vec3, b: sn.Vec3, c: sn.Vec3, material: number)
	table.insert(vertices, a)
	table.insert(vertices, b)
	table.insert(vertices, c)
	pushTriangleData(a, b, c, material)
end

local function addQuad(a: sn.Vec3, b: sn.Vec3, c: sn.Vec3, d: sn.Vec3, material: number)
	addTri(a, b, c, material)
	addTri(a, c, d, material)
end

local function addBox(minCorner: sn.Vec3, maxCorner: sn.Vec3, material: number)
	local x0, y0, z0 = minCorner.x, minCorner.y, minCorner.z
	local x1, y1, z1 = maxCorner.x, maxCorner.y, maxCorner.z
	addQuad(v(x0, y0, z1), v(x1, y0, z1), v(x1, y1, z1), v(x0, y1, z1), material)
	addQuad(v(x1, y0, z0), v(x0, y0, z0), v(x0, y1, z0), v(x1, y1, z0), material)
	addQuad(v(x0, y0, z0), v(x0, y0, z1), v(x0, y1, z1), v(x0, y1, z0), material)
	addQuad(v(x1, y0, z1), v(x1, y0, z0), v(x1, y1, z0), v(x1, y1, z1), material)
	addQuad(v(x0, y1, z1), v(x1, y1, z1), v(x1, y1, z0), v(x0, y1, z0), material)
	addQuad(v(x0, y0, z0), v(x1, y0, z0), v(x1, y0, z1), v(x0, y0, z1), material)
end

local function spherePoint(
	center: sn.Vec3,
	radius: number,
	ring: number,
	segment: number,
	rings: number,
	segments: number
): sn.Vec3
	local theta = math.pi * ring / rings
	local phi = math.pi * 2.0 * segment / segments
	local y = math.cos(theta) * radius
	local r = math.sin(theta) * radius
	return v(center.x + math.cos(phi) * r, center.y + y, center.z + math.sin(phi) * r)
end

local function addSphere(center: sn.Vec3, radius: number, rings: number, segments: number, material: number)
	for ring = 0, rings - 1 do
		for segment = 0, segments - 1 do
			local a = spherePoint(center, radius, ring, segment, rings, segments)
			local b = spherePoint(center, radius, ring + 1, segment, rings, segments)
			local c = spherePoint(center, radius, ring + 1, segment + 1, rings, segments)
			local d = spherePoint(center, radius, ring, segment + 1, rings, segments)
			if ring > 0 then
				addTri(a, b, d, material)
			end
			if ring < rings - 1 then
				addTri(d, b, c, material)
			end
		end
	end
end

local function addMaterial(albedo: sn.Vec3, metallic: number, emission: sn.Vec3, roughness: number)
	table.insert(sceneFloats, albedo.x)
	table.insert(sceneFloats, albedo.y)
	table.insert(sceneFloats, albedo.z)
	table.insert(sceneFloats, metallic)
	table.insert(sceneFloats, emission.x)
	table.insert(sceneFloats, emission.y)
	table.insert(sceneFloats, emission.z)
	table.insert(sceneFloats, roughness)
end

addQuad(v(-2.0, 0.0, -2.0), v(2.0, 0.0, -2.0), v(2.0, 0.0, 2.0), v(-2.0, 0.0, 2.0), 0)
addQuad(v(-2.0, 2.2, 2.0), v(2.0, 2.2, 2.0), v(2.0, 2.2, -2.0), v(-2.0, 2.2, -2.0), 0)
addQuad(v(-2.0, 0.0, 2.0), v(2.0, 0.0, 2.0), v(2.0, 2.2, 2.0), v(-2.0, 2.2, 2.0), 0)
addQuad(v(-2.0, 0.0, -2.0), v(-2.0, 0.0, 2.0), v(-2.0, 2.2, 2.0), v(-2.0, 2.2, -2.0), 1)
addQuad(v(2.0, 0.0, 2.0), v(2.0, 0.0, -2.0), v(2.0, 2.2, -2.0), v(2.0, 2.2, 2.0), 2)
addQuad(v(-0.45, 2.185, -0.35), v(0.45, 2.185, -0.35), v(0.45, 2.185, 0.65), v(-0.45, 2.185, 0.65), 6)

addBox(v(-1.35, 0.0, 0.2), v(-0.55, 0.95, 1.05), 3)
addBox(v(0.65, 0.0, 0.35), v(1.28, 0.58, 1.22), 4)
addSphere(v(0.35, 0.42, -0.62), 0.42, 8, 16, 5)

local materialBase = #sceneFloats * 4
sceneFloats[1] = materialBase
addMaterial(v(0.78, 0.75, 0.68), 0.0, v(0.0, 0.0, 0.0), 0.85)
addMaterial(v(0.80, 0.16, 0.10), 0.0, v(0.0, 0.0, 0.0), 0.9)
addMaterial(v(0.10, 0.23, 0.82), 0.0, v(0.0, 0.0, 0.0), 0.9)
addMaterial(v(0.20, 0.72, 0.45), 0.0, v(0.0, 0.0, 0.0), 0.65)
addMaterial(v(1.00, 0.73, 0.38), 0.82, v(0.0, 0.0, 0.0), 0.18)
addMaterial(v(0.86, 0.90, 1.00), 0.65, v(0.0, 0.0, 0.0), 0.05)
addMaterial(v(1.0, 0.86, 0.62), 0.0, v(12.0, 10.0, 7.0), 0.0)

local vertexBuffer: sn.ComputeBuffer = sn.ComputeBuffer.new(sn.Buffer.new(vertices))
local sceneDataBuffer: sn.ComputeBuffer = sn.ComputeBuffer.new(sn.Buffer.new(sceneFloats))

local blas: sn.RaytracingAccelerationStructure? = nil
local tlas: sn.RaytracingAccelerationStructure? = nil
local shaderTable: sn.ComputeBuffer? = nil
local rendered = false

local function render()
	if not supported or pipeline == nil then
		return
	end
	local p = pipeline :: sn.RaytracingPipeline

	if blas == nil then
		blas = sn.Raytracing.createBottomLevel({
			{
				vertexBuffer = vertexBuffer,
				vertexStride = 12,
				vertexCount = #vertices,
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
			storageBuffers = { outputBuffer, sceneDataBuffer },
			rayGeneration = { offset = 0, size = handleSize, stride = handleSize },
			miss = { offset = handleSize, size = handleSize, stride = handleSize },
			hit = { offset = handleSize * 2, size = handleSize, stride = handleSize },
		})
	end

	local pixels = outputBuffer:download()
	if outputTexture:updatePixels(pixels, width, height, "rgba8") then
		status = "Path traced Cornell scene"
	else
		status = "Path tracing finished, but texture upload failed"
	end
	rendered = true
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/gpu/main.luau")
	end

	if pipeline == nil or not supported then
		return
	end

	if not rendered and pipeline:isReady() then
		status = "Tracing a device-safe preview..."
		render()
	end
end

function draw()
	local center = sn.Window.center()
	local windowSize = sn.Window.size()
	local scale = math.min(windowSize.x / width, (windowSize.y - 72.0) / height)
	local imageRect = sn.Rect.new(sn.Pivot.Center, center, sn.Vec2.new(width * scale, height * scale))

	if rendered then
		sn.Graphics.drawImage(outputTexture, imageRect)
	end

	local textPosition = sn.Vec2.new(center.x - 300, imageRect.y + imageRect.height + 20)
	sn.Graphics.drawText(status, sn.TextStyle.new(font, sn.Color.new(1.0), 22), sn.TextTransform.new(textPosition))
end
