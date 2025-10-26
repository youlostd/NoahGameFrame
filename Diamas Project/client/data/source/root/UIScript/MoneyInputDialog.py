import uiScriptLocale

window = {
    "name": "InputDialog",
    "x": 0,
    "y": 0,
    "style": (
        "moveable",
        "float",
    ),
    "width": 200,
    "height": 110,
    "children": (
        {
            "name": "board",
            "type": "board_with_titlebar",
            "close_button": False,
            "x": 0,
            "y": 0,
            "width": 200,
            "height": 110,
            "title": "",
            "children": (
                ## Input Slot
                {
                    "name": "InputSlot",
                    "type": "slotbar",
                    "x": 0,
                    "y": 34,
                    "width": 90,
                    "height": 18,
                    "horizontal_align": "center",
                    "children": (
                        {
                            "name": "InputValue",
                            "type": "editline",
                            "x": 3,
                            "y": 3,
                            "width": 90,
                            "height": 18,
                            "input_limit": PLAYER_NAME_MAX_LEN,
                        },
                    ),
                },
                ## Input Slot
                {
                    "name": "MoneyValue",
                    "type": "text",
                    "x": 0,
                    "y": 59,
                    "text": "999999999",
                    "horizontal_align": "center",
                    "horizontal_align": "center",
                },
                ## Button
                {
                    "name": "AcceptButton",
                    "type": "button",
                    "x": 30,
                    "y": 78,
                    "horizontal_align": "left",
                    "text": uiScriptLocale.OK,
                    "default_image": "d:/ymir work/ui/public/middle_button_01.sub",
                    "over_image": "d:/ymir work/ui/public/middle_button_02.sub",
                    "down_image": "d:/ymir work/ui/public/middle_button_03.sub",
                },
                {
                    "name": "CancelButton",
                    "type": "button",
                    "x": 30,
                    "y": 78,
                    "horizontal_align": "right",
                    "text": uiScriptLocale.CANCEL,
                    "default_image": "d:/ymir work/ui/public/middle_button_01.sub",
                    "over_image": "d:/ymir work/ui/public/middle_button_02.sub",
                    "down_image": "d:/ymir work/ui/public/middle_button_03.sub",
                },
            ),
        },
    ),
}
