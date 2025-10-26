import uiScriptLocale
import localeInfo

window = {
	"name" : "NationWarDialog",

	"x" : 70,
	"y" : SCREEN_HEIGHT - 150,

	"style" : ("float",),

	"width" : 165,
	"height" : 110,

	"children" :
	(
		{
			"name" : "board",
			"type" : "thinboard",

			"x" : 0,
			"y" : 0,

			"width" : 165,
			"height" : 110,

			"r" : 0.3333,
			"g" : 0.2941,
			"b" : 0.2588,
			"a" : 1.0,
			
			"children" :
			(
				{
					"name" : "empire1",
					"type" : "expanded_image",

					"x" : 25,
					"y" : 20,
					
					"x_scale" : 0.20,
					"y_scale" : 0.20,

					"image" : "d:/ymir work/ui/intro/empire/empireflag_a.sub",
				},
				{
					"name" : "empiretext1",
					"type" : "text",

					"x" : 60,
					"y" : 20,
					"text" : localeInfo.EMPIRE_A
				},
				{
					"name" : "empirevalue1",
					"type" : "text",

					"x" : 140,
					"y" : 20,
					"text" : "0"
				},
				{
					"name" : "empire2",
					"type" : "expanded_image",

					"x" : 25,
					"y" : 40,
					
					"x_scale" : 0.20,
					"y_scale" : 0.20,

					"image" : "d:/ymir work/ui/intro/empire/empireflag_b.sub",
				},
				{
					"name" : "empiretext2",
					"type" : "text",

					"x" : 60,
					"y" : 40,
					"text" : localeInfo.EMPIRE_B
				},
				{
					"name" : "empirevalue2",
					"type" : "text",

					"x" : 140,
					"y" : 40,
					"text" : "0",
				},
				{
					"name" : "empire3",
					"type" : "expanded_image",

					"x" : 25,
					"y" : 60,
					
					"x_scale" : 0.20,
					"y_scale" : 0.20,

					"image" : "d:/ymir work/ui/intro/empire/empireflag_c.sub",
				},
				{
					"name" : "empiretext3",
					"type" : "text",

					"x" : 60,
					"y" : 60,
					"text" : localeInfo.EMPIRE_C
				},
				{
					"name" : "empirevalue3",
					"type" : "text",

					"x" : 140,
					"y" : 60,
					"text" : "0",
					#"horizontal_align" : "center",
					#"horizontal_align" : "center",
				},
				{
					"name" : "live",
					"type" : "text",

					"x" : 60,
					"y" : 80,
					"text" : localeInfo.WAR_DIALOG_LIVES
				},
				{
					"name" : "livevalue",
					"type" : "text",

					"x" : 120,
					"y" : 80,
					"text" : "50"
				},
			),
		},
	),
}
