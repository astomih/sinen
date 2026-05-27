--!strict

local sn = require("@sinen")
local test = require("./sinen_test")

test.installGlobals()

local function endsWith(value: string, suffix: string): boolean
	if #value < #suffix then
		return false
	end
	return string.sub(value, #value - #suffix + 1) == suffix
end

local function loadTests()
	local files = sn.Filesystem.enumerateDirectory(".")
	table.sort(files)

	for _, file in ipairs(files) do
		if endsWith(file, ".test.luau") then
			local ok, err = pcall(function()
				require("./" .. file)
			end)

			if not ok then
				test.it("load " .. file, function()
					error(err, 0)
				end)
			end
		end
	end
end

loadTests()

local font: sn.Font = sn.Font.new()
local result: any = nil

function setup()
	font:load(32)
	result = test.run({
		logInfo = function(message: string)
			sn.Log.info(message)
		end,
		logError = function(message: string)
			sn.Log.error(message)
		end,
	})
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
end

function draw()
	local lines = if result then result.lines else { "Running Luau unit tests..." }
	local titleColor = if result and not result.ok
		then sn.Color.new(1.0, 0.25, 0.25, 1.0)
		else sn.Color.new(0.35, 1.0, 0.55, 1.0)
	local bodyColor = sn.Color.new(1.0)
	local failColor = sn.Color.new(1.0, 0.35, 0.35, 1.0)
	local y = 24

	for index, line in ipairs(lines) do
		local color = bodyColor
		if index == 1 then
			color = titleColor
		elseif string.sub(line, 1, 6) == "[FAIL]" then
			color = failColor
		end

		sn.Graphics.drawText(line, font, sn.Vec2.new(24, y), color, 18)
		y += 24
	end
end
