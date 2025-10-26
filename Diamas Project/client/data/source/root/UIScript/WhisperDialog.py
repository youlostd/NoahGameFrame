import uiScriptLocale

ROOT = 'd:/ymir work/ui/public/'
window = {
    'name': 'WhisperDialog',
    'style': ('moveable', 'float'),
    "type": "window",

	"x" : 0,
	"y" : 0,

	"width" : 280,
	"height" : 200,
    'children': (

        {
            "name": "board",
            "type": "thinboard_old",
			"style" : ("attach",),
            "x": 0,
            "y": 0,

			"width" : 280,
			"height" : 200,

            'children': ({
                             'name': 'name_slot',
                             'type': 'expanded_image',
                             'style': ('attach',),
                             'x': 10,
                             'y': 10,
                             'image':"d:/ymir work/ui/public/Parameter_Slot_05.sub",
                             "x_scale": 1.0,
                             "y_scale": 1.0,
                             'children': ({
                                              'name': 'name',
                                              'type': 'limit_text',
                                              'x': 3,
                                              'y': 3,
                                              "max": 24,
                                              'text': uiScriptLocale.WHISPER_NAME
                                          },

                                          {
                                              'name': 'titlename_edit',
                                              'type': 'editline',
                                              'x': 3,
                                              'y': 3,
                                              'width': 120,
                                              'height': 17,
                                              'input_limit': 16,
                                              'text': uiScriptLocale.WHISPER_NAME
                                          })
                         },
                         {
                             'name': 'AddFriendButton',
                             'type': 'button',
                             'horizontal_align': 'right',
                             'x': 160,
                             'y': 12,
                             'tooltip_text': uiScriptLocale.MESSENGER_ADD_FRIEND,
                             'tooltip_x': 0,
                             'tooltip_y': 20,
                             'default_image': "d:/ymir work/ui/game/windows/messenger_add_friend_01.sub",
                             'over_image': "d:/ymir work/ui/game/windows/messenger_add_friend_02.sub",
                             'down_image': "d:/ymir work/ui/game/windows/messenger_add_friend_03.sub",
                         },

                         {
                             'name': 'ToggleEmojiButton',
                             'type': 'button',
                             'x': 135,
                             'y': 15,
                             'horizontal_align': 'right',
                             'tooltip_text': "Emoji",
                             'tooltip_x': 0,
                             'tooltip_y': 20,
                             'default_image': 'd:/ymir work/ui/game/emote/laugh.sub',
                             'over_image': 'd:/ymir work/ui/game/emote/laugh.sub',
                             'down_image': 'd:/ymir work/ui/game/emote/laugh.sub',
                             'disable_image': 'd:/ymir work/ui/game/emote/laugh.sub'
                         },
                         {
                             'name': 'gamemastermark',
                             'type': 'expanded_image',
                             'style': ('attach',
                                       ),
                             'horizontal_align': 'right',

                             'x': 75,
                             'y': 15,
                             'x_scale': 0.15,
                             'y_scale': 0.15,
                             'image': 'd:/Ymir work/effect/gm_logos/ymirred.DDS'
                         },
                         {
                             'name': 'language_flag',
                             'type': 'expanded_image',
                             'style': ('attach',),
                             'horizontal_align': 'right',
                             'x': 115,
                             'y': 19,
                             'x_scale': 1.0,
                             'y_scale': 1.0,
                             'image': 'd:/ymir work/ui/game/emote/en.sub'
                         },

                         {
                             'name': 'ignorebutton',
                             'type': 'toggle_button',
                             'horizontal_align': 'right',
                             'x': 85,
                             'y': 18,
                             "text_height": -5,
                            #  "text_color": 0xffffc539,

                             'tooltip_text': uiScriptLocale.WHISPER_BAN,
                             "default_image" : "d:/ymir work/ui/public/small_thin_button_01.sub",
                             "over_image" : "d:/ymir work/ui/public/small_thin_button_02.sub",
                             "down_image" : "d:/ymir work/ui/public/small_thin_button_03.sub",
                         },
                         {
                             'name': 'acceptbutton',
                             'type': 'button',
                             'horizontal_align': 'right',
                             'x': 105,
                             'y': 18,
                             "text_height": -5,
                             "text_color": 0xffffc539,

                         },
                         {
                             'name': 'minimizebutton',
                             'type': 'button',
                             "horizontal_align": "right",
                             'x': 42,
                             'y': 18,
                             'tooltip_text': uiScriptLocale.MINIMIZE,
                             "default_image" : "d:/ymir work/ui/public/minimize_button_01.sub",
                             "over_image" : "d:/ymir work/ui/public/minimize_button_02.sub",
                             "down_image" : "d:/ymir work/ui/public/minimize_button_03.sub",
                         },
                         {
                             'name': 'closebutton',
                             'type': 'button',
                             "horizontal_align": "right",

                             'x': 16,
                             'y': 18,
                             'tooltip_text': uiScriptLocale.CLOSE,
                             "default_image" : "d:/ymir work/ui/public/close_button_01.sub",
                             "over_image" : "d:/ymir work/ui/public/close_button_02.sub",
                             "down_image" : "d:/ymir work/ui/public/close_button_03.sub",
                         },
                         {
                             'name': 'scrollbar',
                             'type': 'small_thin_scrollbar',
                             'horizontal_align': 'right',

                             'x': 20,
                             'y': 45,
                             'size': 230
                         },
                         {
                             'name': 'editbar',
                             'type': 'bar',
                             "x" : 10,
                             "y" : 150,
                             "width" : 280 - 18,
                             "height" : 50,
                             "color" : 0x77000000,
                             'children': ({
                                              'name': 'chatline',
                                              'type': 'editline',
                                              "x" : 5,
                                              "y" : 5,
                                              "width" : 280 - 70,
                                              "height" : 40,
                                              'with_codepage': 1,
                                              'input_limit': 100,
                                              'limit_width': 180,
                                              'multi_line': 1,
                                              'color': 0xffe0c086,
                                              'fontname': 'Verdana:11',
                                          },
                                          {
                                              'name': 'sendbutton',
                                              'type': 'button',
                                              'x': 15,
                                              'y': 0,
                                              "text" : uiScriptLocale.WHISPER_SEND,
                                              "text_height": -5,
                                              'horizontal_align': 'right',
                                              'vertical_align': 'center',
                                              "default_image" : "d:/ymir work/ui/public/xlarge_thin_button_01.sub",
                                              "over_image" : "d:/ymir work/ui/public/xlarge_thin_button_02.sub",
                                              "down_image" : "d:/ymir work/ui/public/xlarge_thin_button_03.sub",

                                          })
                         })
        },

        {
            'name': 'emoji_board',
            'type': 'thinboard_old',
            'style': ('attach',),
            'x': 0,
            'y': 388,
            'width': 330,
            'height': 200,
        },
    )
}
