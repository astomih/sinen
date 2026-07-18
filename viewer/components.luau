--!strict

local sn = require("@sinen")
local ECS = sn.ECS

export type Transform2D = {
	x: number,
	y: number,
	rotation: number,
}

export type Rectangle = {
	width: number,
	height: number,
	color: { number },
	visible: boolean,
}

export type Transform3D = {
	position: { number },
	rotation: { number },
	scale: { number },
}

export type Box3D = {
	color: { number },
	visible: boolean,
}

export type Camera3D = {
	position: { number },
	target: { number },
	fov: number,
	near: number,
	far: number,
	active: boolean,
}

local Name: sn.Component<string> = ECS.component("Name")
local Transform2D: sn.Component<Transform2D> = ECS.component("Transform2D")
local Rectangle: sn.Component<Rectangle> = ECS.component("Rectangle")
local Transform3D: sn.Component<Transform3D> = ECS.component("Transform3D")
local Box3D: sn.Component<Box3D> = ECS.component("Box3D")
local Camera3D: sn.Component<Camera3D> = ECS.component("Camera3D")

local Components: any = {
	Name = Name,
	Transform2D = Transform2D,
	Rectangle = Rectangle,
	Transform3D = Transform3D,
	Box3D = Box3D,
	Camera3D = Camera3D,
}

Components.byName = {
	Name = Components.Name,
	Transform2D = Components.Transform2D,
	Rectangle = Components.Rectangle,
	Transform3D = Components.Transform3D,
	Box3D = Components.Box3D,
	Camera3D = Components.Camera3D,
}

Components.defaults = {
	Transform2D = {
		x = 320,
		y = 240,
		rotation = 0,
	},
	Rectangle = {
		width = 96,
		height = 64,
		color = { 0.25, 0.6, 1.0, 1.0 },
		visible = true,
	},
	Transform3D = {
		position = { 0, 0, 0 },
		rotation = { 0, 0, 0 },
		scale = { 1, 1, 1 },
	},
	Box3D = {
		color = { 0.25, 0.6, 1.0, 1.0 },
		visible = true,
	},
	Camera3D = {
		position = { 0, 3, 7 },
		target = { 0, 0, 0 },
		fov = 55,
		near = 0.1,
		far = 100,
		active = true,
	},
}

return Components
