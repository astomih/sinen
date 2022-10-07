---------------------------------------------------------------------------
-- - - - - - - - - - - - - - - - Tile object - - - - - - - - - - - - - - -- 
---------------------------------------------------------------------------
-- Tile objects:
--  * Keeps track of room association, if not in room (default): roomId = 0
--  * Has graphic symbol to represent what kind of tile this is in a level
--    *   " " for empty
--    *   "." for floor
--    *   "#" for wall
--    *   "<" for ascending staircase
--    *   ">" for descending staircase
--    *   "%" for soil
--    *   "*" for mineral vein
--    *   "'" for open door
--    *   "+" for closed door
Tile = {class, roomId}
Tile.__index = Tile

Tile.EMPTY = 1
Tile.FLOOR = 0
Tile.WALL = 1
Tile.A_STAIRCASE = 2
Tile.D_STAIRCASE = 0
Tile.SOIL = 1
Tile.VEIN = 1
Tile.C_DOOR = 0
Tile.O_DOOR = 0

Tile.PLAYER = 3
Tile.BOSS = 0

function Tile:new(t)
    local tile = {}
    tile.class = t
    tile.roomId = 0

    setmetatable(tile, Tile)

    return tile

end

-- ##### -- ##### -- ##### -- ##### -- ##### -- ##### -- ##### -- ##### -- ##### --

function Tile:isWall()
    return (self.class == Tile.WALL or self.class == Tile.SOIL or self.class ==
               Tile.VEIN)
end
