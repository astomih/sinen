local sn = require("@sinen")
local Track = require("./track")

local v3, v2, color = sn.Vec3.new, sn.Vec2.new, sn.Color.new
local keys = sn.TextureKey
local Art = {}

local function flatTexture(r: number, g: number, b: number): sn.Texture
	local texture = sn.Texture.new(1, 1)
	texture:fill(color(r, g, b, 1))
	return texture
end

local function finish(builder: sn.MeshBuilder, material: sn.Texture, emission: sn.Texture?): sn.Model
	local model = builder:toModel()
	model:setTexture(keys.DiffuseRoughness, material)
	if emission then
		model:setTexture(keys.Emissive, emission)
	end
	return model
end

local function textured(mesh: sn.MeshBuilder, name: string): sn.Model
	local model = mesh:toModel()
	local base = "assets/" .. name
	local arm = sn.Texture.new(base .. "_arm_1k.ktx2")
	model:setTexture(keys.BaseColor, sn.Texture.new(base .. "_diff_1k.ktx2"))
	model:setTexture(keys.Normal, sn.Texture.new(base .. "_nor_gl_1k.ktx2"))
	model:setTexture(keys.DiffuseRoughness, arm)
	model:setTexture(keys.LightMap, arm)
	return model
end

local function quad(mesh: sn.MeshBuilder, a: sn.Vec3, b: sn.Vec3, c: sn.Vec3, d: sn.Vec3, tint: sn.Color)
	local normal = (b - a):cross(c - a):normalize()
	mesh:quad(mesh:vertex(a, normal, v2(0, 1), tint), mesh:vertex(b, normal, v2(1, 1), tint),
		mesh:vertex(c, normal, v2(1, 0), tint), mesh:vertex(d, normal, v2(0, 0), tint))
end

local function box(builder: sn.MeshBuilder, x: number, y: number, z: number, w: number, h: number, d: number, tint: sn.Color)
	local left, right, bottom, top, front, back = x - w / 2, x + w / 2, y - h / 2, y + h / 2, z - d / 2, z + d / 2
	local a, b, c, d0 = v3(left, bottom, front), v3(right, bottom, front), v3(right, top, front), v3(left, top, front)
	local e, f, g, h0 = v3(left, bottom, back), v3(right, bottom, back), v3(right, top, back), v3(left, top, back)
	quad(builder, b, a, d0, c, tint)
	quad(builder, e, f, g, h0, tint)
	quad(builder, a, e, h0, d0, tint)
	quad(builder, f, b, c, g, tint)
	quad(builder, d0, h0, g, c, tint)
	quad(builder, a, b, f, e, tint)
end

local function roadPoint(s: number, lateral: number, rise: number): sn.Vec3
	local p = Track.at(s, lateral)
	return v3(p.x, Track.height(p.x, p.z) + rise, p.z)
end

function Art.scenery()
	local road, markings, kerb = sn.MeshBuilder.new(), sn.MeshBuilder.new(), sn.MeshBuilder.new()
	local safety, posts, trim = sn.MeshBuilder.new(), sn.MeshBuilder.new(), sn.MeshBuilder.new()
	local grass, hills, trees = sn.MeshBuilder.new(), sn.MeshBuilder.new(), sn.MeshBuilder.new()
	local white, red = color(0.78, 0.82, 0.83, 1), color(0.67, 0.055, 0.038, 1)
	local grey, steel = color(0.30, 0.36, 0.39, 1), color(0.39, 0.47, 0.52, 1)
	local length = Track.length
	local function strip(mesh: sn.MeshBuilder, s: number, t: number, left: number, right: number, rise: number, tint: sn.Color)
		quad(mesh, roadPoint(s, left, rise), roadPoint(s, right, rise), roadPoint(t, right, rise), roadPoint(t, left, rise), tint)
	end
	local sections = math.ceil(length / 1.6)
	for i = 0, sections - 1 do
		local s, t = i * length / sections, (i + 1) * length / sections
		local u, w = Track.halfWidth, Track.wall
		local normal = v3(0, 1, 0)
		local a, b, c, d = roadPoint(s, -u, 0.08), roadPoint(t, -u, 0.08), roadPoint(t, u, 0.08), roadPoint(s, u, 0.08)
		local tint = color(0.88, 0.91, 0.95, 1)
		road:quad(road:vertex(a, normal, v2(0, s / 2), tint), road:vertex(d, normal, v2(u, s / 2), tint),
			road:vertex(c, normal, v2(u, t / 2), tint), road:vertex(b, normal, v2(0, t / 2), tint))
		for _, side in { -1, 1 } do
			strip(markings, s, t, side * (u - 0.29) - 0.07, side * (u - 0.29) + 0.07, 0.095, white)
			strip(kerb, s, t, side * (u + 0.34) - 0.34, side * (u + 0.34) + 0.34, 0.09,
				if math.floor(i / 2) % 2 == 0 then white else red)
			local a0, b0 = roadPoint(s, w * side, 0.20), roadPoint(t, w * side, 0.20)
			local a1, b1 = roadPoint(s, w * side, 0.77), roadPoint(t, w * side, 0.77)
			if side == 1 then
				quad(safety, b0, a0, a1, b1, steel)
			else
				quad(safety, a0, b0, b1, a1, steel)
			end
			strip(safety, s, t, w * side - 0.10, w * side + 0.10, 0.77, steel)
			if i % 4 == 0 then
				box(posts, a0.x, a0.y + 0.08, a0.z, 0.25, 0.72, 0.29, grey)
			end
		end
	end
	-- Grid slots and a full-width timing stripe.
	for i = 0, 15 do
		local left = (i - 8) * Track.halfWidth / 8
		local right = left + Track.halfWidth / 8
		strip(markings, -0.75, 0, left, right, 0.10, if i % 2 == 0 then white else grey)
		strip(markings, 0, 0.75, left, right, 0.10, if i % 2 == 1 then white else grey)
	end
	for i = 1, 6 do
		local s = -14 - math.floor((i - 1) / 2) * 14 - (if i % 2 == 0 then 4 else 0)
		local side = if i % 2 == 0 then 2.3 else -2.3
		strip(markings, s, s + 0.15, side - 1.0, side + 1.0, 0.10, white)
		strip(markings, s, s + 2.3, side - 1.04, side - 0.94, 0.10, white)
		strip(markings, s, s + 2.3, side + 0.94, side + 1.04, 0.10, white)
	end

	local function terrain(x: number, z: number): sn.Vec3
		return v3(x, Track.height(x, z) - 0.08, z)
	end
	for x = -125, 275, 5 do
		for z = -245, 245, 5 do
			local variation = 0.83 + 0.11 * math.sin(x * 0.047 + z * 0.021) * math.cos(x * 0.014 - z * 0.071)
			local a, b, c, d = terrain(x, z), terrain(x, z + 5), terrain(x + 5, z + 5), terrain(x + 5, z)
			local normal = (b - a):cross(c - a):normalize()
			local tint = color(variation, variation, variation, 1)
			grass:quad(grass:vertex(a, normal, v2(x / 3.5, z / 3.5), tint),
				grass:vertex(b, normal, v2(x / 3.5, (z + 5) / 3.5), tint),
				grass:vertex(c, normal, v2((x + 5) / 3.5, (z + 5) / 3.5), tint),
				grass:vertex(d, normal, v2((x + 5) / 3.5, z / 3.5), tint))
		end
	end
	for i = 0, 47 do
		local a, b = i * math.pi / 24, (i + 1) * math.pi / 24
		local function ridge(angle: number, radius: number, altitude: number)
			return v3(72 + math.cos(angle) * radius, altitude, math.sin(angle) * radius)
		end
		local function altitude(angle: number)
			return 36 + 15 * math.sin(angle * 5) + 12 * math.cos(angle * 11) + 6 * math.sin(angle * 23)
		end
		quad(hills, ridge(a, 185, -4), ridge(b, 185, -4), ridge(b, 418, altitude(b)), ridge(a, 418, altitude(a)),
			color(0.056, 0.104, 0.079, 1))
		quad(hills, ridge(a, 409, altitude(a) - 4), ridge(b, 409, altitude(b) - 4), ridge(b, 584, 58 + 0.86 * altitude(b)),
			ridge(a, 584, 58 + 0.86 * altitude(a)), color(0.111, 0.164, 0.183, 1))
	end
	local seed = 2026
	local function noise()
		seed = (seed * 48271) % 2147483647
		return seed / 2147483647
	end
	for _ = 1, 480 do
		local x, z = -110 + noise() * 365, -221 + noise() * 442
		local _, lateral = Track.project(x, z)
		if math.abs(lateral) > 18 and not (x > -40 and x < 42 and z > 42 and z < 150) then
			local root = Track.height(x, z)
			local tall = 4 + noise() * 8
			local tint = color(0.032 + noise() * 0.028, 0.075 + noise() * 0.055, 0.029 + noise() * 0.021, 1)
			for tier = 0, 2 do
				for j = 0, 5 do
					local u, v = j * math.pi / 3 + tier, (j + 1) * math.pi / 3 + tier
					local radial = tall * (0.20 - tier * 0.045)
					local base = root + tall * (0.18 + tier * 0.22)
					local a = v3(x + radial * math.cos(u), base, z + radial * math.sin(u))
					local b = v3(x + radial * math.cos(v), base, z + radial * math.sin(v))
					local c = v3(x, base + tall * (0.55 - tier * 0.085), z)
					local n = (c - a):cross(b - a):normalize()
					trees:triangle(trees:vertex(a, n, nil, tint), trees:vertex(c, n, nil, tint), trees:vertex(b, n, nil, tint))
				end
			end
		end
	end

	local pit = Track.at(26, -17.0)
	local y = Track.height(pit.x, pit.z)
	box(posts, pit.x - 3, y + 1.8, pit.z, 9, 3.6, 72, color(0.29, 0.31, 0.31, 1))
	box(safety, pit.x - 3, y + 3.65, pit.z, 10.2, 0.20, 73, grey)
	for i = -5, 5 do
		local z = pit.z + i * 6.3
		box(trim, pit.x + 1.54, y + 1.43, z, 0.18, 2.45, 4.8, color(0.13, 0.20, 0.23, 1))
		box(markings, pit.x + 1.64, y + 3.07, z, 0.14, 0.28, 4.85, if i % 2 == 0 then white else red)
	end
	local grid = Track.at(1)
	local gy = Track.height(grid.x, grid.z)
	box(safety, grid.x, gy + 6.45, grid.z, 25.3, 0.6, 0.55, grey)
	box(trim, grid.x, gy + 6.05, grid.z, 11, 0.79, 0.6, color(0.11, 0.16, 0.17, 1))
	for _, x in { -12.3, 12.3 } do
		box(safety, grid.x + x, gy + 3.2, grid.z, 0.43, 6.4, 0.58, steel)
	end
	for i = -2, 2 do
		box(kerb, grid.x + i * 0.72, gy + 6.05, grid.z + 0.32, 0.45, 0.42, 0.08, red)
	end
	for _, s in { 170, 315, 454, 608, 722 } do
		for i = 1, 3 do
			local p = Track.at(s - i * 15, 9.2)
			local height = Track.height(p.x, p.z)
			box(markings, p.x, height + 0.85, p.z, 0.65, 0.74, 0.35, white)
			box(kerb, p.x, height + 1.24, p.z, 0.65, 0.14, 0.39, red)
		end
	end

	local matte, metal = flatTexture(1, 0.89, 0.0), flatTexture(1, 0.37, 0.62)
	return { textured(road, "asphalt_track"), finish(markings, matte), finish(kerb, matte), finish(safety, metal),
		finish(posts, matte), finish(trim, metal), textured(grass, "leafy_grass"), finish(hills, matte), finish(trees, matte) }
end

local function shell(mesh: sn.MeshBuilder, slices: { { number } }, tint: sn.Color)
	local function row(s: { number })
		local z, width, low, top, upperWidth = table.unpack(s)
		return { v3(-width, low, z), v3(-width, top - 0.11, z), v3(-upperWidth, top, z),
			v3(upperWidth, top, z), v3(width, top - 0.11, z), v3(width, low, z) }
	end
	for i = 1, #slices - 1 do
		local a, b = row(slices[i]), row(slices[i + 1])
		for j = 1, #a do
			local k = j % #a + 1
			quad(mesh, a[j], b[j], b[k], a[k], tint)
		end
	end
	local a, b = row(slices[1]), row(slices[#slices])
	quad(mesh, a[6], a[1], a[2], a[5], tint)
	quad(mesh, a[5], a[2], a[3], a[4], tint)
	quad(mesh, b[1], b[6], b[5], b[2], tint)
	quad(mesh, b[2], b[5], b[4], b[3], tint)
end

function Art.cars()
	local paint, dark, glass, rim = flatTexture(1, 0.24, 0.36), flatTexture(1, 0.77, 0.0), flatTexture(1, 0.14, 0.45), flatTexture(1, 0.22, 0.84)
	local black = color(0.043, 0.048, 0.052, 1)
	local cabins, chassis, lampMesh, brakeMesh = sn.MeshBuilder.new(), sn.MeshBuilder.new(), sn.MeshBuilder.new(), sn.MeshBuilder.new()
	shell(cabins, {
		{ -0.92, 0.77, 0.66, 0.79, 0.73 }, { -0.27, 0.74, 0.67, 1.30, 0.59 },
		{ 0.52, 0.73, 0.68, 1.31, 0.58 }, { 1.33, 0.73, 0.66, 0.80, 0.69 },
	}, color(0.055, 0.093, 0.115, 1))
	box(chassis, 0, 0.23, -2.07, 1.76, 0.13, 0.25, black)
	box(chassis, 0, 0.31, 2.14, 1.61, 0.20, 0.18, black)
	box(chassis, 0, 1.03, 1.91, 1.91, 0.095, 0.35, black)
	box(chassis, 0, 0.54, 2.035, 0.47, 0.16, 0.17, color(0.69, 0.72, 0.66, 1))
	for _, side in { -1, 1 } do
		box(chassis, side * 0.61, 0.89, 1.92, 0.11, 0.27, 0.13, black)
		box(chassis, side * 0.83, 0.94, -0.22, 0.26, 0.10, 0.16, black)
		box(chassis, side * 0.87, 0.28, 0, 0.20, 0.17, 1.78, black)
		box(chassis, side * 0.65, 0.50, -2.00, 0.45, 0.12, 0.18, black)
		box(lampMesh, side * 0.67, 0.59, -1.995, 0.36, 0.058, 0.19, color(0.60, 0.71, 0.76, 1))
		box(lampMesh, side * 0.65, 0.62, 2.04, 0.40, 0.065, 0.075, color(0.42, 0.018, 0.012, 1))
		box(brakeMesh, side * 0.65, 0.62, 2.05, 0.38, 0.055, 0.075, color(0.55, 0.014, 0.01, 1))
	end
	local wheels = sn.MeshBuilder.new()
	local hubs = sn.MeshBuilder.new()
	wheels:addCylinder(0.33, 0.27, 28, color(0.032, 0.037, 0.041, 1))
	hubs:addCylinder(0.224, 0.286, 20, color(0.29, 0.34, 0.36, 1))
	for i = 0, 3 do
		local n = i * math.pi * 0.5
		for _, side in { -1, 1 } do
			box(hubs, 0.124 * math.cos(n), side * 0.150, 0.124 * math.sin(n), 0.07, 0.018, 0.07, black)
		end
	end
	local bodies = {}
	for _, tint in {
		color(0.038, 0.37, 0.54, 1), color(0.58, 0.052, 0.037, 1), color(0.62, 0.43, 0.062, 1),
		color(0.57, 0.59, 0.61, 1), color(0.12, 0.41, 0.17, 1), color(0.24, 0.11, 0.29, 1),
	} do
		local body = sn.MeshBuilder.new()
		shell(body, {
			{ -2.14, 0.73, 0.27, 0.54, 0.63 }, { -1.6, 0.94, 0.25, 0.70, 0.82 },
			{ -0.78, 0.96, 0.23, 0.76, 0.90 }, { 0.30, 0.90, 0.25, 0.76, 0.84 },
			{ 1.30, 0.97, 0.25, 0.80, 0.88 }, { 2.02, 0.89, 0.31, 0.71, 0.80 },
		}, tint)
		table.insert(bodies, finish(body, paint))
	end
	local shadow = sn.MeshBuilder.new()
	for i = 0, 23 do
		local a, b = i * math.pi / 12, (i + 1) * math.pi / 12
		local shade = color(0.019, 0.023, 0.025, 1)
		local normal = v3(0, 1, 0)
		shadow:triangle(shadow:vertex(v3(0), normal, nil, shade),
			shadow:vertex(v3(math.cos(b) * 0.99, 0, math.sin(b) * 2.20), normal, nil, shade),
			shadow:vertex(v3(math.cos(a) * 0.99, 0, math.sin(a) * 2.20), normal, nil, shade))
	end
	return { bodies = bodies, chassis = finish(chassis, dark), cabin = finish(cabins, glass), lamp = finish(lampMesh, glass),
		brake = finish(brakeMesh, dark, flatTexture(0.84, 0.046, 0.027)), wheel = finish(wheels, dark),
		hub = finish(hubs, rim), shadow = finish(shadow, dark) }
end

return Art
