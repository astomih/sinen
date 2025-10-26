-- Create a image
local image = sn.Texture.new()
image:load("logo.png")

function update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function draw()
    -- Draw image
    sn.Graphics.drawImage(image, sn.Rect.new(sn.Vec2.new(0), image:size()))
end
