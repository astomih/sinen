local sn = require("@sinen")
local info = require("./module/archive_info")

local font: sn.Font = sn.Font.new()
local rootEntries = {}
local moduleEntries = {}

function setup()
	font:load(24)
	rootEntries = sn.Filesystem.enumerateDirectory(".")
	moduleEntries = sn.Filesystem.enumerateDirectory("module")
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("main", ".")
	end
end

local function join(values: { string }): string
	local text = ""
	for i, value in values do
		if i > 1 then
			text ..= ", "
		end
		text ..= value
	end
	return text
end

function draw()
	local lines = {
		"Loaded from .sna archive",
		"Archive root folder: " .. info.root,
		"Required module says: " .. info.message,
		"Root entries: " .. join(rootEntries),
		"module entries: " .. join(moduleEntries),
		"",
		"ESC returns to the normal example launcher when running unpacked.",
	}

	local y = 80
	for _, line in lines do
		sn.Graphics.drawText(line, font, sn.Vec2.new(60, y), sn.Color.new(1.0), 24)
		y += 34
	end
end
