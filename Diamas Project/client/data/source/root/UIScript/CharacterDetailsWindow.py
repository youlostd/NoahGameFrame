import localeInfo

ROOT_PATH = "d:/ymir work/ui/game/windows/"

MAINBOARD_WIDTH = 235
MAINBOARD_HEIGHT = 408  # 361

LABEL_START_X = 150 + 3
LABEL_START_Y = 39
LABEL_WIDTH = 50
LABEL_HEIGHT = 17
LABEL_GAP = LABEL_HEIGHT + 7
LABEL_NAME_POS_X = 40
TITLE_BAR_POS_X = 20
TITLE_BAR_WIDTH = 163

window = {
    "name": "CharacterDetailsWindow",
    "style": ("float",),
    "x": 277,  # 24+253-3,
    "y": (SCREEN_HEIGHT - 398) / 2,
    "width": MAINBOARD_WIDTH,
    "height": MAINBOARD_HEIGHT,
    "children": (
        ## MainBoard
        {
            "name": "MainBoard",
            "type": "board",
            "style": ("attach",),
            ## CharacterWindow.py ���� ����
            "x": 0,
            "y": 0,
            "width": MAINBOARD_WIDTH,
            "height": MAINBOARD_HEIGHT,
            "children": (
                ## Ÿ��Ʋ��
                {
                    "name": "TitleBar",
                    "type": "titlebar",
                    "style": ("attach",),
                    "x": 6,
                    "y": 0,
                    "width": MAINBOARD_WIDTH - 13,
                    "title": localeInfo.DETAILS_TITLE,
                },
                {
                    "name": "bg",
                    "type": "border_a",
                    "width": MAINBOARD_WIDTH - 20,
                    "height": MAINBOARD_HEIGHT - 40,
                    "x": 10,
                    "y": 30,
                },
            ),
        },  ## MainBoard End
    ),
}
