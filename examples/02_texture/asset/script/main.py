# Create a image
import sinen as sn
image = sn.Texture()
image.load("logo.png")

def update():
    pass
def draw():
    # Draw image
    sn.Graphics.draw_image(image, sn.Rect(sn.Vec2(0), image:size()))