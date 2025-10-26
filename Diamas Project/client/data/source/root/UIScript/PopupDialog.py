import uiScriptLocale

BUTTON_ROOT = "d:/ymir work/ui/public/"

window = {
    "name": "PopupDialog",
    "style": ("float",),
    "x": SCREEN_WIDTH / 2 - 250,
    "y": SCREEN_HEIGHT / 2 - 40,
    "width": 280,
    "height": 90,
    "children": (
        {
            "name": "board",
            "type": "board",
            "x": 0,
            "y": 0,
            "width": 280,
            "height": 90,
            "title": "Bilgi",
            "children": (
                {
                    "name": "content",
                    "type": "window",
                    "x": 0,
                    "y": 0,
                    "width": 280,
                    "height": 60,
                    "children": (
                        {
                            "name": "message",
                            "type": "text",
                            "x": 0,
                            "y": 0,
                            "text": uiScriptLocale.MESSAGE,
                            "horizontal_align": "center",
                            "vertical_align": "center",
                        },
                    ),
                },
                {
                    "name": "accept",
                    "type": "button",
                    "x": 0,
                    "y": 50,
                    "horizontal_align": "center",
                    "default_image": BUTTON_ROOT + "AcceptButton00.sub",
                    "over_image": BUTTON_ROOT + "AcceptButton01.sub",
                    "down_image": BUTTON_ROOT + "AcceptButton02.sub",
                    "x_scale": 1.0,
                    "y_scale": 1.0,
                },
            ),
        },
    ),
}
