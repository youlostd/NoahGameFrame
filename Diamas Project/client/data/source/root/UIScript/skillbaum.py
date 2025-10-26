import uiScriptLocale

SKILLTREE_START_INDEX = 300
ROOT_PATH = "d:/ymir work/ui/"
ICON_SLOT_FILE = "d:/ymir work/ui/skillbaum/slot.png"

window = {
    "name": "board",
    "style": ("moveable", "float",),

    "x": 420,
    "y": 250,

    "width": 795,
    "height": 542,

    "children":
        (
            {
                "name": "BG",
                "type": "expanded_image",
                "style": ("not_pick", "attach", ),

                "x": 0,
                "y": 0,
                "image": ROOT_PATH + "skillbaum/skillbaumbg.png",
            },
            {
                "name": "destroy",
                "type": "button",

                "x": 760,
                "y": 20,

                "default_image": "d:/ymir work/ui/gui/cancel.sub",
                "over_image": "d:/ymir work/ui/gui/cancel_over.sub",
                "down_image": "d:/ymir work/ui/gui/cancel_down.sub",
            },

            ## LINKS
            {
                "name": "Skill_Active_Slot_1",
                "type": "slot",

                "x":  10,
                "y": 0 + 15 + 36,

                "width": 249,
                "height": 473,
                "image": ICON_SLOT_FILE,

                "slot": (
                    # main
                    {"index": SKILLTREE_START_INDEX+180, "x": 98, "y": 393, "placement_x": 4, "placement_y": 5, "width": 48, "height": 48},

                    # row 1
                    {"index": SKILLTREE_START_INDEX+181, "x": -12 + 50, "y": 264 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 48},
                    {"index": SKILLTREE_START_INDEX+182, "x": 110 + 50, "y": 264 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 48},

                    # row 2
                    {"index": SKILLTREE_START_INDEX+184, "x": -12+ 50, "y": 195 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 48},
                    {"index": SKILLTREE_START_INDEX+185, "x": 108+ 50, "y": 195 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 48},

                    # final
                    {"index": SKILLTREE_START_INDEX+183, "x": 48+ 50, "y": 194 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 48},

                    {"index": SKILLTREE_START_INDEX+200, "x": 5, "y": 0, "placement_x": 4, "placement_y": 5, "width": 240, "height": 240},


                ),
            },
            ## Mitte

            {
                "name": "Skill_Active_Slot_2",
                "type": "slot",

                "x": 263,
                "y": 0 + 15 + 36,

                "width": 259 + 10,
                "height": 473,
                "image": ICON_SLOT_FILE,

                "slot": (
                    {"index": SKILLTREE_START_INDEX+186, "x": 50 + 60, "y": 335 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 98},
                    {"index": SKILLTREE_START_INDEX+187, "x": -10+ 60, "y": 265 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 98},
                    {"index": SKILLTREE_START_INDEX+188, "x": 110+ 60, "y": 265 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 98},
                    {"index": SKILLTREE_START_INDEX+189, "x": 50+ 60, "y": 195 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 98},
                    {"index": SKILLTREE_START_INDEX+190, "x": -10+ 60, "y": 195 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 98},
                    {"index": SKILLTREE_START_INDEX+191, "x": 110+ 60, "y": 195 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 98},
                    {"index": SKILLTREE_START_INDEX+192, "x": 146+ 60, "y": 144 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 98},
                    {"index": SKILLTREE_START_INDEX+193, "x": -48+ 60, "y": 144 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 98},
                    {"index": SKILLTREE_START_INDEX+201, "x": 5, "y": 0, "placement_x": 4, "placement_y": 5, "width": 240, "height": 240},

                ),
            },

            ## LINKS
            {
                "name": "Skill_Active_Slot_3",
                "type": "slot",

                "x": 520 + 10,
                "y": 0 + 15 + 36,
                "start_index": SKILLTREE_START_INDEX,

                "width": 259,
                "height": 473,
                "image": ICON_SLOT_FILE,

                "slot": (
                    {"index": SKILLTREE_START_INDEX+194, "x": 50 + 50, "y": 335 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 48},
                    {"index": SKILLTREE_START_INDEX+195, "x": -10 + 50, "y": 265 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 42},
                    {"index": SKILLTREE_START_INDEX+196, "x": 110 + 50, "y": 265 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 42},
                    {"index": SKILLTREE_START_INDEX+197, "x": 50 + 50, "y": 195 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 42},
                    {"index": SKILLTREE_START_INDEX+198, "x": -10 + 50, "y": 195 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 42},
                    {"index": SKILLTREE_START_INDEX+199, "x": 110+ 50, "y": 195 + 60, "placement_x": 4, "placement_y": 5, "width": 48, "height": 42},
                    {"index": SKILLTREE_START_INDEX+202, "x": 10, "y": 0, "placement_x": 4, "placement_y": 5, "width": 240, "height": 240},
                ),
            },
            
            ## SkillPoints          
            
            {
                "name" : "SkillPointsDisplay",
                "type" : "image",
                "x":725,
                "y": 470,

                "width" : 88,
                "height" : 16,
                "image": ROOT_PATH + "skillbaum/pointsbutton.png",


                "children" : (
                        {
                        "name" : "SkillPointsLabel",
                        "type":"text",
                        "text":"0",
                        "x":0,
                        "y":-2,
                        "fontname": "Verdana:12",
                        "color": (0xffffd56f),

                        "all_align" : "left",
                        },
                    ),
            },


        ),
}
