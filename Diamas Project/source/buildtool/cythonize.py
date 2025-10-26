#!/usr/bin/env python
# usage: cythonize src dst <include dirs...>
import os
import sys

from util import GetSourceRootDirectory

# Make sure we import the right Cython
sys.path.insert(0, os.path.join(GetSourceRootDirectory(), "thirdparty", "cython"))

from Cython.Compiler import Options
from Cython.Compiler.Main import CompilationOptions, default_options, compile
from Cython.Compiler.Errors import PyrexError

# [tim] Broken as of 0.21.0
# Options.cache_builtins = False
Options.docstrings = False
Options.error_on_unknown_names = False
Options.error_on_uninitialized = False

options = CompilationOptions(default_options)
options.include_path = sys.argv[3:]
options.language_level = 2
options.output_file = sys.argv[2]
options.cplus = True

if options.working_path:
	os.chdir(options.working_path)

any_failures = 0

try:
	result = compile(sys.argv[1], options)
	if result.num_errors > 0:
		any_failures = 1
except (EnvironmentError, PyrexError) as e:
	sys.stderr.write(str(e) + '\n')
	any_failures = 1

if any_failures:
	sys.exit(1)
