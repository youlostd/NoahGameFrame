import uiScriptLocale
import grp
import item

BOARD_WIDTH = 780
BOARD_HEIGHT = 419

PAGE_LEFT_X_DIST = 40
PAGE_RIGHT_X_DIST = 30
PAGE_X_DIST = PAGE_LEFT_X_DIST + PAGE_RIGHT_X_DIST
PAGE_TOP_DIST = 60
PAGE_BOT_DIST = 45

NAVI_BTN_WIDTH = 100
NAVI_BTN_HEIGHT = 24
NAVI_BTN_COUNT = 5
NAVI_BOT_DIST = 10

EQUIPMENT_START_INDEX = EQUIPMENT_SLOT_START

window = {
	"name" : "AdminManagerWindow",

	"x" : (SCREEN_WIDTH - BOARD_WIDTH) / 2,
	"y" : (SCREEN_HEIGHT - BOARD_HEIGHT) / 2,

	"style" : ("moveable", "float",),

	"width" : BOARD_WIDTH,
	"height" : BOARD_HEIGHT,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board_with_titlebar",
			"style" : ("attach",),

			"x" : 0,
			"y" : 0,

			"width" : BOARD_WIDTH,
			"height" : BOARD_HEIGHT,

			"title" : uiScriptLocale.ADMIN_MANAGER_TITLE,

			"children" :
			(
				## Page1 : General
				{
					"name" : "page_general",
					"type" : "thinboard",

					"x" : PAGE_LEFT_X_DIST,
					"y" : PAGE_TOP_DIST,

					"width" : BOARD_WIDTH - PAGE_X_DIST,
					"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST),

					"children" :
					(
						{
							"name" : "general_online_player_table",
							"type" : "table",

							"x" : 10,
							"y" : 10,

							"width" : 300,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 20 - 30,

							"col_size" : [15, 35, 25, 15, 10],
							"header" : [uiScriptLocale.ADMIN_MANAGER_GENERAL_TABLE_PID,\
										uiScriptLocale.ADMIN_MANAGER_GENERAL_TABLE_NAME,\
										uiScriptLocale.ADMIN_MANAGER_GENERAL_TABLE_MAP_INDEX,\
										uiScriptLocale.ADMIN_MANAGER_GENERAL_TABLE_CHANNEL,\
										uiScriptLocale.ADMIN_MANAGER_GENERAL_TABLE_EMPIRE],
						},
						{
							"name" : "general_online_player_scrollbar",
							"type" : "scrollbar",

							"x" : 10 + 300 + 5,
							"y" : 10,

							"size" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 20 - 30,
						},
						{
							"name" : "general_online_player_dividing_line",
							"type" : "line",

							"x" : 10 + 300 + 5 + 17 - 1,
							"y" : 10,

							"width" : 0,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 20 - 30,

							"color" : grp.GenerateColor(1.0, 1.0, 1.0, 0.8),
						},
						{
							"name" : "general_online_player_search",

							"x" : 20,
							"y" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 20 - 16,

							"width" : BOARD_WIDTH - PAGE_X_DIST - 15 * 2,
							"height" : 24,

							"children" :
							(
								{
									"name" : "general_online_player_search_text",
									"type" : "text",

									"x" : 0,
									"y" : 2,

									"text" : uiScriptLocale.ADMIN_MANAGER_GENERAL_SEARCH_PLAYER,
								},
								{
									"name" : "general_online_player_search_box",
									"type" : "slotbar",

									"x" : 75,
									"y" : 0,

									"width" : 175,
									"height" : 18,

									"children" :
									(
										{
											"name" : "general_online_player_search_edit",
											"type" : "editline",

											"x" : 3,
											"y" : 3,

											"width" : 165,
											"height" : 18,

											"input_limit" : CHARACTER_NAME_MAX_LEN,
										},
									),
								},
								{
									"name" : "general_online_player_search_button",
									"type" : "button",

									"x" : 75 + 175 + 5,
									"y" : 0,

									"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
									"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
									"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",

									"text" : uiScriptLocale.ADMIN_MANAGER_GENERAL_SEARCH_PLAYER_BUTTON,
								},
							),
						},
						{
							"name" : "general_main_information_window",

							"x" : 328,
							"y" : 15,

							"width" : BOARD_WIDTH - PAGE_X_DIST - 332,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

							"children" :
							(
								{
									"name" : "general_current_user_count",
									"type" : "text",

									"x" : 10,
									"y" : 0,
								},
								{
									"name" : "general_gm_item_trade_checkbox",
									"type" : "checkbox",

									"x" : 10,
									"y" : 19 + 5,

									"text" : uiScriptLocale.ADMIN_MANAGER_GENERAL_GM_ITEM_TRADE_OPTION,
								},
							),
						},
					),
				},

				## Page2 : MapViewer
				{
					"name" : "page_mapviewer",
					"type" : "thinboard",

					"x" : PAGE_LEFT_X_DIST,
					"y" : PAGE_TOP_DIST,

					"width" : BOARD_WIDTH - PAGE_X_DIST,
					"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST),

					"children" :
					(
						{
							"name" : "mapviewer_atlas",

							"x" : 10,
							"y" : 10,

							"width" : 300,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 20,
						},
						{
							"name" : "mapviewer_no_map_selected",

							"x" : 10,
							"y" : 10,

							"width" : 300,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 20,

							"children" :
							(
								{
									"name" : "mapviewer_no_map_line1",
									"type" : "line",

									"x" : 0,
									"y" : 0,

									"width" : 300,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 20,

									"color" : grp.GenerateColor(1.0, 1.0, 1.0, 0.8),
								},
								{
									"name" : "mapviewer_no_map_line2",
									"type" : "line",

									"x" : 300,
									"y" : 0,

									"width" : -300,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 20,

									"color" : grp.GenerateColor(1.0, 1.0, 1.0, 0.8),
								},
							),
						},
						{
							"name" : "mapviewer_dividing_line1",
							"type" : "line",

							"x" : 10 + 300 + 5,
							"y" : 10,

							"width" : 0,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 20,

							"color" : grp.GenerateColor(1.0, 1.0, 1.0, 0.8),
						},
						{
							"name" : "mapviewer_right_side",

							"x" : 10 + 300 + 10,
							"y" : 10,

							"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 300 + 10) - 10,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 20,

							"children" :
							(
								{
									"name" : "mapviewer_select_title",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"horizontal_align" : "center",
									"horizontal_align" : "center",

									"text" : uiScriptLocale.ADMIN_MANAGER_MAPVIEWER_SELECT_TITLE,
								},
								{
									"name" : "mapviewer_select_list",
									"type" : "listbox",

									"x" : 0,
									"y" : 20,

									"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 300 + 10) - 10 - 17,
									"height" : 17 * 5,
								},
								{
									"name" : "mapviewer_select_scrollbar",
									"type" : "scrollbar",

									"x" : BOARD_WIDTH - PAGE_X_DIST - (10 + 300 + 10) - 10 - 17,
									"y" : 20,

									"size" : 17 * 5 + 5 + 24,
								},
								{
									"name" : "mapviewer_select_button",
									"type" : "button",

									"x" : (BOARD_WIDTH - PAGE_X_DIST - (10 + 300 + 10) - 10 - 17 - 88) / 2,
									"y" : 20 + 17 * 5 + 5,

									"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

									"text" : uiScriptLocale.ADMIN_MANAGER_MAPVIEWER_SELECT_BUTTON,
								},
								{
									"name" : "mapviewer_dividing_line2",
									"type" : "line",

									"x" : 0,
									"y" : 20 + 17 * 5 + 5 + 27,

									"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 300 + 10) - 10 - 10 * 2,
									"height" : 0,

									"horizontal_align" : "center",

									"color" : grp.GenerateColor(1.0, 1.0, 1.0, 0.8),
								},
								{
									"name" : "mapviewer_option_player",
									"type" : "checkbox",

									"x" : 5,
									"y" : 20 + 17 * 5 + 5 + 38 + (19 + 5) * 0,

									"text" : uiScriptLocale.ADMIN_MANAGER_MAPVIEWER_PLAYER_OPTION,
								},
								{
									"name" : "mapviewer_option_mob",
									"type" : "checkbox",

									"x" : 5,
									"y" : 20 + 17 * 5 + 5 + 38 + (19 + 5) * 1,

									"text" : uiScriptLocale.ADMIN_MANAGER_MAPVIEWER_MOB_OPTION,
								},
								{
									"name" : "mapviewer_option_stone",
									"type" : "checkbox",

									"x" : 5,
									"y" : 20 + 17 * 5 + 5 + 38 + (19 + 5) * 2,

									"text" : uiScriptLocale.ADMIN_MANAGER_MAPVIEWER_STONE_OPTION,
								},
								{
									"name" : "mapviewer_option_npc",
									"type" : "checkbox",

									"x" : 5,
									"y" : 20 + 17 * 5 + 5 + 38 + (19 + 5) * 3,

									"text" : uiScriptLocale.ADMIN_MANAGER_MAPVIEWER_NPC_OPTION,
								},
								{
									"name" : "mapviewer_option_ore",
									"type" : "checkbox",

									"x" : 5 + 110,
									"y" : 20 + 17 * 5 + 5 + 38 + (19 + 5) * 0,

									"text" : uiScriptLocale.ADMIN_MANAGER_MAPVIEWER_ORE_OPTION,
								},
								{
									"name" : "mapviewer_stop_button",
									"type" : "button",

									"x" : 0,
									"y" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 20 - 24,

									"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

									"horizontal_align" : "center",

									"text" : uiScriptLocale.ADMIN_MANAGER_MAPVIEWER_STOP_BUTTON,
								},
							),
						},
					),
				},

				## Page3 : Observer
				{
					"name" : "page_observer",
					"type" : "thinboard",

					"x" : PAGE_LEFT_X_DIST,
					"y" : PAGE_TOP_DIST,

					"width" : BOARD_WIDTH - PAGE_X_DIST,
					"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST),

					"children" :
					(
						{
							"name" : "observer_stopped_wnd",

							"x" : 0,
							"y" : 0,

							"width" : BOARD_WIDTH - PAGE_X_DIST,
							"height" : 20 + 18 + 5 + 25,

							"vertical_align" : "center",

							"children" :
							(
								{
									"name" : "observer_stopped_title",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"horizontal_align" : "center",
									"horizontal_align" : "center",

									"text" : uiScriptLocale.ADMIN_MANAGER_OBSERVER_STOPPED_TITLE,
								},
								{
									"name" : "observer_stopped_name_box",
									"type" : "slotbar",

									"x" : 0,
									"y" : 20,

									"width" : 180,
									"height" : 18,

									"horizontal_align" : "center",

									"children" :
									(
										{
											"name" : "observer_stopped_name_edit",
											"type" : "editline",

											"x" : 3,
											"y" : 3,

											"width" : 170,
											"height" : 18,

											"input_limit" : CHARACTER_NAME_MAX_LEN,
										},
									),
								},
								{
									"name" : "observer_stopped_button",
									"type" : "button",

									"x" : 0,
									"y" : 20 + 18 + 5,

									"horizontal_align" : "center",

									"default_image" : "d:/ymir work/ui/public/XLarge_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/XLarge_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/XLarge_Button_03.sub",

									"text" : uiScriptLocale.ADMIN_MANAGER_OBSERVER_STOPPED_BUTTON,
								},
							),
						},
						# running window
						{
							"name" : "observer_running_wnd",

							"x" : 0,
							"y" : 0,

							"width" : BOARD_WIDTH - PAGE_X_DIST,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST),

							"children" :
							(
								# buttons
								{
									"name" : "observer_button_wnd",

									"x": 10,
									"y" : 10,

									"width" : 88,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

									"children" :
									(
										{
											"name" : "observer_navi_title",
											"type" : "text",

											"x" : 0,
											"y" : 0,

											"horizontal_align" : "center",
											"horizontal_align" : "center",

											"text" : uiScriptLocale.ADMIN_MANAGER_OBSERVER_NAVI_TITLE,
										},
										{
											"name" : "observer_navi_button_1",
											"type" : "radio_button",

											"x" : 0,
											"y" : 20 + (24 + 5) * 0,

											"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_OBSERVER_NAVI_BTN_1,
										},
										{
											"name" : "observer_navi_button_2",
											"type" : "radio_button",

											"x" : 0,
											"y" : 20 + (24 + 5) * 1,

											"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_OBSERVER_NAVI_BTN_2,
										},
										{
											"name" : "observer_navi_button_3",
											"type" : "radio_button",

											"x" : 0,
											"y" : 20 + (24 + 5) * 2,

											"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_OBSERVER_NAVI_BTN_3,
										},
										{
											"name" : "observer_navi_button_4",
											"type" : "radio_button",

											"x" : 0,
											"y" : 20 + (24 + 5) * 3,

											"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_OBSERVER_NAVI_BTN_4,
										},
										{
											"name" : "observer_stop_button",
											"type" : "button",

											"x" : 0,
											"y" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 24,

											"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_OBSERVER_STOP_BTN,
										},
									),
								},
								{
									"name" : "observer_navi_line",
									"type" : "line",

									"x" : 10 + 88 + 6,
									"y" : 10,

									"width" : 0,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

									"color" : grp.GenerateColor(1.0, 1.0, 1.0, 0.8),
								},
								{
									"name" : "observer_subpage_general",

									"x" : 10 + 88 + 13,
									"y" : 10,

									"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 88 + 13) - 10,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,
									
									"children" :
									(
										{
											"name" : "observer_general_face",
											"type" : "image",

											"x" : 4,
											"y" : 4,
										},
										{
											"name" : "observer_general_face_slot",
											"type" : "image",

											"x" : 0,
											"y" : 0,

											"image" : "d:/ymir work/ui/game/admin/box_face.tga",
										},
										{
											"name" : "observer_general_pid",
											"type" : "text",

											"x" : 60,
											"y" : 0,
										},
										{
											"name" : "observer_general_name",
											"type" : "text",

											"x" : 60,
											"y" : 17,
										},
										{
											"name" : "observer_general_level",
											"type" : "text",

											"x" : 60,
											"y" : 34,
										},
										{
											"name" : "observer_general_hpgauge_bg",
											"type" : "image",

											"x" : 0,
											"y" : 55,

											"image" : "d:/ymir work/ui/game/admin/gauge_bg.tga",

											"children" :
											(
												{
													"name" : "observer_general_hpgauge",
													"type" : "ani_image",

													"x" : 4,
													"y" : 0,

													"delay" : 6,

													"images" :
													(
														"d:/ymir work/ui/game/admin/HPGauge/01.tga",
														"d:/ymir work/ui/game/admin/HPGauge/02.tga",
														"d:/ymir work/ui/game/admin/HPGauge/03.tga",
														"d:/ymir work/ui/game/admin/HPGauge/04.tga",
														"d:/ymir work/ui/game/admin/HPGauge/05.tga",
														"d:/ymir work/ui/game/admin/HPGauge/06.tga",
														"d:/ymir work/ui/game/admin/HPGauge/07.tga",
													),
												},
												{
													"name" : "observer_general_hptext",
													"type" : "text",

													"x" : 0,
													"y" : 0,

													"all_align" : True,
													"outline" : True,
												},
											),
										},
										{
											"name" : "observer_general_spgauge_bg",
											"type" : "image",

											"x" : 0,
											"y" : 67,

											"image" : "d:/ymir work/ui/game/admin/gauge_bg.tga",

											"children" :
											(
												{
													"name" : "observer_general_spgauge",
													"type" : "ani_image",

													"x" : 4,
													"y" : 0,

													"delay" : 6,

													"images" :
													(
														"d:/ymir work/ui/game/admin/SPGauge/01.tga",
														"d:/ymir work/ui/game/admin/SPGauge/02.tga",
														"d:/ymir work/ui/game/admin/SPGauge/03.tga",
														"d:/ymir work/ui/game/admin/SPGauge/04.tga",
														"d:/ymir work/ui/game/admin/SPGauge/05.tga",
														"d:/ymir work/ui/game/admin/SPGauge/06.tga",
														"d:/ymir work/ui/game/admin/SPGauge/07.tga",
													),
												},
												{
													"name" : "observer_general_sptext",
													"type" : "text",

													"x" : 0,
													"y" : 0,

													"all_align" : True,
													"outline" : True,
												},
											),
										},
										{
											"name" : "observer_general_exp_bg",
											"type" : "image",

											"x" : 0,
											"y" : 83,

											"image" : "d:/ymir work/ui/game/admin/experience.tga",

											"children" :
											(
												{
													"name" : "observer_general_exp_1",
													"type" : "expanded_image",

													"x" : 3,
													"y" : 3,

													"image" : "d:/ymir work/ui/game/admin/exp_point.tga",
												},
												{
													"name" : "observer_general_exp_2",
													"type" : "expanded_image",

													"x" : 28,
													"y" : 3,

													"image" : "d:/ymir work/ui/game/admin/exp_point.tga",
												},
												{
													"name" : "observer_general_exp_3",
													"type" : "expanded_image",

													"x" : 53,
													"y" : 3,

													"image" : "d:/ymir work/ui/game/admin/exp_point.tga",
												},
												{
													"name" : "observer_general_exp_4",
													"type" : "expanded_image",

													"x" : 78,
													"y" : 3,

													"image" : "d:/ymir work/ui/game/admin/exp_point.tga",
												},
											),
										},
										{
											"name" : "observer_general_empire",
											"type" : "expanded_image",

											"x" : BOARD_WIDTH - PAGE_X_DIST - (10 + 88 + 13) - 10 - 64,
											"y" : 0,
										},
										{
											"name" : "observer_general_dividing_line1",
											"type" : "line",

											"x" : 0,
											"y" : 115,

											"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 88 + 13) - 10,
											"height" : 0,

											"color" : grp.GenerateColor(1.0, 1.0, 1.0, 0.8),
										},
										{
											"name" : "observer_general_account",
											"type" : "text",

											"x" : 0,
											"y" : 127,
										},
										{
											"name" : "observer_general_channel",
											"type" : "text",

											"x" : 0,
											"y" : 140,
										},
										{
											"name" : "observer_general_map_info",
											"type" : "text",

											"x" : 0,
											"y" : 157,
										},
										{
											"name" : "observer_general_gold",
											"type" : "text",

											"x" : 0,
											"y" : 174,
										},
										{
											"name" : "observer_general_skillgroup",
											"type" : "text",

											"x" : 0,
											"y" : 191,
										},
										{
											"name" : "observer_general_dividing_line2",
											"type" : "line",

											"x" : 234,
											"y" : 123,

											"width" : 0,
											"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 123,

											"horizontal_align" : "right",

											"color" : grp.GenerateColor(1.0, 1.0, 1.0, 0.8),
										},
										{
											"name" : "observer_general_skill_bg",
											"type" : "image",

											"x" : 227,
											"y" : 140,

											"horizontal_align" : "right",

											"image" : "d:/ymir work/ui/game/windows/skill_board.sub",

											"children" :
											(
												{
													"name" : "observer_general_skill_title",
													"type" : "text",

													"x" : 0,
													"y" : -17,

													"horizontal_align" : "center",
													"horizontal_align" : "center",

													"text" : uiScriptLocale.ADMIN_MANAGER_GENERAL_SKILL_TITLE,
												},
												{
													"name" : "observer_general_skill_slot",
													"type" : "slot",

													"x" : 3,
													"y" : 0,

													"width" : 223,
													"height" : 223,

													"image" : "d:/ymir work/ui/public/Slot_Base.sub",

													"slot" :
													(
														{"index": 1, "x": 1, "y":  4, "width":32, "height":32},
														{"index":21, "x":38, "y":  4, "width":32, "height":32},
														{"index":41, "x":75, "y":  4, "width":32, "height":32},

														{"index": 3, "x": 1, "y": 40, "width":32, "height":32},
														{"index":23, "x":38, "y": 40, "width":32, "height":32},
														{"index":43, "x":75, "y": 40, "width":32, "height":32},

														{"index": 5, "x": 1, "y": 76, "width":32, "height":32},
														{"index":25, "x":38, "y": 76, "width":32, "height":32},
														{"index":45, "x":75, "y": 76, "width":32, "height":32},

														{"index": 7, "x": 1, "y":112, "width":32, "height":32},
														{"index":27, "x":38, "y":112, "width":32, "height":32},
														{"index":47, "x":75, "y":112, "width":32, "height":32},

														####

														{"index": 2, "x":113, "y":  4, "width":32, "height":32},
														{"index":22, "x":150, "y":  4, "width":32, "height":32},
														{"index":42, "x":187, "y":  4, "width":32, "height":32},

														{"index": 4, "x":113, "y": 40, "width":32, "height":32},
														{"index":24, "x":150, "y": 40, "width":32, "height":32},
														{"index":44, "x":187, "y": 40, "width":32, "height":32},

														{"index": 6, "x":113, "y": 76, "width":32, "height":32},
														{"index":26, "x":150, "y": 76, "width":32, "height":32},
														{"index":46, "x":187, "y": 76, "width":32, "height":32},

														{"index": 8, "x":113, "y":112, "width":32, "height":32},
														{"index":28, "x":150, "y":112, "width":32, "height":32},
														{"index":48, "x":187, "y":112, "width":32, "height":32},
													),
												},
											),
										},
									),
								},
								{
									"name" : "observer_subpage_item",

									"x" : 10 + 88 + 13,
									"y" : 10,

									"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 88 + 13) - 10,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

									"children" :
									(
										{
											"name" : "observer_item_equipment_base",
											"type" : "image",

											"x" : 0,
											"y" : 0,

											"image" : "d:/ymir work/ui/game/admin/inventory/equipment_base.tga",

											"children" :
											(
												{
													"name" : "observer_item_equipment_slot",
													"type" : "slot",

													"x" : 3,
													"y" : 3,

													"width" : 174,
													"height" : 222,

													"slot" : (
														{"index":EQUIPMENT_START_INDEX+0, "x":47, "y":46, "width":32, "height":64},
														{"index":EQUIPMENT_START_INDEX+1, "x":47, "y":8, "width":32, "height":32},
														{"index":EQUIPMENT_START_INDEX+2, "x":47, "y":154, "width":32, "height":32},
														{"index":EQUIPMENT_START_INDEX+3, "x":85, "y":71, "width":32, "height":32},
														{"index":EQUIPMENT_START_INDEX+4, "x":9, "y":8, "width":32, "height":96},
														{"index":EQUIPMENT_START_INDEX+5, "x":121, "y":78, "width":32, "height":32},
														{"index":EQUIPMENT_START_INDEX+6, "x":121, "y":46, "width":32, "height":32},
														{"index":EQUIPMENT_START_INDEX+7, "x":121, "y":116, "width":32, "height":32},
														{"index":EQUIPMENT_START_INDEX+8, "x":121, "y":154, "width":32, "height":32},
														{"index":EQUIPMENT_START_INDEX+9, "x":121, "y":8, "width":32, "height":32},
														{"index":EQUIPMENT_START_INDEX+10, "x":85, "y":39, "width":32, "height":32},
														{"index":item.EQUIPMENT_RING1, "x":13, "y":148, "width":32, "height":32}, ## RING
														{"index":item.EQUIPMENT_RING2, "x":87, "y":148, "width":32, "height":32}, ##RING
														{"index":item.EQUIPMENT_BELT, "x":51, "y":126, "width":32, "height":32}, ##G?tel
														#{"index":item.EQUIPMENT_PET, "x":126, "y":148, "width":32, "height":32}, ##Pet
														#{"index":item.EQUIPMENT_PET+1, "x":126, "y":184, "width":32, "height":32}, ##Mount
													),
												},
											),
										},
										{
											"name" : "observer_item_inventory_tab_01",
											"type" : "radio_button",

											"x" : 7,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/xsmall_button_01.sub",
											"over_image" : "d:/ymir work/ui/public/xsmall_button_02.sub",
											"down_image" : "d:/ymir work/ui/public/xsmall_button_03.sub",

											"tooltip_text" : uiScriptLocale.INVENTORY_PAGE_BUTTON_TOOLTIP_1,

											"children" :
											(
												{
													"name" : "observer_item_inventory_tab_01_text",
													"type" : "text",

													"x" : 0,
													"y" : 0,

													"all_align" : "center",

													"r" : 1.0,
													"g" : 1.0,
													"b" : 1.0,

													"text" : "I",
												},
											),
										},
										{
											"name" : "observer_item_inventory_tab_02",
											"type" : "radio_button",

											"x" : 47,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/xsmall_button_01.sub",
											"over_image" : "d:/ymir work/ui/public/xsmall_button_02.sub",
											"down_image" : "d:/ymir work/ui/public/xsmall_button_03.sub",

											"tooltip_text" : uiScriptLocale.INVENTORY_PAGE_BUTTON_TOOLTIP_2,

											"children" :
											(
												{
													"name" : "observer_item_inventory_tab_02_text",
													"type" : "text",

													"x" : 0,
													"y" : 0,

													"all_align" : "center",

													"r" : 1.0,
													"g" : 1.0,
													"b" : 1.0,

													"text" : "II",
												},
											),
										},
										{
											"name" : "observer_item_inventory_tab_03",
											"type" : "radio_button",

											"x" : 87,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/xsmall_button_01.sub",
											"over_image" : "d:/ymir work/ui/public/xsmall_button_02.sub",
											"down_image" : "d:/ymir work/ui/public/xsmall_button_03.sub",

											"tooltip_text" : uiScriptLocale.INVENTORY_PAGE_BUTTON_TOOLTIP_3,

											"children" :
											(
												{
													"name" : "observer_item_inventory_tab_03_text",
													"type" : "text",

													"x" : 0,
													"y" : 0,

													"all_align" : "center",

													"r" : 1.0,
													"g" : 1.0,
													"b" : 1.0,

													"text" : "III",
												},
											),
										},
										{
											"name" : "observer_item_inventory_tab_04",
											"type" : "radio_button",

											"x" : 127,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/xsmall_button_01.sub",
											"over_image" : "d:/ymir work/ui/public/xsmall_button_02.sub",
											"down_image" : "d:/ymir work/ui/public/xsmall_button_03.sub",

											"tooltip_text" : uiScriptLocale.INVENTORY_PAGE_BUTTON_TOOLTIP_4,

											"children" :
											(
												{
													"name" : "observer_item_inventory_tab_04_text",
													"type" : "text",

													"x" : 0,
													"y" : 0,

													"all_align" : "center",

													"r" : 1.0,
													"g" : 1.0,
													"b" : 1.0,

													"text" : "VI",
												},
											),
										},

										{
											"name": "observer_item_inventory_tab_05",
											"type": "radio_button",

											"x": 167,
											"y": 0,

											"default_image": "d:/ymir work/ui/public/xsmall_button_01.sub",
											"over_image": "d:/ymir work/ui/public/xsmall_button_02.sub",
											"down_image": "d:/ymir work/ui/public/xsmall_button_03.sub",

											"tooltip_text": "Tab 5",

											"children":
												(
													{
														"name": "observer_item_inventory_tab_05_text",
														"type": "text",

														"x": 0,
														"y": 0,

														"all_align": "center",

														"r": 1.0,
														"g": 1.0,
														"b": 1.0,

														"text": "V",
													},
												),
										},

										{
											"name" : "observer_item_money_slot",
											"type" : "image",

											"x" : 12,
											"y" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 24,
											
											"image" : "d:/ymir work/ui/public/parameter_slot_05.sub",

											"children" :
											(
												{
													"name" : "observer_item_money_icon",
													"type" : "image",

													"x" : -16,
													"y" : 5,

													"image" : "d:/ymir work/ui/game/admin/inventory/money_icon.tga",
												},
												{
													"name" : "observer_item_money_text",
													"type" : "text",

													"x" : 10,
													"y" : 9,

													"horizontal_align" : "right",
													"horizontal_align" : "right",

													"r" : 1.0,
													"g" : 1.0 ,
													"b" : 1.0,

													"text" : "123456789",
												},
											),
										},
										{
											"name" : "observer_item_inventory_slot_background",
											"type" : "image",

											"x" : 170,
											"y" : 0,

											"image" : "d:/ymir work/ui/game/admin/inventory/slot_background.tga",
										},
										{
											"name" : "observer_item_inventory_slot",
											"type" : "grid_table",

											"x" : 172,
											"y" : 3,

											"start_index" : 0,
											"x_count" : 5,
											"y_count" : 9,
											"x_step" : 32,
											"y_step" : 32,

											"image" : "d:/ymir work/ui/public/Slot_Base.sub"
										},
									),
								},
								{
									"name" : "observer_subpage_whisper",

									"x" : 10 + 88 + 13,
									"y" : 10,

									"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 88 + 13) - 10,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

									"children" :
									(
										{
											"name" : "observer_whisper_name_title",
											"type" : "text",

											"x" : 50,
											"y" : 0,

											"horizontal_align" : "center",

											"text" : uiScriptLocale.ADMIN_MANAGER_OBSERVER_WHISPER_NAME_TITLE,
										},
										{
											"name" : "observer_whisper_name_list",
											"type" : "listbox",

											"x" : 0,
											"y" : 20,

											"width" : 100,
											"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 20,

											"border" : True,
										},
										{
											"name" : "observer_whisper_name_scroll",
											"type" : "scrollbar",

											"x" : 105,
											"y" : 20,

											"size" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 20,
										},
										{
											"name" : "observer_whisper_line",
											"type" : "line",

											"x" : 105 + 17 + 6,
											"y" : 0,

											"width" : 0,
											"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

											"color" : grp.GenerateColor(1.0, 1.0, 1.0, 0.8),
										},
										{
											"name" : "observer_whisper_text",
											"type" : "multi_text",

											"x" : 105 + 17 + 13,
											"y" : 0,

											"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 88 + 13) - 10 - (105 + 17 + 13) - 17 - 5,
											"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,
										},
										{
											"name" : "observer_whisper_text_scroll",
											"type" : "scrollbar",

											"x" : 17,
											"y" : 0,

											"horizontal_align" : "right",

											"size" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,
										},
									),
								},
								{
									"name" : "observer_subpage_ban",

									"x" : 10 + 88 + 13,
									"y" : 10,

									"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 88 + 13) - 10,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

									"children" :
									(
										{
											"name" : "observer_ban_chatban",

											"x" : 0,
											"y" : 0,

											"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 88 + 13) - 10,
											"height" : (BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2) / 2,

											"children" :
											(
												{
													"name" : "observer_chatban_title",
													"type" : "text",

													"x" : 0,
													"y" : 0,

													"text" : uiScriptLocale.ADMIN_MANAGER_OBSERVER_CHATBAN_TITLE,
												},
												{
													"name" : "observer_chatban_state",
													"type" : "text",

													"x" : 10,
													"y" : 20,

													"text" : "Status: nicht aktiv",
												},
												{
													"name" : "observer_chatban_day",
													"type" : "updownbutton",

													"x" : 10,
													"y" : 38,

													"max" : 365,
												},
												{
													"name" : "observer_chatban_day_desc",
													"type" : "text",

													"x" : 56,
													"y" : 38+16,

													"vertical_align" : "center",

													"text" : "Tage",
												},
												{
													"name" : "observer_chatban_hour",
													"type" : "updownbutton",

													"x" : 81,
													"y" : 38,

													"max" : 23,
												},
												{
													"name" : "observer_chatban_hour_desc",
													"type" : "text",

													"x" : 127,
													"y" : 38+16,

													"vertical_align" : "center",

													"text" : "Stunden",
												},
												{
													"name" : "observer_chatban_minute",
													"type" : "updownbutton",

													"x" : 169,
													"y" : 38,

													"max" : 59,
												},
												{
													"name" : "observer_chatban_minute_desc",
													"type" : "text",

													"x" : 215,
													"y" : 38+16,

													"vertical_align" : "center",

													"text" : "Minuten",
												},
												{
													"name" : "observer_chatban_activate",
													"type" : "button",

													"x" : 10,
													"y" : 82,

													"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
													"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
													"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

													"text" : "Sperre vergeben",
												},
												{
													"name" : "observer_chatban_deactivate",
													"type" : "button",

													"x" : 106,
													"y" : 82,

													"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
													"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
													"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",
													"disable_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

													"text" : "Sperre aufheben",
												},
											),
										},
										{
											"name" : "observer_ban_accountban",

											"x" : 0,
											"y" : (BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2) / 2,

											"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 88 + 13) - 10,
											"height" : (BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2) / 2,

											"children" :
											(
												{
													"name" : "observer_accountban_line",
													"type" : "line",

													"x" : 0,
													"y" : 0,

													"width" : BOARD_WIDTH - PAGE_X_DIST - (10 + 88 + 13) - 10,
													"height" : 0,

													"color" : grp.GenerateColor(1.0, 1.0, 1.0, 0.8),
												},
												{
													"name" : "observer_accountban_title",
													"type" : "text",

													"x" : 0,
													"y" : 7,

													"text" : uiScriptLocale.ADMIN_MANAGER_OBSERVER_ACCOUNTBAN_TITLE,
												},
												{
													"name" : "observer_accountban_state",
													"type" : "text",

													"x" : 10,
													"y" : 27,

													"text" : "Status: nicht aktiv",
												},
												{
													"name" : "observer_accountban_day",
													"type" : "updownbutton",

													"x" : 10,
													"y" : 45,

													"max" : 365,
												},
												{
													"name" : "observer_accountban_day_desc",
													"type" : "text",

													"x" : 56,
													"y" : 45+16,

													"vertical_align" : "center",

													"text" : "Tage",
												},
												{
													"name" : "observer_accountban_hour",
													"type" : "updownbutton",

													"x" : 81,
													"y" : 45,

													"max" : 23,
												},
												{
													"name" : "observer_accountban_hour_desc",
													"type" : "text",

													"x" : 127,
													"y" : 45+16,

													"vertical_align" : "center",

													"text" : "Stunden",
												},
												{
													"name" : "observer_accountban_minute",
													"type" : "updownbutton",

													"x" : 169,
													"y" : 45,

													"max" : 59,
												},
												{
													"name" : "observer_accountban_minute_desc",
													"type" : "text",

													"x" : 215,
													"y" : 45+16,

													"vertical_align" : "center",

													"text" : "Minuten",
												},
												{
													"name" : "observer_accountban_activate",
													"type" : "button",

													"x" : 10,
													"y" : 89,

													"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
													"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
													"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

													"text" : "Sperre vergeben",
												},
											),
										},
									),
								},
							),
						},
					),
				},

				## Page4 : Ban
				{
					"name" : "page_ban",
					"type" : "thinboard",

					"x" : PAGE_LEFT_X_DIST,
					"y" : PAGE_TOP_DIST,

					"width" : BOARD_WIDTH - PAGE_X_DIST,
					"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST),

					"children" :
					(
						{
							"name" : "ban_chat",

							"x" : 10,
							"y" : 10,

							"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

							"children" :
							(
								{
									"name" : "ban_chat_title",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"horizontal_align" : "center",
									"horizontal_align" : "center",

									"text" : uiScriptLocale.ADMIN_MANAGER_BAN_CHAT_TITLE,
								},
								{
									"name" : "ban_chat_search",

									"x" : 0,
									"y" : 20,

									"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5,
									"height" : 24,

									"children" :
									(
										{
											"name" : "ban_chat_search_box",
											"type" : "slotbar",

											"x" : 0,
											"y" : 0,

											"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5 - 61 - 5,
											"height" : 18,

											"children" :
											(
												{
													"name" : "ban_chat_search_edit",
													"type" : "editline",

													"x" : 3,
													"y" : 3,

													"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5 - 61 - 5 - 3 * 2,
													"height" : 18,

													"input_limit" : CHARACTER_NAME_MAX_LEN,
												},
											),
										},
										{
											"name" : "ban_chat_search_button",
											"type" : "button",

											"x" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5 - 61,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
											"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
											"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_CHAT_SEARCH_BUTTON,
										},
									),
								},
								{
									"name" : "ban_chat_normal_wnd",

									"x" : 0,
									"y" : 45,

									"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 45,

									"children" :
									(
										{
											"name" : "ban_chat_list",
											"type" : "listbox",

											"x" : 0,
											"y" : 0,

											"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5 - 17,
											"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 45 - 25 - 5,
										},
										{
											"name" : "ban_chat_scrollbar",
											"type" : "scrollbar",

											"x" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5 - 17,
											"y" : 0,

											"size" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 45 - 25 - 5,
										},
										{
											"name" : "ban_chat_unban_button",
											"type" : "button",

											"x" : 0,
											"y" : 25,

											"horizontal_align" : "center",
											"vertical_align" : "bottom",

											"default_image" : "d:/ymir work/ui/public/XLarge_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/XLarge_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/XLarge_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_CHAT_UNBAN_BUTTON,
										},
									),
								},
								{
									"name" : "ban_chat_search_result_fail_wnd",

									"x" : 0,
									"y" : 45,

									"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 45,

									"children" :
									(
										{
											"name" : "ban_chat_search_fail",
											"type" : "text",

											"x" : 0,
											"y" : 0,

											"all_align" : 1,

											"r" : 1.0,
											"g" : 110.0 / 255.0,
											"b" : 110.0 / 255.0,

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_CHAT_SEARCH_FAIL_TEXT,
										},
										{
											"name" : "ban_chat_search_fail_button",
											"type" : "button",

											"x" : 0,
											"y" : 25,

											"horizontal_align" : "center",
											"vertical_align" : "bottom",

											"default_image" : "d:/ymir work/ui/public/XLarge_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/XLarge_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/XLarge_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_CHAT_SEARCH_FAIL_BUTTON,
										},
									),
								},
								{
									"name" : "ban_chat_search_result_success_wnd",

									"x" : 0,
									"y" : 45,

									"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 45,

									"children" :
									(
										{
											"name" : "ban_chat_result_pid",
											"type" : "text",

											"x" : 0,
											"y" : 0,

											"horizontal_align" : "center",
											"horizontal_align" : "center",
										},
										{
											"name" : "ban_chat_result_name",
											"type" : "text",

											"x" : 0,
											"y" : 17,

											"horizontal_align" : "center",
											"horizontal_align" : "center",
										},
										{
											"name" : "ban_chat_result_face",
											"type" : "button",

											"x" : 0,
											"y" : 37,

											"horizontal_align" : "center",
										},
										{
											"name" : "ban_chat_result_chatban",
											"type" : "text",

											"x" : 0,
											"y" : 83,

											"horizontal_align" : "center",
											"horizontal_align" : "center",
										},
										{
											"name" : "ban_chat_result_accountban",
											"type" : "text",

											"x" : 0,
											"y" : 100,

											"horizontal_align" : "center",
											"horizontal_align" : "center",
										},
										{
											"name" : "ban_chat_result_ban_state",
											"type" : "text",

											"x" : 0,
											"y" : 117,

											"horizontal_align" : "center",
											"horizontal_align" : "center",
										},
										{
											"name" : "ban_chat_result_online_state",
											"type" : "extended_text",

											"x" : 0,
											"y" : 134,

											"horizontal_align" : "center",
										},
										{
											"name" : "ban_chat_result_time_wnd",

											"x" : 0,
											"y" : 92,

											"width" : 0,
											"height" : 38,

											"horizontal_align" : "center",
											"vertical_align" : "bottom",

											"children" :
											(
												{
													"name" : "ban_chat_result_day",
													"type" : "updownbutton",

													"x" : 0,
													"y" : 0,

													"vertical_align" : "center",

													"max" : 365,
												},
												{
													"name" : "ban_chat_result_day_desc",
													"type" : "text",

													"x" : 45,
													"y" : 0,

													"vertical_align" : "center",
													"vertical_align" : "center",

													"text" : "Tage",
												},
												{
													"name" : "ban_chat_result_hour",
													"type" : "updownbutton",

													"x" : 71,
													"y" : 0,

													"vertical_align" : "center",

													"max" : 23,
												},
												{
													"name" : "ban_chat_result_hour_desc",
													"type" : "text",

													"x" : 117,
													"y" : 0,

													"vertical_align" : "center",
													"vertical_align" : "center",

													"text" : "Std.",
												},
												{
													"name" : "ban_chat_result_minute",
													"type" : "updownbutton",

													"x" : 145,
													"y" : 0,

													"vertical_align" : "center",

													"max" : 59,
												},
												{
													"name" : "ban_chat_result_minute_desc",
													"type" : "text",

													"x" : 191,
													"y" : 0,

													"vertical_align" : "center",
													"vertical_align" : "center",

													"text" : "Min.",
												},
											),
										},
										{
											"name" : "ban_chat_result_ban_button",
											"type" : "button",

											"x" : ((BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5) / 2 - 88 - 2,
											"y" : 51,

											"vertical_align" : "bottom",

											"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_CHAT_RESULT_BAN_BUTTON,
										},
										{
											"name" : "ban_chat_result_unban_button",
											"type" : "button",

											"x" : ((BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5) / 2 + 2,
											"y" : 51,

											"vertical_align" : "bottom",

											"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",
											"disable_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_CHAT_RESULT_UNBAN_BUTTON,
										},
										{
											"name" : "ban_chat_result_back_button",
											"type" : "button",

											"x" : 0,
											"y" : 25,

											"horizontal_align" : "center",
											"vertical_align" : "bottom",

											"default_image" : "d:/ymir work/ui/public/XLarge_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/XLarge_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/XLarge_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_CHAT_RESULT_BACK_BUTTON,
										},
									),
								},
							),
						},
						{
							"name" : "ban_line",
							"type" : "line",

							"x" : (BOARD_WIDTH - PAGE_X_DIST) / 2,
							"y" : 10,

							"width" : 0,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

							"color" : grp.GenerateColor(1.0, 1.0, 1.0, 1.0),
						},
						{
							"name" : "ban_account",

							"x" : 10 + (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 + 5,
							"y" : 10,

							"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

							"children" :
							(
								{
									"name" : "ban_account_title",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"horizontal_align" : "center",
									"horizontal_align" : "center",

									"text" : uiScriptLocale.ADMIN_MANAGER_BAN_ACCOUNT_TITLE,
								},
								{
									"name" : "ban_account_search",

									"x" : 0,
									"y" : 20,

									"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5,
									"height" : 24,

									"children" :
									(
										{
											"name" : "ban_account_search_box",
											"type" : "slotbar",

											"x" : 0,
											"y" : 0,

											"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5 - 61 - 5 - 24 - 5,
											"height" : 18,

											"children" :
											(
												{
													"name" : "ban_account_search_edit",
													"type" : "editline",

													"x" : 3,
													"y" : 3,

													"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5 - 61 - 5 - 3 * 2,
													"height" : 18,

													"input_limit" : CHARACTER_NAME_MAX_LEN,
												},
											),
										},
										{
											"name" : "ban_account_search_type_button",
											"type" : "button",

											"x" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5 - 61 - 5 - 24,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/small_button_01.sub",
											"over_image" : "d:/ymir work/ui/public/small_button_02.sub",
											"down_image" : "d:/ymir work/ui/public/small_button_03.sub",
										},
										{
											"name" : "ban_account_search_button",
											"type" : "button",

											"x" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5 - 61,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
											"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
											"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_ACCOUNT_SEARCH_BUTTON,
										},
									),
								},
								{
									"name" : "ban_account_normal_wnd",

									"x" : 0,
									"y" : 45,

									"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 45,

									"children" :
									(
										{
											"name" : "ban_account_list",
											"type" : "listbox",

											"x" : 0,
											"y" : 0,

											"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5 - 17,
											"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 45 - 25 - 5,
										},
										{
											"name" : "ban_account_scrollbar",
											"type" : "scrollbar",

											"x" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5 - 17,
											"y" : 0,

											"size" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 45 - 25 - 5,
										},
										{
											"name" : "ban_account_unban_button",
											"type" : "button",

											"x" : 0,
											"y" : 25,

											"horizontal_align" : "center",
											"vertical_align" : "bottom",

											"default_image" : "d:/ymir work/ui/public/XLarge_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/XLarge_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/XLarge_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_ACCOUNT_UNBAN_BUTTON,
										},
									),
								},
								{
									"name" : "ban_account_search_result_fail_wnd",

									"x" : 0,
									"y" : 45,

									"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 45,

									"children" :
									(
										{
											"name" : "ban_account_search_fail_account",
											"type" : "text",

											"x" : 0,
											"y" : 0,

											"all_align" : 1,

											"r" : 1.0,
											"g" : 110.0 / 255.0,
											"b" : 110.0 / 255.0,

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_ACCOUNT_SEARCH_FAIL_TEXT_ACCOUNT,
										},
										{
											"name" : "ban_account_search_fail_player",
											"type" : "text",

											"x" : 0,
											"y" : 0,

											"all_align" : 1,

											"r" : 1.0,
											"g" : 110.0 / 255.0,
											"b" : 110.0 / 255.0,

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_ACCOUNT_SEARCH_FAIL_TEXT_PLAYER,
										},
										{
											"name" : "ban_account_search_fail_button",
											"type" : "button",

											"x" : 0,
											"y" : 25,

											"horizontal_align" : "center",
											"vertical_align" : "bottom",

											"default_image" : "d:/ymir work/ui/public/XLarge_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/XLarge_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/XLarge_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_ACCOUNT_SEARCH_FAIL_BUTTON,
										},
									),
								},
								{
									"name" : "ban_account_search_result_success_wnd",

									"x" : 0,
									"y" : 45,

									"width" : (BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 45,

									"children" :
									(
										{
											"name" : "ban_account_result_select1_btn",
											"type" : "button",

											"x" : (24 + 5) * 0,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/Small_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Small_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Small_Button_03.sub",

											"text" : "1",
										},
										{
											"name" : "ban_account_result_select2_btn",
											"type" : "button",

											"x" : (24 + 5) * 1,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/Small_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Small_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Small_Button_03.sub",

											"text" : "2",
										},
										{
											"name" : "ban_account_result_select3_btn",
											"type" : "button",

											"x" : (24 + 5) * 2,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/Small_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Small_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Small_Button_03.sub",

											"text" : "3",
										},
										{
											"name" : "ban_account_result_select4_btn",
											"type" : "button",

											"x" : (24 + 5) * 3,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/Small_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Small_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Small_Button_03.sub",

											"text" : "4",
										},
										{
											"name": "ban_account_result_select5_btn",
											"type": "button",

											"x": (24 + 5) * 4,
											"y": 0,

											"default_image": "d:/ymir work/ui/public/Small_Button_01.sub",
											"over_image": "d:/ymir work/ui/public/Small_Button_02.sub",
											"down_image": "d:/ymir work/ui/public/Small_Button_03.sub",

											"text": "4",
										},
										{
											"name" : "ban_account_result_account",
											"type" : "text",

											"x" : 0,
											"y" : 0,

											"horizontal_align" : "center",
											"horizontal_align" : "center",
										},
										{
											"name" : "ban_account_result_player",
											"type" : "text",

											"x" : 0,
											"y" : 17,

											"horizontal_align" : "center",
											"horizontal_align" : "center",
										},
										{
											"name" : "ban_account_result_face",
											"type" : "button",

											"x" : 0,
											"y" : 37,

											"horizontal_align" : "center",
										},
										{
											"name" : "ban_account_result_chatban",
											"type" : "text",

											"x" : 0,
											"y" : 83,

											"horizontal_align" : "center",
											"horizontal_align" : "center",
										},
										{
											"name" : "ban_account_result_accountban",
											"type" : "text",

											"x" : 0,
											"y" : 100,

											"horizontal_align" : "center",
											"horizontal_align" : "center",
										},
										{
											"name" : "ban_account_result_ban_state",
											"type" : "text",

											"x" : 0,
											"y" : 117,

											"horizontal_align" : "center",
											"horizontal_align" : "center",
										},
										{
											"name" : "ban_account_result_online_state",
											"type" : "extended_text",

											"x" : 0,
											"y" : 134,

											"horizontal_align" : "center",
										},
										{
											"name" : "ban_account_result_time_wnd",

											"x" : 0,
											"y" : 92,

											"width" : 0,
											"height" : 38,

											"horizontal_align" : "center",
											"vertical_align" : "bottom",

											"children" :
											(
												{
													"name" : "ban_account_result_day",
													"type" : "updownbutton",

													"x" : 0,
													"y" : 0,

													"vertical_align" : "center",

													"max" : 365,
												},
												{
													"name" : "ban_account_result_day_desc",
													"type" : "text",

													"x" : 45,
													"y" : 0,

													"vertical_align" : "center",
													"vertical_align" : "center",

													"text" : "Tage",
												},
												{
													"name" : "ban_account_result_hour",
													"type" : "updownbutton",

													"x" : 71,
													"y" : 0,

													"vertical_align" : "center",

													"max" : 23,
												},
												{
													"name" : "ban_account_result_hour_desc",
													"type" : "text",

													"x" : 117,
													"y" : 0,

													"vertical_align" : "center",
													"vertical_align" : "center",

													"text" : "Std.",
												},
												{
													"name" : "ban_account_result_minute",
													"type" : "updownbutton",

													"x" : 145,
													"y" : 0,

													"vertical_align" : "center",

													"max" : 59,
												},
												{
													"name" : "ban_account_result_minute_desc",
													"type" : "text",

													"x" : 191,
													"y" : 0,

													"vertical_align" : "center",
													"vertical_align" : "center",

													"text" : "Min.",
												},
											),
										},
										{
											"name" : "ban_account_result_ban_button",
											"type" : "button",

											"x" : ((BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5) / 2 - 88 - 2,
											"y" : 51,

											"vertical_align" : "bottom",

											"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_ACCOUNT_RESULT_BAN_BUTTON,
										},
										{
											"name" : "ban_account_result_unban_button",
											"type" : "button",

											"x" : ((BOARD_WIDTH - PAGE_X_DIST - 10 * 2) / 2 - 5) / 2 + 2,
											"y" : 51,

											"vertical_align" : "bottom",

											"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",
											"disable_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_ACCOUNT_RESULT_UNBAN_BUTTON,
										},
										{
											"name" : "ban_account_result_back_button",
											"type" : "button",

											"x" : 0,
											"y" : 25,

											"horizontal_align" : "center",
											"vertical_align" : "bottom",

											"default_image" : "d:/ymir work/ui/public/XLarge_Button_01.sub",
											"over_image" : "d:/ymir work/ui/public/XLarge_Button_02.sub",
											"down_image" : "d:/ymir work/ui/public/XLarge_Button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_BAN_ACCOUNT_RESULT_BACK_BUTTON,
										},
									),
								},
							),
						},
						{
							"name" : "ban_log",

							"x" : 10,
							"y" : 10,

							"width" : BOARD_WIDTH - PAGE_X_DIST - 10 * 2,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

							"children" :
							(
								{
									"name" : "ban_log_title",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"horizontal_align" : "center",
									"horizontal_align" : "center",
								},
								{
									"name" : "ban_log_nodata",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : 1,

									"text" : uiScriptLocale.ADMIN_MANAGER_BAN_LOG_NODATA,
								},
								{
									"name" : "ban_log_data",
									"type" : "table",

									"x" : 0,
									"y" : 20,

									"width" : BOARD_WIDTH - PAGE_X_DIST - 10 * 2 - 13,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 20 - 30,

									"col_size" : [20, 20, 10, 25, 30, 30, 25],
									"col_length_check" : [4, 5],

									"header" : [uiScriptLocale.ADMIN_MANAGER_BAN_LOG_DATA_TITLE_NAME,\
										uiScriptLocale.ADMIN_MANAGER_BAN_LOG_DATA_TITLE_GM_NAME, uiScriptLocale.ADMIN_MANAGER_BAN_LOG_DATA_TITLE_TYPE,\
										uiScriptLocale.ADMIN_MANAGER_BAN_LOG_DATA_TITLE_DURATION_NEW, uiScriptLocale.ADMIN_MANAGER_BAN_LOG_DATA_TITLE_REASON,\
										uiScriptLocale.ADMIN_MANAGER_BAN_LOG_DATA_TITLE_PROOF,uiScriptLocale.ADMIN_MANAGER_BAN_LOG_DATA_TITLE_DATE],
								},
								{
									"name" : "ban_log_data_scroll",
									"type" : "scrollbar",

									"x" : BOARD_WIDTH - PAGE_X_DIST - 10 * 2 - 13,
									"y" : 20,

									"size" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - 20 - 30,
								},
								{
									"name" : "ban_log_button",
									"type" : "button",

									"x" : 0,
									"y" : 24,

									"horizontal_align" : "center",
									"vertical_align" : "bottom",

									"default_image" : "d:/ymir work/ui/public/XLarge_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/XLarge_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/XLarge_Button_03.sub",

									"text" : uiScriptLocale.ADMIN_MANAGER_BAN_LOG_BUTTON,
								},
							),
						},
					),
				},

				## Page5 : Item
				{
					"name" : "page_item",
					"type" : "thinboard",

					"x" : PAGE_LEFT_X_DIST,
					"y" : PAGE_TOP_DIST,

					"width" : BOARD_WIDTH - PAGE_X_DIST,
					"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST),

					"children" :
					(
						{
							"name" : "item_main",

							"x" : 10,
							"y" : 10,

							"width" : BOARD_WIDTH - PAGE_X_DIST - 10 * 2,
							"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2,

							"children" :
							(
								{
									"name" : "item_main_title",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"horizontal_align" : "center",
									"horizontal_align" : "center",

									"text" : uiScriptLocale.ADMIN_MANAGER_ITEM_MAIN_TITLE,
								},
								{
									"name" : "item_main_search",

									"x" : 0,
									"y" : 20,

									"width" : BOARD_WIDTH - PAGE_X_DIST - 10 * 2,
									"height" : 24,

									"children" :
									(
										{
											"name" : "item_main_search_box",
											"type" : "slotbar",

											"x" : 0,
											"y" : 0,

											"width" : BOARD_WIDTH - PAGE_X_DIST - 10 * 2 - 61 - 5 - 24 - 5,
											"height" : 18,

											"children" :
											(
												{
													"name" : "item_main_search_edit",
													"type" : "editline",

													"x" : 3,
													"y" : 3,

													"width" : BOARD_WIDTH - PAGE_X_DIST - 10 * 2 - 61 - 5 - 3 * 2,
													"height" : 18,

													"input_limit" : CHARACTER_NAME_MAX_LEN,
												},
											),
										},
										{
											"name" : "item_main_search_type_button",
											"type" : "button",

											"x" : BOARD_WIDTH - PAGE_X_DIST - 10 * 2 - 61 - 5 - 24,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/small_button_01.sub",
											"over_image" : "d:/ymir work/ui/public/small_button_02.sub",
											"down_image" : "d:/ymir work/ui/public/small_button_03.sub",
										},
										{
											"name" : "item_main_search_button",
											"type" : "button",

											"x" : BOARD_WIDTH - PAGE_X_DIST - 10 * 2 - 61,
											"y" : 0,

											"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
											"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
											"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",

											"text" : uiScriptLocale.ADMIN_MANAGER_ITEM_MAIN_SEARCH_BUTTON,
										},
									),
								},
								{
									"name" : "item_main_table",
									"type" : "table",

									"x" : 0,
									"y" : 20 + 24 + 5,

									"width" : BOARD_WIDTH - PAGE_X_DIST - 10 * 2 - 13,
									"height" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - (20 + 24 + 5),

									"col_size" : [15, 10, 25, 10, 25, 15, 10, 10],
									"header" : [uiScriptLocale.ADMIN_MANAGER_ITEM_MAIN_TABLE_OTYPE,\
												uiScriptLocale.ADMIN_MANAGER_ITEM_MAIN_TABLE_OID,\
												uiScriptLocale.ADMIN_MANAGER_ITEM_MAIN_TABLE_ONAME,\
												uiScriptLocale.ADMIN_MANAGER_ITEM_MAIN_TABLE_IID,\
												uiScriptLocale.ADMIN_MANAGER_ITEM_MAIN_TABLE_INAME,\
												uiScriptLocale.ADMIN_MANAGER_ITEM_MAIN_TABLE_IWINDOW,\
												uiScriptLocale.ADMIN_MANAGER_ITEM_MAIN_TABLE_IPOS,\
												uiScriptLocale.ADMIN_MANAGER_ITEM_MAIN_TABLE_IGMITEM],
								},
								{
									"name" : "item_main_scroll",
									"type" : "scrollbar",

									"x" : BOARD_WIDTH - PAGE_X_DIST - 10 * 2 - 13,
									"y" : 20 + 24 + 5,

									"size" : BOARD_HEIGHT - (PAGE_TOP_DIST + PAGE_BOT_DIST) - 10 * 2 - (20 + 24 + 5),
								},
							),
						},
					)
				},

				# Navigation Buttons
				{
					"name" : "navi_button_1",
					"type" : "radio_button",

					"x" : BOARD_WIDTH / (NAVI_BTN_COUNT + 1) * 1 - NAVI_BTN_WIDTH / 2,
					"y" : BOARD_HEIGHT - (PAGE_BOT_DIST + NAVI_BTN_HEIGHT) / 2 - NAVI_BOT_DIST,

					"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

					"text" : uiScriptLocale.ADMIN_MANAGER_NAVI_BTN_1,
				},
				{
					"name" : "navi_button_2",
					"type" : "radio_button",

					"x" : BOARD_WIDTH / (NAVI_BTN_COUNT + 1) * 2 - NAVI_BTN_WIDTH / 2,
					"y" : BOARD_HEIGHT - (PAGE_BOT_DIST + NAVI_BTN_HEIGHT) / 2 - NAVI_BOT_DIST,

					"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

					"text" : uiScriptLocale.ADMIN_MANAGER_NAVI_BTN_2,
				},
				{
					"name" : "navi_button_3",
					"type" : "radio_button",

					"x" : BOARD_WIDTH / (NAVI_BTN_COUNT + 1) * 3 - NAVI_BTN_WIDTH / 2,
					"y" : BOARD_HEIGHT - (PAGE_BOT_DIST + NAVI_BTN_HEIGHT) / 2 - NAVI_BOT_DIST,

					"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

					"text" : uiScriptLocale.ADMIN_MANAGER_NAVI_BTN_3,
				},
				{
					"name" : "navi_button_4",
					"type" : "radio_button",

					"x" : BOARD_WIDTH / (NAVI_BTN_COUNT + 1) * 4 - NAVI_BTN_WIDTH / 2,
					"y" : BOARD_HEIGHT - (PAGE_BOT_DIST + NAVI_BTN_HEIGHT) / 2 - NAVI_BOT_DIST,

					"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

					"text" : uiScriptLocale.ADMIN_MANAGER_NAVI_BTN_4,
				},
				{
					"name" : "navi_button_5",
					"type" : "radio_button",

					"x" : BOARD_WIDTH / (NAVI_BTN_COUNT + 1) * 5 - NAVI_BTN_WIDTH / 2,
					"y" : BOARD_HEIGHT - (PAGE_BOT_DIST + NAVI_BTN_HEIGHT) / 2 - NAVI_BOT_DIST,

					"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",

					"text" : uiScriptLocale.ADMIN_MANAGER_NAVI_BTN_5,
				},
			),
		},
	),
}
