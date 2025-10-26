ROOT = "d:/ymir work/ui/game/"

BOARD_ADD_X = 10
# if app.ENABLE_GEM_SYSTEM:
BOARD_ADD_X += 60

BOARD_X = SCREEN_WIDTH - (140 + BOARD_ADD_X)
BOARD_WIDTH = (140 + BOARD_ADD_X)
BOARD_HEIGHT = 50

window = {
	"name" : "ExpandedMoneyTaskbar",
	"x" : BOARD_X -50,
	"y" : SCREEN_HEIGHT - 70,
	"width" : BOARD_WIDTH +50,
	"height" : BOARD_HEIGHT,
	"style" : ("float","animated_board"),
	"children" :
	[
		{
			"name" : "ExpanedMoneyTaskBar_Board",
			"type" : "board",
			"x" : 0,
			"y" : 0,
			"width" : BOARD_WIDTH +50,
			"height" : BOARD_HEIGHT,
			"children" :
			[
				## Print
				{
					"name":"Money_Icon",
					"type":"image",
					"x":18 + BOARD_ADD_X +8,
					"y":11+2,
					"image":"d:/ymir work/ui/game/windows/money_icon.sub",
				},
				{
					"name":"Money_Slot",
					"type":"button",
					"x":39 + BOARD_ADD_X +5,
					"y":10+2,
					#"horizontal_align":"center",
					"default_image" : "d:/ymir work/ui/public/parameter_slot_05.sub",
					"over_image" : "d:/ymir work/ui/public/parameter_slot_05.sub",
					"down_image" : "d:/ymir work/ui/public/parameter_slot_05.sub",
					"children" :
					(
						{
							"name" : "Money",
							"type" : "text",
							"x" : 3,
							"y" : 3,
							"horizontal_align" : "right",
							"text_horizontal_align" : "right",
							"color" : 0xffd1cdcd,
						},
					),
				},
			],
		},		
	],
}
				
# if app.ENABLE_GEM_SYSTEM:
window["children"][0]["children"] = window["children"][0]["children"] + [
				{
					"name":"Cheque_Icon",
					"type":"image",
					"x":BOARD_ADD_X - 57,
					"y":13+1,
					"image": "d:/ymir work/ui/game/windows/cheque_icon.sub",
				},					
				{
					"name":"Cheque_Slot",
					"type":"image",
					"x": BOARD_ADD_X - 38,
					"y":10+2,
					"image" : "d:/ymir work/ui/public/Parameter_Slot_02.sub",
					"children" :
					(
						{
							"name" : "Cheque",
							"type" : "text",
							"x" : 3,
							"y" : 3,
							"horizontal_align" : "right",
							"text_horizontal_align" : "right",
							"color" : 0xffd1cdcd,
						},
					),
				},		
				]