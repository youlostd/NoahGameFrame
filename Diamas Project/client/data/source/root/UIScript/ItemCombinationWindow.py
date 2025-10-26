import uiScriptLocale

BUTTON_ROOT = "d:/ymir work/ui/public/"

window = {
    "name": "Item_Combination_Window",
    "x": 0,
    "y": 0,
    "style": (
        "moveable",
        "float",
    ),
    "width": 205,
    "height": 400,
    "children": (
        {
            "name": "board",
            "type": "board_with_titlebar",
            "style": ("attach",),
            "x": 0,
            "y": 0,
            "width": 205,
            "height": 400,
            "title": uiScriptLocale.COMB_TITLE,
            "children": (
                ## Background Image
                {
                    "name": "Item_Comb_Background_Image",
                    "type": "image",
                    "style": ("not_pick",),
                    "x": 8,
                    "y": 30,
                    "image": "d:/ymir work/ui/combination/comb1.tga",
                },
                ## Slot
                {
                    "name": "MediumSlot",
                    "type": "slot",
                    "image": "d:/ymir work/ui/public/slot_base.sub",
                    "x": 8,
                    "y": 30,
                    "width": 190,
                    "height": 40,
                    "slot": (
                        {
                            "index": 0,
                            "x": 78,
                            "y": 12,
                            "width": 31,
                            "height": 31,
                        },  # ��ũ��
                    ),
                },
                {
                    "name": "CombSlot",
                    "type": "slot",
                    "x": 8,
                    "y": 90,
                    "width": 190,
                    "height": 300,
                    "slot": (
                        {
                            "index": 1,
                            "x": 28,
                            "y": 8,
                            "width": 31,
                            "height": 96,
                        },
                        {
                            "index": 2,
                            "x": 129,
                            "y": 8,
                            "width": 31,
                            "height": 96,
                        },
                        {
                            "index": 3,
                            "x": 79,
                            "y": 125,
                            "width": 31,
                            "height": 96,
                        },
                    ),
                    "children": (
                        {
                            "name": "Slot1_Name",
                            "type": "text",
                            "x": -100,
                            "y": -20,
                            "text": uiScriptLocale.COMB_APPEARANCE,
                            "horizontal_align": "center",
                        },
                        {
                            "name": "Slot2_Name",
                            "type": "text",
                            "x": 100,
                            "y": -20,
                            "text": uiScriptLocale.COMB_ATTRIBUTE,
                            "horizontal_align": "center",
                        },
                    ),
                },
                ## Button
                {
                    "name": "AcceptButton",
                    "type": "button",
                    "x": 40,
                    "y": 360,
                    "default_image": BUTTON_ROOT + "AcceptButton00.sub",
                    "over_image": BUTTON_ROOT + "AcceptButton01.sub",
                    "down_image": BUTTON_ROOT + "AcceptButton02.sub",
                    "x_scale": 1.0,
                    "y_scale": 1.0,
                },
                {
                    "name": "CancelButton",
                    "type": "button",
                    "x": 114,
                    "y": 360,
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
