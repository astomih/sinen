import sinen as sn

from .gui_manager import GUIManager

gm = GUIManager()


class Button:
    def __init__(self):
        pass

    def show(self, text: str, position: sn.Vec2, size: sn.Vec2) -> bool:
        mouse_pos = sn.Mouse.position_on_scene()
        mouse_pressed = sn.Mouse.is_pressed(sn.Mouse.LEFT)
        rect = sn.Rect(position.x, position.y, size.x, size.y)

        hovered = False
        top_left = sn.Vec2(rect.x - rect.width / 2, rect.y - rect.height / 2)
        bottom_right = sn.Vec2(rect.x + rect.width / 2, rect.y + rect.height / 2)
        hovered = (top_left.x <= mouse_pos.x and mouse_pos.x <= bottom_right.x) and (
            top_left.y <= mouse_pos.y and mouse_pos.y <= bottom_right.y
        )
        clicked = hovered and mouse_pressed

        gm.rects.append(rect)
        gm.rect_colors.append(
            sn.Color(0.7, 0.7, 0.7, 1.0) if hovered else sn.Color(0.5, 0.5, 0.5, 1.0)
        )
        gm.texts.append(text)
        gm.text_colors.append(sn.Color(1, 1, 1, 1))

        return clicked
