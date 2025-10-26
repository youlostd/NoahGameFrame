import uiScriptLocale

WINDOW_WIDTH = 537
WINDOW_HEIGHT = 327

ROOT_PATH = "d:/ymir work/ui/game/battle_pass/"

window = {
    "name": "DungeonListWindow",
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
            "title": "Savaş Bileti",
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
                            "type": "window",
                            "x": 8,
                            "y": 0,
                            "width": 298,
                            "height": 255,
                            # "children":
                            # (
                            # # scroll bar here
                            # ),
                        },
                        {
                            "name": "BorderRanking",
                            "type": "window",
                            "x": 8,
                            "y": 0,
                            "width": 307,
                            "height": 275,
                            "children": (
                                {
                                    "name": "RankingTitle",
                                    "type": "image",
                                    "x": 3,
                                    "y": 3,
                                    "image": ROOT_PATH + "ranking_title.tga",
                                },
                                {
                                    "name": "pagination",
                                    "type": "window",
                                    "x": 25,
                                    "y": 5,
                                    "vertical_align": "bottom",
                                    "width": 307,
                                    "height": 22,
                                    "children": (
                                        {
                                            "name": "first_prev_button",
                                            "type": "button",
                                            "x": 0,
                                            "y": 2,
                                            "default_image": ROOT_PATH
                                            + "pagination/btn_first_prev_default.dds",
                                            "over_image": ROOT_PATH
                                            + "pagination/btn_first_prev_hover.dds",
                                            "down_image": ROOT_PATH
                                            + "pagination/btn_first_prev_down.dds",
                                        },
                                        {
                                            "name": "prev_button",
                                            "type": "button",
                                            "x": 23,
                                            "y": 2,
                                            "default_image": ROOT_PATH
                                            + "pagination/btn_prev_default.dds",
                                            "over_image": ROOT_PATH
                                            + "pagination/btn_prev_hover.dds",
                                            "down_image": ROOT_PATH
                                            + "pagination/btn_prev_down.dds",
                                        },
                                        {
                                            "name": "page1_button",
                                            "type": "button",
                                            "x": 55,
                                            "y": 8,
                                            "text": "1",
                                            "default_image": ROOT_PATH
                                            + "pagination/pagenumber_default.png",
                                            "over_image": ROOT_PATH
                                            + "pagination/pagenumber_default.png",
                                            "down_image": ROOT_PATH
                                            + "pagination/pagenumber_down.png",
                                        },
                                        {
                                            "name": "page2_button",
                                            "type": "button",
                                            "x": 88,
                                            "y": 8,
                                            "text": "2",
                                            "default_image": ROOT_PATH
                                            + "pagination/pagenumber_default.png",
                                            "over_image": ROOT_PATH
                                            + "pagination/pagenumber_default.png",
                                            "down_image": ROOT_PATH
                                            + "pagination/pagenumber_down.png",
                                        },
                                        {
                                            "name": "page3_button",
                                            "type": "button",
                                            "x": 121,
                                            "y": 8,
                                            "text": "3",
                                            "default_image": ROOT_PATH
                                            + "pagination/pagenumber_default.png",
                                            "over_image": ROOT_PATH
                                            + "pagination/pagenumber_default.png",
                                            "down_image": ROOT_PATH
                                            + "pagination/pagenumber_down.png",
                                        },
                                        {
                                            "name": "page4_button",
                                            "type": "button",
                                            "x": 154,
                                            "y": 8,
                                            "text": "4",
                                            "default_image": ROOT_PATH
                                            + "pagination/pagenumber_default.png",
                                            "over_image": ROOT_PATH
                                            + "pagination/pagenumber_default.png",
                                            "down_image": ROOT_PATH
                                            + "pagination/pagenumber_down.png",
                                        },
                                        {
                                            "name": "page5_button",
                                            "type": "button",
                                            "x": 187,
                                            "y": 8,
                                            "text": "5",
                                            "default_image": ROOT_PATH
                                            + "pagination/pagenumber_default.png",
                                            "over_image": ROOT_PATH
                                            + "pagination/pagenumber_default.png",
                                            "down_image": ROOT_PATH
                                            + "pagination/pagenumber_down.png",
                                        },
                                        {
                                            "name": "next_button",
                                            "type": "button",
                                            "x": 222,
                                            "y": 2,
                                            "default_image": ROOT_PATH
                                            + "pagination/btn_next_default.dds",
                                            "over_image": ROOT_PATH
                                            + "pagination/btn_next_hover.dds",
                                            "down_image": ROOT_PATH
                                            + "pagination/btn_next_down.dds",
                                        },
                                        {
                                            "name": "last_next_button",
                                            "type": "button",
                                            "x": 242,
                                            "y": 2,
                                            "default_image": ROOT_PATH
                                            + "pagination/btn_last_next_default.dds",
                                            "over_image": ROOT_PATH
                                            + "pagination/btn_last_next_hover.dds",
                                            "down_image": ROOT_PATH
                                            + "pagination/btn_last_next_down.dds",
                                        },
                                    ),
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
                            "x": 5,
                            "y": 78 - 40,
                            "width": 207,
                            "height": 229,
                            "children": (
                                {
                                    "name": "bgImageMission",
                                    "type": "window",
                                    "x": 3,
                                    "y": 3,
                                    "width": 207,
                                    "height": 229,
                                    "children": (
                                        {
                                            "name": "missionImageTitle",
                                            "type": "image",
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
                                                    "text": "Mission name",
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
                                            "x": 0,
                                            "y": 22,
                                            "image": ROOT_PATH + "info_bar_title.tga",
                                            "children": (
                                                {
                                                    "name": "mission_text_10",
                                                    "type": "text",
                                                    "x": 0,
                                                    "y": 2,
                                                    "text": "Görev Statüsü",
                                                    "text_horizontal_align": "center",
                                                    "horizontal_align": "center",
                                                },
                                            ),
                                        },
                                        {
                                            "name": "mission_text_0",
                                            "type": "text",
                                            "x": 5,
                                            "y": 42,
                                            "text": "desc",
                                        },
                                        {
                                            "name": "mission_image_20",
                                            "type": "image",
                                            "x": 0,
                                            "y": 65,
                                            "image": ROOT_PATH + "info_bar_title.tga",
                                            "children": (
                                                {
                                                    "name": "mission_text_20",
                                                    "type": "text",
                                                    "x": 0,
                                                    "y": 2,
                                                    "text": uiScriptLocale.BATTLEPASS_TEXT12,
                                                    "text_horizontal_align": "center",
                                                    "horizontal_align": "center",
                                                },
                                            ),
                                        },
                                        {
                                            "name": "mission_text_1",
                                            "type": "text",
                                            "x": 5,
                                            "y": 83,
                                            "text": "desc",
                                        },
                                        {
                                            "name": "mission_image_2",
                                            "type": "image",
                                            "x": 0,
                                            "y": 101,
                                            "children": (
                                                {
                                                    "name": "mission_text_2",
                                                    "type": "text",
                                                    "x": 5,
                                                    "y": 2,
                                                    "text": "desc",
                                                },
                                            ),
                                        },
                                        {
                                            "name": "mission_text_3",
                                            "type": "text",
                                            "x": 5,
                                            "y": 121,
                                            "text": "desc",
                                        },
                                        {
                                            "name": "mission_image_30",
                                            "type": "image",
                                            "x": 0,
                                            "y": 149,
                                            "image": ROOT_PATH + "info_bar_title.tga",
                                            "children": (
                                                {
                                                    "name": "mission_text_30",
                                                    "type": "text",
                                                    "x": 0,
                                                    "y": 2,
                                                    "text": uiScriptLocale.BATTLEPASS_TEXT11,
                                                    "text_horizontal_align": "center",
                                                    "horizontal_align": "center",
                                                },
                                            ),
                                        },
                                        {
                                            "name": "mission_text_4",
                                            "type": "text",
                                            "x": 5,
                                            "y": 169,
                                            "text": "desc",
                                        },
                                        {
                                            "name": "mission_image_5",
                                            "type": "image",
                                            "x": 0,
                                            "y": 185,
                                            "image": ROOT_PATH + "info_bar_even.tga",
                                            "children": (
                                                {
                                                    "name": "mission_text_5",
                                                    "type": "text",
                                                    "x": 5,
                                                    "y": 2,
                                                    "text": "desc",
                                                },
                                            ),
                                        },
                                        {
                                            "name": "mission_text_6",
                                            "type": "text",
                                            "x": 5,
                                            "y": 205,
                                            "text": "desc",
                                        },
                                    ),
                                },
                            ),
                        },
                        {
                            "name": "BorderInfoGeneral",
                            "type": "window",
                            "x": 5,
                            "y": 78 - 40,
                            "width": 207,
                            "height": 229,
                            "children": (
                                {
                                    "name": "bgImageGeneral",
                                    "type": "window",
                                    "x": 3,
                                    "y": 3,
                                    "width": 207,
                                    "height": 229,
                                    "children": (
                                        {
                                            "name": "generalImageTitle",
                                            "type": "image",
                                            "x": 0,
                                            "y": -2,
                                            "image": ROOT_PATH
                                            + "title_bar_special.tga",
                                            "children": (
                                                {
                                                    "name": "generalTitleText",
                                                    "type": "text",
                                                    "x": 0,
                                                    "y": 3,
                                                    "text": "Battle Pass",
                                                    "text_horizontal_align": "center",
                                                    "horizontal_align": "center",
                                                    # "fontname": "Nunito Sans:13b",
                                                    "outline": True,
                                                    # "color": (0xFFFFD348, 0xFFCA982F),
                                                },
                                            ),
                                        },
                                        {
                                            "name": "general_text_0",
                                            "type": "text",
                                            "x": 5,
                                            "y": 21,
                                            "text": "Name: Month Year",
                                        },
                                        {
                                            "name": "general_image_1",
                                            "type": "image",
                                            "x": 0,
                                            "y": 37,
                                            "image": ROOT_PATH + "info_bar_even.tga",
                                            "children": (
                                                {
                                                    "name": "general_text_1",
                                                    "type": "text",
                                                    "x": 5,
                                                    "y": 2,
                                                    "text": "Quest count: 99",
                                                },
                                            ),
                                        },
                                        {
                                            "name": "general_text_2",
                                            "type": "text",
                                            "x": 5,
                                            "y": 57,
                                            "text": "Completed quest: 99",
                                        },
                                        {
                                            "name": "general_image_3",
                                            "type": "image",
                                            "x": 0,
                                            "y": 73,
                                            "image": ROOT_PATH + "info_bar_even.tga",
                                            "children": (
                                                {
                                                    "name": "general_text_3",
                                                    "type": "text",
                                                    "x": 5,
                                                    "y": 2,
                                                    "text": "Remaining time: 99Days 99Hours",
                                                },
                                            ),
                                        },
                                        {
                                            "name": "general_text_4",
                                            "type": "text",
                                            "x": 5,
                                            "y": 93,
                                            "text": uiScriptLocale.BATTLEPASS_TEXT9,
                                        },
                                        {
                                            "name": "totalProgressText",
                                            "type": "text",
                                            "x": 40,
                                            "y": 120,
                                            "text": uiScriptLocale.BATTLEPASS_TEXT10,
                                            "text_horizontal_align": "center",
                                            "horizontal_align": "center",
                                        },
                                        {
                                            "name": "gaugeImageBack",
                                            "type": "image",
                                            "x": 2,
                                            "y": 137,
                                            "image": ROOT_PATH
                                            + "total_progress_empty.tga",
                                            "children": (
                                                {
                                                    "name": "gaugeImage",
                                                    "type": "expanded_image",
                                                    "x": 0,
                                                    "y": 0,
                                                    "image": ROOT_PATH
                                                    + "total_progress_full.png",
                                                },
                                            ),
                                        },
                                        {
                                            "name": "RankingButton",
                                            "type": "button",
                                            "x": 5,
                                            "y": 160,
                                            "default_image": ROOT_PATH
                                            + "ranking_normal.tga",
                                            "over_image": ROOT_PATH
                                            + "ranking_over.tga",
                                            "down_image": ROOT_PATH
                                            + "ranking_down.tga",
                                        },
                                        {
                                            "name": "RewardButton",
                                            "type": "button",
                                            "x": 5,
                                            "y": 195,
                                            "default_image": ROOT_PATH
                                            + "reward_normal.tga",
                                            "over_image": ROOT_PATH + "reward_over.tga",
                                            "down_image": ROOT_PATH + "reward_down.tga",
                                        },
                                        {
                                            "name": "RewardItems",
                                            "type": "grid_table",
                                            "x": 100,
                                            "y": 155,
                                            "start_index": 0,
                                            "x_count": 3,
                                            "y_count": 2,
                                            "x_step": 32,
                                            "y_step": 32,
                                            "image": "d:/ymir work/ui/public/Slot_Base.sub",
                                        },
                                    ),
                                },
                            ),
                        },
                        {
                            "name": "tab_control",
                            "type": "window",
                            "x": 5,
                            "y": 48 - 40,
                            "width": 171,
                            "height": 29,
                            "children": (
                                {
                                    "name": "tab_01",
                                    "type": "image",
                                    "x": 0,
                                    "y": 0,
                                    "width": 171,
                                    "height": 29,
                                    "image": ROOT_PATH + "category_btn_1.tga",
                                },
                                {
                                    "name": "tab_02",
                                    "type": "image",
                                    "x": 0,
                                    "y": 0,
                                    "width": 171,
                                    "height": 29,
                                    "image": ROOT_PATH + "category_btn_2.tga",
                                },
                                {
                                    "name": "tab_button_01",
                                    "type": "radio_button",
                                    "x": 3,
                                    "y": 0,
                                    "width": 83,
                                    "height": 29,
                                },
                                {
                                    "name": "tab_button_02",
                                    "type": "radio_button",
                                    "x": 86,
                                    "y": 0,
                                    "width": 83,
                                    "height": 29,
                                },
                            ),
                        },
                    ),
                },
            ),
        },
    ),
}
