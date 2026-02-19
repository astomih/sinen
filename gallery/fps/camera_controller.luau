local Player = require("./player")
local CameraController = {}
CameraController.__index = CameraController
CameraController.new = function(): CameraController
	local self = setmetatable(
		{} :: {
			position: Vec3,
			target: Vec3,
			up: Vec3,
			prev_boost: boolean,
			is_tracking_player: boolean,
			player: Player.Player,
			py: number,
			pz: number,
			track_speed: number,
			setup: (self: CameraController, player: Player.Player) -> (),
			update: (self: CameraController) -> (),
		},
		CameraController :: typeof(CameraController)
	)
	self.position = Vec3.new(0, 0, 0)
	self.target = Vec3.new(0, 0, 0)
	self.up = Vec3.new(0, 0, 1)
	self.prev_boost = false
	self.is_tracking_player = false
	self.py = 5
	self.pz = 20
	self.track_speed = 15
	return self
end

CameraController.setup = function(self: CameraController, player: Player.Player)
	self.player = player
	self.position = Vec3.new(
		self.player.transform.position.x,
		self.player.transform.position.y - self.py,
		self.player.transform.position.z + self.pz
	)
	self.target =
		Vec3.new(self.player.transform.position.x, self.player.transform.position.y, self.player.transform.position.z)
end
CameraController.update = function(self)
	self.target.x = math.sin(math.rad(self.player.transform.rotation.z))
	self.target.y = math.cos(math.rad(self.player.transform.rotation.z))
	self.target.z = math.sin(math.rad(self.player.transform.rotation.y))
	self.position = self.player.transform.position:copy()
	Graphics.getCamera3D():lookat(self.position, self.position + self.target, Vec3.new(0, 0, 1))
end

export type CameraController = typeof(CameraController.new())

return CameraController
