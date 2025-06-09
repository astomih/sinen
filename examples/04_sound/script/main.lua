local texture = Texture()
local font = Font()
local draw2d = Draw2D()
local hello_sound = Sound()
hello_sound:load("shot.wav")

-- Create a texture
font:load(16)
font:render_text(texture, "Press SPACE key to play sound", Color(1, 1, 1, 1))

draw2d.material:append(texture)

-- Set scale to texture size
draw2d.scale = texture:size()

function update()
    if Keyboard.is_pressed(Keyboard.SPACE) then
        hello_sound:play()
    end
end

function draw()
    -- Draw texture
    draw2d:draw()
end
