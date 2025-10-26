import localeInfo
import uiScriptLocale

window = {
    "name": "MoveChannelDialog",
    "style": ("moveable", "float",),

    "x": (SCREEN_WIDTH / 2) - (190 / 2),
    "y": (SCREEN_HEIGHT / 2) - 100,

    "width": 0,
    "height": 0,

    "children":
        (
            ## MainBoard
            {
                "name": "MoveChannelBoard",
                "type": "board_with_titlebar",
                "style": ("attach",),

                "x": 0,
                "y": 0,

                "width": 0,
                "height": 0,

                "title": uiScriptLocale.MOVE_CHANNEL_TITLE,

                "children":
                    (
                        {
                            "name": "BlackBoard",
                            "type": "thinboard_circle",
                            "x": 19, "y": 48, "width": 0, "height": 0,
                        },
                    ),
            },  ## MainBoard End
        ),
}
