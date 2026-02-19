local GUIManager = require("./gui/gui_manager")
local G = {} :: {
	NOW_STAGE: number,
	SCORE: number,
	COLLISION_SPACE: any,
	COLLISION_SPACE_DIVISION: number,
	TILE_SIZE: number,
	DEFAULT_TEXTURE: Texture?,
	DEFAULT_FONT_NAME: string,
	MAP_CHIP: {
		FLOOR: number,
		WALL: number,
		STAIR: number,
		KEY: number,
		PLAYER: number,
	},
	MAP_CHIP_WALKABLE: number,
	FONT_DEFAULT32: Font,
	FONT_DEFAULT64: Font,
	GUI_MANAGER: GUIManager.GUIManager,
}
G.NOW_STAGE = 1
G.SCORE = 0
G.COLLISION_SPACE = {}
G.COLLISION_SPACE_DIVISION = 0
G.TILE_SIZE = 4
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

G.GUI_MANAGER = GUIManager.new()
return G
