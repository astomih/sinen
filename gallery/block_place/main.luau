local sn = require("@sinen")

local BOARD_SIZE = 8
local BOARD_CELL = 62.0
local BOARD_LEFT = -248.0
local BOARD_TOP = 420.0

local SLOT_X = { -230.0, 0.0, 230.0 }
local SLOT_Y = -430.0
local SLOT_WIDTH = 200.0
local SLOT_HEIGHT = 170.0

local BOARD_COLOR = sn.Color.new(0.12, 0.15, 0.2, 1.0)
local EMPTY_CELL_COLOR = sn.Color.new(0.17, 0.2, 0.27, 1.0)

local titleFont = sn.Font.new()
local hudFont = sn.Font.new()
local infoFont = sn.Font.new()

local board = {}
local rack = {} :: { { shape: Shape, colorIndex: number } }
local selectedSlot = 0
local score = 0
local bestScore = 0
local combo = 0
local gameOver = false

local PALETTE = {
	{ 0.95, 0.39, 0.3 },
	{ 0.35, 0.78, 0.97 },
	{ 0.4, 0.88, 0.46 },
	{ 0.97, 0.74, 0.3 },
	{ 0.74, 0.55, 0.98 },
	{ 0.98, 0.52, 0.71 },
}
type Shape = {
	cells: { { x: number, y: number } },
	w: number,
	h: number,
}

local function makeShape(cells: { { number } }): Shape
	local packed = {}
	local w = 0
	local h = 0
	for i = 1, #cells do
		local x = cells[i][1]
		local y = cells[i][2]
		packed[i] = { x = x, y = y }
		if x + 1 > w then
			w = x + 1
		end
		if y + 1 > h then
			h = y + 1
		end
	end
	return {
		cells = packed,
		w = w,
		h = h,
	} :: {
		cells: { { x: number, y: number } },
		w: number,
		h: number,
	}
end

local SHAPES = {
	makeShape({ { 0, 0 } }),
	makeShape({ { 0, 0 }, { 1, 0 } }),
	makeShape({ { 0, 0 }, { 0, 1 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 2, 0 } }),
	makeShape({ { 0, 0 }, { 0, 1 }, { 0, 2 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } }),
	makeShape({ { 0, 0 }, { 0, 1 }, { 1, 1 } }),
	makeShape({ { 1, 0 }, { 0, 1 }, { 1, 1 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 0, 1 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 1, 1 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 } }),
	makeShape({ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 1, 1 } }),
	makeShape({ { 0, 1 }, { 1, 1 }, { 2, 1 }, { 1, 0 } }),
	makeShape({ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 1 } }),
	makeShape({ { 1, 0 }, { 1, 1 }, { 1, 2 }, { 0, 1 } }),
	makeShape({ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 2 } }),
	makeShape({ { 1, 0 }, { 1, 1 }, { 1, 2 }, { 0, 2 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 2, 1 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 } }),
	makeShape({ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, { 0, 2 }, { 1, 2 } }),
	makeShape({ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, { 0, 2 }, { 1, 2 }, { 2, 2 } }),
}

local function colorByIndex(index: number, alpha: number)
	local c = PALETTE[index]
	local a = alpha
	if a == nil then
		a = 1.0
	end
	return sn.Color.new(c[1], c[2], c[3], a)
end

local function boardCellCenter(row: number, col: number)
	local x = BOARD_LEFT + (col - 0.5) * BOARD_CELL
	local y = BOARD_TOP - (row - 0.5) * BOARD_CELL
	return x, y
end

local function mouseToBoardCell(mx: number, my: number): (number?, number?)
	local boardWidth = BOARD_SIZE * BOARD_CELL
	local boardHeight = BOARD_SIZE * BOARD_CELL
	if mx < BOARD_LEFT or mx > BOARD_LEFT + boardWidth then
		return nil, nil
	end
	if my > BOARD_TOP or my < BOARD_TOP - boardHeight then
		return nil, nil
	end

	local col: number = math.floor((mx - BOARD_LEFT) / BOARD_CELL) + 1
	local row: number = math.floor((BOARD_TOP - my) / BOARD_CELL) + 1
	if row < 1 or row > BOARD_SIZE or col < 1 or col > BOARD_SIZE then
		return nil, nil
	end
	return row, col
end

local function canPlace(shape: Shape, row: number, col: number): boolean
	for i = 1, #shape.cells do
		local cell = shape.cells[i]
		local r = row + cell.y
		local c = col + cell.x
		if r < 1 or r > BOARD_SIZE or c < 1 or c > BOARD_SIZE then
			return false
		end
		if board[r][c] ~= 0 then
			return false
		end
	end
	return true
end

local function clearFilledLines()
	local rows = {}
	local cols = {}
	local lines = 0

	for r = 1, BOARD_SIZE do
		local full = true
		for c = 1, BOARD_SIZE do
			if board[r][c] == 0 then
				full = false
				break
			end
		end
		if full then
			rows[r] = true
			lines = lines + 1
		end
	end

	for c = 1, BOARD_SIZE do
		local full = true
		for r = 1, BOARD_SIZE do
			if board[r][c] == 0 then
				full = false
				break
			end
		end
		if full then
			cols[c] = true
			lines = lines + 1
		end
	end

	if lines == 0 then
		return 0, 0
	end

	local removed = 0
	for r = 1, BOARD_SIZE do
		for c = 1, BOARD_SIZE do
			if rows[r] or cols[c] then
				if board[r][c] ~= 0 then
					removed = removed + 1
				end
				board[r][c] = 0
			end
		end
	end

	return lines, removed
end

local function randomPiece(): { shape: Shape, colorIndex: number }
	local shapeIndex = sn.Random.getIntRange(1, #SHAPES)
	local colorIndex = sn.Random.getIntRange(1, #PALETTE)
	return {
		shape = SHAPES[shapeIndex],
		colorIndex = colorIndex,
	}
end

local function rackIsEmpty()
	for i = 1, 3 do
		if rack[i] ~= nil then
			return false
		end
	end
	return true
end

local function refillRack()
	for i = 1, 3 do
		rack[i] = randomPiece()
	end
	selectedSlot = 0
end

local function hasAnyMove()
	for slot = 1, 3 do
		local piece = rack[slot]
		if piece ~= nil then
			for row = 1, BOARD_SIZE do
				for col = 1, BOARD_SIZE do
					if canPlace(piece.shape, row, col) then
						return true
					end
				end
			end
		end
	end
	return false
end

local function restartGame()
	if score > bestScore then
		bestScore = score
	end
	for r = 1, BOARD_SIZE do
		board[r] = {}
		for c = 1, BOARD_SIZE do
			board[r][c] = 0
		end
	end
	score = 0
	combo = 0
	gameOver = false
	refillRack()
end

local function updateBestScore()
	if score > bestScore then
		bestScore = score
	end
end

local function placeSelected(slot: number, row: number, col: number)
	local piece = rack[slot]
	if piece == nil then
		return
	end

	for i = 1, #piece.shape.cells do
		local cell = piece.shape.cells[i]
		local r = row + cell.y
		local c = col + cell.x
		board[r][c] = piece.colorIndex
	end

	score = score + #piece.shape.cells
	local lines, removed = clearFilledLines()
	if lines > 0 then
		combo = combo + 1
		score = score + removed * 4 + lines * 35 + combo * 12
	else
		combo = 0
	end

	rack[slot] = nil
	selectedSlot = 0

	if rackIsEmpty() then
		refillRack()
	end

	if not hasAnyMove() then
		gameOver = true
	end

	updateBestScore()
end

local function slotAt(mx, my): number?
	for i = 1, 3 do
		local x = SLOT_X[i]
		if
			mx >= x - SLOT_WIDTH * 0.5
			and mx <= x + SLOT_WIDTH * 0.5
			and my >= SLOT_Y - SLOT_HEIGHT * 0.5
			and my <= SLOT_Y + SLOT_HEIGHT * 0.5
		then
			return i
		end
	end
	return nil
end

local function drawShape(shape: Shape, left: number, top: number, size: number, color: sn.Color)
	for i = 1, #shape.cells do
		local cell = shape.cells[i]
		local x = left + cell.x * size + size * 0.5
		local y = top - cell.y * size - size * 0.5
		sn.Graphics.drawRect(sn.Rect.new(sn.Vec2.new(x, y), sn.Vec2.new(size - 4.0, size - 4.0)), color)
	end
end

local function drawBackground()
	local half = sn.Graphics.getCamera2D():half()
	local width = half.x * 2.0
	local height = half.y * 2.0

	sn.Graphics.drawRect(
		sn.Rect.new(sn.Vec2.new(0, 0), sn.Vec2.new(width + 2.0, height + 2.0)),
		sn.Color.new(0.05, 0.07, 0.1, 1.0)
	)

	local bands = 14
	local step = height / bands
	for i = 0, bands - 1 do
		local t = i / (bands - 1)
		local y = half.y - (i + 0.5) * step
		local r = 0.07 + t * 0.07
		local g = 0.1 + t * 0.08
		local b = 0.14 + t * 0.1
		sn.Graphics.drawRect(
			sn.Rect.new(sn.Vec2.new(0, y), sn.Vec2.new(width + 2.0, step + 1.0)),
			sn.Color.new(r, g, b, 0.42)
		)
	end
end

local function drawBoard()
	local boardSizePx = BOARD_SIZE * BOARD_CELL
	local frameX = BOARD_LEFT + boardSizePx * 0.5
	local frameY = BOARD_TOP - boardSizePx * 0.5
	sn.Graphics.drawRect(
		sn.Rect.new(sn.Vec2.new(frameX, frameY), sn.Vec2.new(boardSizePx + 26.0, boardSizePx + 26.0)),
		sn.Color.new(0.09, 0.11, 0.15, 1.0)
	)
	sn.Graphics.drawRect(
		sn.Rect.new(sn.Vec2.new(frameX, frameY), sn.Vec2.new(boardSizePx + 8.0, boardSizePx + 8.0)),
		BOARD_COLOR
	)

	for row = 1, BOARD_SIZE do
		for col = 1, BOARD_SIZE do
			local x, y = boardCellCenter(row, col)
			local index = board[row][col]
			if index == 0 then
				sn.Graphics.drawRect(
					sn.Rect.new(sn.Vec2.new(x, y), sn.Vec2.new(BOARD_CELL - 6.0, BOARD_CELL - 6.0)),
					EMPTY_CELL_COLOR
				)
			else
				sn.Graphics.drawRect(
					sn.Rect.new(sn.Vec2.new(x, y), sn.Vec2.new(BOARD_CELL - 6.0, BOARD_CELL - 6.0)),
					colorByIndex(index, 1.0)
				)
			end
		end
	end

	if selectedSlot ~= 0 and rack[selectedSlot] ~= nil and not gameOver then
		local m = sn.Mouse.getPositionOnScene()
		local row, col = mouseToBoardCell(m.x, m.y)
		if row ~= nil and col ~= nil then
			local piece = rack[selectedSlot]
			if canPlace(piece.shape, row, col) then
				for i = 1, #piece.shape.cells do
					local cell = piece.shape.cells[i]
					local pr = row + cell.y
					local pc = col + cell.x
					local x, y = boardCellCenter(pr, pc)
					sn.Graphics.drawRect(
						sn.Rect.new(sn.Vec2.new(x, y), sn.Vec2.new(BOARD_CELL - 12.0, BOARD_CELL - 12.0)),
						colorByIndex(piece.colorIndex, 0.45)
					)
				end
			end
		end
	end
end

local function drawRack()
	for i = 1, 3 do
		local x = SLOT_X[i]
		local selected = selectedSlot == i
		local baseColor = sn.Color.new(0.14, 0.17, 0.23, 1.0)
		if selected then
			baseColor = sn.Color.new(0.2, 0.26, 0.34, 1.0)
		end

		sn.Graphics.drawRect(sn.Rect.new(sn.Vec2.new(x, SLOT_Y), sn.Vec2.new(SLOT_WIDTH, SLOT_HEIGHT)), baseColor)
		sn.Graphics.drawText(
			"PIECE " .. i,
			infoFont,
			sn.Vec2.new(x, SLOT_Y + 58.0),
			sn.Color.new(0.85, 0.9, 0.98, 0.95),
			16
		)

		local piece = rack[i]
		if piece == nil then
			sn.Graphics.drawText("USED", hudFont, sn.Vec2.new(x, SLOT_Y - 6.0), sn.Color.new(0.7, 0.75, 0.86, 0.95), 22)
		else
			local previewCell = 26.0
			local drawW = piece.shape.w * previewCell
			local drawH = piece.shape.h * previewCell
			local left = x - drawW * 0.5
			local top = SLOT_Y + drawH * 0.5 - 10.0
			drawShape(piece.shape, left, top, previewCell, colorByIndex(piece.colorIndex, 1.0))
		end
	end
end

local function drawHUD()
	sn.Graphics.drawText("BLOCK PLACE", titleFont, sn.Vec2.new(0.0, 586.0), sn.Color.new(1.0, 1.0, 1.0, 0.95), 42)
	sn.Graphics.drawText("Score  " .. score, hudFont, sn.Vec2.new(0.0, 544.0), sn.Color.new(0.96, 0.97, 1.0, 0.95), 28)
	sn.Graphics.drawText(
		"Best   " .. bestScore,
		hudFont,
		sn.Vec2.new(0.0, 510.0),
		sn.Color.new(0.8, 0.88, 1.0, 0.95),
		24
	)
	if combo > 1 then
		sn.Graphics.drawText(
			"Combo x" .. combo,
			hudFont,
			sn.Vec2.new(0.0, 476.0),
			sn.Color.new(1.0, 0.88, 0.45, 0.98),
			24
		)
	end

	sn.Graphics.drawText(
		"CLICK PIECE -> CLICK BOARD | SPACE: RESTART | ESC: BACK",
		infoFont,
		sn.Vec2.new(0.0, -610.0),
		sn.Color.new(0.85, 0.9, 0.98, 0.92),
		16
	)
end

local function drawGameOver()
	if not gameOver then
		return
	end

	local half = sn.Graphics.getCamera2D():half()
	sn.Graphics.drawRect(
		sn.Rect.new(sn.Vec2.new(0, 0), sn.Vec2.new(half.x * 2.0 + 2.0, half.y * 2.0 + 2.0)),
		sn.Color.new(0, 0, 0, 0.62)
	)
	sn.Graphics.drawText("NO MORE MOVES", titleFont, sn.Vec2.new(0, 70), sn.Color.new(1, 1, 1, 0.98), 48)
	sn.Graphics.drawText(
		"SPACE or LEFT CLICK to restart",
		hudFont,
		sn.Vec2.new(0, 6),
		sn.Color.new(0.88, 0.93, 1.0, 0.95),
		24
	)
	sn.Graphics.drawText(
		"ESC to return gallery",
		infoFont,
		sn.Vec2.new(0, -38),
		sn.Color.new(0.78, 0.86, 0.98, 0.92),
		20
	)
end

function setup()
	sn.Graphics.resetGraphicsPipeline()
	sn.Graphics.getCamera2D():resize(sn.Vec2.new(720, 1280))
	titleFont:load(44)
	hudFont:load(26)
	infoFont:load(20)
	restartGame()
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		sn.Script.load("main", ".")
		return
	end

	if sn.Keyboard.isPressed(sn.Keyboard.SPACE) then
		restartGame()
		return
	end

	if sn.Mouse.isPressed(sn.Mouse.LEFT) then
		if gameOver then
			restartGame()
			return
		end

		local m = sn.Mouse.getPositionOnScene()
		local slot = slotAt(m.x, m.y)
		if slot ~= nil then
			if rack[slot] ~= nil then
				selectedSlot = slot
			end
			return
		end

		if selectedSlot ~= 0 and rack[selectedSlot] ~= nil then
			local row, col = mouseToBoardCell(m.x, m.y)
			if row ~= nil and col ~= nil and canPlace(rack[selectedSlot].shape, row, col) then
				placeSelected(selectedSlot, row, col)
			end
		end
	end
end

function draw()
	drawBackground()
	drawBoard()
	drawRack()
	drawHUD()
	drawGameOver()
end
