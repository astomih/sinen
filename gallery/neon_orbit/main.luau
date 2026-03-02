local sn = require("@sinen")
local titleFont: sn.Font = sn.Font.new()
local infoFont: sn.Font = sn.Font.new()

local paletteShift = 0.0
local mode = 1

local function saturate(v: number): number
	if v < 0.0 then
		return 0.0
	end
	if v > 1.0 then
		return 1.0
	end
	return v
end

local function wave(seed: number, t: number): number
	return math.sin(seed + t) * 0.5 + 0.5
end

local function modeName(): string
	if mode == 1 then
		return "CALM"
	end
	if mode == 2 then
		return "RAPID"
	end
	return "HYPER"
end

local function particleCount(): number
	if mode == 1 then
		return 90
	end
	if mode == 2 then
		return 150
	end
	return 240
end

function setup()
	sn.Graphics.resetGraphicsPipeline()
	sn.Graphics.getCamera2D():resize(sn.Vec2.new(1280, 720))
	titleFont:load(44)
	infoFont:load(20)
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		sn.Script.load("main", ".")
		return
	end

	if sn.Keyboard.isPressed(sn.Keyboard.SPACE) then
		mode = mode + 1
		if mode > 3 then
			mode = 1
		end
	end

	if sn.Mouse.isPressed(sn.Mouse.LEFT) then
		paletteShift = paletteShift + 0.9
	end
end

local function drawBackground(t: number, mx: number, my: number)
	local half = sn.Graphics.getCamera2D():half()
	local width = half.x * 2.0
	local step = 24.0
	local y = -half.y

	while y <= half.y do
		local fy = (y + half.y) / (half.y * 2.0)
		local drift = wave(fy * 7.0 + paletteShift, t * 0.7 + my * 0.002)
		local r = 0.05 + drift * 0.18
		local g = 0.05 + (1.0 - drift) * 0.16
		local b = 0.07 + wave(fy * 9.0, t + mx * 0.001) * 0.24

		sn.Graphics.drawRect(
			sn.Rect.new(sn.Vec2.new(0, y), sn.Vec2.new(width + 2.0, step + 1.0)),
			sn.Color.new(saturate(r), saturate(g), saturate(b), 1.0)
		)
		y = y + step
	end
end

local function drawSwarm(t: number, mx: number, my: number)
	local count = particleCount()
	local modeBoost = 0.7 + mode * 0.45

	for i = 1, count do
		local fi = i * 0.1
		local angle = fi * 1.7 + t * (0.4 + fi * 0.01) * modeBoost
		local orbit = 40.0 + fi * 2.3 + math.sin(t * (1.4 + fi * 0.005) + fi) * 20.0
		local wobbleX = math.sin(t * 0.8 + fi * 3.1) * 22.0
		local wobbleY = math.cos(t * 0.6 + fi * 2.2) * 16.0
		local x = math.cos(angle) * orbit + mx * 0.24 + wobbleX
		local y = math.sin(angle) * orbit + my * 0.24 + wobbleY
		local sz = 3.0 + wave(fi * 2.8, t * 2.8) * (7.0 + mode * 2.5)

		local r = 0.2 + wave(fi + paletteShift, t * 0.8) * 0.75
		local g = 0.15 + wave(fi * 1.8 + 2.0 + paletteShift, t * 0.9) * 0.75
		local b = 0.25 + wave(fi * 1.3 + 4.0 + paletteShift, t * 1.1) * 0.7
		local a = 0.2 + wave(fi * 2.1, t * 1.5) * 0.6

		sn.Graphics.drawRect(
			sn.Rect.new(sn.Vec2.new(x, y), sn.Vec2.new(sz, sz)),
			sn.Color.new(saturate(r), saturate(g), saturate(b), saturate(a))
		)
	end
end

local function drawCore(t: number, mx: number, my: number)
	for i = 1, 12 do
		local size = 38.0 + i * 18.0 + math.sin(t * 1.6 + i * 0.6) * 8.0
		local alpha = 0.06 + (12 - i) * 0.008
		local cx = mx * (0.04 + i * 0.006)
		local cy = my * (0.04 + i * 0.006)

		sn.Graphics.drawRect(
			sn.Rect.new(sn.Vec2.new(cx, cy), sn.Vec2.new(size, size)),
			sn.Color.new(0.85, 0.95, 1.0, saturate(alpha))
		)
	end
end

function draw()
	local t = sn.Time.seconds()
	local m = sn.Mouse.getPositionOnScene()
	local mx = m.x
	local my = m.y

	drawBackground(t, mx, my)
	drawSwarm(t, mx, my)
	drawCore(t, mx, my)

	sn.Graphics.drawRect(sn.Rect.new(sn.Vec2.new(mx, my), sn.Vec2.new(12, 12)), sn.Color.new(1, 1, 1, 0.9))

	sn.Graphics.drawText("NEON ORBIT", titleFont, sn.Vec2.new(0, 320), sn.Color.new(1, 1, 1, 0.95), 44)
	sn.Graphics.drawText(
		"SPACE: MODE  LEFT CLICK: PALETTE SHIFT  ESC: BACK",
		infoFont,
		sn.Vec2.new(0, -330),
		sn.Color.new(1, 1, 1, 0.85),
		20
	)
	sn.Graphics.drawText("MODE: " .. modeName(), infoFont, sn.Vec2.new(0, 285), sn.Color.new(0.9, 1.0, 1.0, 0.9), 20)
end
