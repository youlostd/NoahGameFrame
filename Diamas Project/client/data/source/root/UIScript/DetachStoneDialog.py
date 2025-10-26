import uiScriptLocale

window = {
    "name": "AttachStoneDialog",
    "style": ("moveable", "float",),

    "x": 276,
    "y": 177,

    "width": 276 + 5,
    "height": 187 + 5,

    "children":
        (
            {
                "name": "Board",
                "type": "board",

                "x": 0,
                "y": 0,

                "width": 276,
                "height": 187,
                "children":
                    (
                        {
                            "name": "TitleBar",
                            "type": "titlebar",
                            "style": ("attach",),

                            "x": 6,
                            "y": -3,

                            "width": 276 - 13,

                            "r": 0.95, "g": 0.71, "b": 0.04, "a": 1,
                            "title" : "Stone Extract",

                        },

                        {
                            "name": "main",
                            "type": "thinboard_circle",
                            "x": 12,
                            "y": 43,

                            "width": 53,
                            "height": 117,

                            "alpha": 0.4,

                            "children":
                                (
                                    {
                                        "name": "main_slot",
                                        "type": "slot",

                                        "x": 10,
                                        "y": 12,

                                        "width": 32,
                                        "height": 32 * 3,

                                        "image": "d:/ymir work/ui/public/Slot_Base.sub",

                                        "slot":
                                            (
                                                {"index": 0, "x": 0, "y": 0, "width": 32, "height": 96},
                                                {"index": 1, "x": 0, "y": 32, "width": 32, "height": 32},
                                                {"index": 2, "x": 0, "y": 64, "width": 32, "height": 32},

                                            ),
                                    },

                                ),

                        },


                        {
                            "name": "sub",
                            "type": "thinboard_circle",
                            "x": 71,
                            "y": 43,

                            "width": 189,
                            "height": 127,

                            "alpha": 0.4,

                            "children":
                                (
                                    {
                                        "name": "title_board",
                                        "type": "thinboard_circle",
                                        "x": 2,
                                        "y": 2,

                                        "width": 186,
                                        "height": 20,

                                        "alpha": 0.2,

                                        "children":
                                            (
                                                {
                                                    'name': 'title_text', 'type': 'text',
                                                    "color": 0x000000,
                                                    'x': 0, 'y': 2,
                                                    'text': "Stone +9",
                                                    'horizontal_align': 'center'
                                                },
                                            ),
                                    },


                                    {
                                        "name": "sub_slot",
                                        "type": "slot",

                                        "x": 17,
                                        "y": 43,

                                        "width": 32 * 4 + 10 * 3,
                                        "height": 32,

                                        "image": "d:/ymir work/ui/public/Slot_Base.sub",

                                        "slot":
                                            (
                                                {"index": 0, "x": 0, "y": 0, "width": 32, "height": 32},
                                                {"index": 1, "x": 32 * 1 + 10 * 1, "y": 0, "width": 32, "height": 32},
                                                {"index": 2, "x": 32 * 2 + 10 * 2, "y": 0, "width": 32, "height": 32},
                                                {"index": 3, "x": 32 * 3 + 10 * 3, "y": 0, "width": 32, "height": 32},

                                            ),
                                    },
                                    {
                                        "name": "ExtractButton",
                                        "type": "button",

                                        "x": 54,
                                        "y": 90,

                                        "text": "Extract",
                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                                    },
                                ),

                        },

                        {
                            "name": "explosion_effect",
                            "type": "ani_image",

                            "x": 83,
                            "y": 71,

                            "delay": 6,

                            "images":
                                (
                                    "D:/Ymir Work/UI/minigame/catchking/effect/explosion/1.sub",
                                    "D:/Ymir Work/UI/minigame/catchking/effect/explosion/2.sub",
                                    "D:/Ymir Work/UI/minigame/catchking/effect/explosion/3.sub",
                                    "D:/Ymir Work/UI/minigame/catchking/effect/explosion/4.sub",
                                    "D:/Ymir Work/UI/minigame/catchking/effect/explosion/5.sub",
                                    "D:/Ymir Work/UI/minigame/catchking/effect/explosion/6.sub",
                                    "D:/Ymir Work/UI/minigame/catchking/effect/explosion/7.sub",
                                    "D:/Ymir Work/UI/minigame/catchking/effect/explosion/8.sub",
                                ),
                        },
                    ),
            },
        ),
}
