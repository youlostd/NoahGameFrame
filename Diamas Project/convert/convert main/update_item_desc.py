#!/usr/bin/env python
import sys
import csv

from xml.dom import minidom

def LoadItemProto(filename):
	vnums = set()

	with open(filename, "rb") as cf:
		reader = csv.reader(cf, delimiter="\t", quotechar="\"")
		
		for row in reader:
			if not row:
				continue
				
			if str(row[0]) == 'ID':
				continue
				
			if not row[0]:
				continue
			
			if row[0][0] == '#':
				continue
				
			vnum = row[0].split('~')
			vnum = int(vnum[0])
			
				
			vnums.add(vnum)

	return vnums

# vnum: name
def LoadItemNames(filename):
	names = {}

	with open(filename, "rb") as cf:
		reader = csv.reader(cf, delimiter="\t", quotechar="\"")

		for row in reader:
			if not row:
				continue

			if not len(row):
				continue
				
			if not row[0]:
				continue
				
			try:
				names[int(row[0])] = row[1]
			except Exception as e:
				print "Invalid item_names entry at %s:%d %s" % (filename, reader.line_num, e)

	return names

# vnum: (name, [desc], [summary])
def LoadItemDesc(filename):
	items = {}

	with open(filename) as tf:
		for line in tf:
			line = [t.strip() for t in line.split("\t")]

			try:
				vnum = int(line[0])
			except IndexError:
				continue
			except ValueError:
				continue

			name = line[1]

			try:
				desc = line[2]
			except IndexError:
				desc = None

			try:
				summary = line[3]
			except IndexError:
				summary = None

			items[vnum] = (name, desc, summary)

	return items

def WriteItemDesc(desc, out_path):
	with open(out_path, "wb") as of:
		for vnum, d in desc:
			tokens = [str(vnum)] + list(filter(None, d))
			of.write("\t".join(tokens) + "\n")

def Main(proto_path, names_path, desc_path, out_path):
	proto = LoadItemProto(proto_path)
	names = LoadItemNames(names_path)
	desc = LoadItemDesc(desc_path)

	new_desc = {}
	for vnum in proto:
		try:
			name = names[vnum]
		except KeyError:
			if vnum in desc:
				print("Item {0} in item_desc.txt but on in item_names.txt".format(vnum))

			continue

		try:
			description = desc[vnum][1]
			summary = desc[vnum][2]
		except KeyError:
			description = None
			summary = None

		new_desc[vnum] = (name, description, summary)

	new_desc = [(vnum, d) for vnum, d in new_desc.iteritems()]
	WriteItemDesc(sorted(new_desc, key=lambda new_desc: new_desc[0]), out_path)

if __name__ == "__main__":
	Main(*sys.argv[1:])
