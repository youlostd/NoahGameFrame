import uiScriptLocale

TEMPORARY_HEIGHT = 16
SLOTBAR_HEIGHT = 25


TITLE_Y_INCREASE = SLOTBAR_HEIGHT + 30
EDITLINE_Y_INCREASE = TITLE_Y_INCREASE
window = {
	"name": "RegisterDialog",
	"x": 0,
	"y": 0,
	"style": ("moveable", "float"),
	"width": 280,
	"height": 400,
	"children": 
    (
		{
			"name": "Board",
			"type": "board_with_titlebar",
			"x": 0,
			"y": 0,
			"width": 280,
			"height": 400,
			"title": uiScriptLocale.REGISTER_TITLE_BAR,
			"children": 
            (
                {
					"name" : "InputBoard",
					"type": "thinboard_circle",
					"x":0,
					"y":35,
					"width":250,
					"height": 350,
					"horizontal_align": "center",
					"alpha": 0.6,
					"children" : 
					(    
						{
							"name": "UserNameTitle",
							"type": "text",
							"text": uiScriptLocale.REGISTER_TITLE_USERNAME,
							"horizontal_align": "center",
							"x": 0,
							"y": 8,
						},
						{
							"name": "UserNameInput",
							"type": "slotbar",
							"x": 0,
							"y": 30,
							"width": 225,
							"height": SLOTBAR_HEIGHT,
							"horizontal_align": "center",
							"children": (
								{
									"name": "UserNameValue",
									"type": "editline",
									"x": 3,
									"y": 5,
									"width": 215,
									"height": 22,
									"input_limit": 20,
								},
							),
						},
						{
							"name": "PasswordTitle",
							"type": "text",
							"text": uiScriptLocale.REGISTER_TITLE_PASSWORD,
							"horizontal_align": "center",
							"x": 0,
							"y": 8 + TITLE_Y_INCREASE,
						},
						{
							"name": "PasswordInput",
							"type": "slotbar",
							"x": 0,
							"y": 30 + EDITLINE_Y_INCREASE,
							"width": 225,
							"height": SLOTBAR_HEIGHT,
							"horizontal_align": "center",
							"children": (
								{
									"name": "PasswordValue",
									"type": "editline",
									"x": 3,
									"y": 5,
									"width": 215,
									"height": 22,
									"input_limit": 16,
								},
							),
						},
						{
							"name": "DeleteCodeTitle",
							"type": "text",
							"text": uiScriptLocale.REGISTER_TITLE_DELETE_CODE,
							"horizontal_align": "center",
							"x": 0,
							"y": 8 + TITLE_Y_INCREASE * 2,
						},
						{
							"name": "DeleteCodeInput",
							"type": "slotbar",
							"x": 0,
							"y": 30 + EDITLINE_Y_INCREASE * 2,
							"width": 225,
							"height": SLOTBAR_HEIGHT,
							"horizontal_align": "center",
							"children": (
								{
									"name": "DeleteCodeValue",
									"type": "editline",
									"x": 3,
									"y": 5,
									"width": 215,
									"height": 22,
									"input_limit": 7,
								},
							),
						},
						{
							"name": "MailTitle",
							"type": "text",
							"text": uiScriptLocale.REGISTER_TITLE_MAIL,
							"horizontal_align": "center",
							"x": 0,
							"y": 8 + TITLE_Y_INCREASE * 3,
						},
						{
							"name": "MailInput",
							"type": "slotbar",
							"x": 0,
							"y": 30 + EDITLINE_Y_INCREASE * 3,
							"width": 225,
							"height": SLOTBAR_HEIGHT,
							"horizontal_align": "center",
							"children": (
								{
									"name": "MailValue",
									"type": "editline",
									"x": 3,
									"y": 5,
									"width": 215,
									"height": 22,
									"input_limit": 60,
								},
							),
						},
						{
							"name": "BotControlTitle",
							"type": "text",
							"text": uiScriptLocale.REGISTER_TITLE_BOT_CONTROL,
							"horizontal_align": "center",
							"x": 0,
							"y": 8 + TITLE_Y_INCREASE * 4,
						},
                        {
							"name": "BotControlQuestion",
							"type": "text",
							"text": "2+1",
							"horizontal_align": "center",
							"x": 0,
							"y": 8 + TITLE_Y_INCREASE * 4 + 20,
						},
						{
							"name": "BotControlInput",
							"type": "slotbar",
							"x": 0,
							"y": 30 + EDITLINE_Y_INCREASE * 4 +20,
							"width": 225,
							"height": SLOTBAR_HEIGHT,
							"horizontal_align": "center",
							"children": (
								{
									"name": "BotControlValue",
									"type": "editline",
									"x": 3,
									"y": 5,
									"width": 215,
									"height": 22,
									"input_limit": 4,
								},
							),
						},
						{
								"name": "AcceptButton",
								"type": "button",
								"x": 0,
								"y": 30 + EDITLINE_Y_INCREASE * 4 +20 + 35,
								"default_image": "d:/ymir work/ui/public/acceptbutton00.sub",
								"over_image": "d:/ymir work/ui/public/acceptbutton01.sub",
								"down_image": "d:/ymir work/ui/public/acceptbutton02.sub",
								"x_scale": 1.0,
								"y_scale": 1.0,
                "horizontal_align": "center",
						},
					),
				},
			),
		},
	),
}
