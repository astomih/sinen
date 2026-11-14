local Track = {}

Track.halfWidth = 6.8
Track.wall = 11.8
Track.gates = 8

-- Driving direction starts along the pit straight, towards negative Z.
local anchors = {
	{ 0, 106 }, { 0, 20 }, { 1, -82 }, { 18, -125 }, { 62, -131 },
	{ 107, -100 }, { 135, -55 }, { 114, -25 }, { 101, 3 }, { 142, 34 },
	{ 158, 83 }, { 133, 127 }, { 78, 146 }, { 23, 143 },
}

function Track.height(x: number, z: number): number
	return 1.7 * math.sin(x * 0.017) + 1.15 * math.sin(z * 0.023) * math.sin(x * 0.011 - 0.5)
end

type Sample = { x: number, z: number, s: number }
local nodes: { Sample } = {}
local length = 0
local function curve(a: number, b: number, c: number, d: number, t: number): number
	return 0.5 * (2 * b + (c - a) * t + (2 * a - 5 * b + 4 * c - d) * t ^ 2
		+ (3 * b - a - 3 * c + d) * t ^ 3)
end
for i, b in anchors do
	local a = anchors[(i - 2) % #anchors + 1]
	local c = anchors[i % #anchors + 1]
	local d = anchors[(i + 1) % #anchors + 1]
	for j = 0, 47 do
		local t = j / 48
		local x, z = curve(a[1], b[1], c[1], d[1], t), curve(a[2], b[2], c[2], d[2], t)
		local prev = nodes[#nodes]
		if prev then
			length += math.sqrt((x - prev.x) ^ 2 + (z - prev.z) ^ 2)
		end
		table.insert(nodes, { x = x, z = z, s = length })
	end
end
length += math.sqrt((nodes[1].x - nodes[#nodes].x) ^ 2 + (nodes[1].z - nodes[#nodes].z) ^ 2)
Track.length = length
Track.gateSpacing = length / Track.gates

export type Frame = { x: number, z: number, tx: number, tz: number, s: number }

function Track.at(distance: number, lateral: number?): Frame
	local s = distance % length
	local low, high = 1, #nodes
	while low < high do
		local middle = math.floor((low + high + 1) / 2)
		if nodes[middle].s <= s then
			low = middle
		else
			high = middle - 1
		end
	end
	local a, b = nodes[low], nodes[low % #nodes + 1]
	local span = (if low == #nodes then length else b.s) - a.s
	local tx, tz = (b.x - a.x) / span, (b.z - a.z) / span
	local side = lateral or 0
	return { x = a.x + tx * (s - a.s) - tz * side, z = a.z + tz * (s - a.s) + tx * side, tx = tx, tz = tz, s = s }
end

function Track.project(x: number, z: number): (Frame, number)
	local best = math.huge
	local index, offset, lateral = 1, 0, 0
	for i, a in nodes do
		local b = nodes[i % #nodes + 1]
		local dx, dz = b.x - a.x, b.z - a.z
		local ox, oz = x - a.x, z - a.z
		local t = math.clamp((ox * dx + oz * dz) / (dx * dx + dz * dz), 0, 1)
		local errorX, errorZ = ox - dx * t, oz - dz * t
		local errorSquared = errorX * errorX + errorZ * errorZ
		if errorSquared < best then
			best, index, offset = errorSquared, i, t
			lateral = (dx * oz - dz * ox) / math.sqrt(dx * dx + dz * dz)
		end
	end
	local first = nodes[index].s
	local last = if index == #nodes then length else nodes[index + 1].s
	return Track.at(first + (last - first) * offset), lateral
end

return Track
