--!strict

local Scene = require("../scene")

return Scene.define(function(scene)
	scene:entity("Scene Camera")
		:with("Camera3D", {
			active = true,
			far = 100,
			fov = 55,
			near = 0.1,
			position = { 0, 3, 7 },
			target = { 0, 0, 0 },
		})

	scene:entity("Blue cube")
		:with("Transform3D", {
			position = { -1.4, 0, 0 },
			rotation = { 0, 25, 0 },
			scale = { 1.2, 1.2, 1.2 },
		})
		:with("Box3D", {
			color = { 0.2, 0.55, 1, 1 },
			visible = true,
		})

	scene:entity("Orange cube")
		:with("Transform3D", {
			position = { 1.4, 0.3, 0 },
			rotation = { 20, -30, 10 },
			scale = { 1, 1.6, 1 },
		})
		:with("Box3D", {
			color = { 1, 0.42, 0.1, 1 },
			visible = true,
		})

	scene:entity("2D overlay")
		:with("Transform2D", {
			rotation = 0,
			x = 90,
			y = 70,
		})
		:with("Rectangle", {
			color = { 0.2, 0.85, 0.55, 0.9 },
			height = 36,
			visible = true,
			width = 130,
		})
end)
