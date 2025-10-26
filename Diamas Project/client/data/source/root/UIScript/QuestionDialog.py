import uiScriptLocale

BUTTON_ROOT = "d:/ymir work/ui/public/"
window = {
    "name": "QuestionDialog",
    "style": (
        "moveable",
        "float",
    ),
    "x": SCREEN_WIDTH / 2 - 125,
    "y": SCREEN_HEIGHT / 2 - 52,
    "width": 270,
    "height": 100,
    "children": (
        {
            "name": "board",
            "type": "board_with_titlebar",
            "close_button": False,
            "x": 0,
            "y": 0,
            "width": 270,
            "height": 100,
            "title": "Dikkat!",
            "children": (
                {
                    "name": "message",
                    "type": "text",
                    "x": 0,
                    "y": -10,
                    "text": uiScriptLocale.MESSAGE,
                    "horizontal_align": "center",
                    "vertical_align": "center",
                },
                {
                    "name": "accept",
                    "type": "button",
                    "x": -80,
                    "y": 63,
                    "horizontal_align": "center",
                    "default_image": BUTTON_ROOT + "AcceptButton00.sub",
                    "over_image": BUTTON_ROOT + "AcceptButton01.sub",
                    "down_image": BUTTON_ROOT + "AcceptButton02.sub",
                    "x_scale": 1.0,
                    "y_scale": 1.0,
                },
                {
                    "name": "cancel",
                    "type": "button",
                    "x": 80,
                    "y": 63,
                    "horizontal_align": "center",
                    "default_image": BUTTON_ROOT + "CancleButton00.sub",
                    "over_image": BUTTON_ROOT + "CancleButton01.sub",
                    "down_image": BUTTON_ROOT + "CancleButton02.sub",
                    "x_scale": 1.0,
                    "y_scale": 1.0,
                },
            ),
        },
    ),
}
