import uiScriptLocale
import item

RESOURCE = "d:/ymir work/ui/gui/"
ROOT_PATH = "d:/ymir work/ui/gui/fix/switchbot/"

window = {
    "name": "SwitchbotDialog",
    "style": ("moveable", "float",),

    "x": 550,
    "y": 350,

    "width": 451,
    "height": 500,

    "children":
        (

            {

                "name": "Board",
                "type": "board_with_titlebar",

                "x": 0,
                "y": 0,

                "width": 451 - 10,
                "height": 475,

                "title": uiScriptLocale.SWITCHBOT_TITLE,

                "children":
                    (

						{
							"name" : "MainTabCofntrol",
							"type" : "border_a",
							"style": ("not_pick",),
			
							"x" : 5,
							"y" : 33,
							
							"width" : 430,
							"height" : 434,
							
			
			
						},

                        {
                            "name": "SwitchbotBaseImage",
                            "type": "image",
                            "style": ("not_pick",),

                            "x": -3,
                            "y": 5,
                            "horizontal_align": "center",
                            "vertical_align": "center",

                            "image": ROOT_PATH + "SwitchbotBG.png",

                            "children":
                                (

                                    {
                                        "name": "SwitchbotSlot",
                                        "type": "slot",

                                        "x": 5,
                                        "y": 60,

                                        "width": 440,
                                        "height": 187,

                                        "slot": (
                                            {"index": 0, "x": 29, "y": 53, "width": 32, "height": 96},
                                            {"index": 1, "x": 29 + (70), "y": 53, "width": 32, "height": 96},
                                            {"index": 2, "x": 29 + (140), "y": 53, "width": 32, "height": 96},
                                            {"index": 3, "x": 29 + (210), "y": 53, "width": 32, "height": 96},
                                            {"index": 4, "x": 29 + (280), "y": 53, "width": 32, "height": 96},
                                            {"index": 5, "x": 29 + (350), "y": 53, "width": 32, "height": 96},
                                        ),
                                    },

                                    {
                                        "name": "SwitchbotSlotButton_0",
                                        "type": "image",

                                        "x": 29 + 10 - 2,
                                        "y": 72,

                                        "image": "d:/ymir work/ui/gui/switchbot_icon_inactive.sub",

                                        "children": (

                                            {
                                                "name": "SwitchbotSlotButtonMarker_0",
                                                "type": "image",
                                                "image": "d:/ymir work/ui/game/quest/quest_checked.tga",

                                                "x": 6,
                                                "y": 6,

                                            },

                                        ),
                                    },

                                    {
                                        "name": "SwitchbotSlotButton_1",
                                        "type": "image",

                                        "x": 29 + 10 + 70 - 2,
                                        "y": 72,

                                        "image": "d:/ymir work/ui/gui/switchbot_icon_inactive.sub",
                                        "children": (

                                            {
                                                "name": "SwitchbotSlotButtonMarker_1",
                                                "type": "image",
                                                "image": "d:/ymir work/ui/game/quest/quest_checked.tga",

                                                "x": 6,
                                                "y": 6,

                                            },

                                        ),
                                    },

                                    {
                                        "name": "SwitchbotSlotButton_2",
                                        "type": "image",

                                        "x": 27 + 10 + 70 * 2,
                                        "y": 72,

                                        "image": "d:/ymir work/ui/gui/switchbot_icon_inactive.sub",
                                        "children": (

                                            {
                                                "name": "SwitchbotSlotButtonMarker_2",
                                                "type": "image",
                                                "image": "d:/ymir work/ui/game/quest/quest_checked.tga",

                                                "x": 6,
                                                "y": 6,

                                            },

                                        ),
                                    },

                                    {
                                        "name": "SwitchbotSlotButton_3",
                                        "type": "image",

                                        "x": 27 + 10 + 70 * 3,
                                        "y": 72,

                                        "image": "d:/ymir work/ui/gui/switchbot_icon_inactive.sub",
                                        "children": (

                                            {
                                                "name": "SwitchbotSlotButtonMarker_3",
                                                "type": "image",
                                                "image": "d:/ymir work/ui/game/quest/quest_checked.tga",

                                                "x": 6,
                                                "y": 6,

                                            },

                                        ),
                                    },

                                    {
                                        "name": "SwitchbotSlotButton_4",
                                        "type": "image",

                                        "x": 27 + 10 + 70 * 4,
                                        "y": 72,

                                        "image": "d:/ymir work/ui/gui/switchbot_icon_inactive.sub",
                                        "children": (

                                            {
                                                "name": "SwitchbotSlotButtonMarker_4",
                                                "type": "image",
                                                "image": "d:/ymir work/ui/game/quest/quest_checked.tga",

                                                "x": 6,
                                                "y": 6,

                                            },

                                        ),
                                    },

                                    {
                                        "name": "SwitchbotSlotButton_5",
                                        "type": "image",

                                        "x": 27 + 10 + 70 * 5,
                                        "y": 72,

                                        "image": "d:/ymir work/ui/gui/switchbot_icon_inactive.sub",
                                        "children": (

                                            {
                                                "name": "SwitchbotSlotButtonMarker_5",
                                                "type": "image",
                                                "image": "d:/ymir work/ui/game/quest/quest_checked.tga",

                                                "x": 6,
                                                "y": 6,

                                            },

                                        ),
                                    },




                                    {
                                        "name": "Bonus_Tab_01",
                                        "type": "radio_button",

                                        "x": 310 - 15,
                                        "y": 449 - 130,

                                        "default_image": ROOT_PATH + "slot.png",
                                        "over_image": ROOT_PATH + "slot2.png",
                                        "down_image": ROOT_PATH + "slot3.png",

                                        "children":
                                            (
                                                {
                                                    "name": "Bonus_Tab_01_Print",
                                                    "type": "text",

                                                    "x": -2,
                                                    "y": 0,

                                                    "all_align": "center",

                                                    "text": "I",
                                                },
                                            ),
                                    },
                                    {
                                        "name": "Bonus_Tab_02",
                                        "type": "radio_button",

                                        "x": 350 - 15,
                                        "y": 449 - 130,

                                        "default_image": ROOT_PATH + "slot.png",
                                        "over_image": ROOT_PATH + "slot2.png",
                                        "down_image": ROOT_PATH + "slot3.png",

                                        "children":
                                            (
                                                {
                                                    "name": "Bonus_Tab_02_Print",
                                                    "type": "text",

                                                    "x": -2,
                                                    "y": 0,

                                                    "all_align": "center",

                                                    "text": "II",
                                                },
                                            ),
                                    },
                                    {
                                        "name": "Bonus_Tab_03",
                                        "type": "radio_button",

                                        "x": 390 - 15,
                                        "y": 449 - 130,

                                        "default_image": ROOT_PATH + "slot.png",
                                        "over_image": ROOT_PATH + "slot2.png",
                                        "down_image": ROOT_PATH + "slot3.png",

                                        "children":
                                            (
                                                {
                                                    "name": "Bonus_Tab_03_Print",
                                                    "type": "text",

                                                    "x": -2,
                                                    "y": 0,

                                                    "all_align": "center",

                                                    "text": "III",
                                                },
                                            ),
                                    },
                                    {
                                        "name": "SlotsTXT",
                                        "type": "text",
										"text" : "Alternatif Bonus",
                                        "x": 296,
                                        "y": 268,
										"fontname": "Nunito Sans:15b",
                                        # "image": ROOT_PATH + "Slots.png",
                                    },
                                    {
                                        "name": "BonusTXT",
                                        "type": "text",
										"text" : "Bonus Seçin",
                                        "x": 70,
                                        "y": 268,
										"fontname": "Nunito Sans:15b",
                                        # "image": ROOT_PATH + "SelectBonus.png",
									},
                                    {
                                        "name": "Switchbot_Page",
                                        "type": "text",
										"text" : "I",
                                        "x": 47,
                                        "y": 215,
										"fontname": "Nunito Sans:25b",
                                    },
                                    {
                                        "name": "Switchbot_Page",
                                        "type": "text",
										"text" : "II",
                                        "x": 47 + 68,
                                        "y": 215,
										"fontname": "Nunito Sans:25b",
                                    },
                                    {
                                        "name": "Switchbot_Page",
                                        "type": "text",
										"text" : "III",
                                        "x": 47 + 68 + 68,
                                        "y": 215,
										"fontname": "Nunito Sans:25b",
                                    },
                                    {
                                        "name": "Switchbot_Page",
                                        "type": "text",
										"text" : "IV",
                                        "x": 46 + 68 + 68 + 68,
                                        "y": 215,
										"fontname": "Nunito Sans:25b",
                                    },
                                    {
                                        "name": "Switchbot_Page",
                                        "type": "text",
										"text" : "V",
                                        "x": 51 + 68 + 68 + 68 + 68,
                                        "y": 215,
										"fontname": "Nunito Sans:25b",
                                    },
                                    {
                                        "name": "Switchbot_Page",
                                        "type": "text",
										"text" : "VI",
                                        "x": 50 + 68 + 68 + 68 + 68 + 68,
                                        "y": 215,
										"fontname": "Nunito Sans:25b",
                                    },
                                    {
                                        "name": "startbtn",
                                        "type": "button",
                                        "x": 295,
                                        "y": 435,
                                        "default_image": "d:/ymir work/ui/gui/fix/switchbot/buton_normal.png",
                                        "over_image": "d:/ymir work/ui/gui/fix/switchbot/buton_uzerine.png",
                                        "down_image": "d:/ymir work/ui/gui/fix/switchbot/buton_basıldı.png",
                                    },

                                ),

                        },
                    ),
            },

        ),
}
