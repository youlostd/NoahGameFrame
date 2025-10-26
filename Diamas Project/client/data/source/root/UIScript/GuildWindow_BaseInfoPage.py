import uiScriptLocale
import app

RESOURCE_NAME_POS = 132
if app.ENABLE_GUILDRENEWAL_SYSTEM:

	ROOT_PATH = "d:/ymir work/ui/public/"
	SMALL_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_01.sub"
	LARGE_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_03.sub"
	XLARGE_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_04.sub"
	
	if app.ENABLE_SECOND_GUILDRENEWAL_SYSTEM:
		BUTTON_ROOT = "d:/ymir work/ui/game/guild/guildbuttons/baseinfopage/"
		XXLARGE_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_05.sub"
		PLUS_WITDH = 80
		window = {
			"name" : "GuildWindow_BaseInfoPage",

			"x" : 8,
			"y" : 30,

			"width" : 360 + PLUS_WITDH,
			"height" : 304,

			"children" :
			(
				## ��������
				{
					"name" : "HorizontalBar1",
					"type" : "horizontalbar",
					"x" : 0,
					"y" : 8,
					"width" : 337 + PLUS_WITDH,
					"horizontal_align" : "center",
					
					"children":
					(
						## �������� Text
						{
							"name" : "BaseInfo", "type" : "text", "x" : 8, "y" : 3, "text" : uiScriptLocale.GUILD_BASEINFO_TITLE,
						},				
					),
				},
				## ������ġ
				{
					"name" : "BaseLocale", "type" : "text", "x" : 20, "y" : 30, "text" : uiScriptLocale.GUILD_BASEINFO_LOCALE,
					"children" :
					(
						{
							"name" : "GuildworldSlot", "type" : "image", "x" : 68 + PLUS_WITDH/2, "y" : -2, "image" : XLARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "Base_world", "type":"text", "text":"", "x":0, "y":0, "all_align":"center"},
							),
						},
						{
							"name" : "GuildtownSlot", "type" : "image", "x" : 190 + PLUS_WITDH/2, "y" : -2, "image" : XLARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "Base_town", "type":"text", "text":"", "x":0, "y":0, "all_align":"center"},
							),
						},
					),
				},
				## ��ġ�ǹ�
				{
					"name" : "HorizontalBar1",
					"type" : "horizontalbar",
					"x" : 0,
					"y" : 50,
					"width" : 337 + PLUS_WITDH,
					"horizontal_align" : "center",
					
					"children":
					(
						## ��ġ�ǹ� Text
						{
							"name" : "BaseInfo", "type" : "text", "x" : 8, "y" : 3, "text" : uiScriptLocale.GUILD_BASEINFO_BUILDING,
						},				
					),
				},
				
				## ��������
				{
					"name" : "Guild_Power", "type" : "text" , "x" : 20, "y" : 75, "text" : uiScriptLocale.GUILD_BASEINFO_POWER,
					"children" :
					(
						{
							"name" : "GuildPowerSlot", "type" : "image", "x" : 65 + PLUS_WITDH/3, "y" : 0, "image" : XXLARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "PowerLV", "type":"text", "text": "LV.", "x":8, "y":1,},
								{"name" : "PowerLevel", "type":"text", "text": "", "x":20, "y":0, "all_align":"right"},
							),
						},
					),
				},
				## �뱤��
				{
					"name" : "Guild_smelter", "type" : "text" , "x" : 20, "y" : 100, "text" : uiScriptLocale.GUILD_BASEINFO_SMEITER,
					"children" :
					(
						{
							"name" : "GuildSmelterSlot", "type" : "image", "x" : 65 + PLUS_WITDH/3, "y" : 0, "image" : XXLARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "smeltertype", "type":"text", "text": "", "x":8, "y":1,},
							),
						},
					),
				},
				## ���ü�
				{
					"name" : "Guild_Factory", "type" : "text" , "x" : 20, "y" : 125, "text" : uiScriptLocale.GUILD_BASEINFO_FACTORY,
					"children" :
					(
						{
							"name" : "GuildFactorySlot", "type" : "image", "x" : 65 + PLUS_WITDH/3, "y" : 0, "image" : XXLARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "factorytype", "type":"text", "text": "", "x":8, "y":1,},
							),
						},
					),
				},
				## ��� â��
				{
					"name" : "Guild_Bank", "type" : "text" , "x" : 20, "y" : 150, "text" : uiScriptLocale.GUILD_BASEINFO_BANK,
					"children" :
					(
						{
							"name" : "GuildBankSlot", "type" : "image", "x" : 65 + PLUS_WITDH/3, "y" : 0, "image" : XXLARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "bankLV", "type":"text", "text": "LV.", "x":8, "y":1,},
								{"name" : "banklevel", "type":"text", "text": "", "x":20, "y":0, "all_align":"right"},
							),
						},
					),
				},
				## ���湰
				{
					"name" : "HorizontalBar1",
					"type" : "horizontalbar",
					"x" : 205+PLUS_WITDH/2,
					"y" : 75,
					"width" : 180,
					
					"children":
					(
						## ���湰 Text
						{
							"name" : "BaseInfo", "type" : "text", "x" : 8, "y" : 3, "text" : uiScriptLocale.GUILD_BASEINFO_LANDSCAPE,
						},
					),
				},
				## ���湰 ����Ʈ �ڽ�
				{
					"name" : "Guild_landscape_List",
					"type" : "slotbar",
					"x" : 210 + PLUS_WITDH/2,
					"y" : 75+20,
					"width" : 130 + PLUS_WITDH/2,
					"height" : 90-20,
					
					"children" :
					(
						{
							"name" : "LandscapeList",
							"type" : "listbox",
							"x" : 0,
							"y" : 1,
							"width" : 130 + PLUS_WITDH/2,
							"height" : 85-15,
							"horizontal_align" : "left",
						},
						{
							"name" : "LandscapeScrollBar",
							"type" : "scrollbar",
							"x" : 15,
							"y" : 2,
							"size" : 88-20,
							"horizontal_align" : "right",
						},
					),
				},
			
				## ��� �ڱ� �� â�� ��Ȳ
				{
					"name" : "HorizontalBar1",
					"type" : "horizontalbar",
					"x" : 0,
					"y" : 175,
					"width" : 337 + PLUS_WITDH,
					"horizontal_align" : "center",
					"children" :
					(
						## ��� �ڱ� �� â�� ��Ȳ Ÿ��Ʋ��
						{
							"name" : "Guild_gold_information", "type" : "text", "x" : 8, "y" : 3, "text" : uiScriptLocale.GUILD_BASEINFO_MONEYBANKINFO,
						},
					),
				},	
				## ����ڱ�
				{
					"name" : "ResourceInformationName", "type" : "text", "x" : 20, "y" : 200, "text" : uiScriptLocale.GUILD_BASEINFO_MONEY,
					"children" :
					(
						{
							"name" : "GuildGoldSlot", "type" : "image", "x" : 88 + PLUS_WITDH/2, "y" : -2, "image" : LARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "guildgold", "type":"text", "text": "", "x":0, "y":1,"all_align":"center"},
							),
						},
					),
				},
				## �ڱ�����������
				{
					"name" : "OutGoldMember", "type" : "text", "x" : 20, "y" : 225, "text" : uiScriptLocale.GUILD_BASEINFO_MONEYLASTOUT,
					"children" :
					(
						{
							"name" : "GuildGoldSlot", "type" : "image", "x" : 88 + PLUS_WITDH/2, "y" : -2, "image" : LARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "outgoldmember", "type":"text", "text": "", "x":0, "y":-1,"all_align":"center"},
							),
						},
					),
				},
				## â���������
				{
					"name" : "UseBankMember", "type" : "text", "x" : 20, "y" : 250, "text" : uiScriptLocale.GUILD_BASEINFO_BANKLASTOUT,
					"children" :
					(
						{
							"name" : "GuildGoldSlot", "type" : "image", "x" : 88 + PLUS_WITDH/2, "y" : -2, "image" : LARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "banklistusechr", "type":"text", "text": "", "x":0, "y":-1,"all_align":"center"},
							),
						},
					),
				},
				## ������
				{
					"name" : "HorizontalBar1",
					"type" : "horizontalbar",
					"x" : 205+PLUS_WITDH/2,
					"y" : 200,
					"width" : 180,
					
					"children":
					(
						## ������ Text
						{
							"name" : "BaseInfo", "type" : "text", "x" : 8, "y" : 3, "text" : uiScriptLocale.GUILD_BASEINFO_GUILDLAND,
						},				
					),
				},
				## �ڱݰ���
				{
					"name" : "HorizontalBar1",
					"type" : "horizontalbar",
					"x" : 205+PLUS_WITDH/2,
					"y" : 250,
					"width" : 180,
					
					"children":
					(
						## �ڱݰ��� Text
						{
							"name" : "BaseInfo", "type" : "text", "x" : 8, "y" : 3, "text" : uiScriptLocale.GUILD_BASEINFO_GOLDMANAGE,
						},				
					),
				},
				## ��������ǥ(��ư)
				{
					"name" : "GuildBaseDealVoteButton",
					"type" : "button",

					"x" : 250,
					"y" : 225,
					"default_image" : BUTTON_ROOT+"GuildBaseDealVoteButton00.sub",
					"over_image" : BUTTON_ROOT+"GuildBaseDealVoteButton01.sub",
					"down_image" : BUTTON_ROOT+"GuildBaseDealVoteButton02.sub",
				},
				## �������ŷ�(��ư)
				{
					"name" : "GuildBaseDealButton",
					"type" : "button",

					"x" : 295,
					"y" : 225,
					"default_image" : BUTTON_ROOT+"GuildBaseDealButton00.sub",
					"over_image" : BUTTON_ROOT+"GuildBaseDealButton01.sub",
					"down_image" : BUTTON_ROOT+"GuildBaseDealButton02.sub",
				},
				## ����������(��ư)
				{
					"name" : "GuildBaseAbandonButton",
					"type" : "button",

					"x" : 260 + PLUS_WITDH,
					"y" : 225,
					"default_image" : BUTTON_ROOT+"GuildBaseAbandonButton00.sub",
					"over_image" : BUTTON_ROOT+"GuildBaseAbandonButton01.sub",
					"down_image" : BUTTON_ROOT+"GuildBaseAbandonButton02.sub",
				},
				## â�� �� �ڱ� ���� Ȯ�� ��ư
				{
					"name" : "GuildBankGoldInfoButton",
					"type" : "button",

					"x" : 20,
					"y" : 275,
					"default_image" : BUTTON_ROOT+"GuildBankGoldInfoButton00.sub",
					"over_image" : BUTTON_ROOT+"GuildBankGoldInfoButton01.sub",
					"down_image" : BUTTON_ROOT+"GuildBankGoldInfoButton02.sub",
				},			
				## �ڱ�����(��ư)
				{
					"name" : "GuildGoldOutButton",
					"type" : "button",

					"x" : 250,
					"y" : 275,
					"default_image" : BUTTON_ROOT+"GuildGoldOutButton00.sub",
					"over_image" : BUTTON_ROOT+"GuildGoldOutButton01.sub",
					"down_image" : BUTTON_ROOT+"GuildGoldOutButton02.sub",
				},
				## �ڱ��Ա�(��ư)
				{
					"name" : "GuildGoldinButton",
					"type" : "button",

					"x" : 260 + PLUS_WITDH,
					"y" : 275,
					"default_image" : BUTTON_ROOT+"GuildGoldinButton00.sub",
					"over_image" : BUTTON_ROOT+"GuildGoldinButton01.sub",
					"down_image" : BUTTON_ROOT+"GuildGoldinButton02.sub",
				},
			),
		}
	else:
		window = {
			"name" : "GuildWindow_BaseInfoPage",

			"x" : 8,
			"y" : 30,

			"width" : 360,
			"height" : 304,

			"children" :
			(
				## ��������
				{
					"name" : "HorizontalBar1",
					"type" : "horizontalbar",
					"x" : 0,
					"y" : 8,
					"width" : 337,
					"horizontal_align" : "center",
					
					"children":
					(
						## �������� Text
						{
							"name" : "BaseInfo", "type" : "text", "x" : 8, "y" : 3, "text" : uiScriptLocale.GUILD_BASEINFO_TITLE,
						},				
					),
				},
				## ������ġ
				{
					"name" : "BaseLocale", "type" : "text", "x" : 20, "y" : 30, "text" : uiScriptLocale.GUILD_BASEINFO_LOCALE,
					"children" :
					(
						{
							"name" : "GuildworldSlot", "type" : "image", "x" : 70, "y" : -2, "image" : SMALL_VALUE_FILE,
							"children" :
							(
								{"name" : "Base_world", "type":"text", "text":"", "x":0, "y":0, "all_align":"center"},
							),
						},
						{
							"name" : "GuildtownSlot", "type" : "image", "x" : 133, "y" : -2, "image" : LARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "Base_town", "type":"text", "text":"", "x":0, "y":0, "all_align":"center"},
							),
						},
					),
				},
				## ��ġ�ǹ�
				{
					"name" : "HorizontalBar1",
					"type" : "horizontalbar",
					"x" : 0,
					"y" : 50,
					"width" : 337,
					"horizontal_align" : "center",
					
					"children":
					(
						## ��ġ�ǹ� Text
						{
							"name" : "BaseInfo", "type" : "text", "x" : 8, "y" : 3, "text" : uiScriptLocale.GUILD_BASEINFO_BUILDING,
						},				
					),
				},
				
				## ��������
				{
					"name" : "Guild_Power", "type" : "text" , "x" : 20, "y" : 75, "text" : uiScriptLocale.GUILD_BASEINFO_POWER,
					"children" :
					(
						{
							"name" : "GuildPowerSlot", "type" : "image", "x" : 65, "y" : 0, "image" : XLARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "PowerLV", "type":"text", "text": "LV.", "x":8, "y":1,},
								{"name" : "PowerLevel", "type":"text", "text": "", "x":20, "y":0, "all_align":"right"},
							),
						},
					),
				},
				## �뱤��
				{
					"name" : "Guild_smelter", "type" : "text" , "x" : 20, "y" : 100, "text" : uiScriptLocale.GUILD_BASEINFO_SMEITER,
					"children" :
					(
						{
							"name" : "GuildSmelterSlot", "type" : "image", "x" : 65, "y" : 0, "image" : XLARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "smeltertype", "type":"text", "text": "", "x":8, "y":1,},
							),
						},
					),
				},
				## ���ü�
				{
					"name" : "Guild_Factory", "type" : "text" , "x" : 20, "y" : 125, "text" : uiScriptLocale.GUILD_BASEINFO_FACTORY,
					"children" :
					(
						{
							"name" : "GuildFactorySlot", "type" : "image", "x" : 65, "y" : 0, "image" : XLARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "factorytype", "type":"text", "text": "", "x":8, "y":1,},
							),
						},
					),
				},
				## ��� â��
				{
					"name" : "Guild_Bank", "type" : "text" , "x" : 20, "y" : 150, "text" : uiScriptLocale.GUILD_BASEINFO_BANK,
					"children" :
					(
						{
							"name" : "GuildBankSlot", "type" : "image", "x" : 65, "y" : 0, "image" : XLARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "bankLV", "type":"text", "text": "LV.", "x":8, "y":1,},
								{"name" : "banklevel", "type":"text", "text": "", "x":20, "y":0, "all_align":"right"},
							),
						},
					),
				},
				## ���湰
				{
					"name" : "Guild_landscape", "type" : "text", "x" : 210, "y" : 75, "text" : uiScriptLocale.GUILD_BASEINFO_LANDSCAPE,
				},
				## ���湰 ����Ʈ �ڽ�
				{
					"name" : "Guild_landscape_List",
					"type" : "slotbar",
					"x" : 250,
					"y" : 75,
					"width" : 100,
					"height" : 90,
					
					"children" :
					(
						{
							"name" : "LandscapeList",
							"type" : "listbox",
							"x" : 0,
							"y" : 1,
							"width" : 90,
							"height" : 90,
							"horizontal_align" : "left",
						},
						{
							"name" : "LandscapeScrollBar",
							"type" : "scrollbar",
							"x" : 15,
							"y" : 2,
							"size" : 88,
							"horizontal_align" : "right",
						},
					),
				},
			
				## ��� �ڱ� �� â�� ��Ȳ
				{
					"name" : "HorizontalBar1",
					"type" : "horizontalbar",
					"x" : 0,
					"y" : 175,
					"width" : 337,
					"horizontal_align" : "center",
					"children" :
					(
						## ��� �ڱ� �� â�� ��Ȳ Ÿ��Ʋ��
						{
							"name" : "Guild_gold_information", "type" : "text", "x" : 8, "y" : 3, "text" : uiScriptLocale.GUILD_BASEINFO_MONEYBANKINFO,
						},
					),
				},	
				## ����ڱ�
				{
					"name" : "ResourceInformationName", "type" : "text", "x" : 20, "y" : 200, "text" : uiScriptLocale.GUILD_BASEINFO_MONEY,
					"children" :
					(
						{
							"name" : "GuildGoldSlot", "type" : "image", "x" : 88, "y" : -2, "image" : LARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "guildgold", "type":"text", "text": "", "x":0, "y":1,"all_align":"center"},
							),
						},
					),
				},
				### ���ϼ���
				#{
					#"name" : "TodayincomeTex", "type" : "text", "x" : 205, "y" : 200, "text" : uiScriptLocale.GUILD_BASEINFO_TODAYINCOME,
					#"children" :
					#(
						#{
							#"name" : "GuildToDaySlot", "type" : "image", "x" : 50, "y" : -2, "image" : LARGE_VALUE_FILE,
							#"children" :
							#(
								#{"name" : "todaygold", "type":"text", "text": "0", "x":0, "y":1,"all_align":"center"},
							#),
						#},
					#),
				#},
				## �ڱ�����������
				{
					"name" : "OutGoldMember", "type" : "text", "x" : 20, "y" : 225, "text" : uiScriptLocale.GUILD_BASEINFO_MONEYLASTOUT,
					"children" :
					(
						{
							"name" : "GuildGoldSlot", "type" : "image", "x" : 88, "y" : -2, "image" : LARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "outgoldmember", "type":"text", "text": "", "x":0, "y":-1,"all_align":"center"},
							),
						},
					),
				},
				## â���������
				{
					"name" : "UseBankMember", "type" : "text", "x" : 20, "y" : 250, "text" : uiScriptLocale.GUILD_BASEINFO_BANKLASTOUT,
					"children" :
					(
						{
							"name" : "GuildGoldSlot", "type" : "image", "x" : 88, "y" : -2, "image" : LARGE_VALUE_FILE,
							"children" :
							(
								{"name" : "banklistusechr", "type":"text", "text": "", "x":0, "y":-1,"all_align":"center"},
							),
						},
					),
				},
				## �ڱݰ���
				{
					"name" : "UseBankMember", "type" : "text", "x" : 205, "y" : 250, "text" : uiScriptLocale.GUILD_BASEINFO_GOLDMANAGE,
				},			
				## �ڱ�����(��ư)
				{
					"name" : "GuildGoldOutButton",
					"type" : "button",

					"x" : 260,
					"y" : 250,

					"text" : uiScriptLocale.GUILD_BASEINFO_MONEYOUT,
					"default_image" : ROOT_PATH + "large_button_01.sub",
					"over_image" : ROOT_PATH + "large_button_02.sub",
					"down_image" : ROOT_PATH + "large_button_03.sub",
				},
				## �ڱ��Ա�(��ư)
				{
					"name" : "GuildGoldinButton",
					"type" : "button",

					"x" : 260,
					"y" : 275,

					"text" : uiScriptLocale.GUILD_BASEINFO_MONEYIN,
					"default_image" : ROOT_PATH + "large_button_01.sub",
					"over_image" : ROOT_PATH + "large_button_02.sub",
					"down_image" : ROOT_PATH + "large_button_03.sub",
				},
				## ������
				{
					"name" : "UseBankMember", "type" : "text", "x" : 205, "y" : 200, "text" : uiScriptLocale.GUILD_BASEINFO_GUILDLAND,
				},
				## ��������ǥ(��ư)
				{
					"name" : "GuildBaseDealVoteButton",
					"type" : "button",

					"x" : 260,
					"y" : 200,

					"text" : uiScriptLocale.GUILD_BASEINFO_VOTE,
					"default_image" : ROOT_PATH + "Small_Button_01.sub",
					"over_image" : ROOT_PATH + "Small_Button_02.sub",
					"down_image" : ROOT_PATH + "Small_Button_03.sub",
				},
				## �������ŷ�(��ư)
				{
					"name" : "GuildBaseDealButton",
					"type" : "button",

					"x" : 305,
					"y" : 200,

					"text" : uiScriptLocale.GUILD_BASEINFO_DEAL,
					"default_image" : ROOT_PATH + "Small_Button_01.sub",
					"over_image" : ROOT_PATH + "Small_Button_02.sub",
					"down_image" : ROOT_PATH + "Small_Button_03.sub",
				},
				## ����������(��ư)
				{
					"name" : "GuildBaseAbandonButton",
					"type" : "button",

					"x" : 260,
					"y" : 225,

					"text" : uiScriptLocale.GUILD_BASEINFO_LANDABANDON,
					"default_image" : ROOT_PATH + "large_button_01.sub",
					"over_image" : ROOT_PATH + "large_button_02.sub",
					"down_image" : ROOT_PATH + "large_button_03.sub",
				},
				## â�� �� �ڱ� ���� Ȯ�� ��ư
				{
					"name" : "GuildBankGoldInfoButton",
					"type" : "button",

					"x" : 108,
					"y" : 275,

					"text" : uiScriptLocale.GUILD_BASEINFO_BANKMONEYINFO,
					"default_image" : ROOT_PATH + "large_button_01.sub",
					"over_image" : ROOT_PATH + "large_button_02.sub",
					"down_image" : ROOT_PATH + "large_button_03.sub",
				},
			),
		}
else:
	window = {
		"name" : "GuildWindow_BaseInfoPage",

		"x" : 8,
		"y" : 30,

		"width" : 360,
		"height" : 304,

		"children" :
		(

			## Name
			{
				"name" : "BaseName",
				"type" : "text",
				"x" : 90,
				"y" : 10,
				"text" : uiScriptLocale.GUILD_BASENAME,
			},
			{
				"name" : "BaseNameSlot",
				"type" : "slotbar",
				"x" : 180,
				"y" : 3,
				"width" : 105,
				"height" : 24,
				"text" : uiScriptLocale.GUILD_BASENAME,
				"children" :
				(
					{
						"name" : "BaseNameValue",
						"type" : "text",
						"x" : 0,
						"y" : 0,
						"text" : uiScriptLocale.GUILD_BASENAME,
						"fontsize" : "LARGE",
						"all_align" : "center",
					},
				),
			},

			## Resource Information
			{
				"name" : "HorizontalBar1",
				"type" : "horizontalbar",
				"x" : 0,
				"y" : 32,
				"width" : 337,
				"horizontal_align" : "center",
				"children" :
				(

					{
						"name" : "ResourceInformationName",
						"type" : "text",
						"x" : 0,
						"y" : 0,
						"text" : uiScriptLocale.GUILD_RESOURCE_INFO,
						"all_align" : "center",
					},

					{
						"name" : "ResourceSlot1", "type" : "slotbar", "x" : 5 + 31*0, "y" : 38, "width" : 27, "height" : 18,
						"children" :
						(
							{ "name" : "ResourceName1", "type" : "text", "x" : 0, "y" : -16, "text" : uiScriptLocale.GUILD_WATER_STONE, "horizontal_align" : "center", "horizontal_align" : "center", },
							{ "name" : "ResourceValue1", "type" : "text", "x" : 0, "y" : 0, "text" : "999", "all_align" : "center", },
						),
					},
					{
						"name" : "ResourceSlot2", "type" : "slotbar", "x" : 5 + 31*1, "y" : 38, "width" : 27, "height" : 18,
						"children" :
						(
							{ "name" : "ResourceName2", "type" : "text", "x" : 0, "y" : -16, "text" : uiScriptLocale.GUILD_METIN_STONE, "horizontal_align" : "center", "horizontal_align" : "center", },
							{ "name" : "ResourceValue2", "type" : "text", "x" : 0, "y" : 0, "text" : "999", "all_align" : "center", },
						),
					},
					{
						"name" : "ResourceSlot3", "type" : "slotbar", "x" : 5 + 31*2, "y" : 38, "width" : 27, "height" : 18,
						"children" :
						(
							{ "name" : "ResourceName3", "type" : "text", "x" : 0, "y" : -16, "text" : uiScriptLocale.GUILD_WATER, "horizontal_align" : "center", "horizontal_align" : "center", },
							{ "name" : "ResourceValue3", "type" : "text", "x" : 0, "y" : 0, "text" : "999", "all_align" : "center", },
						),
					},
					{
						"name" : "ResourceSlot4", "type" : "slotbar", "x" : 5 + 31*3, "y" : 38, "width" : 27, "height" : 18,
						"children" :
						(
							{ "name" : "ResourceName4", "type" : "text", "x" : 0, "y" : -16, "text" : uiScriptLocale.GUILD_CRYSTAL, "horizontal_align" : "center", "horizontal_align" : "center", },
							{ "name" : "ResourceValue4", "type" : "text", "x" : 0, "y" : 0, "text" : "999", "all_align" : "center", },
						),
					},
					{
						"name" : "ResourceSlot5", "type" : "slotbar", "x" : 5 + 31*4, "y" : 38, "width" : 27, "height" : 18,
						"children" :
						(
							{ "name" : "ResourceName5", "type" : "text", "x" : 0, "y" : -16, "text" : uiScriptLocale.GUILD_MINENAL, "horizontal_align" : "center", "horizontal_align" : "center", },
							{ "name" : "ResourceValue5", "type" : "text", "x" : 0, "y" : 0, "text" : "999", "all_align" : "center", },
						),
					},
					{
						"name" : "ResourceSlot6", "type" : "slotbar", "x" : 5 + 31*5, "y" : 38, "width" : 27, "height" : 18,
						"children" :
						(
							{ "name" : "ResourceName6", "type" : "text", "x" : 0, "y" : -16, "text" : uiScriptLocale.GUILD_GEM, "horizontal_align" : "center", "horizontal_align" : "center", },
							{ "name" : "ResourceValue6", "type" : "text", "x" : 0, "y" : 0, "text" : "999", "all_align" : "center", },
						),
					},

					{
						"name" : "ResourceBasket", "type" : "bar", "x" : 195, "y" : 21, "width" : 142, "height" : 39,
						"children" :
						(
							{
								"name" : "ResourceBasketDescription1", "type" : "text", "x" : 0, "y" : 5, "text" : uiScriptLocale.GUILD_DROP_RESOURCE1, "horizontal_align" : "center", "horizontal_align" : "center",
							},
							{
								"name" : "ResourceBasketDescription2", "type" : "text", "x" : 0, "y" : 21, "text" : uiScriptLocale.GUILD_DROP_RESOURCE2, "horizontal_align" : "center", "horizontal_align" : "center",
							},
						),
					},

				),
			},

			## Resource Information
			{
				"name" : "HorizontalBar1",
				"type" : "horizontalbar",
				"x" : 0,
				"y" : 95,
				"width" : 337,
				"horizontal_align" : "center",
				"children" :
				(

					{
						"name" : "ResourceInformationName",
						"type" : "text",
						"x" : 0,
						"y" : 0,
						"text" : uiScriptLocale.GUILD_BUILDING_INFO,
						"all_align" : "center",
					},

					## BuildingName
					{
						"name" : "BuildingName", "type" : "text", "x" : 16, "y" : 20, "text" : uiScriptLocale.GUILD_BUILDING_NAME,
					},
					## Grade
					{
						"name" : "Grade", "type" : "text", "x" : 89, "y" : 20, "text" : uiScriptLocale.GUILD_BUILDING_GRADE,
					},

					## Resources
					{ "name" : "ResourceName1", "type" : "text", "x" : RESOURCE_NAME_POS + 29*0, "y" : 20, "text" : uiScriptLocale.GUILD_WATER_STONE, "horizontal_align" : "center", },
					{ "name" : "ResourceName2", "type" : "text", "x" : RESOURCE_NAME_POS + 29*1, "y" : 20, "text" : uiScriptLocale.GUILD_METIN_STONE, "horizontal_align" : "center", },
					{ "name" : "ResourceName3", "type" : "text", "x" : RESOURCE_NAME_POS + 29*2, "y" : 20, "text" : uiScriptLocale.GUILD_WATER, "horizontal_align" : "center", },
					{ "name" : "ResourceName4", "type" : "text", "x" : RESOURCE_NAME_POS + 29*3, "y" : 20, "text" : uiScriptLocale.GUILD_CRYSTAL, "horizontal_align" : "center", },
					{ "name" : "ResourceName5", "type" : "text", "x" : RESOURCE_NAME_POS + 29*4, "y" : 20, "text" : uiScriptLocale.GUILD_MINENAL, "horizontal_align" : "center", },
					{ "name" : "ResourceName6", "type" : "text", "x" : RESOURCE_NAME_POS + 29*5, "y" : 20, "text" : uiScriptLocale.GUILD_GEM, "horizontal_align" : "center", },

					## Power
					{
						"name" : "Power", "type" : "text", "x" : 303, "y" : 20, "text" : uiScriptLocale.GUILD_BUILDING_OPERATE,
					},

				),
			},

			## Buttons
			{
				"name" : "RefreshButton",
				"type" : "button",
				"x" : 337,
				"y" : 5,
				"default_image" : "d:/ymir work/ui/game/guild/Refresh_Button_01.sub",
				"over_image" : "d:/ymir work/ui/game/guild/Refresh_Button_02.sub",
				"down_image" : "d:/ymir work/ui/game/guild/Refresh_Button_03.sub",
				"tooltip_text" : uiScriptLocale.GUILD_BUILDING_REFRESH,
			},

		),
	}
