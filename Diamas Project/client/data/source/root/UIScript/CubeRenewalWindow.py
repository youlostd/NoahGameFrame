import uiScriptLocale

ROOT_PATH = "d:/ymir work/ui/game/cube/"
PUBLIC_PATH = "d:/ymir work/ui/public/"
PATTERN_PATH = "d:/ymir work/ui/pattern/"

LIST_WINDOW_WIDTH = 316
LIST_WINDOW_HEIGHT = 252
LIST_WINDOW_PATTERN_X_COUNT = (LIST_WINDOW_WIDTH - 32) / 16
LIST_WINDOW_PATTERN_Y_COUNT = (LIST_WINDOW_HEIGHT - 32) / 16

window = {
    "name": "CubeRenewalWindow",
    "style": (
        "moveable",
        "float",
    ),
    "x": SCREEN_WIDTH - 175 - 287 - 100 - 50,
    "y": SCREEN_HEIGHT - 37 - 40 - 525,
    "width": 336,
    "height": 498,
    "children": [
        {
            "name": "board",
            "type": "board_with_titlebar",
            "style": (
                "not_pick",
                "attach",
            ),
            "x": 0,
            "y": 0,
            "width": 336,
            "height": 498,
            "title": "Crafting",
            "children": (
                ## ItemList
                {
                    "name": "item_list_board",
                    "type": "window",
                    "x": 10,
                    "y": 32,
                    "width": LIST_WINDOW_WIDTH,
                    "height": LIST_WINDOW_HEIGHT,
                    "children": (
                        ## LeftTop 1
                        {
                            "name": "ListWindowLeftTop",
                            "type": "image",
                            "x": 0,
                            "y": 0,
                            "image": PATTERN_PATH + "border_A_left_top.tga",
                        },
                        ## RightTop 2
                        {
                            "name": "ListWindowRightTop",
                            "type": "image",
                            "x": LIST_WINDOW_WIDTH - 16,
                            "y": 0,
                            "image": PATTERN_PATH + "border_A_right_top.tga",
                        },
                        ## LeftBottom 3
                        {
                            "name": "ListWindowLeftBottom",
                            "type": "image",
                            "x": 0,
                            "y": LIST_WINDOW_HEIGHT - 16,
                            "image": PATTERN_PATH + "border_A_left_bottom.tga",
                        },
                        ## RightBottom 4
                        {
                            "name": "ListWindowRightBottom",
                            "type": "image",
                            "x": LIST_WINDOW_WIDTH - 16,
                            "y": LIST_WINDOW_HEIGHT - 16,
                            "image": PATTERN_PATH + "border_A_right_bottom.tga",
                        },
                        ## topcenterImg 5
                        {
                            "name": "ListWindowTopCenterImg",
                            "type": "expanded_image",
                            "x": 16,
                            "y": 0,
                            "image": PATTERN_PATH + "border_A_top.tga",
                            "rect": (0.0, 0.0, LIST_WINDOW_PATTERN_X_COUNT, 0),
                        },
                        ## leftcenterImg 6
                        {
                            "name": "ListWindowLeftCenterImg",
                            "type": "expanded_image",
                            "x": 0,
                            "y": 16,
                            "image": PATTERN_PATH + "border_A_left.tga",
                            "rect": (0.0, 0.0, 0, LIST_WINDOW_PATTERN_Y_COUNT),
                        },
                        ## rightcenterImg 7
                        {
                            "name": "ListWindowRightCenterImg",
                            "type": "expanded_image",
                            "x": LIST_WINDOW_WIDTH - 16,
                            "y": 16,
                            "image": PATTERN_PATH + "border_A_right.tga",
                            "rect": (0.0, 0.0, 0, LIST_WINDOW_PATTERN_Y_COUNT),
                        },
                        ## bottomcenterImg 8
                        {
                            "name": "ListWindowBottomCenterImg",
                            "type": "expanded_image",
                            "x": 16,
                            "y": LIST_WINDOW_HEIGHT - 16,
                            "image": PATTERN_PATH + "border_A_bottom.tga",
                            "rect": (0.0, 0.0, LIST_WINDOW_PATTERN_X_COUNT, 0),
                        },
                        ## centerImg
                        {
                            "name": "ListWindowCenterImg",
                            "type": "expanded_image",
                            "x": 16,
                            "y": 16,
                            "image": PATTERN_PATH + "border_A_center.tga",
                            "rect": (
                                0.0,
                                0.0,
                                LIST_WINDOW_PATTERN_X_COUNT,
                                LIST_WINDOW_PATTERN_Y_COUNT,
                            ),
                        },
                        ## ���� ó���� ���� �ȿ� �ִ� ������
                        {
                            "name": "item_list_window",
                            "type": "window",
                            "x": 3,
                            "y": 3,
                            "width": LIST_WINDOW_WIDTH - 6,
                            "height": LIST_WINDOW_HEIGHT - 10,
                        },
                    ),
                },
                {
                    "name": "cube_list_scroll_bar",
                    "type": "scrollbar",
                    "x": 25,
                    "y": 38,
                    "size": 225,
                    "horizontal_align": "right",
                },
                ## ItemSlot
                {
                    # Background img
                    "name": "item_slot",
                    "type": "image",
                    "style": ("attach",),
                    "x": 10,
                    "y": 283,
                    "image": ROOT_PATH + "cube_slot_bg.sub",
                    "children": (
                        ## Result Item Qty Text
                        {
                            "name": "result_qty_window",
                            "type": "window",
                            "x": 24,
                            "y": 115,
                            "width": 36,
                            "height": 16,
                            "children": (
                                {
                                    "name": "result_qty",
                                    "type": "editline",
                                    "x": 12,
                                    "y": 2,
                                    "width": 36,
                                    "height": 16,
                                    "input_limit": 4,
                                    "only_number": 1,
                                    "text": "",
                                },
                            ),
                        },
                        {
                            "name": "qty_sub_button",
                            "type": "button",
                            "x": 7,
                            "y": 117,
                            "default_image": ROOT_PATH + "cube_qty_sub_default.sub",
                            "over_image": ROOT_PATH + "cube_qty_sub_over.sub",
                            "down_image": ROOT_PATH + "cube_qty_sub_down.sub",
                        },
                        {
                            "name": "qty_add_button",
                            "type": "button",
                            "x": 62,
                            "y": 117,
                            "default_image": ROOT_PATH + "cube_qty_add_default.sub",
                            "over_image": ROOT_PATH + "cube_qty_add_over.sub",
                            "down_image": ROOT_PATH + "cube_qty_add_down.sub",
                        },
                        # Material Item #1 Qty Text
                        {
                            "name": "material_qty_window_1",
                            "type": "window",
                            "x": 80,
                            "y": 115,
                            "width": 45,
                            "height": 16,
                            "children": (
                                {
                                    "name": "material_qty_text_1",
                                    "type": "text",
                                    "x": 0,
                                    "y": 0,
                                    "horizontal_align": "center",
                                    "text_horizontal_align": "center",
                                    "vertical_align": "center",
                                    "text_vertical_align": "center",
                                    "text": "",
                                },
                            ),
                        },
                        # Material Item #2 Qty Text
                        {
                            "name": "material_qty_window_2",
                            "type": "window",
                            "x": 126,
                            "y": 115,
                            "width": 45,
                            "height": 16,
                            "children": (
                                {
                                    "name": "material_qty_text_2",
                                    "type": "text",
                                    "x": 0,
                                    "y": -1,
                                    "horizontal_align": "center",
                                    "text_horizontal_align": "center",
                                    "vertical_align": "center",
                                    "text_vertical_align": "center",
                                    "text": "",
                                },
                            ),
                        },
                        # Material Item #3 Qty Text
                        {
                            "name": "material_qty_window_3",
                            "type": "window",
                            "x": 172,
                            "y": 115,
                            "width": 45,
                            "height": 16,
                            "children": (
                                {
                                    "name": "material_qty_text_3",
                                    "type": "text",
                                    "x": 0,
                                    "y": -1,
                                    "horizontal_align": "center",
                                    "text_horizontal_align": "center",
                                    "vertical_align": "center",
                                    "text_vertical_align": "center",
                                    "text": "",
                                },
                            ),
                        },
                        # Material Item #4 Qty Text
                        {
                            "name": "material_qty_window_4",
                            "type": "window",
                            "x": 218,
                            "y": 115,
                            "width": 45,
                            "height": 16,
                            "children": (
                                {
                                    "name": "material_qty_text_4",
                                    "type": "text",
                                    "x": 0,
                                    "y": -1,
                                    "horizontal_align": "center",
                                    "text_horizontal_align": "center",
                                    "vertical_align": "center",
                                    "text_vertical_align": "center",
                                    "text": "",
                                },
                            ),
                        },
                        # Material Item #5 Qty Text
                        {
                            "name": "material_qty_window_5",
                            "type": "window",
                            "x": 264,
                            "y": 115,
                            "width": 45,
                            "height": 16,
                            "children": (
                                {
                                    "name": "material_qty_text_5",
                                    "type": "text",
                                    "x": 0,
                                    "y": -1,
                                    "horizontal_align": "center",
                                    "text_horizontal_align": "center",
                                    "vertical_align": "center",
                                    "text_vertical_align": "center",
                                    "text": "",
                                },
                            ),
                        },
                    ),
                },
                ## Yang
                {
                    "name": "yang_icon",
                    "type": "image",
                    "x": 186,
                    "y": 427,
                    "image": ROOT_PATH + "cube_yang_icon.sub",
                },
                {
                    "name": "yang_textbg",
                    "type": "image",
                    "x": 233 - 130,
                    "y": 427,
                    "image": "d:/ymir work/ui/public/parameter_slot_05.sub",
                    "children": (
                        {
                            "name": "yang_text",
                            "type": "text",
                            "x": 7,
                            "y": 3,
                            "horizontal_align": "right",
                            "text_horizontal_align": "right",
                            "text": "",
                        },
                    ),
                },
                {
                    "name": "imporve_slot",
                    "type": "image",
                    "x": 29,
                    "y": 421,
                    "image": ROOT_PATH + "cube_improve_slot.sub",
                },
                {
                    "name": "impove_text_window",
                    "type": "window",
                    "x": 27,
                    "y": 465,
                    "width": 47,
                    "height": 13,
                    "children": (
                        {
                            "name": "improve_text",
                            "type": "text",
                            "x": 0,
                            "y": 0,
                            "horizontal_align": "center",
                            "text_horizontal_align": "center",
                            "vertical_align": "center",
                            "text_vertical_align": "center",
                            "text": "Improve Chance",
                        },
                    ),
                },
                {
                    "name": "button_ok",
                    "type": "button",
                    "x": 100,
                    "y": 453,
                    # "text_auto" : uiScriptLocale.OK,
                    # "text_height": -7,
                    # "outline": True,
                    "default_image": "d:/ymir work/ui/public/acceptbutton00.sub",
                    "over_image": "d:/ymir work/ui/public/acceptbutton01.sub",
                    "down_image": "d:/ymir work/ui/public/acceptbutton02.sub",
                    "x_scale": 1.0,
                    "y_scale": 1.0,
                },
                {
                    "name": "button_cancel",
                    "type": "button",
                    "x": 100 + 70,
                    "y": 453,
                    # "text_auto": uiScriptLocale.CANCEL,
                    # "text_height": -7,
                    # "outline": True,
                    "default_image": "d:/ymir work/ui/public/canclebutton00.sub",
                    "over_image": "d:/ymir work/ui/public/canclebutton01.sub",
                    "down_image": "d:/ymir work/ui/public/canclebutton02.sub",
                    "x_scale": 1.0,
                    "y_scale": 1.0,
                },
            ),
        }
    ],
}
