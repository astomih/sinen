-- Create a image
local image = sn.Texture.new()
image:load("logo.png")

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function Draw()
    -- Draw image
    sn.Graphics.drawImage(image, sn.Rect.new(sn.Vec2.new(0), image:size()))
end
