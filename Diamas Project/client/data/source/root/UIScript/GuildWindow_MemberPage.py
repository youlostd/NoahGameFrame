import uiScriptLocale
import app

if app.ENABLE_GUILDRENEWAL_SYSTEM:
    if app.ENABLE_SECOND_GUILDRENEWAL_SYSTEM:
        BUTTON_ROOT = "d:/ymir work/ui/game/guild/guildbuttons/memberpage/"
        ROOT_DIR = "d:/ymir work/ui/game/guild/guildmemberpage/"
        PLUS_WITDH = 80
        window = {
            "name": "GuildWindow_MemberPage",
            "x": 8,
            "y": 30,
            "width": 360 + PLUS_WITDH,
            "height": 304,
            "children": (
                ## GuildGradeTItle
                {
                    "name": "GuildGradeTItle",
                    "type": "image",
                    "x": 10,
                    "y": 1,
                    "image": ROOT_DIR + "GuildMemberTItle.sub",
                },
                ## IndexName
                {
                    "name": "IndexName",
                    "type": "image",
                    "x": 45,
                    "y": 4,
                    "image": ROOT_DIR + "IndexName.sub",
                },
                ## IndexGrade
                {
                    "name": "IndexGrade",
                    "type": "image",
                    "x": 141,
                    "y": 4,
                    "image": ROOT_DIR + "IndexGrade.sub",
                },
                ## IndexJob
                {
                    "name": "IndexJob",
                    "type": "image",
                    "x": 229,
                    "y": 4,
                    "image": ROOT_DIR + "IndexJob.sub",
                },
                ## IndexLevel
                {
                    "name": "IndexLevel",
                    "type": "image",
                    "x": 294,
                    "y": 4,
                    "image": ROOT_DIR + "IndexLevel.sub",
                },
                ## IndexOffer
                {
                    "name": "IndexOffer",
                    "type": "image",
                    "x": 333,
                    "y": 4,
                    "image": ROOT_DIR + "IndexOffer.sub",
                },
                ## IndexGeneral
                {
                    "name": "IndexGeneral",
                    "type": "image",
                    "x": 382,
                    "y": 4,
                    "image": ROOT_DIR + "IndexGeneral.sub",
                },
                ## ScrollBar
                {
                    "name": "ScrollBar",
                    "type": "scrollbar",
                    "x": 341 + PLUS_WITDH,
                    "y": 20,
                    "scrollbar_type": "normal",
                    "size": 240,
                },
                ##MemberOut Button
                {
                    "name": "MemberOutButton",
                    "type": "button",
                    "x": 10,
                    "y": 270,
                    "default_image": BUTTON_ROOT + "MemberOutButton00.sub",
                    "over_image": BUTTON_ROOT + "MemberOutButton01.sub",
                    "down_image": BUTTON_ROOT + "MemberOutButton02.sub",
                },
                ##Vote Check Button
                {
                    "name": "VoteCheckButton",
                    "type": "button",
                    "x": 130 + PLUS_WITDH / 2,
                    "y": 270,
                    "default_image": BUTTON_ROOT + "VoteCheckButton00.sub",
                    "over_image": BUTTON_ROOT + "VoteCheckButton01.sub",
                    "down_image": BUTTON_ROOT + "VoteCheckButton02.sub",
                },
                ##Master Change Button
                {
                    "name": "MasterChangeButton",
                    "type": "button",
                    "x": 250 + PLUS_WITDH,
                    "y": 270,
                    "default_image": BUTTON_ROOT + "MasterChangeButton00.sub",
                    "over_image": BUTTON_ROOT + "MasterChangeButton01.sub",
                    "down_image": BUTTON_ROOT + "MasterChangeButton02.sub",
                },
            ),
        }
    else:
        window = {
            "name": "GuildWindow_MemberPage",
            "x": 8,
            "y": 30,
            "width": 360,
            "height": 304,
            "children": (
                ## ScrollBar
                {
                    "name": "ScrollBar",
                    "type": "scrollbar",
                    "x": 341,
                    "y": 20,
                    "scrollbar_type": "normal",
                    "size": 240,
                },
                ## Grade
                {
                    "name": "IndexName",
                    "type": "text",
                    "x": 43,
                    "y": 8,
                    "text": uiScriptLocale.GUILD_MEMBER_NAME,
                    "fontname": "Nunito Sans:12b",
                    "outline": True,
                    "color": (0xFFD79929, 0xFFFEDB40),
                },
                {
                    "name": "IndexGrade",
                    "type": "text",
                    "x": 119,
                    "y": 8,
                    "text": uiScriptLocale.GUILD_MEMBER_RANK,
                    "fontname": "Nunito Sans:12b",
                    "outline": True,
                    "color": (0xFFD79929, 0xFFFEDB40),
                },
                {
                    "name": "IndexJob",
                    "type": "text",
                    "x": 175,
                    "y": 8,
                    "text": uiScriptLocale.GUILD_MEMBER_JOB,
                    "fontname": "Nunito Sans:12b",
                    "outline": True,
                    "color": (0xFFD79929, 0xFFFEDB40),
                },
                {
                    "name": "IndexLevel",
                    "type": "text",
                    "x": 217,
                    "y": 8,
                    "text": uiScriptLocale.GUILD_MEMBER_LEVEL,
                    "fontname": "Nunito Sans:12b",
                    "outline": True,
                    "color": (0xFFD79929, 0xFFFEDB40),
                },
                {
                    "name": "IndexOffer",
                    "type": "text",
                    "x": 251,
                    "y": 8,
                    "text": uiScriptLocale.GUILD_MEMBER_SPECIFIC_GRAVITY,
                    "fontname": "Nunito Sans:12b",
                    "outline": True,
                    "color": (0xFFD79929, 0xFFFEDB40),
                },
                {
                    "name": "IndexGeneral",
                    "type": "text",
                    "x": 300,
                    "y": 8,
                    "text": uiScriptLocale.GUILD_MEMBER_KNIGHT,
                    "fontname": "Nunito Sans:12b",
                    "outline": True,
                    "color": (0xFFD79929, 0xFFFEDB40),
                },
                ##MemberOut Button
                {
                    "name": "MemberOutButton",
                    "type": "button",
                    "x": 10,
                    "y": 264,
                    "text": uiScriptLocale.GUILD_MEMBER_OUT,
                    "default_image": "d:/ymir work/ui/public/large_button_01.sub",
                    "over_image": "d:/ymir work/ui/public/large_button_02.sub",
                    "down_image": "d:/ymir work/ui/public/large_button_03.sub",
                },
                ##Vote Check Button
                {
                    "name": "VoteCheckButton",
                    "type": "button",
                    "x": 120,
                    "y": 264,
                    "text": uiScriptLocale.GUILD_MEMBER_VOTE_CHECK,
                    "default_image": "d:/ymir work/ui/public/large_button_01.sub",
                    "over_image": "d:/ymir work/ui/public/large_button_02.sub",
                    "down_image": "d:/ymir work/ui/public/large_button_03.sub",
                },
                ##Master Change Button
                {
                    "name": "MasterChangeButton",
                    "type": "button",
                    "x": 250,
                    "y": 264,
                    "text": uiScriptLocale.GUILD_MASTER_CHANGE,
                    "default_image": "d:/ymir work/ui/public/large_button_01.sub",
                    "over_image": "d:/ymir work/ui/public/large_button_02.sub",
                    "down_image": "d:/ymir work/ui/public/large_button_03.sub",
                },
            ),
        }
else:

    window = {
        "name": "GuildWindow_MemberPageMain",
        "x": 8,
        "y": 30,
        "width": 360,
        "height": 298,
        "children": (
            ## ScrollBar
            {
                "name": "ScrollBar",
                "type": "scrollbar",
                "x": 341,
                "y": 20,
                "scrollbar_type": "normal",
                "size": 270,
            },
            ## Grade
            {
                "name": "IndexName",
                "type": "text",
                "x": 40,
                "y": 8,
                "text": uiScriptLocale.GUILD_MEMBER_NAME,
                "outline": True,
            },
            {
                "name": "IndexGrade",
                "type": "text",
                "x": 119,
                "y": 8,
                "text": uiScriptLocale.GUILD_MEMBER_RANK,
                "outline": True,
            },
            {
                "name": "IndexJob",
                "type": "text",
                "x": 171,
                "y": 8,
                "text": uiScriptLocale.GUILD_MEMBER_JOB,
                "outline": True,
            },
            {
                "name": "IndexLevel",
                "type": "text",
                "x": 216,
                "y": 8,
                "text": uiScriptLocale.GUILD_MEMBER_LEVEL,
                "outline": True,
            },
            {
                "name": "IndexOffer",
                "type": "text",
                "x": 251,
                "y": 8,
                "text": uiScriptLocale.GUILD_MEMBER_SPECIFIC_GRAVITY,
                "outline": True,
            },
            {
                "name": "IndexGeneral",
                "type": "text",
                "x": 297,
                "y": 8,
                "text": uiScriptLocale.GUILD_MEMBER_KNIGHT,
                "outline": True,
            },
        ),
    }
