import uiScriptLocale

BUTTON_ROOT = "d:/ymir work/ui/public/"

window = {
    "name": "PickMoneyDialog",
    "x": 100,
    "y": 100,
    "style": (
        "moveable",
        "float",
    ),
    "width": 160,
    "height": 95,
    "children": (
        {
            "name": "board",
            "type": "board_with_titlebar",
            "x": 0,
            "y": 0,
            "width": 160,
            "height": 95,
            "color": 0x000000,
            "title": uiScriptLocale.PICK_MONEY_TITLE,
            "children": (
                ## Money Slot
                {
                    "name": "money_slot",
                    "type": "image",
                    "x": 20,
                    "y": 35,
                    "image": "d:/ymir work/ui/public/Parameter_Slot_02.sub",
                    "children": (
                        {
                            "name": "money_value",
                            "type": "editline",
                            "x": 3,
                            "y": 2,
                            "width": 60,
                            "height": 18,
                            "input_limit": 3,
                            "only_number": 1,
                            "text": "1",
                        },
                        {
                            "name": "max_value",
                            "type": "text",
                            "x": 63,
                            "y": 3,
                            "text": "/ 999999",
                        },
                    ),
                },
                ## Button
                {
                    "name": "accept_button",
                    "type": "button",
                    "x": 150 / 2 - 61 - 5,
                    "y": 61,
                    "default_image": BUTTON_ROOT + "AcceptButton00.sub",
                    "over_image": BUTTON_ROOT + "AcceptButton01.sub",
                    "down_image": BUTTON_ROOT + "AcceptButton02.sub",
                    "x_scale": 1.0,
                    "y_scale": 1.0,
                },
                {
                    "name": "cancel_button",
                    "type": "button",
                    "x": 165 / 2 + 5,
                    "y": 61,
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
