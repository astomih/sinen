local hello_texture = {}
local hello_font = {}
local hello_drawer = {}

function setup()
	hello_texture = texture()
	hello_drawer = draw2d(hello_texture)
	hello_font = font()
	hello_font:load(DEFAULT_FONT, 64)
	hello_font:render_text(hello_texture, "Hello Sinen World!", color(1, 1, 1, 1))
	hello_drawer.scale = hello_texture:size()
end

function update()
	hello_drawer:draw()
end
