-- Load this scene to reproduce direct.png and gi.png, then exit.
local scene = require("./scene")

function setup()
	scene.setup()
	scene.saveComparison(true)
end
function update() end
function draw() scene.draw() end
