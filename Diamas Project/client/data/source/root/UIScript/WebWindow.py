import uiScriptLocale

WEB_WIDTH = 1024
WEB_HEIGHT = 600

window = {
    "name": "MallWindow",

    "x": 0,
    "y": 0,

    "style": ("moveable", "float",),

    "width": WEB_WIDTH + 20,
    "height": WEB_HEIGHT + 40,

    "children":
        (
            {
                "name": "board",
                "type": "board_with_titlebar",
                "style": ("attach",),

                "x": 0,
                "y": 0,

                "width": WEB_WIDTH + 8,
                "height": WEB_HEIGHT + 45,
                "title": uiScriptLocale.SYSTEM_MALL,

                "children":
                    (
                        {
                            "name": "WebView",
                            "type": "web_view",
                            "x": 4,
                            "y": 42,

                            "width": WEB_WIDTH,
                            "height": WEB_HEIGHT,
                        },
                    ),
            },
        ),
}
