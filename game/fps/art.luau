local sn = require("@sinen")

local v3, v2, color = sn.Vec3.new, sn.Vec2.new, sn.Color.new
local keys = sn.TextureKey
local Art = {}

local function solid(value: sn.Color): sn.Texture
	local texture = sn.Texture.new(1, 1)
	texture:fill(value)
	return texture
end

-- A box with outward normals and a consistent texel density on all six faces.
function Art.box(builder: sn.MeshBuilder, center: sn.Vec3, size: sn.Vec3, tint: sn.Color, tile: number)
	local half = size * v3(0.5)
	local function face(normal: sn.Vec3, across: sn.Vec3, up: sn.Vec3, width: number, height: number)
		local middle = center + normal * half
		local u, v = across * v3(width / 2), up * v3(height / 2)
		local a = builder:vertex(middle - u - v, normal, v2(0, height / tile), tint)
		local b = builder:vertex(middle + u - v, normal, v2(width / tile, height / tile), tint)
		local c = builder:vertex(middle + u + v, normal, v2(width / tile, 0), tint)
		local d = builder:vertex(middle - u + v, normal, v2(0, 0), tint)
		builder:quad(a, b, c, d)
	end
	face(v3(0, 0, 1), v3(1, 0, 0), v3(0, 1, 0), size.x, size.y)
	face(v3(0, 0, -1), v3(-1, 0, 0), v3(0, 1, 0), size.x, size.y)
	face(v3(1, 0, 0), v3(0, 0, -1), v3(0, 1, 0), size.z, size.y)
	face(v3(-1, 0, 0), v3(0, 0, 1), v3(0, 1, 0), size.z, size.y)
	face(v3(0, 1, 0), v3(1, 0, 0), v3(0, 0, -1), size.x, size.z)
	face(v3(0, -1, 0), v3(1, 0, 0), v3(0, 0, 1), size.x, size.z)
end

function Art.transform(position: sn.Vec3, rotation: sn.Vec3?, scale: sn.Vec3?): sn.Transform
	local transform = sn.Transform.new()
	transform.position = position
	transform.rotation = rotation or v3(0)
	transform.scale = scale or v3(1)
	return transform
end

function Art.materials()
	local function maps(name: string)
		return {
			base = sn.Texture.new("assets/" .. name .. "_diff_1k.ktx2"),
			normal = sn.Texture.new("assets/" .. name .. "_nor_gl_1k.ktx2"),
			arm = sn.Texture.new("assets/" .. name .. "_arm_1k.ktx2"),
		}
	end
	local floor, wall, plate = maps("concrete_floor"), maps("concrete"), maps("metal_plate")
	local hardware = solid(color(1, 0.31, 0.73, 1))
	local paint = solid(color(1, 0.47, 0.05, 1))
	local cyan = solid(color(0.05, 0.74, 0.89, 1))
	local amber = solid(color(1, 0.38, 0.075, 1))
	local ivory = solid(color(0.89, 0.88, 0.74, 1))

	-- Poly Haven ARM uses the renderer's green/blue roughness/metallic layout.
	-- The same texture also supplies the red channel to the occlusion sampler.
	local function textured(maps: typeof(floor)): (sn.Model) -> ()
		return function(model: sn.Model)
			model:setTexture(keys.BaseColor, maps.base)
			model:setTexture(keys.Normal, maps.normal)
			model:setTexture(keys.DiffuseRoughness, maps.arm)
			model:setTexture(keys.LightMap, maps.arm)
		end
	end
	return {
		floor = textured(floor),
		wall = textured(wall),
		plate = textured(plate),
		hardware = function(model: sn.Model) model:setTexture(keys.DiffuseRoughness, hardware) end,
		paint = function(model: sn.Model) model:setTexture(keys.DiffuseRoughness, paint) end,
		cyan = function(model: sn.Model) model:setTexture(keys.Emissive, cyan) end,
		amber = function(model: sn.Model) model:setTexture(keys.Emissive, amber) end,
		ivory = function(model: sn.Model) model:setTexture(keys.Emissive, ivory) end,
	}
end

export type Materials = typeof(Art.materials())
export type Part = { model: sn.Model, transform: sn.Transform }
export type Box = { lo: sn.Vec3, hi: sn.Vec3 }

function Art.actors(materials: Materials)
	local shell = sn.MeshBuilder.new()
	local eye = sn.MeshBuilder.new()
	shell:addSphere(0.42, 14, 24, color(0.22, 0.27, 0.29, 1))
	for _, side in { -1, 1 } do
		Art.box(shell, v3(side * 0.47, 0, 0), v3(0.38, 0.20, 0.52), color(0.13, 0.19, 0.21, 1), 1)
		Art.box(shell, v3(side * 0.56, 0.07, 0.12), v3(0.15, 0.08, 0.36), color(0.46, 0.31, 0.14, 1), 1)
		Art.box(eye, v3(side * 0.52, -0.09, 0), v3(0.11, 0.04, 0.34), color(0.12), 1)
	end
	Art.box(eye, v3(0, 0.03, 0.399), v3(0.28, 0.12, 0.055), color(0.10), 1)
	local drone = shell:toModel()
	local visor, alertVisor = eye:toModel(), eye:toModel()
	materials.hardware(drone)
	materials.ivory(visor)
	materials.amber(alertVisor)

	local orb = sn.MeshBuilder.new()
	orb:addSphere(1, 9, 14, color(0.1))
	local bolt, cell, sparks = orb:toModel(), orb:toModel(), orb:toModel()
	materials.amber(bolt)
	materials.cyan(cell)
	materials.ivory(sparks)
	local beamBuilder = sn.MeshBuilder.new()
	beamBuilder:addBox(1, color(0.1))
	local beam = beamBuilder:toModel()
	materials.cyan(beam)

	local receiver = sn.MeshBuilder.new()
	local housing = sn.MeshBuilder.new()
	local strips = sn.MeshBuilder.new()
	local function part(builder: sn.MeshBuilder, x: number, y: number, z: number, w: number, h: number, d: number, tint: sn.Color)
		Art.box(builder, v3(x, y, z), v3(w, h, d), tint, 0.48)
	end
	local graphite = color(0.16, 0.21, 0.24, 1)
	local ceramic = color(0.64, 0.55, 0.33, 1)
	part(receiver, 0, 0, 0, 0.24, 0.23, 0.42, graphite)
	part(receiver, 0, -0.14, 0.13, 0.15, 0.19, 0.16, graphite)
	part(housing, 0, 0.055, -0.16, 0.27, 0.14, 0.25, ceramic)
	part(housing, 0, 0.10, 0.08, 0.17, 0.085, 0.18, ceramic)
	for _, side in { -1, 1 } do
		part(receiver, side * 0.086, -0.01, -0.28, 0.065, 0.14, 0.30, graphite)
		part(strips, side * 0.11, 0.07, -0.22, 0.018, 0.023, 0.22, color(0.12))
	end
	part(strips, 0, 0.148, 0.10, 0.091, 0.016, 0.083, color(0.12))
	local emitter = { receiver:toModel(), housing:toModel(), strips:toModel() }
	materials.hardware(emitter[1])
	materials.paint(emitter[2])
	materials.cyan(emitter[3])

	return { drone = drone, visor = visor, alertVisor = alertVisor, bolt = bolt, cell = cell, sparks = sparks, beam = beam, emitter = emitter }
end

return Art
