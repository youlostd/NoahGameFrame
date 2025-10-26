import uiScriptLocale

window = {
    'name': 'ShopDialog',
    'x': SCREEN_WIDTH - 400,
    'y': 10,
    'style': ('moveable', 'float'),
    'width': 184 + 160,
    'height': 400,
    'children':
        (
            {
                'name': 'board',
                'type': 'board',
                'style': ('attach',),
                'x': 0,
                'y': 0,
                'width': 184 + 160,
                'height': 380,
                'children': (
                    {
                        'name': 'TitleBar',
                        'type': 'titlebar',
                        'style': ('attach',),
                        'x': 8,
                        'y': 0,
                        'width': 169 + 160,
                        'color': 'gray',
                        "title": uiScriptLocale.SHOP_TITLE,

                    },
                    {
                        'name': 'ItemSlot',
                        'type': 'grid_table',
                        'x': 12,
                        'y': 49,
                        'start_index': 0,
                        'x_count': 10,
                        'y_count': 9,
                        'x_step': 32,
                        'y_step': 32,
                        'image': 'd:/ymir work/ui/public/Slot_Base.sub'
                    },
                    {
                        'name': 'BuyButton',
                        'type': 'toggle_button',
                        'x': 21,
                        'y': 327,
                        'width': 61,
                        'height': 21,
						'hidden': True,
                        'text': uiScriptLocale.SHOP_BUY,
                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.5,
                    'y_scale': 1.0,
                    },
                    {
                        'name': 'SellButton',
                        'type': 'toggle_button',
                        'x': 104,
                        'y': 327,
                        'width': 61,
                        'height': 21,
						'hidden': True,
                        'text': uiScriptLocale.SHOP_SELL,
                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.5,
                    'y_scale': 1.0,
                    },
                    {
                        'name': 'CloseButton',
                        'type': 'button',
                        'x': 0,
                        'y': 327,
                        'horizontal_align': 'center',
                        'text': uiScriptLocale.PRIVATE_SHOP_CLOSE_BUTTON,
                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.5,
                    'y_scale': 1.0,
                    }
                )

            },
        )
}
