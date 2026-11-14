local sn = require("@sinen")

local games = {
	{ title = "FOUNDRY", genre = "PBR FPS  /  Drone survival", path = "/fps/main.luau" },
	{ title = "RIDGELINE", genre = "PBR RACING  /  Three-lap circuit race", path = "/racing/main.luau" },
}

local v2, rect, color = sn.Vec2.new, sn.Rect.new, sn.Color.new
local keyboard, keys = sn.Keyboard, sn.Scancode
local pad, buttons = sn.Gamepad, sn.GamepadButton
local font = sn.Font.new(32)
local white = color(0.93, 0.96, 0.95, 1)
local muted = color(0.69, 0.76, 0.77, 1)
local accent = color(0.29, 0.89, 0.91, 1)
local background = color(0.025, 0.044, 0.052, 1)
local heading = sn.TextStyle.new(font, white, 42)
local description = sn.TextStyle.new(font, muted, 16)
local instructions = sn.TextStyle.new(font, accent, 15)
local selected = 1
local stickDirection = 0
local loading = false

local function launch()
	if not loading then
		loading = true
		sn.Script.load(games[selected].path)
	end
end

function setup()
	sn.Window.rename("Sinen | Games")
	sn.Mouse.setRelative(false)
	sn.Mouse.hideCursor(false)
	sn.Graphics.resetGraphicsPipeline()
	sn.Graphics.setClearColor(background)
end

function update()
	if loading or not sn.Window.hasFocus() then return end

	local stick = pad.getLeftStick().y
	local direction = if stick > 0.55 then -1 elseif stick < -0.55 then 1 else 0
	local move = if direction ~= stickDirection then direction else 0
	stickDirection = direction
	if keyboard.isPressed(keys.UP) or pad.isPressed(buttons.DPAD_UP) then
		move = -1
	elseif keyboard.isPressed(keys.DOWN) or pad.isPressed(buttons.DPAD_DOWN) then
		move = 1
	end
	selected = (selected - 1 + move) % #games + 1

	if keyboard.isPressed(keys.ESCAPE) or pad.isPressed(buttons.B) then
		sn.Event.quit()
	elseif keyboard.isPressed(keys.ENTER) or pad.isPressed(buttons.A) then
		launch()
	end
end

function draw()
	local size = sn.Window.size()
	local w = math.max(1, math.min(480, size.x - 36))
	local x, y = (size.x - w) * 0.5, math.max(8, (size.y - 296) * 0.5)
	local center = size.x * 0.5
	local function text(value: string, style: sn.TextStyle, py: number)
		sn.Graphics.drawText(value, style, sn.TextTransform.new(v2(center, py), 0, sn.Pivot.TopCenter))
	end

	sn.Graphics.drawRect(rect(0, 0, size.x, size.y), background)
	text("Sinen Games", heading, y)
	sn.Gui.setFont(font)
	sn.Gui.setFontSize(24)
	for i, game in games do
		local row = y + 74 + (i - 1) * 88
		local buttonColor = if selected == i then color(0.13, 0.29, 0.31, 1) else color(0.073, 0.13, 0.15, 1)
		sn.Gui.setThemeColor(buttonColor, color(0.16, 0.34, 0.36, 1), color(0.18, 0.42, 0.43, 1), white, accent)
		if not loading and sn.Gui.button(game.title, rect(x, row, w, 52)) then
			selected = i
			launch()
		end
		text(game.genre, description, row + 54)
	end
	local hint = if pad.isConnected() then "D-Pad / Stick  Select     A / Click  Play     B  Quit"
		else "Up / Down  Select     Enter / Click  Play     Esc  Quit"
	text(hint, instructions, y + 266)
end
