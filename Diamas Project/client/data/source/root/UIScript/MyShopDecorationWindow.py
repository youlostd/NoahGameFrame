import uiScriptLocale

MAINBOARD_WIDTH = 402
MAINBOARD_HEIGHT = 320
MAINBOARD_X = (SCREEN_WIDTH / 2) - (MAINBOARD_WIDTH / 2)
MAINBOARD_Y = (SCREEN_HEIGHT / 2) - (MAINBOARD_HEIGHT / 2)

LEFTBOARD_WIDTH = 174
LEFTBOARD_HEIGHT = 269
LEFTBOARD_X = 13
LEFTBOARD_Y = 36

RIGHTBOARD_WIDTH = 196
RIGHTBOARD_HEIGHT = 269
RIGHTBOARD_X = 192
RIGHTBOARD_Y = 36

window = {
    "name": "MyShopDecoWindow",
	"style" : ("moveable", "float",),

    "x": MAINBOARD_X,
    "y": MAINBOARD_Y,

    "width": MAINBOARD_WIDTH,
    "height": MAINBOARD_HEIGHT,

    "children":
        (
            ## MainBoard
            {
                "name": "MyShopDecoBoard",
                "type": "board",
                "style": ("attach", "ltr"),

                "x": 0,
                "y": 0,

                "width": MAINBOARD_WIDTH,
                "height": MAINBOARD_HEIGHT,

                "children":
                    (
                        ## Title Bar
                        {
                            "name": "MyShopTitleBar",
                            "type": "titlebar",
                            "style": ("attach",),

                            "x": 6, "y": 7, "width": MAINBOARD_WIDTH - 13,
                            "title": uiScriptLocale.MYSHOP_DECO_SELECT_MODEL,

                        },

                        # Left Board
                        {
                            "name": "LeftBoard",
                            "type": "thinboard_circle",
                            "x": LEFTBOARD_X, "y": LEFTBOARD_Y, "width": LEFTBOARD_WIDTH, "height": LEFTBOARD_HEIGHT,

                            "children":
                                (
                                    {
                                        "name": "ScrollBar",
                                        "type": "scrollbar",

                                        "x": 17,
                                        "y": 6,
                                        "size": LEFTBOARD_HEIGHT - 12,
                                        "horizontal_align": "right",
                                    },
                                ),
                        },  ## Left Board End

                        ## Right Board
                        {
                            "name": "RightBoard",
                            "type": "thinboard_circle",
                            "x": RIGHTBOARD_X, "y": RIGHTBOARD_Y, "width": RIGHTBOARD_WIDTH,
                            "height": RIGHTBOARD_HEIGHT,

                            "children":
                                (
                                    ## Title Bar
                                    {
                                        "name": "ModelView",
                                        "type": "image",

                                        "x": 195 - RIGHTBOARD_X, "y": 39 - RIGHTBOARD_Y,
                                        "image": "d:/ymir work/ui/game/myshop_deco/model_view_title.sub",

                                        "children":
                                            (
                                                {"name": "ModelName", "type": "text", "x": 0, "y": 0, "text": "123",
                                                 "all_align": "center"},
                                            ),
                                    },

                                    {
                                        "name": "RenderTarget",
                                        "type": "render_target",

                                        "x": 195 - RIGHTBOARD_X,
                                        "y": 62 - RIGHTBOARD_Y,

                                        "width": 190,
                                        "height": 210,  # 306

                                        "index": 1,
                                    },

                                    ## Button Next
                                    {
                                        "name": "NextButton",
                                        "type": "button",

                                        "x": 293 - RIGHTBOARD_X,
                                        "y": 268 - RIGHTBOARD_Y,

                                        "text": uiScriptLocale.MYSHOP_DECO_NEXT,
                                        "text_height": 6,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                                    },

                                    ## Button Cancel
                                    {
                                        "name": "CancelButton",
                                        "type": "button",

                                        "x": 201 - RIGHTBOARD_X,
                                        "y": 268 - RIGHTBOARD_Y,

                                        "text": uiScriptLocale.MYSHOP_DECO_CANCEL,
                                        "text_height": 6,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                                    },

                                    ## Button Previous
                                    {
                                        "name": "PrevButton",
                                        "type": "button",

                                        "x": 201 - RIGHTBOARD_X,
                                        "y": 268 - RIGHTBOARD_Y,

                                        "text": uiScriptLocale.MYSHOP_DECO_PREV,
                                        "text_height": 6,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                                    },

                                    ## Button Complete
                                    {
                                        "name": "CompleteButton",
                                        "type": "button",

                                        "x": 293 - RIGHTBOARD_X,
                                        "y": 268 - RIGHTBOARD_Y,

                                        "text": uiScriptLocale.MYSHOP_DECO_OK,
                                        "text_height": 6,
                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                                    },
                                ),
                        },  ## Right Board End
                    ),
            },  ## MainBoard End
        ),
}
