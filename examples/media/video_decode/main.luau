local sn = require("@sinen")

local font = sn.Font.new()
font:load(24)

local reader = sn.VideoReader.new()
local texture = sn.Texture.new()
local info = nil
local status = ""
local frameIndex = -1
local playing = true
local videoPath = "video_demo.avi"

local function openVideo()
	if not reader:open(videoPath) then
		videoPath = "video_demo.avi"
		if not reader:open(videoPath) then
			status = "open failed: " .. reader:lastError()
			return
		end
	end

	info = reader:info()
	status = "Loaded " .. videoPath .. " (" .. info.width .. "x" .. info.height .. ", " .. info.frameCount .. " frames)"
end

local function uploadFrame(index: number)
	if not info then
		return
	end

	local frame = reader:readFrame(index)
	if not frame then
		status = "decode failed at frame " .. index
		return
	end

	if texture:loadPixels(frame.pixels, frame.width, frame.height, "rgba8") then
		frameIndex = index
	else
		status = "texture upload failed at frame " .. index
	end
end

function setup()
	openVideo()
	uploadFrame(0)
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		reader:close()
		sn.Script.load("main", ".")
	end
	if sn.Keyboard.isPressed(sn.Scancode.SPACE) then
		playing = not playing
	end

	if info and playing and info.frameCount > 0 then
		local fps = math.max(info.fps, 1)
		local nextFrame = math.floor(sn.Time.seconds() * fps) % info.frameCount
		if nextFrame ~= frameIndex then
			uploadFrame(nextFrame)
		end
	end
end

local function drawCentered(text: string, y: number, size: number, color)
	sn.Graphics.drawText(
		text,
		font,
		font:region(text, size, sn.Pivot.Center, sn.Window.center() + sn.Vec2.new(0, y)):topLeft(),
		color,
		size
	)
end

function draw()
	local center = sn.Window.center()
	if info then
		local displayWidth = 640
		local displayHeight = displayWidth * info.height / info.width
		sn.Graphics.drawImage(
			texture,
			sn.Rect.new(sn.Pivot.Center, center + sn.Vec2.new(0, -18), sn.Vec2.new(displayWidth, displayHeight))
		)
		drawCentered(
			"Frame " .. (frameIndex + 1) .. " / " .. info.frameCount,
			displayHeight * 0.5 + 10,
			16,
			sn.Color.new(1)
		)
	end

	drawCentered("Video decode demo", -230, 24, sn.Color.new(1))
	drawCentered(status, 220, 14, sn.Color.new(0.8, 0.9, 1.0, 1))
	drawCentered("SPACE: play/pause / ESC: back", 246, 14, sn.Color.new(0.75, 0.75, 0.75, 1))
end
