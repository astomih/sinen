NOW_STAGE = 1
SCORE = 0
COLLISION_SPACE = {}
COLLISION_SPACE_DIVISION = 0
TILE_SIZE = 4
DEFAULT_TEXTURE = {}
DEFAULT_FONT_NAME = "x16y32pxGridGazer.ttf"
MAP_CHIP = {
  -- Under 0 is Not Walkable
  FLOOR = 0,
  WALL = -1,
  STAIR = 2,
  KEY = 3,
  PLAYER = 4,
}
MAP_CHIP_WALKABLE = 0

GUI_MANAGER = require("gui/gui_manager")()
