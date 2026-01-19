local image = sn.Texture.new()
function setup()
    -- Create a image
    image:load("logo.png")
end

function update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
end

function draw()
    -- Draw image
    sn.Graphics.drawImage(image,
        sn.Rect.new(sn.Vec2.new(0), image:size()))
end
