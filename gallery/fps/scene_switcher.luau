---@class SceneSwitcher

local SceneSwitcher = {
	color = Color.new(0, 0, 0, 0.0),
	time = 0.25,
	timer = 0.0,
	flag = false,
	scene_name = "",
	is_launch = false,
}
SceneSwitcher.__index = SceneSwitcher

SceneSwitcher.new = function(): SceneSwitcher
	return setmetatable(
		{} :: {
			color: Color,
			time: number,
			timer: number,
			flag: boolean,
			scene_name: string,
			is_launch: boolean,

			update: (self: SceneSwitcher) -> nil,
		},
		SceneSwitcher
	)
end
export type SceneSwitcher = typeof(SceneSwitcher.new())

SceneSwitcher.update = function(self: SceneSwitcher)
	if self.flag then
		if not self.is_launch then
			if self.timer < self.time then
				self.timer = self.timer + Time.delta()
				local t = self.timer * (1.0 / self.time)
				if t > 1.0 then
					t = 1.0
				end
				if t < 0.0 then
					t = 1.0
				end
				self.color = Color.new(self.color.r, self.color.g, self.color.b, t)
			else
				self.timer = 0
				self.color = Color.new(self.color.r, self.color.g, self.color.b, 1.0)
				self.flag = false
				Script.load(self.scene_name)
			end
		else
			if self.timer > 0.0 then
				self.timer = self.timer - Time.delta()
				local t = self.timer * (1.0 / self.time)
				if t < 0.0 then
					t = 0.0
				end
				self.color = Color.new(self.color.r, self.color.g, self.color.b, t)
			else
				self.flag = false
				self.color = Color.new(self.color.r, self.color.g, self.color.b, 0.0)
				self.timer = 0.0
			end
		end
	end
end

SceneSwitcher.draw = function(self)
	Graphics.drawRect(Rect.new(Vec2.new(0), Window.size()), self.color)
end

SceneSwitcher.start = function(self, scene_name)
	self.is_launch = string.len(scene_name) == 0
	if self.is_launch then
		self.color = Color.new(self.color.r, self.color.g, self.color.b, 1.0)
		self.flag = false
		self.timer = self.time
	else
		self.color = Color.new(self.color.r, self.color.g, self.color.b, 0.0)
		self.timer = 0.0
	end
	self.scene_name = scene_name
	self.flag = true
end

return SceneSwitcher
