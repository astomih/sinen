-- Create a image
local image = sn.Texture()
image:Load("logo.png")

function Update()
end

function Draw()
    -- Draw image
    sn.Graphics.DrawImage(image, sn.Rect(sn.Vec2(0), image:Size()))
end
