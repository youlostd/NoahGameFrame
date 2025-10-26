import uiScriptLocale

ROOT_PATH = "d:/ymir work/ui/gui/"

window = {
    "name": "MainWindow",
    "style": ("moveable", "float",),

    "x": 300,
    "y": 300,

    "width": 468,
    "height": 230,

    "children":
        (

            {
                "name": "board",
                "type": "board_with_titlebar",
                "style": ("not_pick",),
                "width": 468,
                "height": 230,
                "x": 0,
                "y": 0,

                "title": "Pet",

                "children": (
                    {
                        "name": "BG2",
                        "type": "thinboard_red",
                        "style": ("attach", "not_pick", ),

                        "width": 458,
                        "height": 203,

                        "x": 7,
                        "y": 20,

                    },

                    {
                        "name": "PetImage",
                        "type": "image",
                        "style": ("not_pick", ),

                        "x": -3,
                        "y": 30,

                        "image": "d:/ymir work/ui/gui/petsystem/pet_flag.png",

                        "children": (
							{
								"name": "PetIcon",
								"type": "expanded_image",
								"style": ("not_pick",),

								"x": 35,
								"y": 30,
								"x_scale": 1.3,
								"y_scale": 1.3,

								"image": "icon/item/53001.tga",
							},
                            {
                                "name": "PetLevelText", "type": "text", "x": 0, "y": 83,
                                "horizontal_align": "center", "text": "Pet Level",
                                "fontname": "Nunito Sans:14b",
                                "color": (0xffffd348, 0xffca982f),
                            },
                            {
                                "name": "PetLevelValue", "type": "text", "x": -3, "y": 106,
                                "horizontal_align": "center", "text": "1",
                                "fontname": "Nunito Sans:14b",
                                "color": 0xff9e5753,
                            },

                        ),

                    },

                    {
                        "name": "status_bar",
                        "type": "image",

                        "x": 100,
                        "y": 45,

                        "image": "d:/ymir work/ui/gui/petsystem/info_bar.png",

                        "children": (
                            {
                                "name": "CharacterBarText", "type": "text", "x": 10, "y": 3,
                                "horizontal_align": "left", "text": "Statistics",
                                "fontname": "Nunito Sans:12b",
                                "outline": True,
                                "color": (0xffffd348, 0xffca982f),
                            },
                            {

                                "name": "Status_Plus_Label", "type": "text", "x": 50, "y": 3,
                                "horizontal_align": "right", "text": "Available",
                                "fontname": "Nunito Sans:12b",
                                "outline": True,
                                "color": (0xffffd348, 0xffca982f),

                                "children":
                                    (
                                        {
                                            "name": "Status_Plus_Label", "type": "text", "x": -38, "y": 0,
                                            "horizontal_align": "right", "text": "[      ]",
                                            "fontname": "Nunito Sans:12b",
                                            "outline": True,
                                            "color": (0xffffd348, 0xffca982f),
                                            "children": (
                                                {"name": "Status_Plus_Value", "type": "text", "x": 0, "y": 2,
                                                 "text": "9",
                                                 "horizontal_align": "center",
                                                 "text_color": 0xff9e5753,
                                                 "color": (0xff6fdc3e, 0xff5cb134),
                                                 },
                                            ),
                                        },

                                    ),
                            },

                        ),

                    },

                    {
                        "name": "PetAttr0",
                        "type": "window",
                        "style": ("not_pick",),

                        "x": 105,
                        "y": 73 + (0 * 20),

                        "width": 340,
                        "height": 20,

                        "children": (
                            {
                                "name": "PetAttr0Text", "type": "text", "x": 0, "y": 0,
                                "horizontal_align": "left", "text": "PetAttr0",
                                "fontname": "Nunito Sans:13b",
                                "outline": True,
                                "color": (0xffffd348, 0xffca982f),
                            },

                            {
                                "name": "PetAttr0ValueField",
                                "type": "image",
                                "style": ("not_pick",),
                                "horizontal_align": "right",

                                "x": 10,
                                "y": 0,

                                "image": "d:/ymir work/ui/gui/petsystem/info_slot.png",
                                "children": (
                                    {"name": "PetAttrValueField0_Value", "type": "text", "x": 0, "y": 2, "text": "999",
                                     "color": 0xff9e5753, "horizontal_align": "center"},

                                ),

                            },

                            {"name": "PetAttr0_Plus", "type": "button", "horizontal_align": "right", "x": -10, "y": 2,
                             "default_image": "d:/ymir work/ui/gui/stat-plus.sub",
                             "over_image": "d:/ymir work/ui/gui/stat-plus-hover.sub",
                             "down_image": "d:/ymir work/ui/gui/stat-plus-down.sub", },

                        ),

                    },
                    {
                        "name": "PetAttr1",
                        "type": "window",
                        "style": ("not_pick",),

                        "x": 105,
                        "y": 73 + (1 * 20),

                        "width": 340,
                        "height": 20,

                        "children": (
                            {
                                "name": "PetAttr1Text", "type": "text", "x": 0, "y": 0,
                                "horizontal_align": "left", "text": "PetAttr1",
                                "fontname": "Nunito Sans:13b",
                                "outline": True,
                                "color": (0xffffd348, 0xffca982f),
                            },

                            {
                                "name": "PetAttr1ValueField",
                                "type": "image",
                                "style": ("not_pick",),
                                "horizontal_align": "right",

                                "x": 10,
                                "y": 0,

                                "image": "d:/ymir work/ui/gui/petsystem/info_slot.png",
                                "children": (
                                    {"name": "PetAttrValueField1_Value", "type": "text", "x": 0, "y": 2, "text": "999",
                                     "color": 0xff9e5753, "horizontal_align": "center"},

                                ),

                            },

                            {"name": "PetAttr1_Plus", "type": "button", "horizontal_align": "right", "x": -10, "y": 2,
                             "default_image": "d:/ymir work/ui/gui/stat-plus.sub",
                             "over_image": "d:/ymir work/ui/gui/stat-plus-hover.sub",
                             "down_image": "d:/ymir work/ui/gui/stat-plus-down.sub", },

                        ),

                    },
                    {
                        "name": "PetAttr2",
                        "type": "window",
                        "style": ("not_pick",),

                        "x": 105,
                        "y": 73 + (2 * 20),

                        "width": 340,
                        "height": 20,

                        "children": (
                            {
                                "name": "PetAttr2Text", "type": "text", "x": 0, "y": 0,
                                "horizontal_align": "left", "text": "PetAttr2",
                                "fontname": "Nunito Sans:13b",
                                "outline": True,
                                "color": (0xffffd348, 0xffca982f),
                            },

                            {
                                "name": "PetAttr2ValueField",
                                "type": "image",
                                "style": ("not_pick",),
                                "horizontal_align": "right",

                                "x": 10,
                                "y": 0,

                                "image": "d:/ymir work/ui/gui/petsystem/info_slot.png",
                                "children": (
                                    {"name": "PetAttrValueField2_Value", "type": "text", "x": 0, "y": 2, "text": "999",
                                     "color": 0xff9e5753, "horizontal_align": "center"},

                                ),

                            },

                            {"name": "PetAttr2_Plus", "type": "button", "horizontal_align": "right", "x": -10, "y": 2,
                             "default_image": "d:/ymir work/ui/gui/stat-plus.sub",
                             "over_image": "d:/ymir work/ui/gui/stat-plus-hover.sub",
                             "down_image": "d:/ymir work/ui/gui/stat-plus-down.sub", },

                        ),

                    },
                    {
                        "name": "PetAttr3",
                        "type": "window",
                        "style": ("not_pick",),

                        "x": 105,
                        "y": 73 + (3 * 20),

                        "width": 340,
                        "height": 20,

                        "children": (
                            {
                                "name": "PetAttr3Text", "type": "text", "x": 0, "y": 0,
                                "horizontal_align": "left", "text": "PetAttr3",
                                "fontname": "Nunito Sans:13b",
                                "outline": True,
                                "color": (0xffffd348, 0xffca982f),
                            },

                            {
                                "name": "PetAttr3ValueField",
                                "type": "image",
                                "style": ("not_pick",),
                                "horizontal_align": "right",

                                "x": 10,
                                "y": 0,

                                "image": "d:/ymir work/ui/gui/petsystem/info_slot.png",
                                "children": (
                                    {"name": "PetAttrValueField3_Value", "type": "text", "x": 0, "y": 2, "text": "999",
                                     "color": 0xff9e5753, "horizontal_align": "center"},

                                ),

                            },

                            {"name": "PetAttr3_Plus", "type": "button", "horizontal_align": "right", "x": -10, "y": 2,
                             "default_image": "d:/ymir work/ui/gui/stat-plus.sub",
                             "over_image": "d:/ymir work/ui/gui/stat-plus-hover.sub",
                             "down_image": "d:/ymir work/ui/gui/stat-plus-down.sub", },

                        ),

                    },
                    {
                        "name": "PetAttr4",
                        "type": "window",
                        "style": ("not_pick",),

                        "x": 105,
                        "y": 73 + (4 * 20),

                        "width": 340,
                        "height": 20,

                        "children": (
                            {
                                "name": "PetAttr4Text", "type": "text", "x": 0, "y": 0,
                                "horizontal_align": "left", "text": "PetAttr4",
                                "fontname": "Nunito Sans:13b",
                                "outline": True,
                                "color": (0xffffd348, 0xffca982f),
                            },

                            {
                                "name": "PetAttr4ValueField",
                                "type": "image",
                                "style": ("not_pick",),
                                "horizontal_align": "right",

                                "x": 10,
                                "y": 0,

                                "image": "d:/ymir work/ui/gui/petsystem/info_slot.png",
                                "children": (
                                    {"name": "PetAttrValueField4_Value", "type": "text", "x": 0, "y": 2, "text": "999",
                                     "color": 0xff9e5753, "horizontal_align": "center"},

                                ),

                            },

                            {"name": "PetAttr4_Plus", "type": "button", "horizontal_align": "right", "x": -10, "y": 2,
                             "default_image": "d:/ymir work/ui/gui/stat-plus.sub",
                             "over_image": "d:/ymir work/ui/gui/stat-plus-hover.sub",
                             "down_image": "d:/ymir work/ui/gui/stat-plus-down.sub", },

                        ),

                    },
                    {
                        "name": "PetAttr5",
                        "type": "window",
                        "style": ("not_pick",),

                        "x": 105,
                        "y": 73 + (5 * 20),

                        "width": 340,
                        "height": 20,

                        "children": (
                            {
                                "name": "PetAttr5Text", "type": "text", "x": 0, "y": 0,
                                "horizontal_align": "left", "text": "PetAttr5",
                                "fontname": "Nunito Sans:13b",
                                "outline": True,
                                "color": (0xffffd348, 0xffca982f),
                            },

                            {
                                "name": "PetAttr5ValueField",
                                "type": "image",
                                "style": ("not_pick",),
                                "horizontal_align": "right",

                                "x": 10,
                                "y": 0,

                                "image": "d:/ymir work/ui/gui/petsystem/info_slot.png",
                                "children": (
                                    {"name": "PetAttrValueField5_Value", "type": "text", "x": 0, "y": 2, "text": "999",
                                     "color": 0xff9e5753, "horizontal_align": "center"},

                                ),

                            },

                            {"name": "PetAttr5_Plus", "type": "button", "horizontal_align": "right", "x": -10, "y": 2,
                             "default_image": "d:/ymir work/ui/gui/stat-plus.sub",
                             "over_image": "d:/ymir work/ui/gui/stat-plus-hover.sub",
                             "down_image": "d:/ymir work/ui/gui/stat-plus-down.sub", },

                        ),

                    },
                    {
                        "name": "PetAttr6",
                        "type": "window",
                        "style": ("not_pick",),

                        "x": 105,
                        "y": 73 + (6 * 20),

                        "width": 340,
                        "height": 20,

                        "children": (
                            {
                                "name": "PetAttr6Text", "type": "text", "x": 0, "y": 0,
                                "horizontal_align": "left", "text": "PetAttr6",
                                "fontname": "Nunito Sans:13b",
                                "outline": True,
                                "color": (0xffffd348, 0xffca982f),
                            },

                            {
                                "name": "PetAttr6ValueField",
                                "type": "image",
                                "style": ("not_pick",),
                                "horizontal_align": "right",

                                "x": 10,
                                "y": 0,

                                "image": "d:/ymir work/ui/gui/petsystem/info_slot.png",
                                "children": (
                                    {"name": "PetAttrValueField6_Value", "type": "text", "x": 0, "y": 2, "text": "999",
                                     "color": 0xff9e5753, "horizontal_align": "center"},

                                ),

                            },

                            {"name": "PetAttr6_Plus", "type": "button", "horizontal_align": "right", "x": -10, "y": 2,
                             "default_image": "d:/ymir work/ui/gui/stat-plus.sub",
                             "over_image": "d:/ymir work/ui/gui/stat-plus-hover.sub",
                             "down_image": "d:/ymir work/ui/gui/stat-plus-down.sub", },

                        ),

                    },

                ),
            },

        ),
}
