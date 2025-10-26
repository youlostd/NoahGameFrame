import uiScriptLocale

PATH_GUI_LOGIN = "d:/ymir work/ui/gui/login/"

SERVER_BOARD_HEIGHT = 220 + 180
SERVER_LIST_HEIGHT = 171 + 180

CHANNEL_BUTTON_Y = 160
CHANNEL_BUTTON_STATE_Y = 26
CHANNEL_BASE_POS = 65
CHANNEL_BASE_DIF = 65

LOGIN_FIELD_POS_X = 322 - 50 + 6
LOGIN_FIELD_POS_Y = 165 + 227
PASSWORD_FIELD_POS_X = 322 - 50 + 6
PASSWORD_FIELD_POS_Y = 220 + 252 - 36
# 147

CHANNEL_BG_HEIGHT_PERC = 70.5
LOGO_HEIGHT_PERC = 3.5
LOGIN_BOARD_HEIGHT_PERC = 5
EXIT_GAME_HEIGHT_PERC = 85.8
ACCOUNT_DATA1_HEIGHT_PERC = 78.4
ACCOUNT_DATA2_HEIGHT_PERC = 78.4
ACCOUNT_DATA3_HEIGHT_PERC = 78.4
ACCOUNT_DATA4_HEIGHT_PERC = 78.4

ACCOUNT_DATA_WIDTH = 200
ACCOUNT_DATA_SEP_DIST = 220
ACCOUNT_ADD_POSITION = 200 - 27 - 10
ACCOUNT_DELETE_POSITION = 200 - 56 - 10
ACCOUNT_ONE_WIDTH_PERC = 8.1
ACCOUNT_TWO_WIDTH_PERC = 29.5
ACCOUNT_THREE_WIDTH_PERC = 51
ACCOUNT_FOUR_WIDTH_PERC = 72.5

LANG_BASE_HEIGHT_PERC = 30.9

LOGO_HIDDEN = False

if SCREEN_WIDTH <= 1024:
	LOGO_HIDDEN = True
	CHANNEL_BG_HEIGHT_PERC = 70.5
	# LOGIN_BOARD_HEIGHT_PERC = 15.9
	EXIT_GAME_HEIGHT_PERC = 85.8
	ACCOUNT_DATA1_HEIGHT_PERC = 67
	ACCOUNT_DATA2_HEIGHT_PERC = 75
	ACCOUNT_DATA3_HEIGHT_PERC = 67
	ACCOUNT_DATA4_HEIGHT_PERC = 75
	ACCOUNT_ONE_WIDTH_PERC = 4
	ACCOUNT_TWO_WIDTH_PERC = 4
	ACCOUNT_THREE_WIDTH_PERC = 70
	ACCOUNT_FOUR_WIDTH_PERC = 70
	LANG_BASE_HEIGHT_PERC = 10.9

LANG_BASE_X = 282
LANG1_POS = 0

if SCREEN_HEIGHT > 768:
	LOGIN_BOARD_HEIGHT_PERC = 10
elif SCREEN_HEIGHT > 720:
	LOGIN_BOARD_HEIGHT_PERC = 5
elif SCREEN_HEIGHT > 600:
	LOGIN_BOARD_HEIGHT_PERC = 0
else:
	LOGIN_BOARD_HEIGHT_PERC = -20


BACKGROUND2_POS = 410
if SCREEN_WIDTH <= 1366:
	BACKGROUND2_POS = 310

BACKGROUND1_POS = 0
if SCREEN_WIDTH <= 1366:
	BACKGROUND1_POS = -100


def SCREEN_HEIGHT_PERCENT(pct):
	return float(SCREEN_HEIGHT) / 100.0 * pct  # 3.5% Top


def SCREEN_WIDTH_PERCENT(pct):
	return float(SCREEN_WIDTH) / 100.0 * pct  # 3.5% Top


# Colors
COLOR_NORMAL = 0xff494949
COLOR_HOVER = 0xffe0ffff
COLOR_LOGIN_TEXT = 0xffcbab9d
# LOGIN Interface
SAVE_EMPTY = uiScriptLocale.LOGIN_INTERFACE_SAVE_EMPTY
TITLE_BOARD = uiScriptLocale.LOGIN_INTERFAE_TITLE_BOARD

window = {
	"name": "LoginWindow",
	"style": ("moveable",),

	"x": 0,
	"y": 0,

	"width": SCREEN_WIDTH,
	"height": SCREEN_HEIGHT,

	"children":
		(
			## Board
			# {
			#	"name" : "bg1", "type" : "expanded_image", "x" : 0, "y" : 0,
			#	"x_scale" : float(SCREEN_WIDTH) / 1024.0, "y_scale" : float(SCREEN_HEIGHT) / 768.0,
			#	"image" :LOCALE_PATH + "/ui/serverlist.sub",
			# },

			# Main Background
			{
				"name": "Background",
				"type": "expanded_image",
				"x": 0, "y": 0,
				"image": "d:/ymir work/ui/gui/login/bbg.png",
				"x_scale": float(SCREEN_WIDTH) / 1920.0,
				"y_scale": float(SCREEN_HEIGHT) / 1080.0,
				"horizontal_align": "center",
				"vertical_align" : "center",
				"children": (

				{
					"name": "Logo",
					"type": "image",
					"x": 35, "y": 10,
					"horizontal_align": "center",
					"image": PATH_GUI_LOGIN + "logo.tga",
				},

				{
					"name" : "Login",
					"type" : "image",
					
					"x" : 0,
					"y" : 70,
					"horizontal_align": "center",
					"vertical_align" : "center",
					"image" : PATH_GUI_LOGIN + "dialogbg.dds",
					
					"children" : 
					[
						{
							"name": "accountLog",

							"x": 550,
							"y": 70,

							"width": 224,
							"height": 350,

							"children":
								(
								
								
								 {
								"name": "btn_bestpvm",
								"type": "expanded_image",
								"x": -40, "y": -155,
								# "image": PATH_GUI_LOGIN + "bestpvm.sub",

							},


							{
								"name": "btn3",
								"type": "button",
								"x": 40, "y": 255,
								"default_image": PATH_GUI_LOGIN + "discord_btn_down.dds",
								"over_image": PATH_GUI_LOGIN + "discord_btn_over.dds",
								"down_image": PATH_GUI_LOGIN + "discord_btn_over.dds",
							},

							 {
								"name": "btn4",
								"type": "button",
								"x": 100, "y": 255,
								"default_image": PATH_GUI_LOGIN + "web_btn_down.dds",
								"over_image": PATH_GUI_LOGIN + "web_btn_over.dds",
								"down_image": PATH_GUI_LOGIN + "web_btn_over.dds",
							},

							{
								"name": "btn2",
								"type": "button",
								"x": 160, "y": 255,
								"default_image": PATH_GUI_LOGIN + "twitch_btn_down.dds",
								"over_image": PATH_GUI_LOGIN + "twitch_btn_over.dds",
								"down_image": PATH_GUI_LOGIN + "twitch_btn_over.dds",
							},

							{
								"name": "btn1",
								"type": "button",
								"x": -7, "y": 280,
								# "default_image": PATH_GUI_LOGIN + "forum.sub",
								# "over_image": PATH_GUI_LOGIN + "forum2.sub",
								# "down_image": PATH_GUI_LOGIN + "forum3.sub",
							},

							{
								"name": "btn5",
								"type": "button",
								"x": 17 + 70, "y": 255,
								# "default_image": PATH_GUI_LOGIN + "itemshop.sub",
								# "over_image": PATH_GUI_LOGIN + "itemshop2.sub",
								# "down_image": PATH_GUI_LOGIN + "itemshop3.sub",
							},
							{
								"name": "btn6",
								"type": "button",
								"x": 17 + 140, "y": 255,
								# "default_image": PATH_GUI_LOGIN + "vote.sub",
								# "over_image": PATH_GUI_LOGIN + "vote2.sub",
								# "down_image": PATH_GUI_LOGIN + "vote3.sub",
							},                          
								
								
									{
										"name": "titleAccount",
										"type": "image",
										"horizontal_align": "center",
										"x": 5,
										"y": 0,
										"image": PATH_GUI_LOGIN + "save_account.sub",
									},
									
									
									
									
									{
										"name": "accountLogLeft",
										"type": "scrollwindow",

										"x": 0,
										"y": 30,
										"horizontal_align": "center",

										"width": 231,
										"height": 195,

										"children":
											(
												{
													"name": "accountsContent",
													"content_window": 1,

													"x": 20,
													"y": 0,

													"width": 224,
													"height": 400,

													"children": (
														{
															"name": "account_slot_0",
															"type": "window",

															"x": 3,
															"y": 1,

															"width": 188,
															"height": 34,

															"children":
																(

																	{
																		"name": "account_input_0",
																		"type": "expanded_image",
																		"x": 0,
																		"y": 10,
																		"image": PATH_GUI_LOGIN + "account.dds",
																	},
																	{
																		"name": "account_prefix_0",
																		"type": "text",

																		"x": 15,
																		"y": 17,

																		"text": "1. ",

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,

																	},

																	{
																		"name": "account_0",
																		"type": "text",

																		"x": 32,
																		"y": 17,

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,
																		"text": SAVE_EMPTY,

																	},
																	{
																		"name": "account_save_0",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 3,
																		"y": 10,
																		"default_image": PATH_GUI_LOGIN + "accountsave2.dds",
																		"over_image": PATH_GUI_LOGIN + "accountsave.dds",
																		"down_image": PATH_GUI_LOGIN + "accountsave.dds",
																	},
																	{
																		"name": "account_delete_0",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 3,
																		"y": 17,

																		"default_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"over_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"down_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																	},
																),

														},
														{
															"name": "account_slot_1",
															"type": "window",

															"x": 3,
															"y": 2 + 34 + 3,

															"width": 188,
															"height": 34,

															"children":
																(
																	{
																		"name": "account_input_1",
																		"type": "expanded_image",
																		"x": 0,
																		"y": 10,
																		"image": PATH_GUI_LOGIN + "account.dds",
																	},
																	{
																		"name": "account_prefix_1",
																		"type": "text",

																		"x": 15,
																		"y": 17,

																		"text": "2. ",

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,

																	},

																	{
																		"name": "account_1",
																		"type": "text",

																		"x": 32,
																		"y": 17,

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,
																		"text": SAVE_EMPTY,

																	},
																	{
																		"name": "account_save_1",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 23,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "accountsave2.dds",
																		"over_image": PATH_GUI_LOGIN + "accountsave.dds",
																		"down_image": PATH_GUI_LOGIN + "accountsave.dds",
																	},
																	{
																		"name": "account_delete_1",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 3,
																		"y": 17,

																		"default_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"over_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"down_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																	},
																),

														},
														{
															"name": "account_slot_2",
															"type": "window",

															"x": 3,
															"y": 0 + (2 + 34 * 2) + (3 * 2),

															"width": 188,
															"height": 34,

															"children":
																(
																	{
																		"name": "account_input_2",
																		"type": "expanded_image",
																		"x": 0,
																		"y": 10,
																		"image": PATH_GUI_LOGIN + "account.dds",
																	},
																	{
																		"name": "account_prefix_2",
																		"type": "text",

																		"x": 15,
																		"y": 17,

																		"text": "3. ",

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,

																	},

																	{
																		"name": "account_2",
																		"type": "text",

																		"x": 32,
																		"y": 17,

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,
																		"text": SAVE_EMPTY,

																	},
																	{
																		"name": "account_save_2",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 43,
																		"y": 10,

																		"fontname": "Verdana:12",

																		"default_image": PATH_GUI_LOGIN + "accountsave2.dds",
																		"over_image": PATH_GUI_LOGIN + "accountsave.dds",
																		"down_image": PATH_GUI_LOGIN + "accountsave.dds",
																	},
																	{
																		"name": "account_delete_2",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 3,
																		"y": 17,

																		"default_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"over_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"down_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																	},
																),

														},
														{
															"name": "account_slot_3",
															"type": "window",

															"x": 3,
															"y": 0 + 2 + 34 * 3 + (3 * 3),

															"width": 188,
															"height": 34,

															"children":
																(
																	{
																		"name": "account_input_3",
																		"type": "expanded_image",
																		"x": 0,
																		"y": 10,
																		"image": PATH_GUI_LOGIN + "account.dds",
																	},
																	{
																		"name": "account_prefix_3",
																		"type": "text",

																		"x": 15,
																		"y": 17,

																		"text": "4. ",

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,

																	},

																	{
																		"name": "account_3",
																		"type": "text",

																		"fontname": "Verdana:12",

																		"x": 32,
																		"y": 17,

																		"color": 0xffbf7c61,
																		"text": SAVE_EMPTY,

																	},
																	{
																		"name": "account_save_3",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 43,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "accountsave2.dds",
																		"over_image": PATH_GUI_LOGIN + "accountsave.dds",
																		"down_image": PATH_GUI_LOGIN + "accountsave.dds",
																	},
																	{
																		"name": "account_delete_3",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 3,
																		"y": 27,

																		"default_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"over_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"down_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																	},
																),

														},
														{
															"name": "account_slot_4",
															"type": "window",

															"x": 3,
															"y": 0 + (2 + 34 * 4) + (3 * 4),

															"width": 188,
															"height": 34,

															"children":
																(
																	{
																		"name": "account_input_4",
																		"type": "expanded_image",
																		"x": 0,
																		"y": 10,
																		"image": PATH_GUI_LOGIN + "account.dds",
																	},
																	{
																		"name": "account_prefix_4",
																		"type": "text",

																		"x": 15,
																		"y": 17,

																		"text": "5. ",

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,

																	},

																	{
																		"name": "account_4",
																		"type": "text",

																		"fontname": "Verdana:12",

																		"x": 32,
																		"y": 17,

																		"color": 0xffbf7c61,
																		"text": SAVE_EMPTY,

																	},
																	{
																		"name": "account_save_4",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 43,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "accountsave2.dds",
																		"over_image": PATH_GUI_LOGIN + "accountsave.dds",
																		"down_image": PATH_GUI_LOGIN + "accountsave.dds",
																	},
																	{
																		"name": "account_delete_4",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 3,
																		"y": 17,

																		"default_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"over_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"down_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																	},
																),

														},
														{
															"name": "account_slot_5",
															"type": "window",

															"x": 3,
															"y": 0 + (2 + 34 * 5) + (3 * 5),

															"width": 188,
															"height": 34,

															"children":
																(
																	{
																		"name": "account_input_5",
																		"type": "expanded_image",
																		"x": 0,
																		"y": 10,
																		"image": PATH_GUI_LOGIN + "account.dds",
																	},
																	{
																		"name": "account_prefix_5",
																		"type": "text",

																		"x": 15,
																		"y": 12,

																		"text": "6. ",

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,

																	},

																	{
																		"name": "account_5",
																		"type": "text",

																		"x": 32,
																		"y": 12,

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,
																		"text": SAVE_EMPTY,

																	},
																	{
																		"name": "account_save_5",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 23,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "accountsave2.dds",
																		"over_image": PATH_GUI_LOGIN + "accountsave.dds",
																		"down_image": PATH_GUI_LOGIN + "accountsave.dds",
																	},
																	{
																		"name": "account_delete_5",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 3,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"over_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"down_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																	},
																),

														},
														{
															"name": "account_slot_6",
															"type": "window",

															"x": 3,
															"y": 0 + (2 + 34 * 6) + (3 * 6),

															"width": 188,
															"height": 34,

															"children":
																(
																	{
																		"name": "account_input_6",
																		"type": "expanded_image",
																		"x": 0,
																		"y": 10,
																		"image": PATH_GUI_LOGIN + "account.dds",
																	},
																	{
																		"name": "account_prefix_6",
																		"type": "text",

																		"x": 15,
																		"y": 12,

																		"text": "7. ",

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,

																	},

																	{
																		"name": "account_6",
																		"type": "text",

																		"x": 32,
																		"y": 12,

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,
																		"text": SAVE_EMPTY,

																	},
																	{
																		"name": "account_save_6",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 23,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "accountsave2.dds",
																		"over_image": PATH_GUI_LOGIN + "accountsave.dds",
																		"down_image": PATH_GUI_LOGIN + "accountsave.dds",
																	},
																	{
																		"name": "account_delete_6",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 3,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"over_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"down_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																	},
																),

														},
														{
															"name": "account_slot_7",
															"type": "window",

															"x": 3,
															"y": (2 + 34 * 7) + (3 * 7),

															"width": 188,
															"height": 34,

															"children":
																(
																	{
																		"name": "account_input_7",
																		"type": "expanded_image",
																		"x": 0,
																		"y": 10,
																		"image": PATH_GUI_LOGIN + "account.dds",
																	},
																	{
																		"name": "account_prefix_7",
																		"type": "text",

																		"x": 15,
																		"y": 12,

																		"text": "8. ",

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,

																	},

																	{
																		"name": "account_7",
																		"type": "text",

																		"x": 32,
																		"y": 12,

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,
																		"text": SAVE_EMPTY,

																	},
																	{
																		"name": "account_save_7",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 23,
																		"y": 10,

																		"fontname": "Verdana:12",

																		"default_image": PATH_GUI_LOGIN + "accountsave2.dds",
																		"over_image": PATH_GUI_LOGIN + "accountsave.dds",
																		"down_image": PATH_GUI_LOGIN + "accountsave.dds",
																	},
																	{
																		"name": "account_delete_7",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 3,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"over_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"down_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																	},
																),

														},
														{
															"name": "account_slot_8",
															"type": "window",

															"x": 3,
															"y": 2 + 34 * 8 + (3 * 8),

															"width": 188,
															"height": 34,

															"children":
																(
																	{
																		"name": "account_input_8",
																		"type": "expanded_image",
																		"x": 0,
																		"y": 10,
																		"image": PATH_GUI_LOGIN + "account.dds",
																	},
																	{
																		"name": "account_prefix_8",
																		"type": "text",

																		"x": 15,
																		"y": 12,

																		"text": "9. ",

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,

																	},

																	{
																		"name": "account_8",
																		"type": "text",

																		"fontname": "Verdana:12",

																		"x": 32,
																		"y": 12,

																		"color": 0xffbf7c61,
																		"text": SAVE_EMPTY,

																	},
																	{
																		"name": "account_save_8",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 23,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "accountsave2.dds",
																		"over_image": PATH_GUI_LOGIN + "accountsave.dds",
																		"down_image": PATH_GUI_LOGIN + "accountsave.dds",
																	},
																	{
																		"name": "account_delete_8",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 3,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"over_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"down_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																	},
																),

														},
														{
															"name": "account_slot_9",
															"type": "window",

															"x": 3,
															"y": (2 + 34 * 9) + (3 * 9),

															"width": 188,
															"height": 34,

															"children":
																(
																	{
																		"name": "account_input_9",
																		"type": "expanded_image",
																		"x": 0,
																		"y": 10,
																		"image": PATH_GUI_LOGIN + "account.dds",
																	},
																	{
																		"name": "account_prefix_9",
																		"type": "text",

																		"x": 15,
																		"y": 12,

																		"text": "10.",

																		"fontname": "Verdana:12",

																		"color": 0xffbf7c61,

																	},

																	{
																		"name": "account_9",
																		"type": "text",

																		"fontname": "Verdana:12",

																		"x": 32,
																		"y": 12,

																		"color": 0xffbf7c61,
																		"text": SAVE_EMPTY,

																	},
																	{
																		"name": "account_save_9",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 23,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "accountsave2.dds",
																		"over_image": PATH_GUI_LOGIN + "accountsave.dds",
																		"down_image": PATH_GUI_LOGIN + "accountsave.dds",
																	},
																	{
																		"name": "account_delete_9",
																		"type": "button",

																		"horizontal_align": "right",
																		"x": 3,
																		"y": 10,

																		"default_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"over_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																		"down_image": PATH_GUI_LOGIN + "delete_account_btn.sub",
																	},
																	
																	
																	
																					  
																  
					 
																),

														},
													),

												},

											),
									},
								),
						},

						{
							"name": "channelBoard",

							"x": 25,
							"y": 52,

							"width": 295,
							"height": 335,

							"children":
								(
									{
										"name": "titleChannel",
										"type": "expanded_image",

										"x": 18,
										"y": 15,
										"image": PATH_GUI_LOGIN + "select_channel.sub",
									},
									
									
									
									{
								"name": "btn_bestpvp",
								"type": "expanded_image",
								"x": -44 , "y": -135,
								# "image": PATH_GUI_LOGIN + "bestpvp.sub",
									},

							
							
							
									{
										"name": "channelSlot0",
										"type": "window",

										"width": 291,
										"height": 42,

										"x": 3 - 10,
										"y": 45,

										"children": (
											{
												"name": "channelText0",
												"type": "expanded_image",
												"color": (0xff6d737b, 0xffe6e6e6),

												"text": "CHANNEL 1",
												"fontname": "Roboto:15",
												"image": PATH_GUI_LOGIN + "ch_btn_default.dds",

												"x": 16,
												"y": 9,
											},

											{
												"name": "channel1Button",
												"type": "button",

												"horizontal_align": "right",
												"x": 130,
												"y": 18,

												"default_image": "d:/ymir work/ui/login_interface/channel_checkbox.sub",
												"over_image": "d:/ymir work/ui/login_interface/channel_checkbox_hover.sub",
												"down_image": "d:/ymir work/ui/login_interface/channel_checkbox_hover.sub",
												"disable_image": "d:/ymir work/ui/login_interface/channel_checkbox_selected.sub"

											},
											{
												"name": "hrns1",
												"type": "text",
												"x": "30",
												"y": "15",
												"text": "CH-1",
												"fontname": "Nunito Sans:15S",
											}
										),
									},

									{
										"name": "channelSlot1",
										"type": "window",

										"width": 291,
										"height": 42,

										"x": 3 - 10,
										"y": 85,

										"children": (
											{
												"name": "channelText1",
												"type": "expanded_image",
												"color": (0xff6d737b, 0xffe6e6e6),

												"text": "CHANNEL 2",
												"fontname": "Roboto:15",
												"image": PATH_GUI_LOGIN + "ch_btn_default.dds",

												"x": 16,
												"y": 9,
											},

											{
												"name": "channel2Button",
												"type": "button",

												"horizontal_align": "right",
												"x": 130,
												"y": 18,

												"default_image": "d:/ymir work/ui/login_interface/channel_checkbox.sub",
												"over_image": "d:/ymir work/ui/login_interface/channel_checkbox_hover.sub",
												"down_image": "d:/ymir work/ui/login_interface/channel_checkbox_hover.sub",
												"disable_image": "d:/ymir work/ui/login_interface/channel_checkbox_selected.sub"

											},
											{
												"name": "hrns2",
												"type": "text",
												"x": "30",
												"y": "15",
												"text": "CH-2",
												"fontname": "Nunito Sans:15S",
											}
										),
									},

									{
										"name": "channelSlot2",
										"type": "window",

										"width": 291,
										"height": 42,

										"x": 3 - 10,
										"y": 125,

										"children": (
											{
												"name": "channelText2",
												"type": "expanded_image",
												"color": (0xff6d737b, 0xffe6e6e6),

												"text": "CHANNEL 3",
												"fontname": "Roboto:15",
												"image": PATH_GUI_LOGIN + "ch_btn_default.dds",

												"x": 16,
												"y": 9,
											},

											{
												"name": "channel3Button",
												"type": "button",

												"horizontal_align": "right",
												"x": 130,
												"y": 15,

												"default_image": "d:/ymir work/ui/login_interface/channel_checkbox.sub",
												"over_image": "d:/ymir work/ui/login_interface/channel_checkbox_hover.sub",
												"down_image": "d:/ymir work/ui/login_interface/channel_checkbox_hover.sub",
												"disable_image": "d:/ymir work/ui/login_interface/channel_checkbox_selected.sub"

											},
											{
												"name": "hrns3",
												"type": "text",
												"x": "30",
												"y": "15",
												"text": "CH-3",
												"fontname": "Nunito Sans:15S",
											}
										),
									},

									{
										"name": "channelSlot3",
										"type": "window",

										"width": 291,
										"height": 42,

										"x": 3 - 10,
										"y": 165,

										"children": (
											{
												"name": "channelText3",
												"type": "image",
												"image": PATH_GUI_LOGIN + "ch_btn_default.dds",
												"fontname": "Roboto:15",
												"text": "CHANNEL 4",

												"x": 16,
												"y": 9,
											},

											{
												"name": "channel4Button",
												"type": "button",

												"horizontal_align": "right",
												"x": 130,
												"y": 15,

												"default_image": "d:/ymir work/ui/login_interface/channel_checkbox.sub",
												"over_image": "d:/ymir work/ui/login_interface/channel_checkbox_hover.sub",
												"down_image": "d:/ymir work/ui/login_interface/channel_checkbox_hover.sub",
												"disable_image": "d:/ymir work/ui/login_interface/channel_checkbox_selected.sub"

											},
											{
												"name": "hrns4",
												"type": "text",
												"x": "30",
												"y": "15",
												"fontname": "Nunito Sans:15S",
												"text": "CH-4",
											}
										),
									},

								),
						},

						## Panel_Login
						{
							"name": "LoginBoard",
							# "type" : "thinboard_login",

							"x": 0,
							"y": -10,
							"x_scale": float(SCREEN_WIDTH) / 1920.0,
							"y_scale": float(SCREEN_HEIGHT) / 1080.0,
							"width": 230,
							"height": 592,

							"horizontal_align": "center",

							"children":
								(

									{
										"name": "titleBoard",
										"type": "text",

										"x": -10,
										"y": 52,

										"horizontal_align": "center",

										"color": COLOR_HOVER,
										"text": "",

										"fontname": "Nunito Sans:17b",

										"r": 0.79607843,
										"g": 0.71764705,
										"b": 0.47450980,
										"a": 1.0,
									},
									{
										"name": "id_title",
										"type": "expanded_image",

										"x": 80 - 45,
										"y": 60,

										"image": PATH_GUI_LOGIN + "userpanel.sub",

									},

									## id slot
									{
										"name": "ID_Slot",
										"type": "expanded_image",

										"x": 10 - 10,
										"y": 110,
										"horizontal_align": "center",

										"image": PATH_GUI_LOGIN + "user_input.dds",

										"children":
											(
												{
													"name": "ID_EditLine",
													"type": "editline",

													"x": 50,
													"y": 10,

													"width": 180,
													"height": 20,

													"input_limit": 30,

													"r": 1.0,
													"g": 1.0,
													"b": 0.757,
													"a": 1.0,
													"fontname": "Roboto:13l",

													"placeholder_text": "Username..",
													"placeholder_color": 0xf8d982,

												},
											),
									},

									## password slot
									{
										"name": "Password_Slot",
										"type": "expanded_image",

										"x": 10 - 10,
										"y": 160,

										"horizontal_align": "center",

										"image": PATH_GUI_LOGIN + "pw_input.dds",

										"children":
											(
												{
													"name": "Password_EditLine",
													"type": "editline",

													"x": 50,
													"y": 10,

													"width": 180,
													"height": 20,

													"input_limit": 16,
													"secret_flag": 1,
													"text": "Password",
													"fontname": "Roboto:13l",

													"placeholder_text": "Password..",
													"placeholder_color": 0xf8d982,

													"r": 1.0,
													"g": 1.0,
													"b": 0.757,
													"a": 1.0,

												},
											),
									},

									# {
									# 	"name": "id_editline_s",
									# 	"type": "editline",

									# 	"x": 50,
									# 	"y": -5,

									# 	"width" : 212,
									# 	"height" : 35,

									# 	"input_limit" : 16,
									# 	"enable_codepage" : 0,
									# },
									# {
									# 	"name": "pwd_editlines",
									# 	"type": "editline",

									# 	"x": 50,
									# 	"y": 50-5,

									# 	"width" : 212,
									# 	"height" : 35,

									# 	"input_limit" : 16,
									# 	"secret_flag" : 1,
									# 	"enable_codepage" : 0,
									# },

									{
										"name": "LoginButton",
										"type": "button",
										"x": 5 - 4,
										"y": 218,
										"horizontal_align": "center",
										"default_image": PATH_GUI_LOGIN + "register_btn_default_1.png",
										"over_image": PATH_GUI_LOGIN + "register_btn_over_1.png",
										"down_image": PATH_GUI_LOGIN + "register_btn_down._1png",
										"text": "|cffffa07aLogin",
										"fontname": "UnZialish:20",
									},

									{
										"name": "RegisterButton",
										"type": "button",
										"x": 1,
										"y": 218+45,
										"horizontal_align": "center",
										"default_image": PATH_GUI_LOGIN + "register_btn_default_1.png",
										"over_image": PATH_GUI_LOGIN + "register_btn_over_1.png",
										"down_image": PATH_GUI_LOGIN + "register_btn_down._1png",
										"text": "|cffff8c00Register",
										"fontname": "UnZialish:20",
										# "fontname": "Roboto:16s",
										# "x": 0,
										# "y": 0,
									},

									{
										"name": "exit_button",
										"type": "button",
										"x": 120 - 50,
										"y": 275+27,
										"default_image": PATH_GUI_LOGIN + "exit.sub",
										"over_image": PATH_GUI_LOGIN + "exit2.sub",
										"down_image": PATH_GUI_LOGIN + "exit3.sub",
									},
									{
										"name": "dropdownboard",
										'style': ('not_pick',),
										"x": 0,
										"y": 80,
										"width": 200,
										"height": 304,
									},
								),
						},
					],
					
				},		


					{
						"name": "HGuardBoard",
						"type": "thinboard",

						"x": 0,
						"y": 90,
						"width": 430,
						"height": 185,
						"horizontal_align": "center",

						"children":
							(

								## Title
								{
									"name": "Title",
									"type": "text",

									"x": 0,
									"y": 24,
									"horizontal_align": "center",
									"text": "Account Security",
								},


								{
									"name": "FirstTextLine",
									"type": "text",

									"x": 20,
									"y": 50,
									# "horizontal_align" : "center",
									# "horizontal_align" : "center",
									"text": uiScriptLocale.SENTRY_DESC_FIRSTLINE,
								},
								{
									"name": "SecondTextLine",
									"type": "text",

									"x": 20,
									"y": 65,
									# "horizontal_align" : "center",
									# "horizontal_align" : "center",
									"text": uiScriptLocale.SENTRY_DESC_SECONDLINE,
								},
								{
									"name": "ThirdTextLine",
									"type": "text",

									"x": 20,
									"y": 80,
									# "horizontal_align" : "center",
									# "horizontal_align" : "center",
									"text": uiScriptLocale.SENTRY_DESC_THIRDLINE,
								},

								## Input Slot
								{
									"name": "InputSlot",
									"type": "slotbar",

									"x": 0,
									"y": 102,
									"width": 300,
									"height": 25,
									"horizontal_align": "center",
									"children":
										(
											{
												"name": "HGuardCode",
												"type": "editline",

												"x": 3,
												"y": 5,

												"width": 300,
												"height": 35,

												"fontsize": "LARGE",

												"input_limit": 6,
												"horizontal_align": "center",
												# "horizontal_align" : "center",

												"r": 1.0,
												"g": 1.0,
												"b": 1.0,
												"a": 0.0,
												"outline": True,
											},
										),
								},

								## Buttons
								{
									"name": "SendHGuardCode",
									"type": "button",

									"x": 88,
									"y": 135,

									"text_height": -5,
									"text_color": 0xffffc539,
									'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
									'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
									'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
									'x_scale': 0.4,
									'y_scale': 1.0,

									"text": uiScriptLocale.SENTRY_CODE_SEND,
								},
								## Buttons
								{
									"name": "CancelHGuardCode",
									"type": "button",

									"x": 228,
									"y": 135,

									"text_height": -5,
									"text_color": 0xffffc539,
									'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
									'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
									'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
									'x_scale': 0.4,
									'y_scale': 1.0,

									"text": uiScriptLocale.SENTRY_CODE_CANCEL,
								},
							),
					},
				),
			},
		 


		),
}
