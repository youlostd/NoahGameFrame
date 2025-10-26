import uiScriptLocale

ROOT_PATH = 'd:/ymir work/ui/gui/'
SMALL_BUTTON_WIDTH = 65
MIDDLE_BUTTON_WIDTH = 75
window = {
    'name': 'BlockModeDialog',
    'x': 0,
    'y': 0,
    "style" : ("moveable", "float",),
    'width': 290,
    'height': 140 + 50,
    'children':
    (
        {
            'name': 'Board',
            'type': 'board_with_titlebar',
            'x': 0,
            'y': 0,
            'width': 290,
            'height': 140 + 80,
            'title': 'Blocken',
            'children':
            (
                
                    {
                        'name': 'InnweBoard',
                        'type': 'thinboard_red',
                        'x': 5,
                        'y': 40,
                        'width': 278,
                        'height': 170 ,
                        'children':
                        (
                            {
                                'name': 'Button Center',
                                'type': 'window',
                                'x': 0,
                                'y': 20,
                                
                                'width': 230,
                                'height': 94,
                                'horizontal_align': 'center',
                                'children':
                                (
                                    {
                                        'name': 'block_exchange_button',
                                        'type': 'toggle_button',
                                        'x': MIDDLE_BUTTON_WIDTH * 0,
                                        'y': 0,
                                        'text': uiScriptLocale.OPTION_BLOCK_EXCHANGE,
                                        "text_height": -5,
                                        "text_color": 0xffffc539,
                                        'default_image': ROOT_PATH + 'normal_button.sub',
                                        'over_image': ROOT_PATH + 'normal_button_hover.sub',
                                        'down_image': ROOT_PATH + 'normal_button_down.sub',
                                        'x_scale': 0.4,
                                        'y_scale': 1.0,
                                    },
                                    {
                                        'name': 'block_party_button',
                                        'type': 'toggle_button',
                                        'x': MIDDLE_BUTTON_WIDTH * 1,
                                        'y': 0,
                                        'text': uiScriptLocale.OPTION_BLOCK_PARTY,
                                        "text_height": -5,
                                        "text_color": 0xffffc539,
                                        'default_image': ROOT_PATH + 'normal_button.sub',
                                        'over_image': ROOT_PATH + 'normal_button_hover.sub',
                                        'down_image': ROOT_PATH + 'normal_button_down.sub',
                                        'x_scale': 0.4,
                                        'y_scale': 1.0,
                                    },
                                    {
                                        'name': 'block_guild_button',
                                        'type': 'toggle_button',
                                        'x': MIDDLE_BUTTON_WIDTH * 2,
                                        'y': 0,
                                        'text': uiScriptLocale.OPTION_BLOCK_GUILD,
                                        "text_height": -5,
                                        "text_color": 0xffffc539,
                                        'default_image': ROOT_PATH + 'normal_button.sub',
                                        'over_image': ROOT_PATH + 'normal_button_hover.sub',
                                        'down_image': ROOT_PATH + 'normal_button_down.sub',
                                        'x_scale': 0.4,
                                        'y_scale': 1.0,
                                    },
                                    
                                    {
                                        'name': 'block_whisper_button',
                                        'type': 'toggle_button',
                                        'x': MIDDLE_BUTTON_WIDTH * 0,
                                        'y': 33,
                                        'text': uiScriptLocale.OPTION_BLOCK_WHISPER,
                                        "text_height": -5,
                                        "text_color": 0xffffc539,
                                        'default_image': ROOT_PATH + 'normal_button.sub',
                                        'over_image': ROOT_PATH + 'normal_button_hover.sub',
                                        'down_image': ROOT_PATH + 'normal_button_down.sub',
                                        'x_scale': 0.4,
                                        'y_scale': 1.0,
                                    },
                                    {
                                        'name': 'block_friend_button',
                                        'type': 'toggle_button',
                                        'x': MIDDLE_BUTTON_WIDTH * 1,
                                        'y': 33,
                                        'text': uiScriptLocale.OPTION_BLOCK_FRIEND,
                                        "text_height": -5,
                                        "text_color": 0xffffc539,
                                        'default_image': ROOT_PATH + 'normal_button.sub',
                                        'over_image': ROOT_PATH + 'normal_button_hover.sub',
                                        'down_image': ROOT_PATH + 'normal_button_down.sub',
                                        'x_scale': 0.4,
                                        'y_scale': 1.0,
                                    },
                                    {
                                        'name': 'block_party_request_button',
                                        'type': 'toggle_button',
                                        'x': MIDDLE_BUTTON_WIDTH * 2,
                                        'y': 33,
                                        'text': uiScriptLocale.OPTION_BLOCK_PARTY_REQUEST,
                                        "text_height": -5,
                                        "text_color": 0xffffc539,
                                        'default_image': ROOT_PATH + 'normal_button.sub',
                                        'over_image': ROOT_PATH + 'normal_button_hover.sub',
                                        'down_image': ROOT_PATH + 'normal_button_down.sub',
                                        'x_scale': 0.4,
                                        'y_scale': 1.0,
                                    },
                                    {
                                        'name': 'block_duel_request_button',
                                        'type': 'toggle_button',
                                        'x': MIDDLE_BUTTON_WIDTH * 0,
                                        'y':66,
                                        'text': uiScriptLocale.OPTION_BLOCK_DUEL_REQUEST,
                                        "text_height": -5,
                                        "text_color": 0xffffc539,
                                        'default_image': ROOT_PATH + 'normal_button.sub',
                                        'over_image': ROOT_PATH + 'normal_button_hover.sub',
                                        'down_image': ROOT_PATH + 'normal_button_down.sub',
                                        'x_scale': 0.4,
                                        'y_scale': 1.0,
                                    },
                                ),
                            
                            },
                        ),
                    
                    },



                {
                    'name': 'AcceptButton',
                    'type': 'button',
                    'x': -95,
                    'y': 16,
                    'horizontal_align': 'center',
                    'vertical_align': 'bottom',
                    'text': uiScriptLocale.OK,
                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                },
                {
                    'name': 'CancelButton',
                    'type': 'button',
                    'x': 95,
                    'y': 16,
                    'horizontal_align': 'center',
                    'vertical_align': 'bottom',
                    'text': uiScriptLocale.CANCEL,
                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
                }
            )
        },
    )
}
