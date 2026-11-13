local sn = require("@sinen")
local Art = require("./art")

local v3, color = sn.Vec3.new, sn.Color.new
local Arena = {}

Arena.spawn = v3(0, 0, 12.4)
Arena.docks = {
	v3(0, 0, -13.5), v3(-9, 0, -11), v3(9, 0, -11),
	v3(-10, 0, 8), v3(10, 0, 8), v3(0, 0, 15),
}

function Arena.build(materials: Art.Materials): ({ Art.Part }, { Art.Box })
	local builders: { [string]: sn.MeshBuilder } = {}
	local solids: { Art.Box } = {}
	local white = color(1)
	local dark = color(0.17, 0.22, 0.25, 1)
	local yellow = color(0.73, 0.49, 0.12, 1)
	local blue = color(0.26, 0.39, 0.43, 1)

	local function block(material: string, x: number, y: number, z: number, w: number, h: number, d: number, tint: sn.Color, tile: number, solid: boolean?)
		local builder = builders[material]
		if not builder then
			builder = sn.MeshBuilder.new()
			builders[material] = builder
		end
		local p, size = v3(x, y, z), v3(w, h, d)
		Art.box(builder, p, size, tint, tile)
		if solid then
			local half = size * v3(0.5)
			table.insert(solids, { lo = p - half, hi = p + half })
		end
	end

	block("floor", 0, -0.22, 0, 26, 0.44, 34, white, 2.1, true)
	block("wall", 0, 6.65, 0, 26, 0.60, 34, color(0.61), 4, true)
	for _, side in { -1, 1 } do
		block("wall", side * 13, 3.2, 0, 0.6, 6.4, 34, white, 4, true)
		block("wall", 0, 3.2, side * 17, 26, 6.4, 0.6, white, 4, true)
		block("hardware", side * 12.61, 0.44, 0, 0.24, 0.88, 33.4, dark, 2)
		block("paint", side * 12.47, 1.05, 0, 0.04, 0.14, 33.2, yellow, 2)
		for _, z in { -12, -6, 0, 6, 12 } do
			block("hardware", side * 12.53, 3.1, z, 0.50, 6.2, 0.48, blue, 1)
			block("hardware", side * 12.23, 4.31, z, 0.16, 0.92, 1.24, dark, 1)
			block("cyan", side * 12.13, 4.31, z, 0.055, 0.72, 0.075, dark, 1)
			block("hardware", side * 10.8, 5.54, z, 3.5, 0.24, 0.34, blue, 1)
		end
	end

	for _, z in { -12, -6, 0, 6, 12 } do
		block("hardware", 0, 5.92, z, 25.4, 0.38, 0.40, dark, 2)
		for _, x in { -7.2, 0, 7.2 } do
			block("hardware", x, 5.68, z, 2.76, 0.19, 0.60, dark, 1)
			block("ivory", x, 5.56, z, 2.51, 0.07, 0.15, dark, 1)
		end
	end

	-- Central service lane and docking strips.
	for _, x in { -2.6, 2.6 } do
		block("paint", x, 0.006, 0, 0.075, 0.012, 29, yellow, 1)
		for z = -13, 13, 2 do
			block("paint", x * 1.07, 0.009, z, 0.10, 0.015, 0.62, yellow, 1)
		end
	end
	for _, dock in Arena.docks do
		for _, x in { -0.95, 0.95 } do
			block("plate", dock.x + x, 0.025, dock.z, 0.20, 0.05, 1.82, white, 0.55)
			block("amber", dock.x + x, 0.057, dock.z, 0.045, 0.014, 1.53, dark, 1)
		end
	end

	-- Box dimensions drive both the rendered cover and collision bounds.
	for _, p in {
		{ -5.0, -6.4, 3.6, 1.35, 1.55 }, { 5.8, -6.4, 3.2, 1.75, 1.55 },
		{ -7.4, 2.8, 2.6, 1.9, 1.8 }, { 6.6, 3.7, 3.1, 1.30, 1.7 },
		{ 0, -7.8, 2.6, 1.04, 1.25 }, { 0, 4.5, 2.3, 1.08, 1.6 },
	} do
		local x, z, w, h, d = table.unpack(p)
		block("plate", x, h * 0.5, z, w, h, d, white, 0.8, true)
		block("hardware", x, h + 0.025, z, w + 0.08, 0.06, d + 0.08, dark, 1)
		for _, edge in { -1, 1 } do
			block("paint", x + edge * (w / 2 - 0.15), h / 2, z, 0.12, h + 0.05, d + 0.05, yellow, 1)
			block("cyan", x, h - 0.16, z + edge * (d / 2 + 0.017), 0.53, 0.035, 0.03, dark, 1)
		end
	end

	-- End bulkhead, cooling fins and maintenance rails.
	for _, side in { -1, 1 } do
		block("hardware", side * 2.52, 2.5, -16.47, 0.38, 5.0, 0.38, dark, 1)
		block("cyan", side * 2.26, 2.5, -16.23, 0.10, 4.5, 0.08, dark, 1)
		block("plate", side * 7.5, 2.35, -16.59, 4.7, 3.9, 0.25, color(0.55), 0.75)
		for row = 0, 9 do
			block("hardware", side * 7.5, 0.72 + row * 0.34, -16.37, 4.5, 0.12, 0.31, dark, 1)
		end
	end
	block("hardware", 0, 4.92, -16.48, 5.35, 0.33, 0.44, blue, 1)
	block("ivory", 0, 4.66, -16.20, 4.12, 0.08, 0.08, dark, 1)
	block("plate", 0, 2.22, -16.59, 4.37, 4.42, 0.20, color(0.66), 0.9)
	block("amber", 0, 2.25, -16.43, 0.055, 3.8, 0.045, dark, 1)

	local parts: { Art.Part } = {}
	for name, builder in builders do
		local model = builder:toModel()
		local apply = (materials :: { [string]: (sn.Model) -> () })[name]
		apply(model)
		table.insert(parts, { model = model, transform = Art.transform(v3(0)) })
	end
	return parts, solids
end

return Arena
