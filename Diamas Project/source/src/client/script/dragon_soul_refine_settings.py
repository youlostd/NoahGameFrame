# -*- coding: utf-8 -*-

import item

default_grade_need_count = [2, 2, 2, 2, 2, 2]
default_grade_fee = [100000, 300000, 600000, 900000, 1000000, 20000000]
default_step_need_count = [2, 2, 2, 2,	2]
default_step_fee = [100000, 300000, 600000, 10000000, 20000000]

strength_fee = {
	item.MATERIAL_DS_REFINE_NORMAL : 2000000,
	item.MATERIAL_DS_REFINE_BLESSED : 4000000,
	item.MATERIAL_DS_REFINE_HOLLY : 6000000,
}


default_strength_max_table = [
	[2, 2, 3, 3, 4],
	[3, 3, 3, 4, 4],
	[4, 4, 4, 4, 4],
	[4, 4, 4, 4, 5],
	[4, 4, 4, 5, 6],
	[4, 4, 4, 5, 6],
	[4, 4, 4, 5, 6],
]


default_refine_info = {
	"grade_need_count" : default_grade_need_count,
	"grade_fee" : default_grade_fee,
	"step_need_count" : default_step_need_count,
	"step_fee" : default_step_fee,
	"strength_max_table" : default_strength_max_table,
}

dragon_soul_refine_info = {
	11 : default_refine_info,
	12 : default_refine_info,
	13 : default_refine_info,
	14 : default_refine_info,
	15 : default_refine_info,
	16 : default_refine_info,
}
