import uiScriptLocale
import app

ROOT_PATH = "d:/ymir work/ui/public/"
SMALL_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_02.sub"
MIDDLE_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_03.sub"
window = {
	"name" : "BuildGuildBuildingChangeWindow",
	"style" : ("moveable", "float",),

	"x" : 10,
	"y" : SCREEN_HEIGHT/2-100,

	"width" : 215,
	"height" : 340,

	"children" :
	(
		{
			"name" : "Board",
			"type" : "board_with_titlebar",

			"x" : 0,
			"y" : 0,

			"width" : 215,
			"height" : 340,

			"title" : uiScriptLocale.GUILD_BUILDING_CHANGE_FIX,

			"children" :
			(
				{
					"name" : "temp_window",
					"type" : "window",
					"style" : ("not_pick",),

					"x" : 0,
					"y" : 0,
					"width" : 215,
					"height" : 340,

					"children" :
					(

						### START_TEMP

						{
							"name" : "BuildingListBar",
							"type" : "slotbar",

							"x" : 15,
							"y" : 50,

							"width" : 180,
							"height" : 172,

							"children" :
							(
								{
									"name" : "ChangeBuildingList",
									"type" : "listbox",

									"x" : 0,
									"y" : 1,

									"width" : 165,
									"height" : 170,
								},
								{
									"name" : "ChangeListScrollBar",
									"type" : "scrollbar",

									"x" : 15,
									"y" : 2,
									"size" : 172-2,
									"horizontal_align" : "right",
								},
							),
						},
						{
							"name" : "PositionButton",
							"type" : "button", 
							"x" : 25,
							"y" : 260, 
							"text" : uiScriptLocale.GUILD_BUILDING_CHANGE,
                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
						},
						{
							"name" : "DeleteButton",
							"type" : "button", 
							"x" : 115, 
							"y" : 260, 
							"text" : uiScriptLocale.GUILD_BUILDING_CHANGE_DELETE,
                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
						},
						{
							"name" : "BuildingRotationZTitle",
							"type" : "text", "x" : 15, "y" : 240, "text" : "Z",
							"text_horizontal_align" : "center",
						},
						{
							"name" : "BuildingRotationZ",
							"type" : "sliderbar",
							"x" : 23, "y" : 240,
						},
						{
							"name" : "AcceptButton",
							"type" : "button",

							"x" : 25,
							"y" : 295,

							"text" : uiScriptLocale.ACCEPT,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
						},
						{
							"name" : "CancelButton",
							"type" : "button",

							"x" : 115,
							"y" : 295,

							"text" : uiScriptLocale.CANCEL,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 1.0,
						},
					),
				},
			),
		},
	),
}