import uiScriptLocale

ROOT = "d:/ymir work/ui/game/"
FACE_SLOT_FILE = "d:/ymir work/ui/game/windows/box_face.sub"
LARGE_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_03.sub"

window = {
    "name": "ExchangeDialog",
    "x": 0,
    "y": 0,
    "style": (
        "moveable",
        "float",
    ),
    "width": 462,
    "height": 280,
    "children": (
        {
            "name": "board",
            "type": "board_with_titlebar",
            "style": ("attach",),
            "x": 0,
            "y": 0,
            "width": 462,
            "height": 280,
            "title": uiScriptLocale.EXCHANGE_TITLE,
            "children": (
                {
                    "name": "inner_board",
                    "type": "thinboard_old",
                    "style": ("attach",),
                    "x": 5,
                    "y": 40,
                    "width": 450,
                    "height": 229,
                    "children": (
                        ## Target (left)
                        {
                            "name": "Target",
                            "type": "window",
                            "x": 10,
                            "y": 0,
                            "width": 202,
                            "height": 230,
                            "children": (
                                # Character Info
                                {
                                    "name": "InfoParent",
                                    "x": 57,
                                    "y": 2,
                                    "width": 120,
                                    "height": 5,
                                    "children": (
                                        {
                                            "name": "Target_Level",
                                            "type": "text",
                                            "text": "Lv. 999",
                                            "horizontal_align": "left",
                                            "x": 0,
                                            "y": 8,
                                            "color": 0xDE5912,
                                        },
                                        {
                                            "name": "Target_Name",
                                            "type": "text",
                                            "text": "TargetPlayer",
                                            "horizontal_align": "left",
                                            "x": 46,
                                            "y": 8,
                                            "color": 0xDEB639,
                                        },
                                    ),
                                },
                                # Trade board and light
                                {
                                    "name": "Target_Money",
                                    "type": "image",
                                    "x": 17,
                                    "y": 172,
                                    "image": "d:/ymir work/ui/public/parameter_slot_05.sub",
                                    "children": (
                                        {
                                            "name": "Target_Money_Value",
                                            "type": "text",
                                            "x": 14,
                                            "y": 3,
                                            "text": "1.234.567",
                                            "horizontal_align": "right",
                                        },
                                    ),
                                },
                                {
                                    "name": "Target_Money_Hint",
                                    "type": "image",
                                    "x": 27,
                                    "y": 175,
                                    "image": "d:/ymir work/ui/gui/sibtrade/money_icon.png",
                                },
                                # Trade board
                                {
                                    "name": "Target_Slot",
                                    "type": "grid_table",
                                    "start_index": 0,
                                    "x": 10,
                                    "y": 30,
                                    "x_count": 6,
                                    "y_count": 4,
                                    "x_step": 32,
                                    "y_step": 32,
                                    "x_blank": 0,
                                    "y_blank": 0,
                                    "image": "d:/ymir work/ui/public/slot_base.sub",
                                },
                                # Trade overlay (when accepted)
                                {
                                    "name": "Target_Overlay",
                                    "type": "image",
                                    "x": 10,
                                    "y": 30,
                                    "style": ("not_pick",),
                                    "image": "d:/ymir work/ui/public/exchange_accepted_overlay.tga",
                                    "text": "OK"
                                },
                                {
                                    "name": "Middle_Exchange_Button2",
                                    "type": "button",
                                    "x": 50,
                                    "y": 200,
                                    "default_image": "d:/ymir work/ui/gui/sibtrade/decline.png",
                                    "over_image": "d:/ymir work/ui/gui/sibtrade/decline2.png",
                                    "down_image": "d:/ymir work/ui/gui/sibtrade/decline.png",
                                    "text": "OK"
                                },
                            ),
                        },
                        ## Middle exchange button
                        {
                            "name": "MiddleSeperator",
                            "type": "image",
                            "x": 210,
                            "y": 0,
                            "image": "d:/ymir work/ui/gui/sibtrade/Strich.png",
                        },
                        ## Owner (right)
                        {
                            "name": "Owner",
                            "type": "window",
                            "x": 241,
                            "y": 0,
                            "width": 205,
                            "height": 230,
                            "children": (
                                # Info about the other player
                                # { "name" : "Owner_Race_Image", "type" : "image", "x" : 5, "y" : 5, "image" : "d:/ymir work/ui/game/windows/face_warrior.sub" },
                                # { "name" : "Owner_Race_Slot", "type" : "image", "x" : 2, "y" : 2, "image" : FACE_SLOT_FILE, },
                                # Character Info
                                {
                                    "name": "InfoParent",
                                    "x": 57,
                                    "y": 2,
                                    "width": 120,
                                    "height": 5,
                                    "children": (
                                        {
                                            "name": "Character_Level",
                                            "type": "text",
                                            "text": "Lv. 999",
                                            "horizontal_align": "left",
                                            "x": 0,
                                            "y": 8,
                                            "color": 0xDE5912,
                                        },
                                        {
                                            "name": "Character_Name",
                                            "type": "text",
                                            "text": "NotLoaded",
                                            "horizontal_align": "left",
                                            "x": 46,
                                            "y": 8,
                                            "color": 0xDEB639,
                                        },
                                    ),
                                },
                                {
                                    "name": "Owner_Money",
                                    "type": "button",
                                    "x": 17,
                                    "y": 172,
                                    "default_image": "d:/ymir work/ui/public/parameter_slot_05.sub",
                                    "over_image": "d:/ymir work/ui/public/parameter_slot_05.sub",
                                    "down_image": "d:/ymir work/ui/public/parameter_slot_05.sub",
                                    "children": (
                                        {
                                            "name": "Owner_Money_Value",
                                            "type": "text",
                                            "x": 12,
                                            "y": 3,
                                            "text": "3.456.789",
                                            "horizontal_align": "right",
                                        },
                                    ),
                                },
                                {
                                    "name": "Owner_Money_Hint",
                                    "type": "image",
                                    "x": 25,
                                    "y": 175,
                                    "image": "d:/ymir work/ui/gui/sibtrade/money_icon.png",
                                },
                                {
                                    "name": "Owner_Slot",
                                    "type": "grid_table",
                                    "start_index": 0,
                                    "x": 10,
                                    "y": 30,
                                    "x_count": 6,
                                    "y_count": 4,
                                    "x_step": 32,
                                    "y_step": 32,
                                    "x_blank": 0,
                                    "y_blank": 0,
                                    "image": "d:/ymir work/ui/public/slot_base.sub",
                                },
                                # Trade overlay (when accepted)
                                {
                                    "name": "Owner_Overlay",
                                    "type": "image",
                                    "x": 10,
                                    "y": 30,
                                    "style": ("not_pick",),
                                    "image": "d:/ymir work/ui/public/exchange_accepted_overlay.tga",
                                    "text": "OK"
                                },
                                {
                                    "name": "Middle_Exchange_Button",
                                    "type": "button",
                                    "x": 50,
                                    "y": 200,
                                    "default_image": "d:/ymir work/ui/gui/sibtrade/decline.png",
                                    "over_image": "d:/ymir work/ui/gui/sibtrade/decline2.png",
                                    "down_image": "d:/ymir work/ui/gui/sibtrade/decline.png",
                                    "text": "OK"
                                },
                            ),
                        },
                    ),
                },
            ),
        },
    ),
}
