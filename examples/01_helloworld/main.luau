local font: Font = Font.new()
font:load(32)
function update()
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
end

function draw()
	Graphics.drawText("Hello World!", font, Vec2.new(0, 0), Color.new(1.0), 32)
end
