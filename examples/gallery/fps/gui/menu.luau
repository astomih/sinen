local sn = require("@sinen")
local G = require("../global")

local function centeredRect(pos: sn.Vec2, size: sn.Vec2): sn.Rect
	return sn.Rect.new(sn.Pivot.Center, sn.Window.center() + sn.Vec2.new(pos.x, -pos.y), size)
end

local function centeredLabel(text: string, pos: sn.Vec2, size: number)
	sn.Gui.label(
		text,
		G.FONT_DEFAULT64
			:region(text, size, sn.Pivot.Center, sn.Window.center() + sn.Vec2.new(pos.x, -pos.y))
			:topLeft(),
		sn.Color.new(1),
		size
	)
end

local function menu()
	local object = {
		hide = true,
		hide_next = false,
		option = false,
		option_Window_size = false,
		setup = function(self) end,
		update = function(self)
			local is_esc = sn.Keyboard.isPressed(sn.Scancode.ESCAPE)
			if self.hide_next then
				self.hide = not self.hide
				self.hide_next = false
			else
				if self.hide and is_esc then
					self.hide_next = true
				end
			end
			if not self.hide then
				sn.Mouse.hideCursor(false)
				sn.Mouse.setRelative(false)
			end
		end,
		draw = function(self)
			local is_esc = sn.Keyboard.isPressed(sn.Scancode.ESCAPE)
			sn.Gui.setFont(G.FONT_DEFAULT64)
			sn.Gui.setFontSize(24)
			if self.option_Window_size then
				centeredLabel("Window Size", sn.Vec2.new(0, 260), 50)
				local offset = 0
				local start = 100
				if sn.Gui.button("1120x630", centeredRect(sn.Vec2.new(0, start + offset), sn.Vec2.new(220, 44))) then
					sn.Window.resize(sn.Vec2.new(1120, 630))
				end
				start = start - 54
				if sn.Gui.button("1280x720", centeredRect(sn.Vec2.new(0, start + offset), sn.Vec2.new(220, 44))) then
					sn.Window.resize(sn.Vec2.new(1280, 720))
				end
				start = start - 54
				if sn.Gui.button("1440x810", centeredRect(sn.Vec2.new(0, start + offset), sn.Vec2.new(220, 44))) then
					sn.Window.resize(sn.Vec2.new(1440, 810))
				end
				start = start - 54
				if sn.Gui.button("1600x900", centeredRect(sn.Vec2.new(0, start + offset), sn.Vec2.new(220, 44))) then
					sn.Window.resize(sn.Vec2.new(1600, 900))
				end
				start = start - 54
				if sn.Gui.button("1760x990", centeredRect(sn.Vec2.new(0, start + offset), sn.Vec2.new(220, 44))) then
					sn.Window.resize(sn.Vec2.new(1760, 990))
				end
				start = start - 54
				if sn.Gui.button("1920x1080", centeredRect(sn.Vec2.new(0, start + offset), sn.Vec2.new(220, 44))) then
					sn.Window.resize(sn.Vec2.new(1920, 1080))
				end
				if sn.Gui.button("Back", centeredRect(sn.Vec2.new(0, -260), sn.Vec2.new(200, 44))) or is_esc then
					self.option_Window_size = false
				end

				return
			end
			if self.option then
				centeredLabel("Option", sn.Vec2.new(0, 200), 50)
				if sn.Gui.button("Window Size", centeredRect(sn.Vec2.new(0, 100), sn.Vec2.new(300, 50))) then
					self.option_Window_size = true
				end
				if sn.Gui.button("Back", centeredRect(sn.Vec2.new(0, -100), sn.Vec2.new(200, 50))) or is_esc then
					self.option = false
				end
				return
			end
			if not self.hide then
				centeredLabel("Menu", sn.Vec2.new(0, 200), 50)
				if sn.Gui.button("Resume", centeredRect(sn.Vec2.new(0, 70), sn.Vec2.new(200, 60))) or is_esc then
					self.hide_next = true
				end
				if sn.Gui.button("Option", centeredRect(sn.Vec2.new(0, 0), sn.Vec2.new(200, 60))) then
					self.option = true
				end
				if sn.Gui.button("Quit", centeredRect(sn.Vec2.new(0, -70), sn.Vec2.new(200, 60))) then
					sn.Script.load("/main.luau")
				end
			end
		end,
	}
	return object
end

return menu
