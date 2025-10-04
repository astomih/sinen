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
        if mouse_pos.x >= rect.x and mouse_pos.x <= rect.x + rect.width:
            if mouse_pos.y >= rect.y and mouse_pos.y <= rect.y + rect.height:
                hovered = True

        clicked = hovered and mouse_pressed

        gm.rects.append(rect)
        gm.rect_colors.append(
            sn.Color(0.7, 0.7, 0.7, 1.0) if hovered else sn.Color(0.5, 0.5, 0.5, 1.0)
        )
        gm.texts.append(text)
        gm.text_colors.append(sn.Color(1, 1, 1, 1))

        return clicked
