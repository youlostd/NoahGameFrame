import uiScriptLocale

ROOT_PATH = 'd:/ymir work/ui/public/'
SMALL_VALUE_FILE = 'd:/ymir work/ui/public/Parameter_Slot_02.sub'
MIDDLE_VALUE_FILE = 'd:/ymir work/ui/public/Parameter_Slot_03.sub'
TEMP_X = 110
PLUS_WIDTH = 60
window = {
    "name": "BuildGuildBuildingWindow",
    "style": ("moveable", "float",),

    "x": 10,
    "y": SCREEN_HEIGHT - 240 - 50,

    "width": 355 + TEMP_X + PLUS_WIDTH,
    "height": 260,

    "children":
        (

            {
                "name": "Board",
                "type": "board_with_titlebar",

                "x": 0,
                "y": 0,

                "width": 355 + TEMP_X + PLUS_WIDTH,
                "height": 260,

                "title": uiScriptLocale.GUILD_BUILDING_TITLE,

                "children":
                    (

                        
  
                        {
                            "name": "BuildingCategoryBar",
                            "type": "slotbar",

                            "x": 15,
                            "y": 50,

                            "width": 100 + PLUS_WIDTH / 2,
                            "height": 80,

                            "children":
                                (
                                    {
                                        "name": "CategoryList",
                                        "type": "listbox",

                                        "x": 0,
                                        "y": 1,

                                        "width": 100 + PLUS_WIDTH / 2,
                                        "height": 80,
                                    },
                                ),
                        },

                        {
                            "name": "BuildingPriceTitle",
                            "type": "text", "x": 10, "y": 143, "text": uiScriptLocale.GUILD_BUILDING_PRICE,
                            "text_horizontal_align": "center",
                            "children":
                                (
                                    {
                                        "name": "BuildingPriceSlot",
                                        "type": "slotbar", "x": 30 + PLUS_WIDTH / 3, "y": -4,
                                        "width": 75 + PLUS_WIDTH / 4, "height": 17,
                                        "children":
                                            (
                                                {"name": "BuildingPriceValue", "type": "text", "x": 0, "y": 1,
                                                 "all_align": "center", "text": "2000000000", },
                                            ),
                                    },
                                ),
                        },

                        {
                            "name": "BuildingMaterialStoneTitle",
                            "type": "text", "x": 10, "y": 163, "text": uiScriptLocale.GUILD_BUILDING_STONE,
                            "text_horizontal_align": "center",
                            "children":
                                (
                                    {
                                        "name": "BuildingMaterialStoneSlot",
                                        "type": "slotbar", "x": 30 + PLUS_WIDTH / 3, "y": -4,
                                        "width": 75 + PLUS_WIDTH / 4, "height": 17,
                                        "children":
                                            (
                                                {"name": "BuildingMaterialStoneValue", "type": "text", "x": 0, "y": 1,
                                                 "all_align": "center", "text": "50000000", },
                                            ),
                                    },
                                ),
                        },

                        {
                            "name": "BuildingMaterialLogTitle",
                            "type": "text", "x": 10, "y": 183, "text": uiScriptLocale.GUILD_BUILDING_LOG,
                            "text_horizontal_align": "center",
                            "children":
                                (
                                    {
                                        "name": "BuildingMaterialLogSlot",
                                        "type": "slotbar", "x": 30 + PLUS_WIDTH / 3, "y": -4,
                                        "width": 75 + PLUS_WIDTH / 4, "height": 17,
                                        "children":
                                            (
                                                {"name": "BuildingMaterialLogValue", "type": "text", "x": 0, "y": 1,
                                                 "all_align": "center", "text": "50000000", },
                                            ),
                                    },
                                ),
                        },

                        {
                            "name": "BuildingMaterialPlywoodTitle",
                            "type": "text", "x": 10, "y": 203, "text": uiScriptLocale.GUILD_BUILDING_PLY,
                            "text_horizontal_align": "center",
                            "children":
                                (
                                    {
                                        "name": "BuildingMaterialPlywoodSlot",
                                        "type": "slotbar", "x": 30 + PLUS_WIDTH / 3, "y": -4,
                                        "width": 75 + PLUS_WIDTH / 4, "height": 17,
                                        "children":
                                            (
                                                {"name": "BuildingMaterialPlywoodValue", "type": "text", "x": 0, "y": 1,
                                                 "all_align": "center", "text": "50000000", },
                                            ),
                                    },
                                ),
                        },

                        {
                            "name": "temp_window",
                            "type": "window",
                            "style": ("not_pick",),

                            "x": TEMP_X,
                            "y": 0,
                            "width": 355 + PLUS_WIDTH,
                            "height": 240,

                            "children":
                                (

       
                                    {
                                        "name": "BuildingListBar",
                                        "type": "slotbar",

                                        "x": PLUS_WIDTH - 15,
                                        "y": 50,

                                        "width": 120 + PLUS_WIDTH / 2,
                                        "height": 172,

                                        "children":
                                            (
                                                {
                                                    "name": "BuildingList",
                                                    "type": "listbox",

                                                    "x": 0,
                                                    "y": 1,

                                                    "width": 105 + PLUS_WIDTH / 2,
                                                    "height": 170,
                                                },
                                                {
                                                    "name": "ListScrollBar",
                                                    "type": "scrollbar",

                                                    "x": 15,
                                                    "y": 2,
                                                    "size": 172 - 2,
                                                    "horizontal_align": "right",
                                                },
                                            ),
                                    },


                                    {
                                        "name": "PositionButton",
                                        "type": "radio_button", "x": 270 + PLUS_WIDTH, "y": 54,
                                        "text": uiScriptLocale.GUILD_BUILDING_CHANGE,
                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                                    },
                                    {
                                        "name": "BuildingPositionXTitle",
                                        "type": "text", "x": 150 + PLUS_WIDTH, "y": 53, "text": "X",
                                        "text_horizontal_align": "center",
                                        "children":
                                            (
                                                {
                                                    "name": "BuildingPositionXSlot",
                                                    "type": "image", "x": 23, "y": -3, "image": MIDDLE_VALUE_FILE,
                                                    "children":
                                                        (
                                                            {"name": "BuildingPositionXValue", "type": "text", "x": 0,
                                                             "y": 0, "all_align": "center", "text": "999", },
                                                        ),
                                                },
                                            ),
                                    },
                                    {
                                        "name": "BuildingPositionY",
                                        "type": "text", "x": 150 + PLUS_WIDTH, "y": 73, "text": "Y",
                                        "text_horizontal_align": "center",
                                        "children":
                                            (
                                                {
                                                    "name": "BuildingPositionYSlot",
                                                    "type": "image", "x": 23, "y": -3, "image": MIDDLE_VALUE_FILE,
                                                    "children":
                                                        (
                                                            {"name": "BuildingPositionYValue", "type": "text", "x": 0,
                                                             "y": 0, "all_align": "center", "text": "999", },
                                                        ),
                                                },
                                            ),
                                    },

                                    {
                                        "name": "BuildingRotationTitle",
                                        "type": "text",

                                        "x": 250 + PLUS_WIDTH,
                                        "y": 95,
                                        "text_horizontal_align": "center",

                                        "text": uiScriptLocale.GUILD_BUILDING_DIRECTION,
                                    },
                                    {
                                        "name": "BuildingRotationXTitle",
                                        "type": "text", "x": 150 + PLUS_WIDTH, "y": 115, "text": "X",
                                        "text_horizontal_align": "center",
                                    },
                                    {
                                        "name": "BuildingRotationX",
                                        "type": "sliderbar",
                                        "x": 158 + PLUS_WIDTH, "y": 115,
                                    },
                                    {
                                        "name": "BuildingRotationYTitle",
                                        "type": "text", "x": 150 + PLUS_WIDTH, "y": 135, "text": "Y",
                                        "text_horizontal_align": "center",
                                    },
                                    {
                                        "name": "BuildingRotationY",
                                        "type": "sliderbar",
                                        "x": 158 + PLUS_WIDTH, "y": 135,
                                    },
                                    {
                                        "name": "BuildingRotationZTitle",
                                        "type": "text", "x": 150 + PLUS_WIDTH, "y": 155, "text": "Z",
                                        "text_horizontal_align": "center",
                                    },
                                    {
                                        "name": "BuildingRotationZ",
                                        "type": "sliderbar",
                                        "x": 158 + PLUS_WIDTH, "y": 155,
                                    },

                                    {
                                        "name": "PreviewButton",
                                        "type": "toggle_button",

                                        "x": 170 + PLUS_WIDTH,
                                        "y": 35,
                                        "vertical_align": "bottom",

                                        "text": uiScriptLocale.GUILD_BUILDING_PREVIEW,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                                    },

                                    {
                                        "name": "AcceptButton",
                                        "type": "button",

                                        "x": 170 + PLUS_WIDTH,
                                        "y": 0,

                                        "text": uiScriptLocale.ACCEPT,
                                        "vertical_align": "bottom",

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                                    },
                                    {
                                        "name": "CancelButton",
                                        "type": "button",

                                        "x": 245 + PLUS_WIDTH,
                                        "y": 0,

                                        "text": uiScriptLocale.CANCEL,
                                        "vertical_align": "bottom",

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                                    },
                                    {
                                        "name": "ChangeButton",
                                        "type": "button",

                                        "x": 245 + PLUS_WIDTH,
                                        "y": 35,

                                        "text": uiScriptLocale.GUILD_BUILDING_FIX,
                                        "vertical_align": "bottom",

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                                    },
                                    ### END_TEMP
                                ),
                        },
                    ),
            },
        ),
}