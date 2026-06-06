local sn = require("@sinen")
local G = require("../global")

local examples = {
	{ label = "Archive", dir = "archive" },
	{ label = "Video Encode", dir = "video_encode" },
	{ label = "Video Decode", dir = "video_decode" },
}

local numExamples = #examples
local scrollY = 0.0

function setup()
	sn.Graphics.resetGraphicsPipeline()
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
end

local buttonSize = sn.Vec2.new(512, 64)
local buttonStep = 70.0
local headerHeight = 88.0
local margin = 24.0

local function itemRect(index: number, viewport: sn.Rect): sn.Rect
	local centerX = viewport.x + viewport.width * 0.5
	local y = viewport.y + (index - 1) * buttonStep - scrollY
	return sn.Rect.new(sn.Pivot.TopCenter, centerX, y, buttonSize.x, buttonSize.y)
end

local function intersectsViewport(rect: sn.Rect, viewport: sn.Rect): boolean
	return rect.y + rect.height >= viewport.y and rect.y <= viewport.y + viewport.height
end

function draw()
	sn.Gui.setFont(G.font)
	sn.Gui.setFontSize(24)
	local windowSize = sn.Window.size()
	local title = "Media Examples"
	sn.Graphics.drawText(
		title,
		sn.TextStyle.new(G.font, sn.Color.new(1, 1, 1, 1), 24),
		sn.TextTransform.new(sn.Vec2.new(windowSize.x * 0.5, margin), 0.0, sn.Pivot.TopCenter)
	)

	local viewport = sn.Rect.new(
		margin,
		headerHeight,
		windowSize.x - margin * 2.0,
		math.max(80.0, windowSize.y - headerHeight - margin)
	)
	local contentHeight = numExamples * buttonStep - (buttonStep - buttonSize.y)
	scrollY = sn.Gui.scrollVertical(scrollY, viewport, contentHeight, buttonStep)

	for i = 1, numExamples do
		local rect = itemRect(i, viewport)
		if intersectsViewport(rect, viewport) and sn.Gui.button(examples[i].label, rect) then
			sn.Script.load("/media/" .. examples[i].dir .. "/main.luau")
		end
	end
end
