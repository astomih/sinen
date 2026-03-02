local sn = require("@sinen")
local GUIManager = require("./gui/gui_manager")
local G = {}
G.font = sn.Font.new()
G.font:load(24)
G.guiManager = GUIManager.new()
return G
