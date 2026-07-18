local sn = require("@sinen")

local BFSGrid = {}
BFSGrid.__index = BFSGrid

local function truncToInt(v: number): number
	if v >= 0 then
		return math.floor(v)
	end
	return math.ceil(v)
end

local function index0(w: number, x0: number, y0: number): number
	return y0 * w + x0 + 1
end

local function inBounds0(self, x0: number, y0: number): boolean
	return x0 >= 0 and y0 >= 0 and x0 < self._w and y0 < self._h
end

local function queueClear(self: BFSGrid)
	self._qx = {}
	self._qy = {}
	self._qh = 1
	self._qt = 0
end

local function queuePush(self: BFSGrid, x0: number, y0: number)
	self._qt += 1
	self._qx[self._qt] = x0
	self._qy[self._qt] = y0
end

local function queuePop(self: BFSGrid): (number, number)
	local x0 = self._qx[self._qh]
	local y0 = self._qy[self._qh]
	self._qx[self._qh] = nil
	self._qy[self._qh] = nil
	self._qh += 1
	return x0 :: number, y0 :: number
end

local function queueEmpty(self: BFSGrid): boolean
	return self._qh > self._qt
end

local function stackClear(self: BFSGrid)
	self._shortest = {}
end

local function distAt(self, x0: number, y0: number): number
	return self._dist[index0(self._w, x0, y0)]
end

local function setDist(self, x0: number, y0: number, value: number)
	self._dist[index0(self._w, x0, y0)] = value
end

local function setPrev(self, x0: number, y0: number, px0: number, py0: number)
	local idx = index0(self._w, x0, y0)
	self._prevX[idx] = px0
	self._prevY[idx] = py0
end

local function fieldAt(self, x0: number, y0: number): number
	return self._field[index0(self._w, x0, y0)]
end

local function backtrace(self: BFSGrid, endX0: number, endY0: number)
	local x = endX0
	local y = endY0
	while x ~= -1 and y ~= -1 do
		table.insert(self._shortest, sn.Vec2.new(x, y))
		local idx = index0(self._w, x, y)
		local px = self._prevX[idx]
		local py = self._prevY[idx]
		x = px
		y = py
	end
end

BFSGrid.new = function(grid)
	local width = grid:width()
	local height = grid:height()
	local field = table.create(width * height, 0)
	for y0 = 0, height - 1 do
		for x0 = 0, width - 1 do
			field[index0(width, x0, y0)] = grid:_at0(x0, y0)
		end
	end

	local self = setmetatable(
		{} :: {
			_w: number,
			_h: number,
			_field: { number },
			_dist: { number },
			_prevX: { number },
			_prevY: { number },
			_shortest: { sn.Vec2 },
			_qx: { number? },
			_qy: { number? },
			_qh: number,
			_qt: number,
			width: (self: any) -> number,
			height: (self: any) -> number,
			findPath: (self: any, start: sn.Vec2, end_: sn.Vec2) -> any,
			trace: (self: any) -> sn.Vec2,
			traceable: (self: any) -> boolean,
			reset: (self: any) -> nil,
		},
		BFSGrid
	)
	self._w = width
	self._h = height
	self._field = field
	self._dist = table.create(width * height, -1)
	self._prevX = table.create(width * height, -1)
	self._prevY = table.create(width * height, -1)
	self._shortest = {}
	self._qx = {}
	self._qy = {}
	self._qh = 1
	self._qt = 0
	self:reset()
	return self
end

BFSGrid.width = function(self): number
	return self._w
end

BFSGrid.height = function(self): number
	return self._h
end

BFSGrid.reset = function(self: BFSGrid)
	for i = 1, #self._dist do
		self._dist[i] = -1
		self._prevX[i] = -1
		self._prevY[i] = -1
	end
	stackClear(self)
	queueClear(self)
end

BFSGrid.findPath = function(self: BFSGrid, start: sn.Vec2, end_: sn.Vec2): any
	local startX0 = truncToInt(start.x)
	local startY0 = truncToInt(start.y)
	local endX0 = truncToInt(end_.x)
	local endY0 = truncToInt(end_.y)

	if not inBounds0(self, startX0, startY0) then
		return false
	end
	if not inBounds0(self, endX0, endY0) then
		return false
	end

	setDist(self, startX0, startY0, 0)
	queuePush(self, startX0, startY0)

	local dx = { 0, 1, 0, -1 }
	local dy = { 1, 0, -1, 0 }

	while not queueEmpty(self) do
		local x, y = queuePop(self)

		for i = 1, 4 do
			local nextX = x + dx[i]
			local nextY = y + dy[i]

			if not inBounds0(self, nextX, nextY) then
				continue
			end
			if fieldAt(self, nextX, nextY) < 0 then
				continue
			end
			if distAt(self, nextX, nextY) ~= -1 then
				continue
			end

			queuePush(self, nextX, nextY)
			setDist(self, nextX, nextY, distAt(self, x, y) + 1)
			setPrev(self, nextX, nextY, x, y)

			if nextX == endX0 and nextY == endY0 then
				backtrace(self, endX0, endY0)
				return true
			end
		end
	end

	return false
end

BFSGrid.traceable = function(self: BFSGrid): boolean
	return #self._shortest > 0
end

BFSGrid.trace = function(self: BFSGrid): sn.Vec2
	local nextNode = table.remove(self._shortest)
	if nextNode == nil then
		error("BFSGrid.trace called with no traceable path")
	end
	return nextNode
end

export type BFSGrid = typeof(BFSGrid.new({
	width = function()
		return 1
	end,
	height = function()
		return 1
	end,
	_at0 = function(_, _, _)
		return 0
	end,
} :: any))

return BFSGrid
