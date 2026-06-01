local sn = require("@sinen")

local font: sn.Font = sn.Font.new()
font:load(24)
local width = 96
local height = 54
local frames = 24
local outputPath = "video_encode_demo.avi"
local status = "Press SPACE to encode " .. frames .. " RGBA frames to " .. outputPath
local verified = ""

local function makeFrame(frameIndex: number)
	local bytes: { number } = table.create(width * height * 4, 0.0)
	local t = frameIndex / frames
	local k = 1

	for y = 0, height - 1 do
		for x = 0, width - 1 do
			local wave = math.floor((math.sin((x * 0.14) + (frameIndex * 0.35)) * 0.5 + 0.5) * 255)
			local r = math.floor(((x / (width - 1)) * 210) + (45 * t))
			local g = math.floor((y / (height - 1)) * 255)
			local b = wave

			bytes[k] = r
			bytes[k + 1] = g
			bytes[k + 2] = b
			bytes[k + 3] = 255
			k += 4
		end
	end

	return sn.Buffer.fromBytes(bytes)
end

local function encode()
	local writer = sn.VideoWriter.new()
	if not writer:open(outputPath, width, height, 12) then
		status = "open failed: " .. writer:lastError()
		return
	end

	for i = 0, frames - 1 do
		if not writer:addFrame(makeFrame(i), width, height, "rgba8", 90) then
			status = "encode failed: " .. writer:lastError()
			writer:close()
			return
		end
	end

	if not writer:close() then
		status = "finalize failed: " .. writer:lastError()
		return
	end

	local reader = sn.VideoReader.new()
	if not reader:open(outputPath) then
		status = "readback failed: " .. reader:lastError()
		return
	end

	local info = reader:info()
	local first = reader:readFrame(0)
	reader:close()

	status = "Encoded " .. info.frameCount .. " frames to " .. outputPath
	if first then
		verified = "Readback OK: " .. info.width .. "x" .. info.height .. " @ " .. info.fps .. "fps"
	else
		verified = "Readback opened, but frame decode failed"
	end
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
	if sn.Keyboard.isPressed(sn.Scancode.SPACE) then
		encode()
	end
end

local function drawCentered(text: string, y: number, size: number, color)
	sn.Graphics.drawText(
		text,
		sn.TextStyle.new(font, color, size),
		sn.TextTransform.new(sn.Window.center() + sn.Vec2.new(0, y), 0.0, sn.Pivot.Center)
	)
end

function draw()
	drawCentered("Video encode demo", -44, 24, sn.Color.new(1))
	drawCentered(status, -8, 16, sn.Color.new(1))
	drawCentered(verified, 18, 16, sn.Color.new(0.6, 1.0, 0.7, 1))
	drawCentered("SPACE: encode / ESC: back", 48, 14, sn.Color.new(0.75, 0.75, 0.75, 1))
end
