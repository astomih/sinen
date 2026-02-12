local image: Texture = Texture.new()
function setup()
	-- Create a image
	image:load("logo.png")
end

function update()
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
end

function draw()
	-- Draw image
	Graphics.drawImage(image, Rect.new(Vec2.new(0), image:size()))
end
