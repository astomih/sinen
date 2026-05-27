local sn = require("@sinen")
local font: sn.Font = sn.Font.new()

local payloadBase = nil
local lines = {
	"Sinen archive example",
	"",
	"Create an archive:",
	"  build_sna.bat",
	"",
	"Run it from this directory:",
	"  ..\\..\\..\\build\\msvc2026-debug\\sinen.exe archive_sample.sna",
	"",
	"This launcher preview runs the unpacked sna/ payload.",
}

local function directoryContains(path: string, expected: string): boolean
	local entries = sn.Filesystem.enumerateDirectory(path)
	for _, name in entries do
		if name == expected then
			return true
		end
	end
	return false
end

function setup()
	font:load(24)

	if directoryContains("sna", "main.luau") then
		payloadBase = "sna"
	end
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end

	if sn.Keyboard.isPressed(sn.Scancode.ENTER) and payloadBase then
		sn.Script.load(payloadBase .. "/main.luau")
	end
end

function draw()
	local y = 80
	for _, line in lines do
		sn.Graphics.drawText(line, font, sn.Vec2.new(60, y), sn.Color.new(1.0), 24)
		y += 34
	end

	local action = payloadBase and "Press Enter to preview sna/main.luau" or "sna/main.luau was not found"
	sn.Graphics.drawText(action, font, sn.Vec2.new(60, y + 24), sn.Color.new(0.5, 0.9, 1.0, 1.0), 24)
end
