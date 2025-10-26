import uiScriptLocale
import grp
import localeInfo

BOARD_X = 623
BOARD_Y = 200
COLOR_LINE = 0xff5b5e5e

window = {
	"name" : "WhisperManager",
	"x" : 0,
	"y" : 0,
	"style" : ("moveable", "float",),
	"width" : BOARD_X + 65,
	"height" : BOARD_Y,
	"children" :
	(
		{
			"name" : "ColorBoard",
			"type" : "board",
			"style" : ("attach",),
			"x" : BOARD_X-30,
			"y" : 10,
			"vertical_align":"center",
			"width" : 60,
			"height" : 133,
		},
				
		{
			"name" : "board",
			"type" : "board",
			"x" : 0,
			"y" : 0,
			"width" : BOARD_X,
			"height" : BOARD_Y,
			"children" :
			(
				{
					"name" : "titlebar",
					"type" : "titlebar",
					"style" : ("attach",),

					"x" : 8,
					"y" : 0,
 
					"width" : BOARD_X - 13,
					"color" : "yellow",
                    "title" : uiScriptLocale.WHISPER_ADMIN_TITLE_NAME,

				},

				{
					"name" : "bg1",
					"type" : "bar",
					"x" : 9,
					"y" : 32,
					"width" : BOARD_X - 19,
					"height" : BOARD_Y - 42,
					"color" : grp.GenerateColor(0.0, 0.0, 0.0, 0.5),
				},
				
				{
					"name" : "bg2",
					"type" : "bar",
					"x" : 9,
					"y" : 95,
					"width" : BOARD_X-20,
					"height" : BOARD_Y-133,
					"color" : grp.GenerateColor(0.0, 0.0, 0.0, 0.9),
				},
				
				{ "name" : "LINE_BGCREATE_LEFT", "type" : "line", "x" : 8, "y" : 30, "width" : 0, "height" : BOARD_Y-40, "color" : COLOR_LINE, },

				{ "name" : "LINE_BGCREATE_RIGHT", "type" : "line", "x" : BOARD_X-10, "y" : 30, "width" : 0, "height" : BOARD_Y-40, "color" : COLOR_LINE, },

				{ "name" : "LINE_BGCREATE_DOWN", "type" : "line", "x" : 8, "y" : BOARD_Y-10, "width" : BOARD_X-17, "height" : 0, "color" : COLOR_LINE, },

				{ "name" : "LINE_BGCREATE_UP", "type" : "line", "x" : 8, "y" : 30, "width" : BOARD_X-17, "height" : 0, "color" : COLOR_LINE, },		
				
				{ "name" : "LINE_FLAG_DOWN", "type" : "line", "x" : 8, "y" : 49, "width" : BOARD_X-17, "height" : 0, "color" : COLOR_LINE, },	
				
				{ "name" : "LINE_SELECT_DOWN", "type" : "line", "x" : 8, "y" : 72, "width" : BOARD_X-17, "height" : 0, "color" : COLOR_LINE, },	
				
				{ "name" : "LINE_BAR_START", "type" : "line", "x" : 9, "y" : 93, "width" : BOARD_X-19, "height" : 0, "color" : COLOR_LINE, },

				{ "name" : "LINE_BAR_END", "type" : "line", "x" : 9, "y" : 159, "width" : BOARD_X-19, "height" : 0, "color" : COLOR_LINE, },	
				
				{
					"name": "horizontalbar",
					"type":"horizontalbar",
					"x": 9,
					"y": 75,
					"width": BOARD_X-19,
					"children" :
					(
						{
							"name":"textLine1",
							"type":"text",
							"x": 5,
							"y": 1,
							"text": "",
							"color" : 0xffc3b168,
						},
						{
							"name":"textLine2",
							"type":"text",
							"x": 445,
							"y": 1,
							"text": "",
							"color" : 0xffc3b168,
						},

					),
				},
				
				{
					"name" : "currentLine_Value",
					"type" : "editline",
					"x" : 15,
					"y" : 95,
					"width" : 540,
					"height" : 80,
					"input_limit" : 512,
					"multi_line" : 1,
					"limit_width" : 590,
					#"text" : "Dear Community Metin2 N2, At the moment we got some issues with our hardware and dont know about the reason. All day we're looking for the error and we hope that we can find the error in the next hours and solve it. We don't know if third persons are involved with the error but we can't suspend this at the moment. We are looking for evidences and tell you if we know more about it, we cant do more at this particular time. We're trying our best. Best regards and good evening, your Sinclair.",
				},
				
				{
					"name" : "accept_button",
					"type" : "button",
					"x" : 16,
					"y" : 163,
					"text" : localeInfo.WHISPER_ADMIN_SEND_MESSAGE,
					"default_image" : "d:/ymir work/ui/public/Xlarge_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Xlarge_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Xlarge_button_03.sub",
				},

				{
					"name" : "clear_button",
					"type" : "button",
					"x" : 16+205,
					"y" : 163,
					"text" : localeInfo.WHISPER_ADMIN_CLEAR_MESSAGE,
					"default_image" : "d:/ymir work/ui/public/Xlarge_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Xlarge_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Xlarge_button_03.sub",
				},
				
				{
					"name" : "cancel_button",
					"type" : "button",
					"x" : 16+205+205,
					"y" : 163,
					"text" : localeInfo.WHISPER_ADMIN_CANCEL_ACTION,
					"default_image" : "d:/ymir work/ui/public/Xlarge_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Xlarge_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Xlarge_button_03.sub",
				},
			),
		},
	),
}