local texture = sn.Texture.new()
local font = sn.Font.new()
font:load(16)
font:renderText(texture, "Press SPACE key to play sound", sn.Color.new(1, 1, 1, 1))
local draw2d = sn.Draw2D.new(texture)
draw2d.scale = texture:size()
local sound = sn.Sound.new()
sound:load("shot.wav")



-- Set scale to texture size

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    if sn.Keyboard.isPressed(sn.Keyboard.SPACE) then
        sound:play()
    end
end

function Draw()
    -- Draw texture
    sn.Graphics.draw2D(draw2d)
end
