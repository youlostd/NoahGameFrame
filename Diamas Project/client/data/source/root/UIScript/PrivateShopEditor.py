import uiScriptLocale
PUBLIC_PATH =  "d:/ymir work/ui/public/"

THINBOARD_WIDTH = 320

def PercentPosition(total, percent):
	return int(total / 100 * percent)

window = {
	"name" : "PrivateShopEditor",

	"x" : 0,
	"y" : 0,

	"style" : ("moveable", "float",),

	"width" : 333,
	"height" : 328 + 26 + 26 + 32 + 48 + 15 + 2,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board",
			"style" : ("attach",),

			"x" : 0,
			"y" : 0,

			"width" : 333,
			"height" : 328 + 26 + 26 + 32 + 48 + 15,

			"children" :
			(
				## Title
				{
					"name" : "TitleBar",
					"type" : "titlebar",

					"x" : 5,
					"y" : 6,

					"width" : 333,
					"color" : "yellow",
                    "title": uiScriptLocale.PRIVATE_SHOP_EDIT_TITLE,

				},




				## Item Slot
				{
					"name" : "ItemSlot",
					"type" : "grid_table",

					"x" : 15,
					"y" : 34 + 26 + 20 + 65 + 5,

					"start_index" : 0,
					"x_count" : 10,
					"y_count" : 9,
					"x_step" : 32,
					"y_step" : 32,

					"image" : "d:/ymir work/ui/public/Slot_Base.sub",
				},

				{
					"name" : "OfflineWrapper",
					"type" : "thinboard_circle",
					"x" : 15,
					"y" : 25 + 9,

					"width" : 320,
					"height" : 108,

					"children":
					(
									## Name
						{
							"name" : "NameSlot",
							"type" : "slotbar",
							"x" : 13,
							"y" : 65,
							"width" : 90 + 45 * 2,
							"height" : 18,

							"children" :
							(
								{
									"name" : "NameLine",
									"type" : "editline",
									"x" : 3,
									"y" : 3,
									"width" : 320 - 6,
									"height" : 15,
									"input_limit" : 25,
									"text" : "1234567890123456789012345",
								},
							),
						},

						{
							"name" : "LocationValue",
							"type" : "text",

							"x" : 15,
							"y" : 65 + 20,

							"width" : 184 - 20,
							"height" : 20,

							"text": uiScriptLocale.PRIVATE_SHOP_LOCATION,
						},
						{
							"name" : "OfflineTime",
							"type" : "image",

							"x" : PercentPosition(THINBOARD_WIDTH, 4),
							"y" : 10,

							"height": 48,
							"width": 48,

							"image" : "d:/ymir work/ui/shop/shop_offline_hourglass.tga",
						},

						{
							"name": "OfflineTime",
							"type": "text",

							"x": PercentPosition(THINBOARD_WIDTH, 5),
							"y": 32 + 2 + 10,

							"text": "0h 00m",
						},
						{
							"name" : "ShopSearchButton",
							"type" : "button",

							"x" : PercentPosition(THINBOARD_WIDTH, 50),
							"y" : 10,


							"width" : 50,
							"height" : 20,

							"default_image" : "d:/ymir work/ui/shop/shop_search_btn.tga",
							"over_image" : "d:/ymir work/ui/shop/shop_search_btn_hover.tga",
							"down_image" : "d:/ymir work/ui/shop/shop_search_btn_push.tga",

							"text": "",
						},
						{
							"name": "ShopSearchText",
							"type": "text",

							"x": PercentPosition(THINBOARD_WIDTH, 49),
							"y": 32 + 2 + 10,

							"text": uiScriptLocale.PRIVATE_SHOP_EDIT_SEARCH,
						},
						{
							"name" : "CloseButton",
							"type" : "button",

							"x" : PercentPosition(THINBOARD_WIDTH, 90),
							"y" : 10,

							"width" : 50,
							"height" : 20,
							
							"default_image" : "d:/ymir work/ui/shop/shop_close.tga",
							"over_image" : "d:/ymir work/ui/shop/shop_close_hover.tga",
							"down_image" : "d:/ymir work/ui/shop/shop_close_push.tga",
						},
						{
							"name": "CloseShopText",
							"type": "text",

							"x": PercentPosition(THINBOARD_WIDTH, 89),
							"y": 32 + 2 + 10,

							"text": uiScriptLocale.PRIVATE_SHOP_EDIT_CLOSE,
						},
						{
							"name" : "OpenShopButton",
							"type" : "button",

							"x": PercentPosition(THINBOARD_WIDTH, 90),
							"y": 10,

							"width" : 50,
							"height" : 20,
							
							"default_image" : "d:/ymir work/ui/shop/shop_add.tga",
							"over_image" : "d:/ymir work/ui/shop/shop_add_hover.tga",
							"down_image" : "d:/ymir work/ui/shop/shop_add_push.tga",
						},
						{
							"name": "OpenShopText",
							"type": "text",

							"x": PercentPosition(THINBOARD_WIDTH, 89),
							"y": 32 + 2 + 10,

							"text": uiScriptLocale.PRIVATE_SHOP_EDIT_OPEN,
						},			

					)
				},

				{
					"name" : "GoldStashWrapper",

					"x" : 0,
					"y" : 0,

					"horizontal_align": "center",
					"vertical_align": "top",

					"width" : 184*2,
					"height" : 40,

					"children" :
					(
						{
							"name":"GoldStash_Slot",
							"type":"button",

							"x":8,
							"y":28,

							"horizontal_align":"center",
							"vertical_align":"bottom",

							"default_image" : "d:/ymir work/ui/public/parameter_slot_05.sub",
							"over_image" : "d:/ymir work/ui/public/parameter_slot_05.sub",
							"down_image" : "d:/ymir work/ui/public/parameter_slot_05.sub",

							"children" :
							(
								{
									"name":"WithdrawStash",
									"type":"image",

									"x":-18,
									"y":2,

									"image":"d:/ymir work/ui/game/windows/money_icon.sub",
								},

								{
									"name" : "GoldStashValue",
									"type" : "text",

									"x" : 3,
									"y" : 3,

									"horizontal_align" : "right",


									"text" : "123456789",
								},
							),
						},
					),					
				},

	
			),
		},
	),
}