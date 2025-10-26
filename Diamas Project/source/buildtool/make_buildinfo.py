#!/usr/bin/env python
import os
import sys
import socket
import getpass

from util import GetSourceRootDirectory, GetLongRevisionHash, GetBranchName, CatFile

def Main(build_name):
	root = GetSourceRootDirectory()
	long_rev = GetLongRevisionHash(root)

	path = os.path.join(root, "src", "share", "revision.txt")
	if CatFile(path) != long_rev:
		# Sometimes it's handy to have this in a file too
		with open(path, "wb") as file:
			file.write(long_rev)

	branch = GetBranchName(root)
	user = getpass.getuser()
	hostname = socket.getfqdn()

	lines = (
		"#ifndef METIN2_BUILDINFO_HPP",
		"#define METIN2_BUILDINFO_HPP", "",

		# git
		"#define METIN2_GIT_REV \"{0}\"".format(long_rev),
		"#define METIN2_GIT_BRANCH \"{0}\"".format(branch),

		# builder
		"#define METIN2_BUILD_USER \"{0}\"".format(user),
		"#define METIN2_BUILD_MACHINE \"{0}\"".format(hostname),

		# Hopefully-unique name that will be stored in the binary
		"#define METIN2_BUILD_NAME \"{0}\"".format(build_name),

		"", "#endif",

		# File has to be terminated with a newline.
		# MSVC's resource compiler fails otherwise. - [tim]
		"",
	)

	path = os.path.join(root, "src", "share", "BuildInfo.hpp")
	if CatFile(path) != "\n".join(lines):
		with open(path, "wb") as file:
			file.write("\n".join(lines))

	lines = (
		# one-liner
		"{1}@{0}".format(long_rev, name),
		"",

		# git
		"METIN2_GIT_REV {0}".format(long_rev),
		"METIN2_GIT_BRANCH \"{0}\"".format(branch),

		# builder
		"METIN2_BUILD_USER \"{0}\"".format(user),
		"METIN2_BUILD_MACHINE \"{0}\"".format(hostname),

		# Hopefully-unique name that will be stored in the binary
		"METIN2_BUILD_NAME \"{0}\"".format(build_name),
	)

	path = os.path.join(root, "src", "share", "version.txt")
	if CatFile(path) != "\n".join(lines):
		with open(path, "wb") as file:
			file.write("\n".join(lines))

if __name__ == "__main__":
	if len(sys.argv) != 1:
		name = sys.argv[1]
	else:
		try:
			name = "{0}-{1}".format(os.environ["BUILDBOT_BUILDERNAME"],
			                        os.environ["BUILDBOT_BUILDNUMBER"])
		except KeyError:
			name = "<unknown>-0"

	Main(name)
