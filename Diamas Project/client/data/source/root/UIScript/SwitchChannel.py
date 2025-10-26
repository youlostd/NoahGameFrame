import uiScriptLocale

ROOT_PATH = "d:/ymir work/ui/public/"
window = {
	"name" : "SwitchChannelWindow",

	"x" : (SCREEN_WIDTH - 138) / 2,
	"y" : (SCREEN_HEIGHT - 200) / 2,

	"style" : ("moveable", "float",),

	"width" : 175,
	"height" : 210,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : 175,
			"height" : 210,
			"title" : uiScriptLocale.SWITCH_CHANNEL,
		},
		{
			"name" : "channel",
			"type" : "button",

			"x" : 4,
			"y" : 40,

			"text" : "Channel X",

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.9,
                    'y_scale': 1.0,
		},
		{
			"name" : "cancel",
			"type" : "button",

			"x" : 5,
			"y" : 165,

			"width" : 41,
			"height" : 21,

			"text" : uiScriptLocale.CANCEL,
            
                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.9,
                    'y_scale': 1.0,
		},
	)
}