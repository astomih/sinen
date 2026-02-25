--!nonstrict

local Grid = {}
Grid.__index = Grid

local function isInteger(v: number): boolean
	return v == math.floor(v)
end

local function assertInteger(name: string, v: number)
	if not isInteger(v) then
		error(string.format("Grid.%s must be an integer, got %s", name, tostring(v)))
	end
end

local function assertPositiveOrZero(name: string, v: number)
	assertInteger(name, v)
	if v < 0 then
		error(string.format("Grid.%s must be >= 0, got %d", name, v))
	end
end

local function index0(w: number, x0: number, y0: number): number
	return y0 * w + x0 + 1
end

local function inBounds0(self, x0: number, y0: number): boolean
	return x0 >= 0 and y0 >= 0 and x0 < self._w and y0 < self._h
end

local function inBounds1(self, x1: number, y1: number): boolean
	return x1 >= 1 and y1 >= 1 and x1 <= self._w and y1 <= self._h
end

Grid.new = function(w: number, h: number)
	assertPositiveOrZero("width", w)
	assertPositiveOrZero("height", h)

	local width = math.floor(w)
	local height = math.floor(h)
	local self = setmetatable(
		{} :: {
			_w: number,
			_h: number,
			_data: { number },
			at: (self: any, x: number, y: number) -> number,
			set: (self: any, x: number, y: number, v: number) -> nil,
			width: (self: any) -> number,
			height: (self: any) -> number,
			size: (self: any) -> number,
			clear: (self: any) -> nil,
			resize: (self: any, w_: number, h_: number) -> nil,
			fill: (self: any, value: number) -> nil,
			fillRect: (self: any, index: any, value: number) -> nil,
			setRow: (self: any, index: number, value: number) -> nil,
			setColumn: (self: any, index: number, value: number) -> nil,
			_at0: (self: any, x0: number, y0: number) -> number,
			_set0: (self: any, x0: number, y0: number, v: number) -> nil,
		},
		Grid
	)
	self._w = width
	self._h = height
	self._data = table.create(width * height, 0)
	return self
end

Grid.width = function(self): number
	return self._w
end

Grid.height = function(self): number
	return self._h
end

Grid.size = function(self): number
	return #self._data
end

Grid.clear = function(self)
	self._w = 0
	self._h = 0
	self._data = {}
end

Grid._at0 = function(self, x0: number, y0: number): number
	if not inBounds0(self, x0, y0) then
		error(string.format("Grid index out of bounds (%d, %d)", x0, y0))
	end
	return self._data[index0(self._w, x0, y0)]
end

Grid._set0 = function(self, x0: number, y0: number, v: number)
	if not inBounds0(self, x0, y0) then
		error(string.format("Grid index out of bounds (%d, %d)", x0, y0))
	end
	self._data[index0(self._w, x0, y0)] = v
end

Grid.at = function(self, x: number, y: number): number
	assertInteger("at.x", x)
	assertInteger("at.y", y)
	local xi = math.floor(x)
	local yi = math.floor(y)
	if not inBounds1(self, xi, yi) then
		error(string.format("Grid index out of bounds (%d, %d)", xi, yi))
	end
	return self._data[(yi - 1) * self._w + xi]
end

Grid.set = function(self, x: number, y: number, v: number)
	assertInteger("set.x", x)
	assertInteger("set.y", y)
	local xi = math.floor(x)
	local yi = math.floor(y)
	if not inBounds1(self, xi, yi) then
		error(string.format("Grid index out of bounds (%d, %d)", xi, yi))
	end
	self._data[(yi - 1) * self._w + xi] = v
end

Grid.resize = function(self, w_: number, h_: number)
	assertPositiveOrZero("resize.w", w_)
	assertPositiveOrZero("resize.h", h_)

	local w = math.floor(w_)
	local h = math.floor(h_)
	if w == 0 or h == 0 then
		self:clear()
		return
	end

	local oldW = self._w
	local oldH = self._h
	local oldData = self._data

	local newData = table.create(w * h, 0)
	local copyW = math.min(oldW, w)
	local copyH = math.min(oldH, h)
	for y = 1, copyH do
		for x = 1, copyW do
			newData[(y - 1) * w + x] = oldData[(y - 1) * oldW + x]
		end
	end

	self._w = w
	self._h = h
	self._data = newData
end

Grid.fill = function(self, value: number)
	for i = 1, #self._data do
		self._data[i] = value
	end
end

Grid.fillRect = function(self, indexOrRect, value: number)
	if type(indexOrRect) == "number" then
		self:setRow(indexOrRect, value)
		return
	end

	if type(indexOrRect) ~= "table" then
		error("Grid.fillRect expects row index or Rect-like table")
	end

	local rect = indexOrRect
	local x1 = math.floor(rect.x)
	local y1 = math.floor(rect.y)
	local w = math.floor(rect.width)
	local h = math.floor(rect.height)

	if w <= 0 or h <= 0 then
		return
	end

	local x2 = x1 + w - 1
	local y2 = y1 + h - 1
	for y = y1, y2 do
		for x = x1, x2 do
			if inBounds1(self, x, y) then
				self._data[(y - 1) * self._w + x] = value
			end
		end
	end
end

Grid.setRow = function(self, index: number, value: number)
	assertInteger("setRow.index", index)
	local row = math.floor(index)
	if row < 1 or row > self._h then
		error(string.format("Grid row out of bounds (%d)", row))
	end
	local offset = (row - 1) * self._w
	for x = 1, self._w do
		self._data[offset + x] = value
	end
end

Grid.setColumn = function(self, index: number, value: number)
	assertInteger("setColumn.index", index)
	local col = math.floor(index)
	if col < 1 or col > self._w then
		error(string.format("Grid column out of bounds (%d)", col))
	end
	for y = 1, self._h do
		self._data[(y - 1) * self._w + col] = value
	end
end

export type Grid = typeof(Grid.new(1, 1))

return Grid
