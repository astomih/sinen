local DungeonModule = require("dungeon_generator/Dungeon")
local LevelModule = require("dungeon_generator/Level")
local FuncModule = require("dungeon_generator/MainHelpFunc")
local function dungeon_generator()
    local generator = {
        generate = function(self, map, mx, my)
            -- Settings for level sizes and number of levels in dungeon.
            local height = my - 1
            local width = mx - 1
            nrOfLevels = 5

            local dungeon = Dungeon:new(nrOfLevels, height, width)

            -- generate with default settings
            dungeon:generateDungeon()

            -- generate with advanced settings, 
            -- params: (advanced, maxRooms, maxRoomSize, scatteringFactor)
            -- dungeon:generateDungeon(true, 30, 10, 30)

            -- inits a player in level 1, a boss in last level
            initPlayer(dungeon.levels[1])
            initBoss(dungeon.levels[#dungeon.levels])

            -- dungeon:printDungeon()
            for i = 1, mx do
                map[i] = {}
                for j = 1, my do
                    map[i][j] = dungeon.levels[1].matrix[i - 1][j - 1].class
                end
            end
            for i = 1, mx do
                for j = 1, my do
                    if i == 1 or i == mx then map[i][j] = 1 end
                    if j == 1 or j == my then map[i][j] = 1 end
                end
            end

        end
    }
    return generator
end

return dungeon_generator
