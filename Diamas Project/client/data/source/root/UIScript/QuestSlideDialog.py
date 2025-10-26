ROOT = "d:/ymir work/ui/game/quest"

window = {
	"name" : "QuestSlideDialog",
	#"style" : ("float",),#"moveable", 
	"style" : ("moveable", "float",),

	"x" : 200,
	"y" : 200,

	#"width" : 334,
	#"height" : 320,
	"width" : 678,
	"height" : 30,

	"children" :
	(
		{
			"name" : "board",
			"type" : "small_dark_thinboard",
			"style" : ("attach", "not_pick",),
			#"image" : 'd:/ymir work/ui/game/quest/quest_content_bg.tga',

			"x" : 0,
			"y" : 0,

			"horizontal_align" : "center",
			"vertical_align" : "center",

			"width" : 678,
			"height" : 30,


		},
	),
}
