local sn = require("@sinen")

local width = 128
local height = 128
local threadGroupSize = 8

local font: sn.Font = sn.Font.new()
font:load(24)

local shader: sn.Shader = sn.Shader.new()
shader:compileAndLoad("compute_image.slang", sn.ShaderStage.Compute)

local pipeline: sn.ComputePipeline = sn.ComputePipeline.new()
local outputBuffer: sn.ComputeBuffer = sn.ComputeBuffer.new(width * height * 4)
local outputTexture: sn.Texture = sn.Texture.new(width, height)

local built = false
local dispatched = false
local status = "Compiling compute shader..."

local function dispatchCompute()
	sn.Compute.setComputePipeline(pipeline)
	sn.Compute.setStorageBuffer(0, outputBuffer)
	sn.Compute.dispatch(width / threadGroupSize, height / threadGroupSize, 1)

	local pixels = outputBuffer:download()
	if outputTexture:loadPixels(pixels, width, height, "rgba8") then
		status = "Compute shader generated an RGBA texture"
	else
		status = "Compute dispatch finished, but texture upload failed"
	end
	dispatched = true
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end

	if not built and shader:isReady() then
		pipeline:setShader(shader)
		pipeline:setThreadGroupSize(threadGroupSize, threadGroupSize, 1)
		pipeline:build()
		built = true
		status = "Dispatching compute shader..."
	end

	if built and not dispatched then
		dispatchCompute()
	end
end

function draw()
	local center = sn.Window.center()
	local scale = 3
	local imageSize = sn.Vec2.new(width * scale, height * scale)
	local imageRect = sn.Rect.new(sn.Pivot.Center, center, imageSize)

	if dispatched then
		sn.Graphics.drawImage(outputTexture, imageRect)
	end

	local textPosition = sn.Vec2.new(center.x - 240, imageRect.y + imageRect.height + 24)
	sn.Graphics.drawText(
		status,
		sn.TextStyle.new(font, sn.Color.new(1.0), 24),
		sn.TextTransform.new(textPosition)
	)
end
