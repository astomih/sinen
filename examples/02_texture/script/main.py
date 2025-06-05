import sinen as sn

# Create a texture
texture = sn.Texture()
texture.load("logo.png")
# Create a draw2D
draw2d = sn.Draw2D()
draw2d.material.append(texture)

# Set scale to texture size
draw2d.scale = texture.size()

def update():
    pass
def draw():
    # Draw texture
    draw2d.draw()
