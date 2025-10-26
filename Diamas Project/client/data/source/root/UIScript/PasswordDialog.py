import uiScriptLocale

window = {
    "name": "PasswordDialog",
    "x": 0,
    "y": 0,
    "style": (
        "moveable",
        "float",
    ),
    "width": 220,
    "height": 180,
    "children": (
        {
            "name": "board",
            "type": "board",
            "x": 0,
            "y": 0,
            "width": 220,
            "height": 180,
            "children": (
                # ³»¿ë
                {
                    "name": "Desc1",
                    "type": "text",
                    "x": 0,
                    "y": 45,
                    "text": uiScriptLocale.PASSWORD_DESC_1,
                    "horizontal_align": "center",
                },
                # ³»¿ë2
                {
                    "name": "Desc2",
                    "type": "text",
                    "x": 0,
                    "y": 62,
                    "text": uiScriptLocale.PASSWORD_DESC_2,
                    "horizontal_align": "center",
                },
                ## Title
                {
                    "name": "titlebar",
                    "type": "titlebar",
                    "style": ("attach",),
                    "x": 8,
                    "y": 0,
                    "width": 204,
                    "color": "gray",
                    "title": uiScriptLocale.PASSWORD_TITLE,
                },
                ## Password Slot
                {
                    "name": "password_slot",
                    "type": "image",
                    "x": 0,
                    "y": 76,
                    "horizontal_align": "center",
                    "image": "d:/ymir work/ui/public/Parameter_Slot_02.sub",
                    "children": (
                        {
                            "name": "password_value",
                            "type": "editline",
                            "x": 3,
                            "y": 3,
                            "width": 60,
                            "height": 18,
                            "input_limit": 6,
                            "secret_flag": 1,
                        },
                    ),
                },
                {
                    "name": "Desc3",
                    "type": "text",
                    "x": 0,
                    "y": 100,
                    "text": uiScriptLocale.PASSWORD_DESC_3,
                    "horizontal_align": "center",
                },
                {
                    "name": "Desc4",
                    "type": "text",
                    "x": 0,
                    "y": 112,
                    "text": uiScriptLocale.PASSWORD_DESC_4,
                    "horizontal_align": "center",
                },
                {
                    "name": "Desc5",
                    "type": "text",
                    "x": 0,
                    "y": 124,
                    "text": uiScriptLocale.PASSWORD_DESC_5,
                    "horizontal_align": "center",
                },
                ## Button
                ## Button
                {
                    "name": "accept_button",
                    "type": "button",
                    "x": 50,
                    "y": 145,
                    "text": uiScriptLocale.OK,
                    "default_image": "d:/ymir work/ui/public/middle_button_01.sub",
                    "over_image": "d:/ymir work/ui/public/middle_button_02.sub",
                    "down_image": "d:/ymir work/ui/public/middle_button_03.sub",
                },
                {
                    "name": "cancel_button",
                    "type": "button",
                    "x": 110,
                    "y": 145,
                    "text": uiScriptLocale.CANCEL,
                    "default_image": "d:/ymir work/ui/public/middle_button_01.sub",
                    "over_image": "d:/ymir work/ui/public/middle_button_02.sub",
                    "down_image": "d:/ymir work/ui/public/middle_button_03.sub",
                },
            ),
        },
    ),
}
