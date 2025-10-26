import os
import sys
import shutil
import subprocess

from util import platform, EnsurePathExists

kExecutables = [
	("db", "server/db"),
	("game", "server/game"),

	("dump_proto", "tool/dump_proto"),
	("format_proto", "tool/format_proto"),

	("pyc", "tool/pyc"),
	("qc", "tool/qc"),
]

kExecutables2 = []

if platform == "win":
	kExecutables.extend([
		("CrashSender", "client/CrashSender"),
		("run", "client/run"),

		("make_motion_proto", "tool/make_motion_proto"),
		("world_editor", "tool/world_editor"),
	])

# windows-only
kDllDependencies = [
	# Client
	("DevIL.dll", "client/DevIL.dll"),

	("mss32.dll", "client/mss32.dll"),
	("mssds3d.m3d", "client/miles/m3dds3d.m3d"),
	("mssdx7.m3d", "client/miles/m3ddx7.m3d"),
	("msseax.m3d", "client/miles/m3deax.m3d"),
	("msssoft.m3d", "client/miles/m3dsoft.m3d"),
	("mssdsp.flt", "client/miles/mssdsp.flt"),
	("mssmp3.asi", "client/miles/mssmp3.asi"),

	# Server
	("DevIL.dll", "server/DevIL.dll"),

	# Tools
	("DevIL.dll", "server/DevIL.dll"),
]

# unix only
kSoDependencies = [
	# Server
	("libDevIL.so", "server/libDevIL.so"),

	# Tools
	("libDevIL.so", "tool/libDevIL.so"),
]

def InstallElf(src, dst):
	dbg_path = os.path.splitext(dst)[0] + ".dbg"
	subprocess.check_call(["objcopy", "--only-keep-debug", src, dbg_path])
	subprocess.check_call(["strip", "--strip-debug", "--strip-unneeded",
	                       "-o", dst, src])
	subprocess.check_call(["objcopy", "--add-gnu-debuglink=" + dbg_path, dst])

def InstallExecutable(src, dst):
	EnsurePathExists(dst)

	if platform == "win":
		shutil.copyfile(src + ".exe", dst + ".exe")
		shutil.copyfile(src + ".exe.pdb", dst + ".exe.pdb")
	else:
		InstallElf(src, dst)

def InstallLibrary(src, dst):
	EnsurePathExists(dst)

	if platform == "win":
		shutil.copyfile(src, dst)
		shutil.copyfile(src + ".pdb", dst + ".pdb")
	else:
		InstallElf(src, dst)

def Install(build_dir, install_dir):
	for src, dst in kExecutables:
		print("Installing {0}".format(src))
		InstallExecutable(os.path.join(build_dir, src),
		                  os.path.join(install_dir, dst))

	if platform == "win":
		other_deps = kDllDependencies
	else:
		other_deps = kSoDependencies

	for src, dst in other_deps:
		print("Installing {0} to {1}".format(src, dst))
		InstallLibrary(os.path.join(build_dir, src),
		               os.path.join(install_dir, dst))

if __name__ == "__main__":
	Install(sys.argv[1], sys.argv[2])
