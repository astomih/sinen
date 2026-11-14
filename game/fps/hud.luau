local sn = require("@sinen")

local v2, rect, color = sn.Vec2.new, sn.Rect.new, sn.Color.new
local pivot = sn.Pivot
local Hud = {}
Hud.__index = Hud

function Hud.new()
	local font = sn.Font.new(32)
	local white = color(0.93, 0.95, 0.92, 1)
	local teal = color(0.29, 0.89, 0.91, 1)
	local faded = color(0.68, 0.75, 0.75, 1)
	return setmetatable({
		headline = sn.TextStyle.new(font, white, 56),
		subhead = sn.TextStyle.new(font, white, 22),
		number = sn.TextStyle.new(font, white, 30),
		label = sn.TextStyle.new(font, teal, 13),
		text = sn.TextStyle.new(font, faded, 15),
		teal = teal,
		white = white,
		panel = color(0.023, 0.036, 0.043, 0.88),
		track = color(0.14, 0.22, 0.24, 0.90),
		alert = color(1, 0.52, 0.28, 1),
	}, Hud)
end

local function text(pass: sn.Render2DPass, message: string, style: sn.TextStyle, x: number, y: number, anchor: sn.Pivot)
	pass:drawText(message, style, sn.TextTransform.new(v2(x, y), 0, anchor))
end

function Hud:draw(state: string, wave: number, total: number, enemies: number, health: number, charge: number,
	reserve: number, capacity: number, score: number, intermission: number, reloadProgress: number?,
	hit: number, damage: number, aiming: boolean, fromLauncher: boolean)
	local pass = sn.Graphics.begin2DPass()
	local size = sn.Window.size()
	local w, h = size.x, size.y
	local cx, cy = w * 0.5, h * 0.5
	local function fill(x: number, y: number, width: number, height: number, tint: sn.Color)
		pass:drawRect(rect(x, y, width, height), tint)
	end

	if state == "playing" then
		if damage > 0 then
			local edge = color(0.96, 0.20, 0.08, damage * 0.83)
			fill(0, 0, w, 9, edge)
			fill(0, h - 9, w, 9, edge)
			fill(0, 0, 9, h, edge)
			fill(w - 9, 0, 9, h, edge)
		end
		local margin = if w >= 700 then 30 else 12
		local y = h - 98
		fill(margin, y, 184, 70, self.panel)
		fill(margin, y, 3, 70, if health > 35 then self.teal else self.alert)
		text(pass, "SUIT INTEGRITY", self.label, margin + 14, y + 11, pivot.TopLeft)
		text(pass, tostring(health), self.number, margin + 14, y + 23, pivot.TopLeft)
		fill(margin + 72, y + 46, 96, 5, self.track)
		if health > 0 then
			fill(margin + 72, y + 46, 96 * health / 100, 5, if health > 35 then self.teal else self.alert)
		end

		local ammoX = w - margin - 184
		fill(ammoX, y, 184, 70, self.panel)
		fill(ammoX + 181, y, 3, 70, if charge == 0 then self.alert else self.teal)
		text(pass, if reloadProgress then "RECHARGING" elseif charge == 0 and reserve > 0 then "R  RECHARGE" elseif charge == 0 then "NO CHARGE" else "PULSE EMITTER", self.label, ammoX + 14, y + 11, pivot.TopLeft)
		text(pass, string.format("%02d", charge), self.number, ammoX + 14, y + 23, pivot.TopLeft)
		text(pass, string.format("/ %03d", reserve), self.text, ammoX + 65, y + 35, pivot.TopLeft)
		fill(ammoX + 124, y + 46, 42, 5, self.track)
		local fraction = reloadProgress or charge / capacity
		if fraction > 0 then fill(ammoX + 124, y + 46, 42 * fraction, 5, self.teal) end

		fill(cx - 118, 24, 236, 65, self.panel)
		fill(cx - 118, 24, 236, 2, self.teal)
		local objective = if enemies == 0 then string.format("INCOMING  %.1fs", intermission) else string.format("WAVE  %02d / %02d", wave, total)
		text(pass, objective, self.subhead, cx, 30, pivot.TopCenter)
		text(pass, string.format("%d ACTIVE     /     %04d SCORE", enemies, score), self.label, cx, 65, pivot.TopCenter)

		local gap = if aiming then 4 else 7
		local tint = if hit > 0 then self.alert else self.white
		fill(cx - gap - 6, cy - 1, 6, 2, tint)
		fill(cx + gap, cy - 1, 6, 2, tint)
		fill(cx - 1, cy - gap - 6, 2, 6, tint)
		fill(cx - 1, cy + gap, 2, 6, tint)
		fill(cx - 1, cy - 1, 2, 2, tint)
	else
		fill(0, 0, w, h, color(0.014, 0.024, 0.028, 0.58))
		local width = math.max(1, math.min(680, w - 24))
		local x, y = cx - width / 2, cy - 160
		fill(x, y, width, 320, self.panel)
		fill(x, y, width, 2, self.teal)
		local title = if state == "title" then "FOUNDRY" elseif state == "paused" then "PAUSED" elseif state == "won" then "BAY SECURED" else "SUIT OFFLINE"
		local action = if state == "title" then "ENTER / CLICK   DEPLOY" elseif state == "paused" then "ENTER / CLICK   RESUME" else "ENTER / CLICK   RETRY"
		text(pass, title, if w >= 560 then self.headline else self.subhead, cx, y + 27, pivot.TopCenter)
		if state ~= "paused" then
			local description = if state == "title" then "Clear three waves of security drones." else string.format("SCORE  %04d     /     WAVE  %02d", score, wave)
			text(pass, description, self.subhead, cx, y + 119, pivot.TopCenter)
		end
		fill(cx - 132, y + 165, 264, 41, self.track)
		text(pass, action, self.label, cx, y + 178, pivot.TopCenter)
		if state == "title" or state == "paused" then
			text(pass, "WASD Move   /   Shift Sprint   /   Space Jump", self.text, cx, y + 223, pivot.TopCenter)
			text(pass, "LMB Pulse   /   RMB Aim   /   R Recharge", self.text, cx, y + 248, pivot.TopCenter)
		end
		text(pass, if fromLauncher then "Q  GAME SELECT" else "Q  QUIT", self.label, cx, y + 285, pivot.TopCenter)
	end

	sn.Graphics.endPass(pass)
end

return Hud
