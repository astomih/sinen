-- Create a image
local image = sn.Texture()
image:load("logo.png")

function update()
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function draw()
    -- Draw image
    sn.Graphics.draw_image(image, sn.Rect(sn.Vec2(0), image:size()))
end
