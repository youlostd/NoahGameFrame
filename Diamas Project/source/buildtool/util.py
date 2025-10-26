import sys
import os
import shutil
import subprocess

# Detect host platform
if sys.platform.startswith("linux"):
	platform = "linux"
elif sys.platform.startswith("freebsd"):
	platform = "freebsd"
elif sys.platform.startswith("gnukfreebsd8"):
	platform = "freebsd"
elif sys.platform.startswith("openbsd"):
	platform = "openbsd"
elif sys.platform.startswith("win") or sys.platform.startswith("mingw"):
	platform = "win"

def GetLongRevisionHash(path):
	git_dir = os.path.join(path, ".git")
	return subprocess.check_output(["git", "--git-dir", git_dir, "rev-parse", "HEAD"]).strip()

def GetBranchName(path):
	git_dir = os.path.join(path, ".git")
	return subprocess.check_output(["git", "--git-dir", git_dir, "symbolic-ref", "--short", "HEAD"]).strip()

def GetSourceRootDirectory():
	# ./build/make_buildinfo.py -> .
	head = os.path.realpath(__file__)

	for i in range(2):
		head, _ = os.path.split(head)

	return head

def FormatCygwinPath(path):
	# too short or no drive spec.
	if len(path) < 2 or path[1] != ':':
		return path.replace("\\", "/")

	return "/cygdrive/{0}{1}".format(path[0], path[2:]).replace("\\", "/")

def EnsurePathExists(file_path):
	try:
		os.makedirs(os.path.split(file_path)[0])
	except OSError:
		pass

def CatFile(file_path):
	try:
		with open(file_path, "r") as file:
			return file.read()
	except IOError:
		return None

def WriteDepfile(path, dependencies):
	with open(path, 'wb') as depfile:
		depfile.write(path)
		depfile.write(': ')
		depfile.write(' '.join(dependencies))
		depfile.write('\n')
