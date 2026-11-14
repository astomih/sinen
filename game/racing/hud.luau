local sn = require("@sinen")
local Track = require("./track")
local Grid = require("./grid")
local Hud = {}
Hud.__index = Hud

local v2, rect, color, pivot = sn.Vec2.new, sn.Rect.new, sn.Color.new, sn.Pivot
local white, lime, light = color(0.92, 0.96, 0.94, 1), color(0.72, 0.95, 0.32, 1), color(0.65, 0.74, 0.75, 1)
local panel, recessed = color(0.026, 0.043, 0.045, 0.89), color(0.10, 0.15, 0.15, 0.95)

function Hud.new()
	local font = sn.Font.new(34)
	local route = {}
	for i = 0, 127 do
		table.insert(route, Track.at(Track.length * i / 128))
	end
	return setmetatable({
		route = route,
		title = sn.TextStyle.new(font, white, 51),
		big = sn.TextStyle.new(font, white, 37),
		medium = sn.TextStyle.new(font, white, 24),
		text = sn.TextStyle.new(font, light, 15),
		label = sn.TextStyle.new(font, lime, 14),
	}, Hud)
end

local function clock(t: number): string
	local ms = math.floor(t * 1000)
	return string.format("%02d:%02d.%03d", math.floor(ms / 60000), math.floor(ms / 1000) % 60, ms % 1000)
end

function Hud:draw(phase: string, field: { Grid.Driver }, laps: number, lapTime: number, last: number, best: number,
	elapsed: number, countdown: number, go: number, gamepad: string?)
	local pass = sn.Graphics.begin2DPass()
	local size = sn.Window.size()
	local w, h = size.x, size.y
	local mx, my = w / 2, h / 2
	local car = field[1].car
	local position = Grid.position(field)
	local function fill(x: number, y: number, width: number, height: number, tint: sn.Color)
		pass:drawRect(rect(x, y, width, height), tint)
	end
	local function text(value: string, style: sn.TextStyle, x: number, y: number, anchor: sn.Pivot?)
		pass:drawText(value, style, sn.TextTransform.new(v2(x, y), 0, anchor or pivot.TopLeft))
	end

	if phase == "racing" or phase == "countdown" then
		local margin = if w >= 720 then 28 else 12
		local height = if best > 0 then 113 else 91
		fill(margin, 24, 232, height, panel)
		fill(margin, 24, 3, height, lime)
		text("POSITION", self.label, margin + 15, 35)
		text("LAP", self.label, margin + 145, 35)
		text(string.format("%d / %d", position, #field), self.medium, margin + 15, 54)
		text(string.format("%d / %d", math.min(laps, car.lap + 1), laps), self.medium, margin + 145, 54)
		text(clock(lapTime), self.text, margin + 15, 87)
		if best > 0 then text("BEST  " .. clock(best), self.text, margin + 15, 107) end

		local x, y = w - 224 - margin, h - 121
		fill(x, y, 224, 94, panel)
		fill(x + 221, y, 3, 94, lime)
		text("KM/H", self.label, x + 127, y + 46)
		text(string.format("%03d", math.floor(car.speed * 3.6 + 0.5)), self.big, x + 14, y + 12)
		local signed = car.vx * math.sin(car.yaw) - car.vz * math.cos(car.yaw)
		text(if signed < -0.35 then "R" else tostring(car.gear), self.medium, x + 194, y + 17)
		fill(x + 15, y + 73, 192, 5, recessed)
		fill(x + 15, y + 73, 192 * math.clamp(car.rpm / 7500, 0, 1), 5, if car.rpm > 6500 then color(1, 0.38, 0.20, 1) else lime)

		if w >= 720 and h >= 450 then
			local mapX, mapY = w - margin - 150, 24
			fill(mapX, mapY, 150, 152, panel)
			local function project(px: number, pz: number): (number, number)
				return mapX + 34 + px * 0.48, mapY + 73 + pz * 0.43
			end
			for _, point in self.route do
				local px, pz = project(point.x, point.z)
				fill(px - 1.6, pz - 1.6, 3.2, 3.2, light)
			end
			for i = #field, 1, -1 do
				local px, pz = project(field[i].car.x, field[i].car.z)
				local dot = if i == 1 then 7 else 4
				fill(px - dot * 0.5, pz - dot * 0.5, dot, dot, if i == 1 then lime else white)
			end
		end

		if phase == "countdown" or go > 0 then
			fill(mx - 66, my - 130, 132, 82, panel)
			text(if phase == "countdown" then tostring(math.ceil(countdown)) else "GO", self.title, mx, my - 127, pivot.TopCenter)
		elseif car.progress >= car.nextGate * Track.gateSpacing + 4 then
			local hint = if gamepad then "Y   RESET TO TRACK" else "R   RESET TO TRACK"
			fill(mx - 177, 28, 354, 35, panel)
			text(hint, self.label, mx, 36, pivot.TopCenter)
		elseif math.abs(car.margin) > Track.halfWidth + 0.55 then
			text("OFF TRACK", self.label, mx, 30, pivot.TopCenter)
		end
	else
		fill(0, 0, w, h, color(0.011, 0.022, 0.025, 0.45))
		local width = math.min(w - 24, 608)
		local x, y = mx - width / 2, my - 160
		fill(x, y, width, 320, panel)
		fill(x, y, width, 3, lime)
		local title = if phase == "title" then "RIDGELINE" elseif phase == "paused" then "PAUSED" else "RACE COMPLETE"
		text(title, if w >= 590 then self.title else self.big, mx, y + 30, pivot.TopCenter)
		local subtitle = if phase == "title" then string.format("PINE RIDGE  /  %.2f KM  /  %d LAPS", Track.length / 1000, laps)
			elseif phase == "finished" then string.format("P%d / %d    %s", position, #field, clock(elapsed))
			elseif gamepad then "GAMEPAD CONNECTED" else "RACE ON HOLD"
		text(subtitle, self.medium, mx, y + 112, pivot.TopCenter)
		local action = if phase == "title" then "START RACE" elseif phase == "paused" then "RESUME" else "RACE AGAIN"
		fill(mx - 144, y + 151, 288, 40, recessed)
		text((if gamepad then "A / ENTER   " else "ENTER / CLICK   ") .. action, self.label, mx, y + 162, pivot.TopCenter)
		if phase == "finished" then
			text("BEST   " .. clock(best) .. "     LAST   " .. clock(last), self.text, mx, y + 216, pivot.TopCenter)
		elseif gamepad then
			text("LEFT STICK   STEER     RT   GAS     LT   BRAKE", self.text, mx, y + 213, pivot.TopCenter)
			text("X   CAMERA     Y   RESET CAR     START   PAUSE", self.text, mx, y + 237, pivot.TopCenter)
		else
			text("W / S   GAS / BRAKE     A / D   STEER", self.text, mx, y + 213, pivot.TopCenter)
			text("C   CAMERA     R   RESET CAR     ESC   PAUSE", self.text, mx, y + 237, pivot.TopCenter)
		end
		text(if gamepad then "B / Q   QUIT" else "Q   QUIT", self.label, mx, y + 276, pivot.TopCenter)
	end
	sn.Graphics.endPass(pass)
end

return Hud
