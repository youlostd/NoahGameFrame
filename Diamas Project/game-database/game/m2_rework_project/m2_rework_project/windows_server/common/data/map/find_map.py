#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os

maps = []

for l in open("index"):
	index, path = l.split(" ")

	for line in open(path.rstrip() + "/" + "Setting.txt"):

		tokens = line.split()
		if not tokens:
			continue

		if tokens[0] == "BasePosition":
			x, y = [int(item)/100 for item in tokens[1:3]]
		elif tokens[0] == "MapSize":
			w, h = [int(item) * 256 for item in tokens[1:3]]
	print (index, path, x, y, x+w, y+h)
	maps.append((index, path, x, y, x+w, y+h))
def id(x,y):
	for rect in maps:
		if x >= rect[2] and y >= rect[3] and x <= rect[4] and y<= rect[5]:
			return rect[0], x-rect[2], y-rect[3]

# shitty portal mob name translation code
#for mob in mobs.splitlines():
#	vnum, n = mob.split("\t")
#	v = n.rstrip().split(" ")
#	name, x, y = v[:3]
#	res = id(int(x),int(y))
#	if res:
#		print vnum+"\t"+name, res[0], res[1], res[2]
#	
while 1:
	xy = raw_input("X Y:").split(" ")
	if len(xy) < 2:
		continue
	res = id(int(xy[0]), int(xy[1]))
	if res: print res[0], res[1], res[2] 