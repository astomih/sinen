---@class camera_controller
---@field position Vec3
---@field target Vec3
---@field up Vec3
---@field prev_boost boolean
---@field is_tracking_player boolean
---@field player Player
---@field py number
---@field pz number
---@field track_speed number
---@field setup fun(self: camera_controller, player: Player)
---@field update fun(self: camera_controller)
---@return camera_controller
local function camera_controller()
	local object = {
		position = Vec3.new(0, 0, 0),
		target = Vec3.new(0, 0, 0),
		up = Vec3.new(0, 0, 1),
		prev_boost = false,
		is_tracking_player = false,
		player = {},
		py = 5,
		pz = 20,
		track_speed = 15,
		---@paramv self camera_controller
		---@param player Player
		setup = function(self, player)
			self.player = player
			self.position = Vec3.new(
				self.player.transform.position.x,
				self.player.transform.position.y - self.py,
				self.player.transform.position.z + self.pz
			)
			self.target = Vec3.new(
				self.player.transform.position.x,
				self.player.transform.position.y,
				self.player.transform.position.z
			)
		end,
		---@param self camera_controller
		update = function(self)
			self.target.x = math.sin(math.rad(self.player.transform.rotation.z))
			self.target.y = math.cos(math.rad(self.player.transform.rotation.z))
			self.target.z = math.sin(math.rad(self.player.transform.rotation.y))
			self.position = self.player.transform.position:copy()
			Graphics.getCamera3D():lookat(self.position, self.position + self.target, Vec3.new(0, 0, 1))
		end,
	}
	return object
end

return camera_controller
