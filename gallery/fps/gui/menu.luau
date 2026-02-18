local button = require("./button").new()
local text = require("./text").new()
local scroll = require("./scroll")()
local function menu()
	local object = {
		hide = true,
		hide_next = false,
		option = false,
		option_Window_size = false,
		setup = function(self) end,
		update = function(self)
			local is_esc = Keyboard.isPressed(Keyboard.ESCAPE)
			if self.hide_next then
				self.hide = not self.hide
				self.hide_next = false
			else
				if self.hide and is_esc then
					self.hide_next = true
				end
			end
			if self.option_Window_size then
				text:show("Window Size", Vec2.new(0, 200), 50)
				scroll:show(Vec2.new(200, 0), Vec2.new(20, 400))
				local offset = scroll.pos.y * 1.5
				local start = 100
				if button:show("1120x630", Vec2.new(0, start + offset), Vec2.new(200, 50)) then
					Window.resize(Vec2.new(1120, 630))
				end
				start = start - 100
				if button:show("1280x720", Vec2.new(0, start + offset), Vec2.new(200, 50)) then
					Window.resize(Vec2.new(1280, 720))
				end
				start = start - 100
				if button:show("1440x810", Vec2.new(0, start + offset), Vec2.new(200, 50)) then
					Window.resize(Vec2.new(1440, 810))
				end
				start = start - 100
				if button:show("1600x900", Vec2.new(0, start + offset), Vec2.new(200, 50)) then
					Window.resize(Vec2.new(1600, 900))
				end
				start = start - 100
				if button:show("1760x990", Vec2.new(0, start + offset), Vec2.new(200, 50)) then
					Window.resize(Vec2.new(1760, 990))
				end
				start = start - 100
				if button:show("1920x1080", Vec2.new(0, start + offset), Vec2.new(200, 50)) then
					Window.resize(Vec2.new(1920, 1080))
				end
				if button:show("Back", Vec2.new(0, -300), Vec2.new(200, 50)) or is_esc then
					self.option_Window_size = false
				end

				return
			end
			if self.option then
				text:show("Option", Vec2.new(0, 200), 50)
				if button:show("Window Size", Vec2.new(0, 100), Vec2.new(300, 50)) then
					self.option_Window_size = true
				end
				if button:show("Back", Vec2.new(0, -100), Vec2.new(200, 50)) or is_esc then
					self.option = false
				end
				return
			end
			if not self.hide then
				Mouse.hideCursor(false)
				Mouse.setRelative(false)
				text:show("Menu", Vec2.new(0, 200), 50)
				if button:show("Resume", Vec2.new(0, 70), Vec2.new(200, 60)) or is_esc then
					self.hide_next = true
				end
				if button:show("Option", Vec2.new(0, 0), Vec2.new(200, 60)) then
					self.option = true
				end
				if button:show("Quit", Vec2.new(0, -70), Vec2.new(200, 60)) then
					Event.quit()
				end
			end
		end,
		draw = function(self) end,
	}
	return object
end

return menu
