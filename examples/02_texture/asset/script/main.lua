-- Create a image
local image = sn.Texture()
image:Load("logo.png")

function Update()
    if sn.Keyboard.IsPressed(sn.Keyboard.ESCAPE) then
        sn.Scene.Change("main", ".")
    end
end

function Draw()
    -- Draw image
    sn.Graphics.DrawImage(image, sn.Rect(sn.Vec2(0), image:Size()))
end
