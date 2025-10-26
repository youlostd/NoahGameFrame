import uiScriptLocale
import item
import app


window = {
    "name": "NewCostumeWindow",

    "x": SCREEN_WIDTH - 175 - 185,
    "y": SCREEN_HEIGHT - 37 - 605,

    "style" : ("moveable", "float",),

    "width": 140,
    "height": (180 + 47 + 47 + 50),

    "children":
        (
            {
                "name": "board",
                "type": "board",

                "x": 0,
                "y": 0,

                "width": 140,
                "height": (180 + 47 + 47 + 50),

                "children":
                    (
                        ## Title
                        {
                            "name": "TitleBar",
                            "type": "titlebar",
                            "style": ("attach",),

                            "x": 6,
                            "y": -3,

                            "width": 130,
                            "r":0.95,"g":0.71,"b":0.04,"a":1,
                            "title" : uiScriptLocale.COSTUME_WINDOW_TITLE,

      
                        },

                        ## Equipment Slot
                        {
                            "name": "Costume_Base",
                            "type": "expanded_image",

                            "x": 13,
                            "y": 38,

                            "image": "d:/ymir work/ui/costume/new_costume_bg.jpg",

                            "children":
                                (

                                    {
                                        "name": "CostumeSlot",
                                        "type": "slot",

                                        "x": 3,
                                        "y": 3,

                                        "width": 131,
                                        "height": 155 + 47 + 50,

                                        "slot": (
                                            {"index": 0, "x": 62, "y": 45, "width": 32,
                                             "height": 64},
                                            {"index": 1, "x": 62, "y": 9, "width": 32,
                                             "height": 32},
                                            {"index": 2, "x": 13, "y": 14, "width": 32,
                                             "height": 96},
                                            {"index": 3, "x": 13, "y": 127, "width": 32,
                                             "height": 32},
                                            {"index": 4, "x": 13, "y": 177, "width": 32,
                                             "height": 32},
                                            {"index": 5, "x": 62, "y": 176, "width": 32,
                                             "height": 32},
                                            {"index": 6, "x": 62, "y": 126, "width": 32,
                                             "height": 32},
                                            {"index": 7, "x": 13, "y": 176 + 50, "width": 32,
                                             "height": 32},
                                        ),
                                    },
                                ),
                        },

                    ),
            },
        ),
}
