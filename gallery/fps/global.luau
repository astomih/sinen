local G = {}
G.NOW_STAGE = 1
G.SCORE = 0
G.COLLISION_SPACE = {}
G.COLLISION_SPACE_DIVISION = 0
G.TILE_SIZE = 4
G.DEFAULT_TEXTURE = {}
G.DEFAULT_FONT_NAME = "font/x16y32pxGridGazer.ttf"
G.MAP_CHIP = {
	-- Under 0 is Not Walkable
	FLOOR = 0,
	WALL = -1,
	STAIR = 2,
	KEY = 3,
	PLAYER = 4,
}
G.MAP_CHIP_WALKABLE = 0
G.FONT_DEFAULT32 = Font.new()
G.FONT_DEFAULT32:load(32, G.DEFAULT_FONT_NAME)
G.FONT_DEFAULT64 = Font.new()
G.FONT_DEFAULT64:load(64, G.DEFAULT_FONT_NAME)
-- FONT_DEFAULT128 = sn.Font.new()
-- FONT_DEFAULT128:load(128, DEFAULT_FONT_NAME)

local GuiManager = require("./gui/gui_manager")
G.GUI_MANAGER = GuiManager.new()
return G
