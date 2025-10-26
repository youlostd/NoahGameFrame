import uiScriptLocale

window = {
    "name": "SplitItemDialog",
    "x": 100,
    "y": 100,
    "style": (
        "moveable",
        "float",
    ),
    "width": 170,
    "height": 125,
    "children": (
        {
            "name": "board",
            "type": "board_with_titlebar",
            "x": 0,
            "y": 0,
            "width": 170,
            "height": 125,
            "color": 0x000000,
            "title": uiScriptLocale.PICK_MONEY_TITLE,
            "children": (
                ## Money Slot
                {
                    "name": "money_slot",
                    "type": "image",
                    "x": -50,
                    "y": 40,
                    "horizontal_align": "center",
                    "image": "d:/ymir work/ui/public/Parameter_Slot_02.sub",
                    "children": (
                        {
                            "name": "money_value",
                            "type": "editline",
                            "x": 3,
                            "y": 2,
                            "width": 60,
                            "height": 18,
                            "input_limit": 6,
                            "only_number": 1,
                            "text": "1",
                        },
                        {
                            "name": "max_value",
                            "type": "text",
                            "x": 63,
                            "y": 0,
                            "text": "/ 999999",
                        },
                    ),
                },
                ## Split checkbox
                {
                    "name": "split_checkbox",
                    "type": "checkbox",
                    "x": 35,  # -6 - 61 / 2,
                    "y": 67,
                    "checked": False,
                    "text": uiScriptLocale.SPLIT_ITEM,
                },
                ## Button
                {
                    "name": "accept_button",
                    "type": "button",
                    "x": 158 / 2 - 61 - 5,
                    "y": 90,
                    "default_image": "d:/ymir work/ui/public/acceptbutton00.sub",
                    "over_image": "d:/ymir work/ui/public/acceptbutton01.sub",
                    "down_image": "d:/ymir work/ui/public/acceptbutton02.sub",
                    "x_scale": 1.0,
                    "y_scale": 1.0,
                },
                {
                    "name": "cancel_button",
                    "type": "button",
                    "x": 178 / 2 + 5,
                    "y": 90,
                    "default_image": "d:/ymir work/ui/public/canclebutton00.sub",
                    "over_image": "d:/ymir work/ui/public/canclebutton01.sub",
                    "down_image": "d:/ymir work/ui/public/canclebutton02.sub",
                    "x_scale": 1.0,
                    "y_scale": 1.0,
                },
            ),
        },
    ),
}
