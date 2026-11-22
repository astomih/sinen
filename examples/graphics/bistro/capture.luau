local scene = require("./scene")

function setup() scene.setup() end
function ready()
	scene.ready()
	scene.capture(true)
end
function update() end
function draw() scene.draw() end
