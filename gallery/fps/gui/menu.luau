local sn = require("@sinen")
local button = require("./button").new()
local text = require("./text").new()
local scroll = require("./scroll").new()
local function menu()
	local object = {
		hide = true,
		hide_next = false,
		option = false,
		option_Window_size = false,
		setup = function(self) end,
		update = function(self)
			local is_esc = sn.Keyboard.isPressed(sn.Keyboard.ESCAPE)
			if self.hide_next then
				self.hide = not self.hide
				self.hide_next = false
			else
				if self.hide and is_esc then
					self.hide_next = true
				end
			end
			if self.option_Window_size then
				text:show("Window Size", sn.Vec2.new(0, 200), 50)
				scroll:show(sn.Vec2.new(200, 0), sn.Vec2.new(20, 400))
				local offset = scroll.pos.y * 1.5
				local start = 100
				if button:show("1120x630", sn.Vec2.new(0, start + offset), sn.Vec2.new(200, 50)) then
					sn.Window.resize(sn.Vec2.new(1120, 630))
				end
				start = start - 100
				if button:show("1280x720", sn.Vec2.new(0, start + offset), sn.Vec2.new(200, 50)) then
					sn.Window.resize(sn.Vec2.new(1280, 720))
				end
				start = start - 100
				if button:show("1440x810", sn.Vec2.new(0, start + offset), sn.Vec2.new(200, 50)) then
					sn.Window.resize(sn.Vec2.new(1440, 810))
				end
				start = start - 100
				if button:show("1600x900", sn.Vec2.new(0, start + offset), sn.Vec2.new(200, 50)) then
					sn.Window.resize(sn.Vec2.new(1600, 900))
				end
				start = start - 100
				if button:show("1760x990", sn.Vec2.new(0, start + offset), sn.Vec2.new(200, 50)) then
					sn.Window.resize(sn.Vec2.new(1760, 990))
				end
				start = start - 100
				if button:show("1920x1080", sn.Vec2.new(0, start + offset), sn.Vec2.new(200, 50)) then
					sn.Window.resize(sn.Vec2.new(1920, 1080))
				end
				if button:show("Back", sn.Vec2.new(0, -300), sn.Vec2.new(200, 50)) or is_esc then
					self.option_Window_size = false
				end

				return
			end
			if self.option then
				text:show("Option", sn.Vec2.new(0, 200), 50)
				if button:show("Window Size", sn.Vec2.new(0, 100), sn.Vec2.new(300, 50)) then
					self.option_Window_size = true
				end
				if button:show("Back", sn.Vec2.new(0, -100), sn.Vec2.new(200, 50)) or is_esc then
					self.option = false
				end
				return
			end
			if not self.hide then
				sn.Mouse.hideCursor(false)
				sn.Mouse.setRelative(false)
				text:show("Menu", sn.Vec2.new(0, 200), 50)
				if button:show("Resume", sn.Vec2.new(0, 70), sn.Vec2.new(200, 60)) or is_esc then
					self.hide_next = true
				end
				if button:show("Option", sn.Vec2.new(0, 0), sn.Vec2.new(200, 60)) then
					self.option = true
				end
				if button:show("Quit", sn.Vec2.new(0, -70), sn.Vec2.new(200, 60)) then
					sn.Script.load("main", ".")
				end
			end
		end,
		draw = function(self) end,
	}
	return object
end

return menu
