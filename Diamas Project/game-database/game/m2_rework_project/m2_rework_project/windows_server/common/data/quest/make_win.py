#!/usr/local/bin/python
import pre_qc
import os
import sys

os.system("rd /s /q object")

if os.system('qc.exe --output-path object --stamp qc.cache  @locale_list'):
	import sys
	sys.exit(-1)
