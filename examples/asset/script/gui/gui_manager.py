import sinen as sn


class GUIManager:
    rects = []
    rect_colors = []
    texts = []
    text_colors = []

    def __init__(self):
        pass

    def update(self):
        list.clear(self.rects)
        list.clear(self.rect_colors)
        list.clear(self.texts)
        list.clear(self.text_colors)

    def draw(self):
        for i in range(len(self.rects)):
            sn.Graphics.draw_rect(self.rects[i], self.rect_colors[i])
            if self.texts[i] != "":
                sn.Graphics.draw_text(
                    self.texts[i],
                    sn.Vec2(self.rects[i].x, self.rects[i].y),
                    self.text_colors[i],
                    16
                )
