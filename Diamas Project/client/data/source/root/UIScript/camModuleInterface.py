import uiScriptLocale
window = {
	"name" : "camModuleInterface",
	"style" : ("moveable", "float",),
	
	"x" : (SCREEN_WIDTH / 2) - 250,
	"y" : (SCREEN_HEIGHT / 2) - 250,
	"width" : 448,
	"height" : 500,
	
	"children" :
	(
		{
			"name" : "board",
			"type" : "board_with_titlebar",
			"style" : ("attach",),
			
			"x" : 0,
			"y" : 0,
			"width" : 448,
			"height" : 500,
			
			"title" : "Camy - Metin2 Module",
			
			"children" :
			(
				{
					"name" : "kfWindow",
					"type" : "window",
					
					"x" : 15,
					"y" : 37,
					"width" : 150,
					"height" : 420,
					
					"children" :
					(
						{
							"name" : "kfHeadBar",
							"type" : "horizontalbar",
							
							"x" : 0,
							"y" : 0,
							"width" : 150,
							
							"children" :
							(
								{
									"name" : "kfTitle",
									"type" : "text",
									
									"x" : 5,
									"y" : 2,
									
									"text" : "Kamera-Punkte",
								},
								{
									"name" : "kfDelete",
									"type" : "button",
									
									"x" : 114,
									"y" : 2,
									
									"default_image" : "d:/ymir work/ui/game/windows/btn_minus_up.sub",
									"over_image" : "d:/ymir work/ui/game/windows/btn_minus_over.sub",
									"down_image" : "d:/ymir work/ui/game/windows/btn_minus_down.sub",
								},
								{
									"name" : "kfAdd",
									"type" : "button",
									
									"x" : 132,
									"y" : 2,
									
									"default_image" : "d:/ymir work/ui/game/windows/btn_plus_up.sub",
									"over_image" : "d:/ymir work/ui/game/windows/btn_plus_over.sub",
									"down_image" : "d:/ymir work/ui/game/windows/btn_plus_down.sub",
								},
							),
						},
						{
							"name" : "kfBackgroundBar",
							"type" : "bar",
							
							"x" : 0,
							"y" : 20,
							"width" : 150,
							"height" : 400,
							
							"color" : 0x93000000,
							
							"children" :
							(
								{
									"name" : "kfList",
									"type" : "listboxex",
									
									"x" : 0,
									"y" : 0,
									"width" : 129,
									"height" : 400,
									
									"viewcount" : 19,
								},
								{
									"name" : "kfScroll",
									"type" : "scrollbar",
									
									"x" : 132,
									"y" : 3,
									
									"size" : 394,
								},
							),
						},
					),
				},
				{
					"name" : "kfSettingsWindow",
					"type" : "window",
					
					"x" : 175,
					"y" : 37,
					"width" : 258,
					"height" : 296,
					
					"children" :
					(
						{
							"name" : "kfSettingsHeadBar",
							"type" : "horizontalbar",
							
							"x" : 0,
							"y" : 0,
							"width" : 258,
							
							"children" :
							(
								{
									"name" : "kfSettingsTitle",
									"type" : "text",
									
									"x" : 5,
									"y" : 2,
									
									"text" : "Kamera-Punkt Eigenschaften",
								},
							),
						},
						{
							"name" : "kfPosX",
							"type" : "window",
							
							"x" : 0,
							"y" : 27,
							"width" : 250,
							"height" : 18,
							
							"children" :
							(
								{
									"name" : "kfPosXInfo",
									"type" : "text",
									
									"x" : 8,
									"y" : 2,
									
									"text" : "X:",
								},
								{
									"name" : "kfPosXValueSlot",
									"type" : "image",
									
									"x" : 18,
									"y" : 0,
									
									"image" : "d:/ymir work/ui/public/Parameter_Slot_01.sub",
									
									"children" :
									(
										{
											"name" : "kfPosXValue",
											"type" : "editline",
											
											"x" : 3,
											"y" : 2,
											"width" : 47,
											"height" : 14,
											
											"input_limit" : 8,
											"only_number" : 1,
											"text" : "0",
										},
									),
								},
								{
									"name" : "kfPosXSlider",
									"type" : "sliderbar",
									
									"x" : 75,
									"y" : 3,
								},
							),
						},
						{
							"name" : "kfPosY",
							"type" : "window",
							
							"x" : 0,
							"y" : 50,
							"width" : 250,
							"height" : 18,
							
							"children" :
							(
								{
									"name" : "kfPosYInfo",
									"type" : "text",
									
									"x" : 8,
									"y" : 2,
									
									"text" : "Y:",
								},
								{
									"name" : "kfPosYValueSlot",
									"type" : "image",
									
									"x" : 18,
									"y" : 0,
									
									"image" : "d:/ymir work/ui/public/Parameter_Slot_01.sub",
									
									"children" :
									(
										{
											"name" : "kfPosYValue",
											"type" : "editline",
											
											"x" : 3,
											"y" : 2,
											"width" : 47,
											"height" : 14,
											
											"input_limit" : 8,
											"only_number" : 1,
											"text" : "0",
										},
									),
								},
								{
									"name" : "kfPosYSlider",
									"type" : "sliderbar",
									
									"x" : 75,
									"y" : 3,
								},
							),
						},
						{
							"name" : "kfPosZ",
							"type" : "window",
							
							"x" : 0,
							"y" : 73,
							"width" : 250,
							"height" : 18,
							
							"children" :
							(
								{
									"name" : "kfPosZInfo",
									"type" : "text",
									
									"x" : 8,
									"y" : 2,
									
									"text" : "Z:",
								},
								{
									"name" : "kfPosZValueSlot",
									"type" : "image",
									
									"x" : 18,
									"y" : 0,
									
									"image" : "d:/ymir work/ui/public/Parameter_Slot_01.sub",
									
									"children" :
									(
										{
											"name" : "kfPosZValue",
											"type" : "editline",
											
											"x" : 3,
											"y" : 2,
											"width" : 47,
											"height" : 14,
											
											"input_limit" : 8,
											"only_number" : 1,
											"text" : "0",
										},
									),
								},
								{
									"name" : "kfPosZSlider",
									"type" : "sliderbar",
									
									"x" : 75,
									"y" : 3,
								},
							),
						},
						{
							"name" : "kfPosZoom",
							"type" : "window",
							
							"x" : 0,
							"y" : 104,
							"width" : 250,
							"height" : 18,
							
							"children" :
							(
								{
									"name" : "kfPosZoomInfo",
									"type" : "text",
									
									"x" : 8,
									"y" : 2,
									
									"text" : "D:",
								},
								{
									"name" : "kfPosZoomValueSlot",
									"type" : "image",
									
									"x" : 18,
									"y" : 0,
									
									"image" : "d:/ymir work/ui/public/Parameter_Slot_01.sub",
									
									"children" :
									(
										{
											"name" : "kfPosZoomValue",
											"type" : "editline",
											
											"x" : 3,
											"y" : 2,
											"width" : 47,
											"height" : 14,
											
											"input_limit" : 8,
											"only_number" : 1,
											"text" : "0",
										},
									),
								},
								{
									"name" : "kfPosZoomSlider",
									"type" : "sliderbar",
									
									"x" : 75,
									"y" : 3,
								},
							),
						},
						{
							"name" : "kfPosPitch",
							"type" : "window",
							
							"x" : 0,
							"y" : 127,
							"width" : 250,
							"height" : 18,
							
							"children" :
							(
								{
									"name" : "kfPosPitchInfo",
									"type" : "text",
									
									"x" : 8,
									"y" : 2,
									
									"text" : "P:",
								},
								{
									"name" : "kfPosPitchValueSlot",
									"type" : "image",
									
									"x" : 18,
									"y" : 0,
									
									"image" : "d:/ymir work/ui/public/Parameter_Slot_01.sub",
									
									"children" :
									(
										{
											"name" : "kfPosPitchValue",
											"type" : "editline",
											
											"x" : 3,
											"y" : 2,
											"width" : 47,
											"height" : 14,
											
											"input_limit" : 8,
											"only_number" : 1,
											"text" : "0",
										},
									),
								},
								{
									"name" : "kfPosPitchSlider",
									"type" : "sliderbar",
									
									"x" : 75,
									"y" : 3,
								},
							),
						},
						{
							"name" : "kfPosRotation",
							"type" : "window",
							
							"x" : 0,
							"y" : 151,
							"width" : 250,
							"height" : 18,
							
							"children" :
							(
								{
									"name" : "kfPosRotationInfo",
									"type" : "text",
									
									"x" : 8,
									"y" : 2,
									
									"text" : "R:",
								},
								{
									"name" : "kfPosRotationValueSlot",
									"type" : "image",
									
									"x" : 18,
									"y" : 0,
									
									"image" : "d:/ymir work/ui/public/Parameter_Slot_01.sub",
									
									"children" :
									(
										{
											"name" : "kfPosRotationValue",
											"type" : "editline",
											
											"x" : 3,
											"y" : 2,
											"width" : 47,
											"height" : 14,
											
											"input_limit" : 8,
											"only_number" : 1,
											"text" : "0",
										},
									),
								},
								{
									"name" : "kfPosRotationSlider",
									"type" : "sliderbar",
									
									"x" : 75,
									"y" : 3,
								},
							),
						},
						{
							"name" : "kfSettingsTime",
							"type" : "window",
							
							"x" : 0,
							"y" : 182,
							"width" : 250,
							"height" : 18,
							
							"children" :
							(
								{
									"name" : "kfSettingsTimeInfo",
									"type" : "text",
									
									"x" : 8,
									"y" : 2,
									
									"text" : "T:",
								},
								{
									"name" : "kfSettingsTimeValueSlot",
									"type" : "image",
									
									"x" : 18,
									"y" : 0,
									
									"image" : "d:/ymir work/ui/public/Parameter_Slot_01.sub",
									
									"children" :
									(
										{
											"name" : "kfSettingsTimeValue",
											"type" : "editline",
											
											"x" : 3,
											"y" : 2,
											"width" : 47,
											"height" : 14,
											
											"input_limit" : 3,
											"only_number" : 1,
											"text" : "0",
										},
									),
								},
								{
									"name" : "kfSettingsTimeSlider",
									"type" : "sliderbar",
									
									"x" : 75,
									"y" : 3,
								},
							),
						},
						{
							"name" : "kfSettingsEase",
							"type" : "window",
							
							"x" : 0,
							"y" : 213,
							"width" : 250,
							"height" : 42,
							
							"children" :
							(
								{
									"name" : "kfBtnEase_0",
									"type" : "radio_button",
									
									"x" : 3,
									"y" : 0,
									
									"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
								},
								{
									"name" : "kfBtnEase_1",
									"type" : "radio_button",
									
									"x" : 64,
									"y" : 0,
									
									"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
								},
								{
									"name" : "kfBtnEase_2",
									"type" : "radio_button",
									
									"x" : 125,
									"y" : 0,
									
									"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
								},
								{
									"name" : "kfBtnEase_3",
									"type" : "radio_button",
									
									"x" : 186,
									"y" : 0,
									
									"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
								},
								{
									"name" : "kfBtnEase_4",
									"type" : "radio_button",
									
									"x" : 3,
									"y" : 21,
									
									"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
								},
								{
									"name" : "kfBtnEase_5",
									"type" : "radio_button",
									
									"x" : 64,
									"y" : 21,
									
									"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
								},
								{
									"name" : "kfBtnEase_6",
									"type" : "radio_button",
									
									"x" : 125,
									"y" : 21,
									
									"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
								},
								{
									"name" : "kfBtnEase_7",
									"type" : "radio_button",
									
									"x" : 186,
									"y" : 21,
									
									"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
									"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
									"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
								},
							),
						},
						{
							"name" : "kfBtnResetChanges",
							"type" : "button",
							
							"x" : 5,
							"y" : 275,
							
							"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
							"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
							"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",
							"text" : "Verwerfen",
						},
						{
							"name" : "kfBtnApplyChanges",
							"type" : "button",
							
							"x" : 162,
							"y" : 275,
							
							"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
							"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
							"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",
							"text" : "Übernehmen",
						},
					),
				},
				{
					"name" : "kfManageWindow",
					"type" : "window",
					
					"x" : 175,
					"y" : 393,
					"width" : 258,
					"height" : 231,
					
					"children" :
					(
						{
							"name" : "kfManageHeadBar",
							"type" : "horizontalbar",
							
							"x" : 0,
							"y" : 0,
							"width" : 258,
							
							"children" :
							(
								{
									"name" : "kfManageTitle",
									"type" : "text",
									
									"x" : 5,
									"y" : 2,
									
									"text" : "Script-Verwaltung",
								},
							),
						},
						{
							"name" : "btnPlay",
							"type" : "button",
							
							"x" : 5,
							"y" : 20,
							
							"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
							"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
							"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",
							"text" : "Abspielen",
						},
						{
							"name" : "btnPlaySmooth",
							"type" : "button",
							
							"x" : 5,
							"y" : 43,
							
							"default_image" : "d:/ymir work/ui/public/Large_Button_01.sub",
							"over_image" : "d:/ymir work/ui/public/Large_Button_02.sub",
							"down_image" : "d:/ymir work/ui/public/Large_Button_03.sub",
							"text" : "Abspielen (Smooth)",
						},
						{
							"name" : "btnSaveScript",
							"type" : "button",
							
							"x" : 126,
							"y" : 20,
							
							"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
							"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
							"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
							"text" : "Speichern",
						},
						{
							"name" : "btnLoadScript",
							"type" : "button",
							
							"x" : 192,
							"y" : 20,
							
							"default_image" : "d:/ymir work/ui/public/Middle_Button_01.sub",
							"over_image" : "d:/ymir work/ui/public/Middle_Button_02.sub",
							"down_image" : "d:/ymir work/ui/public/Middle_Button_03.sub",
							"text" : "Laden",
						},
					),
				},
				{
					"name" : "txtCopyright",
					"type" : "text",
					
					"x" : 15,
					"y" : 470,
					
					"text" : "Copyright (C) 2015 Yiv",
				},
				{
					"name" : "txtCamyCompatible",
					"type" : "text",
					
					"x" : 224,
					"y" : 470,
					
					"horizontal_align" : "center",
					"r" : 1.0,
					"g" : 0.0,
					"b" : 0.0,
					"text" : "Camy ist nicht kompatibel!",
				},
				{
					"name" : "txtVersion",
					"type" : "text",
					
					"x" : 433,
					"y" : 470,
					
					"text" : "v1.2",
					"horizontal_align" : "right",
				},
			),
		},
	),
}