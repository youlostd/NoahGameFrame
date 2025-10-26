import sys
import os
import subprocess
import shutil
import stat

from xml.dom.minidom import parseString

from util import platform
from install import Install

# Requires: 
def Main(build_dir, bin_dir, dest_url, dest_port, base_url):
	# remove the previous bin. directory
	try:
		shutil.rmtree(bin_dir)
	except OSError:
		pass

	Install(build_dir, bin_dir)

	builder_name = os.environ["BUILDBOT_BUILDERNAME"]
	build_number = os.environ["BUILDBOT_BUILDNUMBER"]
	output_name = "{0}-{1}.7z".format(builder_name, build_number)

	subprocess.check_call(["7za", "a", "-m0=lzma", "-mx=3", "-ms=off", "-r", output_name, "{0}/*".format(bin_dir)])

	os.chmod(output_name, 0644)

	subprocess.check_call([
		"scp", "-P", dest_port,
		"-o", "UserKnownHostsFile=/dev/null", "-o", "StrictHostKeyChecking=no",
		output_name, dest_url
	])

	os.remove(output_name)

	# Add a link to the publicly available download directory to the BuildBot display
	print("@@@STEP_LINK@download@{0}@@@".format(base_url + output_name))

if __name__ == "__main__":
	Main(*sys.argv[1:])
