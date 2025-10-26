import uiScriptLocale
window = {
	"name" : "camModuleInterface",
	"style" : ("moveable", "float",),
	
	"x" : (SCREEN_WIDTH / 2) - 125,
	"y" : (SCREEN_HEIGHT / 2) - 50,
	"width" : 250,
	"height" : 100,
	
	"children" :
	(
		{
			"name" : "board",
			"type" : "board_with_titlebar",
			"style" : ("attach",),
			
			"x" : 0,
			"y" : 0,
			"width" : 250,
			"height" : 100,
			
			"title" : "",
			
			"children" :
			(
				{
					"name" : "imgScriptNameSlot",
					"type" : "image",
					
					"x" : 10,
					"y" : 35,
					
					"image" : "d:/ymir work/ui/public/Parameter_Slot_06.sub",
					
					"children" :
					(
						{
							"name" : "editlineScriptName",
							"type" : "editline",
							
							"x" : 3,
							"y" : 2,
							"width" : 214,
							"height" : 14,
							
							"input_limit" : 24,
							"text" : "",
						},
					),
				},
				{
					"name" : "btnDone",
					"type" : "button",
					
					"x" : 142,
					"y" : 60,
					
					"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",
					"text" : "",
				},
			),
		},
	),
}