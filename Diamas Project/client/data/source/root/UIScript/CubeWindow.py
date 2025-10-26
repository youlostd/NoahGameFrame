import localeInfo
import uiScriptLocale

ICON_SLOT_FILE = "d:/ymir work/ui/public/Slot_Base.sub"

window = {
	"name" : "CubeWindow",

#	"x" : 430,
#	"y" : 130,
	"x" : SCREEN_WIDTH - 176 - 200 - 80,
	"y" : SCREEN_HEIGHT - 37 - 563,


	"style" : ("moveable", "float",),

	"width" : 285,
	"height" : 571,

	"children" :
	(
		{
			"name" : "board",
			"type" : "expanded_image",
			"style" : ("attach",),

			"x" : 0,
			"y" : 0,

			"width"		: 285,
			"height"	: 571,

			"image"		: "d:/ymir work/ui/new_cube_bg.tga",

			"children" :
			(
				## Title
				{
					"name" : "TitleBar",
					"type" : "titlebar",
					"style" : ("attach",),

					"x" : 12,
					"y" : -4,

					"width" : 262,
					"color" : "yellow",
                    "title" : uiScriptLocale.CUBE_TITLE,


				},

				## Cube Slot
				{
					"name" : "CubeSlot",
					"type" : "grid_table",

					"x" : 14,
					"y" : 364,

					"start_index" : 0,
					"x_count" : 8,
					"y_count" : 3,
					"x_step" : 32,
					"y_step" : 32,

					"image" : "d:/ymir work/ui/public/Slot_Base.sub"
				},
				## Print
				{
					"name" : "NeedMoney",
					"type" : "text",

					"x" : 20,
					"y" : 468,

					"width" : 200,

					"horizontal_align" : "right",
					"horizontal_align" : "right",

					"text" : localeInfo.NumberToMoneyString(0),
				},

				{
					"name" : "contentScrollbar",
					"type" : "thin_scrollbar",

					"x" : 253,
					"y" : 38,

					"size" : 315,
				},

				# Cube Result List. 1 ~ 3
				{
					"name" : "result1board",
					"type" : "window",
					
					"x" : 25,
					"y" : 41,
					
					"width" : 216,
					"height" : 64,

					"children" : 
					(
						{
							"name" : "result1", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 0,
							"y" : 0,
						},
						{
							"name" : "material11", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 57,
							"y" : 0,
						},
						{
							"name" : "material12", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 90,
							"y" : 0,
						},
						{
							"name" : "material13", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 123, 
							"y" : 0,
						},
						{
							"name" : "material14", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 156, 
							"y" : 0,
						},
						{
							"name" : "material15", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 189, 
							"y" : 0,
						},
					),

				},
				{
					"name" : "result2board",
					"type" : "window",
					
					"x" : 25,
					"y" : 147,
					
					"width" : 216,
					"height" : 64,

					"children" : 
					(
						{
							"name" : "result2", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 0,
							"y" : 0,
						},
						{
							"name" : "material21", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 57,
							"y" : 0,
						},
						{
							"name" : "material22", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 90,
							"y" : 0,
						},
						{
							"name" : "material23", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 123, 
							"y" : 0,
						},
						{
							"name" : "material24", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 156, 
							"y" : 0,
						},
						{
							"name" : "material25", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 189, 
							"y" : 0,
						},
					),
				},
				{
					"name" : "result3board",
					"type" : "window",
					
					"x" : 25,
					"y" : 253,
					
					"width" : 216,
					"height" : 64,

					"children" : 
					(
						{
							"name" : "result3", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 0,
							"y" : 0,
						},
						{
							"name" : "material31", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 57,
							"y" : 0,
						},
						{
							"name" : "material32", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 90,
							"y" : 0,
						},
						{
							"name" : "material33", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 123, 
							"y" : 0,
						},
						{
							"name" : "material34", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 156, 
							"y" : 0,
						},
						{
							"name" : "material35", 
							"type" : "grid_table",
							"start_index" : 0,
						
							"x_count" : 1,
							"y_count" : 3,
							"x_step" : 32,
							"y_step" : 32,

							"x" : 189, 
							"y" : 0,
						},
					),
				},


				## Button
				{
					"name" : "AcceptButton",
					"type" : "button",

					"x" : 64,
					"y" : 30,
					"vertical_align" : "bottom",

					"text" : uiScriptLocale.OK,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 0.8,
				},
				{
					"name" : "CancelButton",
					"type" : "button",

					"x" : 161,
					"y" : 30,
					"vertical_align" : "bottom",

					"text" : uiScriptLocale.CANCEL,

                    "text_height": -5,
                    "text_color": 0xffffc539,
                    'default_image': 'd:/ymir work/ui/gui/normal_button.sub',
                    'over_image': 'd:/ymir work/ui/gui/normal_button_hover.sub',
                    'down_image': 'd:/ymir work/ui/gui/normal_button_down.sub',
                    'x_scale': 0.4,
                    'y_scale': 0.8,
				},
				
			),
		},
	),
}
