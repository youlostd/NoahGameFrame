import uiScriptLocale

ROOT = "d:/ymir work/ui/game/"

window = {
	"name" : "EnergyBar",

	"x" : 0,
	"y" : SCREEN_HEIGHT - 65,

	"width" : 68,
	"height" : 40,

	"children" :
	(
		# ±â·Â
		{
			"name" : "EnergyGauge_Board",
			"type" : "image",

			"x" : 0,
			"y" : 0,

			"image" : "D:/Ymir Work/UI/Pattern/EnergyGauge/energygauge_base.tga",

			"children" :
			(
				{
					"name" : "EnergyGauge_Empty",
					"type" : "expanded_image",

					"x" : 5,
					"y" : 8,

					"image" : "D:/Ymir Work/UI/Pattern/EnergyGauge/gauge_empty.tga",
				},
				{
					"name" : "EnergyGauge_Hungry",
					"type" : "expanded_image",

					"x" : 5,
					"y" : 8,

					"image" : "D:/Ymir Work/UI/Pattern/EnergyGauge/gauge_hungry.tga",
				},
				{
					"name" : "EnergyGauge_Full",
					"type" : "expanded_image",

					"x" : 5,
					"y" : 8,

					"image" : "D:/Ymir Work/UI/Pattern/EnergyGauge/gauge_full.tga",
				},
			),
		},
		{
			"name" : "EnergyGauge_ToolTip",

			"x" : 0,
			"y" : 0,
			
			"width"  : 68,
			"height" : 40,
			"type" : "window",
		},		
	),
}

