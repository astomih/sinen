local texture = sn.Texture()
local font = sn.Font()
font:load(16)
font:render_text(texture, "Press SPACE key to play sound", sn.Color(1, 1, 1, 1))
local draw2d = sn.Draw2D(texture)
draw2d.scale = texture:size()
local sound = sn.Sound()
sound:load("shot.wav")



-- Set scale to texture size

function update()
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    if sn.Keyboard.is_pressed(sn.Keyboard.SPACE) then
        sound:play()
    end
end

function draw()
    -- Draw texture
    sn.Graphics.
        draw2d(draw2d)
end
