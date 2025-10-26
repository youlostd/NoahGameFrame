import uiScriptLocale

WINDOW_WIDTH = 537
WINDOW_HEIGHT = 327

ROOT_PATH = "d:/ymir work/ui/game/battle_pass/"

window = {
    "name": "HuntingMissionsWindow",
    "x": 0,
    "y": 0,
    "style": (
        "moveable",
        "float",
    ),
    "width": WINDOW_WIDTH,
    "height": WINDOW_HEIGHT,
    "children": (
        {
            "name": "board",
            "type": "board_with_titlebar",
            "style": ("attach",),
            "x": 0,
            "y": 0,
            "width": WINDOW_WIDTH,
            "height": WINDOW_HEIGHT,
            "title": uiScriptLocale.HUNT_MISSION_TITLE,
            "children": (
                {
                    "name": "InnerBoard1",
                    "type": "border_a",
                    "x": 8,
                    "y": 35,
                    "width": 316,
                    "height": 280,
                    "children": (
                        {
                            "name": "BorderScroll",
                            "type": "window",
                            "size_skip": True,
                            "x": 295,
                            "y": 0,
                            "width": 20,
                            "height": 255,
                            # "children":
                            # (
                            # # scroll bar here
                            # ),
                        },
                        {
                            "name": "BorderMissions",
                            "type": "scrollwindow",
                            "x": 8,
                            "y": 0,
                            "width": 298,
                            "height": 255,
                            "children": (
                                {
                                    "name": "BorderMissionsContent",
                                    "type": "auto_grow_vertical_ex",
                                    "x": 0,
                                    "y": 0,
                                    "width": 298,
                                    "height": 255,
                                    "content_window": 1,
                                },
                            ),
                        },
                    ),
                },
                {
                    "name": "InnerBoard1",
                    "type": "border_a",
                    "style": ("not_pick",),
                    "x": 325,
                    "y": 35,
                    "width": 205,
                    "height": 280,
                    "children": (
                        {
                            "name": "BorderInfoMission",
                            "type": "window",
                            "style": ("not_pick",),
                            "x": 5,
                            "y": 0,
                            "width": 215,
                            "height": 277,
                            "children": (
                                {
                                    "name": "bgImageMission",
                                    "type": "window",
                                    "style": ("not_pick",),
                                    "x": 3,
                                    "y": 3,
                                    "width": 207,
                                    "height": 277,
                                    "children": (
                                        {
                                            "name": "missionImageTitle",
                                            "type": "image",
                                            "style": ("not_pick",),
                                            "x": 0,
                                            "y": -2,
                                            "image": ROOT_PATH
                                            + "title_bar_special.tga",
                                            "children": (
                                                {
                                                    "name": "missionTitleText",
                                                    "type": "text",
                                                    "x": 0,
                                                    "y": 3,
                                                    "text": uiScriptLocale.HUNT_MISSIONS,
                                                    "text_horizontal_align": "center",
                                                    "horizontal_align": "center",
                                                    # "fontname": "Nunito Sans:13b",
                                                    "outline": True,
                                                    # "color": (0xFFFFD348, 0xFFCA982F),
                                                },
                                            ),
                                        },
                                        {
                                            "name": "mission_image_10",
                                            "type": "image",
                                            "style": ("not_pick",),
                                            "x": 0,
                                            "y": 22,
                                            "image": ROOT_PATH + "info_bar_title.tga",
                                            "children": (
                                                {
                                                    "name": "mission_text_10",
                                                    "type": "text",
                                                    "x": 0,
                                                    "y": 2,
                                                    "text": "Status",
                                                    "text_horizontal_align": "center",
                                                    "horizontal_align": "center",
                                                },
                                            ),
                                        },
                                        {
                                            "name": "mission_target_scroll",
                                            "type": "scrollwindow",
                                            "style": ("not_pick",),
                                            "x": 5,
                                            "y": 42,
                                            "width": 200,
                                            "height": 105,
                                            "children": (
                                                {
                                                    "name": "mission_target_content",
                                                    "type": "auto_grow_vertical_ex",
                                                    "content_window": 1,
                                                    "x": 0,
                                                    "y": 0,
                                                    "width": 200,
                                                    "height": 90,
                                                },
                                            ),
                                        },
                                        {
                                            "name": "mission_image_30",
                                            "type": "image",
                                            "style": ("not_pick",),
                                            "x": 0,
                                            "y": 149,
                                            "image": ROOT_PATH + "info_bar_title.tga",
                                            "children": (
                                                {
                                                    "name": "mission_text_30",
                                                    "type": "text",
                                                    "x": 0,
                                                    "y": 2,
                                                    "text": uiScriptLocale.HUNT_REWARDS,
                                                    "text_horizontal_align": "center",
                                                    "horizontal_align": "center",
                                                },
                                            ),
                                        },
                                        {
                                            "name": "mission_reward_scroll",
                                            "type": "scrollwindow",
                                            "style": ("not_pick",),
                                            "x": 5,
                                            "y": 169,
                                            "width": 200,
                                            "height": 105,
                                            "children": (
                                                {
                                                    "name": "mission_reward_content",
                                                    "type": "auto_grow_vertical_ex",
                                                    "content_window": 1,
                                                    "x": 0,
                                                    "y": 0,
                                                    "width": 200,
                                                    "height": 90,
                                                },
                                            ),
                                        },
                                    ),
                                },
                            ),
                        },
                    ),
                },
            ),
        },
    ),
}
