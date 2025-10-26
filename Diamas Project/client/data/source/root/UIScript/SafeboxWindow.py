import uiScriptLocale

ROOT_PATH = "d:/ymir work/ui/gui/fix/"
window = {
    "name": "SafeboxWindow",
    "x": 100,
    "y": 20,
    "style": (
        "moveable",
        "float",
    ),
    "width": 176,
    "height": 250,
    "children": (
        {
            "name": "board",
            "type": "board",
            "x": 0,
            "y": 0,
            "width": 176,
            "height": 250,
            "children": (
                ## Title
                {
                    "name": "TitleBar",
                    "type": "titlebar",
                    "style": ("attach",),
                    "x": 8,
                    "y": 0,
                    "width": 191,
                    "title": uiScriptLocale.MALL_TITLE,
                },
                ## Button
                ## Button
                {
                    "name": "ChangePasswordButton",
                    "type": "button",
                    "x": 0,
                    "y": 58,
                    "text": uiScriptLocale.SAFE_CHANGE_PASSWORD,
                    "horizontal_align": "center",
                    "vertical_align": "bottom",
                    "default_image": "d:/ymir work/ui/public/large_button_01.sub",
                    "over_image": "d:/ymir work/ui/public/large_button_02.sub",
                    "down_image": "d:/ymir work/ui/public/large_button_03.sub",
                },
                {
                    "name": "ExitButton",
                    "type": "button",
                    "x": 0,
                    "y": 25,
                    "text": uiScriptLocale.CLOSE,
                    "horizontal_align": "center",
                    "vertical_align": "bottom",
                    "default_image": "d:/ymir work/ui/public/large_button_01.sub",
                    "over_image": "d:/ymir work/ui/public/large_button_02.sub",
                    "down_image": "d:/ymir work/ui/public/large_button_03.sub",
                },
            ),
        },
    ),
}
