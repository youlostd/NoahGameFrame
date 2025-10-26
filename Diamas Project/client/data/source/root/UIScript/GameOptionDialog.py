import uiScriptLocale
import app

ROOT_PATH = "d:/ymir work/ui/public/"
LOCALE_PATH = "d:/ymir work/ui/game/windows/"

TEMPORARY_X = +13
BUTTON_TEMPORARY_X = 5
PVP_X = -10

LINE_LABEL_X = 20
LINE_DATA_X = 120
LINE_LABEL_X_2 = 340
LINE_DATA_X_COL2 = 320 + 90 + 35
LINE_BEGIN = 40
LINE_STEP = 25
SMALL_BUTTON_WIDTH = 45
MIDDLE_BUTTON_WIDTH = 65

window = {
    "name": "GameOptionDialog",
    "style": ["moveable", "float", ],

    "x": 0,
    "y": 0,

    "width": 650,
    "height": 25 * 11 + 8 + 20,

    "children":
        [
            {
                "name": "board",
                "type": "board",

                "x": 0,
                "y": 0,

                "width": 650,
                "height": 25 * 11 + 8 + 20,

                "children":
                    [
                        ## Title
                        {
                            "name": "titlebar",
                            "type": "titlebar",
                            "style": ["attach", ],

                            "x": 8,
                            "y": 0,

                            "width": 584 + 50,
                            "color": "gray",
                            "title" : uiScriptLocale.GAMEOPTION_TITLE,

                        },

                        ## �̸���
                        {
                            "name": "name_color",
                            "type": "button",

                            "x": LINE_LABEL_X,
                            "y": 40 + 2 + 25,

                            "text_left": uiScriptLocale.OPTION_NAME_COLOR,
                            "text_color": 0xff9e9a9a,

                            "extended_data": uiScriptLocale.OPTION_NAME_COLOR_DESC,
                            "fontname": "Verdana:11",

                            "default_image": LOCALE_PATH + "details.sub",
                            "over_image": LOCALE_PATH + "details.sub",
                            "down_image": LOCALE_PATH + "details.sub",
                        },
                        {
                            "name": "name_color_normal",
                            "type": "radio_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH * 0,
                            "y": 40 + 25,

                            "text": uiScriptLocale.OPTION_NAME_COLOR_NORMAL,
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,

                            "default_image": ROOT_PATH + "Middle_Button_01.sub",
                            "over_image": ROOT_PATH + "Middle_Button_02.sub",
                            "down_image": ROOT_PATH + "Middle_Button_03.sub",
                        },
                        {
                            "name": "name_color_empire",
                            "type": "radio_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH * 1,
                            "y": 40 + 25,

                            "text": uiScriptLocale.OPTION_NAME_COLOR_EMPIRE,

                            "default_image": ROOT_PATH + "Middle_Button_01.sub",
                            "fontname": "Verdana:9",
                            "over_image": ROOT_PATH + "Middle_Button_02.sub",
                            "down_image": ROOT_PATH + "Middle_Button_03.sub",
                        },

                        ## Ÿ��â
                        {
                            "name": "target_board",
                            "type": "button",

                            "x": LINE_LABEL_X,
                            "y": 65 + 2 + 25,

                            "extended_data": uiScriptLocale.OPTION_TARGET_BOARD_DESC,
                            "text_color": 0xff9e9a9a,

                            "text_left": uiScriptLocale.OPTION_TARGET_BOARD,

                            "default_image": LOCALE_PATH + "details.sub",
                            "over_image": LOCALE_PATH + "details.sub",
                            "down_image": LOCALE_PATH + "details.sub",
                        },
                        {
                            "name": "target_board_no_view",
                            "type": "radio_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH * 0,
                            "y": 65 + 25,

                            "text": uiScriptLocale.OPTION_TARGET_BOARD_NO_VIEW,
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,


                            "default_image": ROOT_PATH + "Middle_Button_01.sub",
                            "over_image": ROOT_PATH + "Middle_Button_02.sub",
                            "down_image": ROOT_PATH + "Middle_Button_03.sub",
                        },
                        {
                            "name": "target_board_view",
                            "type": "radio_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH * 1,
                            "y": 65 + 25,

                            "text": uiScriptLocale.OPTION_TARGET_BOARD_VIEW,
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,

                            "default_image": ROOT_PATH + "Middle_Button_01.sub",
                            "over_image": ROOT_PATH + "Middle_Button_02.sub",
                            "down_image": ROOT_PATH + "Middle_Button_03.sub",
                        },

                        ## PvP Mode
                        {
                            "name": "pvp_mode",
                            "type": "button",

                            "x": LINE_LABEL_X,
                            "y": 90 + 2 + 25,

                            "extended_data": uiScriptLocale.OPTION_PVPMODE_DESC,
                            "text_left": uiScriptLocale.OPTION_PVPMODE,
                            "text_color": 0xff9e9a9a,

                            "default_image": LOCALE_PATH + "details.sub",
                            "over_image": LOCALE_PATH + "details.sub",
                            "down_image": LOCALE_PATH + "details.sub",
                        },
                        {
                            "name": "pvp_peace",
                            "type": "radio_button",

                            "x": LINE_DATA_X + SMALL_BUTTON_WIDTH * 0,
                            "y": 90 + 25,

                            "text": uiScriptLocale.OPTION_PVPMODE_PEACE,
                            "tooltip_text": uiScriptLocale.OPTION_PVPMODE_PEACE_TOOLTIP,
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,

                            "default_image": ROOT_PATH + "small_Button_01.sub",
                            "over_image": ROOT_PATH + "small_Button_02.sub",
                            "down_image": ROOT_PATH + "small_Button_03.sub",
                        },
                        {
                            "name": "pvp_revenge",
                            "type": "radio_button",

                            "x": LINE_DATA_X + SMALL_BUTTON_WIDTH * 1,
                            "y": 90 + 25,

                            "text": uiScriptLocale.OPTION_PVPMODE_REVENGE,
                            "tooltip_text": uiScriptLocale.OPTION_PVPMODE_REVENGE_TOOLTIP,

                            "default_image": ROOT_PATH + "small_Button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "small_Button_02.sub",
                            "down_image": ROOT_PATH + "small_Button_03.sub",
                        },
                        {
                            "name": "pvp_guild",
                            "type": "radio_button",

                            "x": LINE_DATA_X + SMALL_BUTTON_WIDTH * 2,
                            "y": 90 + 25,

                            "text": uiScriptLocale.OPTION_PVPMODE_GUILD,
                            "tooltip_text": uiScriptLocale.OPTION_PVPMODE_GUILD_TOOLTIP,

                            "default_image": ROOT_PATH + "small_Button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "small_Button_02.sub",
                            "down_image": ROOT_PATH + "small_Button_03.sub",
                        },
                        {
                            "name": "pvp_free",
                            "type": "radio_button",

                            "x": LINE_DATA_X + SMALL_BUTTON_WIDTH * 3,
                            "y": 90 + 25,

                            "text": uiScriptLocale.OPTION_PVPMODE_FREE,
                            "tooltip_text": uiScriptLocale.OPTION_PVPMODE_FREE_TOOLTIP,

                            "default_image": ROOT_PATH + "small_Button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "small_Button_02.sub",
                            "down_image": ROOT_PATH + "small_Button_03.sub",
                        },

                        ## Block
                        {
                            "name": "block",
                            "type": "button",

                            "x": LINE_LABEL_X,
                            "y": 115 + 2 + 25,

                            "text_left": uiScriptLocale.OPTION_BLOCK,
                            "text_color": 0xff9e9a9a,

                            "extended_data": uiScriptLocale.OPTION_BLOCK_DESC,

                            "default_image": LOCALE_PATH + "details.sub",
                            "over_image": LOCALE_PATH + "details.sub",
                            "down_image": LOCALE_PATH + "details.sub",
                        },
                        {
                            "name": "block_exchange_button",
                            "type": "toggle_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH * 0,
                            "y": 115 + 25,

                            "text": uiScriptLocale.OPTION_BLOCK_EXCHANGE,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },
                        {
                            "name": "block_party_button",
                            "type": "toggle_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH * 1,
                            "y": 115 + 25,

                            "text": uiScriptLocale.OPTION_BLOCK_PARTY,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },
                        {
                            "name": "block_guild_button",
                            "type": "toggle_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH * 2,
                            "y": 115 + 25,

                            "text": uiScriptLocale.OPTION_BLOCK_GUILD,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },
                        {
                            "name": "block_whisper_button",
                            "type": "toggle_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH * 0,
                            "y": 140 + 25,

                            "text": uiScriptLocale.OPTION_BLOCK_WHISPER,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },
                        {
                            "name": "block_friend_button",
                            "type": "toggle_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH * 1,
                            "y": 140 + 25,

                            "text": uiScriptLocale.OPTION_BLOCK_FRIEND,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },
                        {
                            "name": "block_party_request_button",
                            "type": "toggle_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH * 2,
                            "y": 140 + 25,

                            "text": uiScriptLocale.OPTION_BLOCK_PARTY_REQUEST,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },
                        {
                            "name": "block_duel_request_button",
                            "type": "toggle_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH * 0,
                            "y": 140 + 25 + 25,

                            "text": uiScriptLocale.OPTION_BLOCK_DUEL_REQUEST,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },

                        ## Chat
                        {
                            "name": "chat",
                            "type": "button",

                            "x": LINE_LABEL_X,
                            "y": 165 + 2 + 50,

                            "extended_data": uiScriptLocale.OPTION_VIEW_CHAT_DESC,
                            "text_left": uiScriptLocale.OPTION_VIEW_CHAT,
                            "text_color": 0xff9e9a9a,

                            "default_image": LOCALE_PATH + "details.sub",
                            "over_image": LOCALE_PATH + "details.sub",
                            "down_image": LOCALE_PATH + "details.sub",
                        },
                        {
                            "name": "view_chat_on_button",
                            "type": "radio_button",

                            "x": LINE_DATA_X,
                            "y": 165 + 50,

                            "text": uiScriptLocale.OPTION_VIEW_CHAT_ON,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },
                        {
                            "name": "view_chat_off_button",
                            "type": "radio_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
                            "y": 165 + 50,

                            "text": uiScriptLocale.OPTION_VIEW_CHAT_OFF,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },

                        ## Always Show Name
                        {
                            "name": "always_show_name",
                            "type": "button",

                            "x": LINE_LABEL_X,
                            "y": 190 + 2 + 50,

                            "extended_data": uiScriptLocale.OPTION_ALWAYS_SHOW_NAME_DESC,
                            "text_left": uiScriptLocale.OPTION_ALWAYS_SHOW_NAME,
                            "text_color": 0xff9e9a9a,

                            "default_image": LOCALE_PATH + "details.sub",
                            "over_image": LOCALE_PATH + "details.sub",
                            "down_image": LOCALE_PATH + "details.sub",
                        },
                        {
                            "name": "always_show_name_on_button",
                            "type": "radio_button",

                            "x": LINE_DATA_X,
                            "y": 190 + 50,

                            "text": uiScriptLocale.OPTION_ALWAYS_SHOW_NAME_ON,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },
                        {
                            "name": "always_show_name_off_button",
                            "type": "radio_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
                            "y": 190 + 50,

                            "text": uiScriptLocale.OPTION_ALWAYS_SHOW_NAME_OFF,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },

                        ## Effect On/Off
                        {
                            "name": "effect_on_off",
                            "type": "button",

                            "x": LINE_LABEL_X,
                            "y": 215 + 2 + 50,

                            "extended_data": uiScriptLocale.OPTION_EFFECT_DESC,
                            "text_left": uiScriptLocale.OPTION_EFFECT,
                            "text_color": 0xff9e9a9a,

                            "default_image": LOCALE_PATH + "details.sub",
                            "over_image": LOCALE_PATH + "details.sub",
                            "down_image": LOCALE_PATH + "details.sub",
                        },
                        {
                            "name": "show_damage_on_button",
                            "type": "radio_button",

                            "x": LINE_DATA_X,
                            "y": 215 + 50,

                            "text": uiScriptLocale.OPTION_VIEW_CHAT_ON,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },
                        {
                            "name": "show_damage_off_button",
                            "type": "radio_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
                            "y": 215 + 50,

                            "text": uiScriptLocale.OPTION_VIEW_CHAT_OFF,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },

                        ## �ǸŹ���
                        {
                            "name": "salestext_on_off",
                            "type": "button",

                            "x": LINE_LABEL_X,
                            "y": 240 + 2 + 50,

                            "extended_data": uiScriptLocale.OPTION_SALESTEXT_DESC,
                            "text_left": uiScriptLocale.OPTION_SALESTEXT,
                            "text_color": 0xff9e9a9a,

                            "default_image": LOCALE_PATH + "details.sub",
                            "over_image": LOCALE_PATH + "details.sub",
                            "down_image": LOCALE_PATH + "details.sub",
                        },
                        {
                            "name": "salestext_on_button",
                            "type": "radio_button",

                            "x": LINE_DATA_X,
                            "y": 240 + 50,

                            "text": uiScriptLocale.OPTION_SALESTEXT_VIEW_ON,
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },
                        {
                            "name": "salestext_off_button",
                            "type": "radio_button",

                            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
                            "y": 240 + 50,

                            "text": uiScriptLocale.OPTION_SALESTEXT_VIEW_OFF,

                            "default_image": ROOT_PATH + "middle_button_01.sub",
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
                            "over_image": ROOT_PATH + "middle_button_02.sub",
                            "down_image": ROOT_PATH + "middle_button_03.sub",
                        },


                        {
                            "name": "horizontalbar0", "type": "horizontalbar", "x": LINE_LABEL_X, "y": 42, "width": 250,
                            "children": ({"name": "horizontalbarName0", "type": "text", "x": 0, "y": 0, "color" : 0xf5f5dc,
                                          "text": uiScriptLocale.STANDARD_SETTINGS, "all_align": "center", },),
                        },
                        {
                            "name": "horizontalbar0", "type": "horizontalbar", "x": LINE_LABEL_X_2, "y": 42,
                            "width": 250,
                            "children": ({"name": "horizontalbarName0", "type": "text", "x": 0, "y": 0, "color" : 0xf5f5dc,
                                          "text": uiScriptLocale.SPECIAL_SETTINGS, "all_align": "center", },),
                        },

                    ],
            },
        ],
}

LINE_DATA_X = LINE_DATA_X_COL2

CUR_LINE_Y = LINE_BEGIN

if app.WJ_SHOW_MOB_INFO:
    CUR_LINE_Y += LINE_STEP
    window["children"][0]["children"] = window["children"][0]["children"] + [

        ## Show Mob Info List
        {
            "name": "show_mob_info",
            "type": "button",

            "multi_line": 1,

            "x": LINE_LABEL_X_2,
            "y": CUR_LINE_Y + 2,

            "extended_data": uiScriptLocale.OPTION_MOB_INFO_DESC,
            "text_left": uiScriptLocale.OPTION_MOB_INFO,
            "text_color": 0xff9e9a9a,

            "default_image": LOCALE_PATH + "details.sub",
            "over_image": LOCALE_PATH + "details.sub",
            "down_image": LOCALE_PATH + "details.sub",
        },
        {
            "name": "show_mob_level_button",
            "type": "toggle_button",

            "x": LINE_DATA_X,
            "y": CUR_LINE_Y,

            "text": uiScriptLocale.OPTION_MOB_INFO_LEVEL,
            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,

            "default_image": ROOT_PATH + "middle_button_01.sub",
            "over_image": ROOT_PATH + "middle_button_02.sub",
            "down_image": ROOT_PATH + "middle_button_03.sub",
        },
        {
            "name": "show_mob_AI_flag_button",
            "type": "toggle_button",

            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
            "y": CUR_LINE_Y,

            "text": uiScriptLocale.OPTION_MOB_INFO_AGGR,
            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,

            "default_image": ROOT_PATH + "middle_button_01.sub",
            "over_image": ROOT_PATH + "middle_button_02.sub",
            "down_image": ROOT_PATH + "middle_button_03.sub",
        }, ]

if app.WJ_SHOW_SHOP_CONTROLLER:
    CUR_LINE_Y += LINE_STEP
    window["children"][0]["children"] = window["children"][0]["children"] + [

        {
            "name": "displayed_shop_info",
            "type": "button",

            "x": LINE_LABEL_X_2,
            "y": CUR_LINE_Y,

            "extended_data": uiScriptLocale.OPTION_DISPLAYED_AROUND_DESC,
            "text_left": uiScriptLocale.OPTION_DISPLAYED_AROUND,
            "text_color": 0xff9e9a9a,

            "default_image": LOCALE_PATH + "details.sub",
            "over_image": LOCALE_PATH + "details.sub",
            "down_image": LOCALE_PATH + "details.sub",
        },
        {
            "name": "displayed_shops_controller",
            "type": "sliderbar",

            "x": LINE_DATA_X,
            "y": CUR_LINE_Y,
        },
        {
            "name": "displayed_shops_amount",
            "type": "text",
            "text": "20",

            "x": LINE_DATA_X + 180,
            "y": CUR_LINE_Y,
        }, ]

if app.WJ_SHOW_ACCE_ONOFF:
    CUR_LINE_Y += LINE_STEP
    window["children"][0]["children"] = window["children"][0]["children"] + [

        ## ??Z???
        {
            "name": "acce_on_off",
            "type": "button",

            "x": LINE_LABEL_X_2,
            "y": CUR_LINE_Y + 2,

            "extended_data": uiScriptLocale.OPTION_ACCE_DESC,
            "text_left": uiScriptLocale.OPTION_ACCE,
            "text_color": 0xff9e9a9a,

            "default_image": LOCALE_PATH + "details.sub",
            "over_image": LOCALE_PATH + "details.sub",
            "down_image": LOCALE_PATH + "details.sub",
        },
        {
            "name": "acce_on_button",
            "type": "radio_button",

            "x": LINE_DATA_X,
            "y": CUR_LINE_Y,

            "text": uiScriptLocale.OPTION_EMOJI_ON,
            "text_color": 0xffe2e2af,

            "default_image": ROOT_PATH + "middle_button_01.sub",
            "over_image": ROOT_PATH + "middle_button_02.sub",
            "down_image": ROOT_PATH + "middle_button_03.sub",
        },
        {
            "name": "acce_off_button",
            "type": "radio_button",

            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
            "y": CUR_LINE_Y,

            "text": uiScriptLocale.OPTION_EMOJI_OFF,
            "text_color": 0xffe2e2af,

            "default_image": ROOT_PATH + "middle_button_01.sub",
            "over_image": ROOT_PATH + "middle_button_02.sub",
            "down_image": ROOT_PATH + "middle_button_03.sub",
        },
    ]

CUR_LINE_Y += LINE_STEP
window["children"][0]["children"] = window["children"][0]["children"] + [

    ## ??Z???
    {
        "name": "costume_on_off",
        "type": "button",

        "x": LINE_LABEL_X_2,
        "y": CUR_LINE_Y + 2,

        "extended_data": uiScriptLocale.OPTION_COSTUME_DESC,
        "text_left": uiScriptLocale.OPTION_COSTUME,
        "text_color": 0xff9e9a9a,

        "default_image": LOCALE_PATH + "details.sub",
        "over_image": LOCALE_PATH + "details.sub",
        "down_image": LOCALE_PATH + "details.sub",
    },
    {
        "name": "costume_on_button",
        "type": "radio_button",

        "x": LINE_DATA_X,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_EMOJI_ON,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
    {
        "name": "costume_off_button",
        "type": "radio_button",

        "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_EMOJI_OFF,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
    {
        "name": "costume_apply_button",
        "type": "button",

        "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH + MIDDLE_BUTTON_WIDTH,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_COSTUME_VIEW_APPLY,
        "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
]

if app.ENABLE_SPECULAR_ONOFF:
    CUR_LINE_Y += LINE_STEP
    window["children"][0]["children"] = window["children"][0]["children"] + [

        ## specular npc/mob
        {
            "name": "specular_on_off",
            "type": "button",

            "x": LINE_LABEL_X_2,
            "y": CUR_LINE_Y + 2,
            "extended_data": uiScriptLocale.OPTION_SPECULAR_DESC,
            "text_color": 0xff9e9a9a,

            "text_left": uiScriptLocale.OPTION_SPECULAR,
            "default_image": LOCALE_PATH + "details.sub",
            "over_image": LOCALE_PATH + "details.sub",
            "down_image": LOCALE_PATH + "details.sub",
        },
        {
            "name": "specular_on_button",
            "type": "radio_button",

            "x": LINE_DATA_X,
            "y": CUR_LINE_Y,

            "text": uiScriptLocale.OPTION_EMOJI_ON,
            "text_color": 0xffe2e2af,

            "default_image": ROOT_PATH + "middle_button_01.sub",
            "over_image": ROOT_PATH + "middle_button_02.sub",
            "down_image": ROOT_PATH + "middle_button_03.sub",
        },
        {
            "name": "specular_off_button",
            "type": "radio_button",

            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
            "y": CUR_LINE_Y,

            "text": uiScriptLocale.OPTION_EMOJI_OFF,
            "text_color": 0xffe2e2af,

            "default_image": ROOT_PATH + "middle_button_01.sub",
            "over_image": ROOT_PATH + "middle_button_02.sub",
            "down_image": ROOT_PATH + "middle_button_03.sub",
        },
    ]

CUR_LINE_Y += LINE_STEP
window["children"][0]["children"] = window["children"][0]["children"] + [

    ## ??Z???
    {
        "name": "gold_on_off",
        "type": "button",

        "x": LINE_LABEL_X_2,
        "y": CUR_LINE_Y + 2,
        "extended_data": uiScriptLocale.OPTION_YANG_SHOW_DESC,
        "text_color": 0xff9e9a9a,

        "text_left": uiScriptLocale.OPTION_YANG_SHOW,
        "default_image": LOCALE_PATH + "details.sub",
        "over_image": LOCALE_PATH + "details.sub",
        "down_image": LOCALE_PATH + "details.sub",
    },
    {
        "name": "gold_on_button",
        "type": "radio_button",

        "x": LINE_DATA_X,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_EMOJI_ON,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
    {
        "name": "gold_off_button",
        "type": "radio_button",

        "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_EMOJI_OFF,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
]

CUR_LINE_Y += LINE_STEP
window["children"][0]["children"] = window["children"][0]["children"] + [

    {
        "name": "ihigh_on_off",
        "type": "button",

        "x": LINE_LABEL_X_2,
        "y": CUR_LINE_Y + 2,
        "extended_data": uiScriptLocale.OPTION_ITEM_HIGHLIGHT_DESC,
        "text_color": 0xff9e9a9a,

        "text_left": uiScriptLocale.OPTION_ITEM_HIGHLIGHT,
        "default_image": LOCALE_PATH + "details.sub",
        "over_image": LOCALE_PATH + "details.sub",
        "down_image": LOCALE_PATH + "details.sub",
    },
    {
        "name": "ihigh_on_button",
        "type": "radio_button",

        "x": LINE_DATA_X,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_EMOJI_ON,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
    {
        "name": "ihigh_off_button",
        "type": "radio_button",

        "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_EMOJI_OFF,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
]

CUR_LINE_Y += LINE_STEP
window["children"][0]["children"] = window["children"][0]["children"] + [

    {
        "name": "emoji_on_off",
        "type": "button",

        "x": LINE_LABEL_X_2,
        "y": CUR_LINE_Y + 2,
        "extended_data": uiScriptLocale.OPTION_EMOJI_DESC,
        "text_color": 0xff9e9a9a,

        "text_left": uiScriptLocale.OPTION_EMOJI,
        "default_image": LOCALE_PATH + "details.sub",
        "over_image": LOCALE_PATH + "details.sub",
        "down_image": LOCALE_PATH + "details.sub",
    },
    {
        "name": "emoji_on_button",
        "type": "radio_button",

        "x": LINE_DATA_X,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_EMOJI_ON,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
    {
        "name": "emoji_off_button",
        "type": "radio_button",

        "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_EMOJI_OFF,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
]

CUR_LINE_Y += LINE_STEP
window["children"][0]["children"] = window["children"][0]["children"] + [

    {
        "name": "country_flags",
        "type": "button",

        "x": LINE_LABEL_X_2,
        "y": CUR_LINE_Y + 2,
        "extended_data": uiScriptLocale.OPTION_COUNTRY_FLAGS_DESC,
        "text_color": 0xff9e9a9a,

        "text_left": uiScriptLocale.OPTION_COUNTRY_FLAGS,
        "default_image": LOCALE_PATH + "details.sub",
        "over_image": LOCALE_PATH + "details.sub",
        "down_image": LOCALE_PATH + "details.sub",
    },
    {
        "name": "country_flags_on_button",
        "type": "radio_button",

        "x": LINE_DATA_X,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_EMOJI_ON,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
    {
        "name": "country_flags_off_button",
        "type": "radio_button",

        "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_EMOJI_OFF,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
]

if app.ENABLE_SPECULAR_ONOFF:
    CUR_LINE_Y += LINE_STEP
    window["children"][0]["children"] = window["children"][0]["children"] + [

        ## specular npc/mob
        {
            "name": "specular_npc_on_off",
            "type": "button",

            "x": LINE_LABEL_X_2,
            "y": CUR_LINE_Y + 2,
            "extended_data": uiScriptLocale.OPTION_SPECULAR_NPC_DESC,
            "text_color": 0xff9e9a9a,

            "text_left": uiScriptLocale.OPTION_SPECULAR_NPC,
            "default_image": LOCALE_PATH + "details.sub",
            "over_image": LOCALE_PATH + "details.sub",
            "down_image": LOCALE_PATH + "details.sub",
        },
        {
            "name": "specular_npc_on_button",
            "type": "radio_button",

            "x": LINE_DATA_X,
            "y": CUR_LINE_Y,

            "text": uiScriptLocale.OPTION_EMOJI_ON,
            "text_color": 0xffe2e2af,

            "default_image": ROOT_PATH + "middle_button_01.sub",
            "over_image": ROOT_PATH + "middle_button_02.sub",
            "down_image": ROOT_PATH + "middle_button_03.sub",
        },
        {
            "name": "specular_npc_off_button",
            "type": "radio_button",

            "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
            "y": CUR_LINE_Y,

            "text": uiScriptLocale.OPTION_EMOJI_OFF,
            "text_color": 0xffe2e2af,

            "default_image": ROOT_PATH + "middle_button_01.sub",
            "over_image": ROOT_PATH + "middle_button_02.sub",
            "down_image": ROOT_PATH + "middle_button_03.sub",
        },
    ]

CUR_LINE_Y += LINE_STEP
window["children"][0]["children"] = window["children"][0]["children"] + [

    {
        "name": "collect_equip",
        "type": "button",

        "x": LINE_LABEL_X_2,
        "y": CUR_LINE_Y + 2,
        "extended_data": uiScriptLocale.OPTION_COLLECT_EQUIP_DESC,
        "text_color": 0xff9e9a9a,

        "text_left": uiScriptLocale.OPTION_COLLECT_EQUIP,
        "default_image": LOCALE_PATH + "details.sub",
        "over_image": LOCALE_PATH + "details.sub",
        "down_image": LOCALE_PATH + "details.sub",
    },
    {
        "name": "collect_equip_on_button",
        "type": "radio_button",

        "x": LINE_DATA_X,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_COLLECT_EQUIP_ON,
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
    {
        "name": "collect_equip_off_button",
        "type": "radio_button",

        "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_COLLECT_EQUIP_OFF,
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
    {
        "name": "collect_equip_useable_button",
        "type": "toggle_button",

        "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH + MIDDLE_BUTTON_WIDTH,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_COLLECT_EQUIP_USEABLE,
                            "fontname": "Verdana:10",
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
]

CUR_LINE_Y += LINE_STEP
window["children"][0]["children"] = window["children"][0]["children"] + [

    {
        "name": "show_companions",
        "type": "button",

        "x": LINE_LABEL_X_2,
        "y": CUR_LINE_Y + 2,
        "extended_data": uiScriptLocale.OPTION_SHOW_COMPANIONS_DESC,
        "text_color": 0xff9e9a9a,

        "text_left": uiScriptLocale.OPTION_SHOW_COMPANIONS,
        "default_image": LOCALE_PATH + "details.sub",
        "over_image": LOCALE_PATH + "details.sub",
        "down_image": LOCALE_PATH + "details.sub",
    },
    {
        "name": "show_companions_pets_button",
        "type": "toggle_button",

        "x": LINE_DATA_X,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_SHOW_COMPANIONS_PETS,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
    {
        "name": "show_companions_mounts_button",
        "type": "toggle_button",

        "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_SHOW_COMPANIONS_MOUNTS,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
    {
        "name": "show_companions_shops_button",
        "type": "toggle_button",

        "x": LINE_DATA_X + MIDDLE_BUTTON_WIDTH + MIDDLE_BUTTON_WIDTH,
        "y": CUR_LINE_Y,

        "text": uiScriptLocale.OPTION_SHOW_SHOPS,
        "text_color": 0xffe2e2af,

        "default_image": ROOT_PATH + "middle_button_01.sub",
        "over_image": ROOT_PATH + "middle_button_02.sub",
        "down_image": ROOT_PATH + "middle_button_03.sub",
    },
]
CUR_LINE_Y += LINE_STEP

# so oft wie nötig
for i in xrange(3):
    window["height"] = window["height"] + 25
    window["children"][0]["height"] = window["children"][0]["height"] + 25
