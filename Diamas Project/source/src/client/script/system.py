# -*- coding: utf-8 -*-

import imp
import marshal
import sys

import __builtin__
import app
import dbg
# Import handling
from pack import *

if not app.ENABLE_CYTHON:
    sys.path.append("stdlib")

class EterPackModuleLoader(object):
    def __init__(self, filename, code, is_package):
        self.filename = filename
        self.code = code
        self.is_package = is_package

    def load_module(self, fullname):
        mod = sys.modules.setdefault(fullname, imp.new_module(fullname))
        mod.__file__ = self.filename
        mod.__loader__ = self

        if self.is_package:
            mod.__path__ = []
            mod.__package__ = fullname
        else:
            mod.__package__ = fullname.rpartition('.')[0]

        exec (self.code, mod.__dict__)
        return mod


class EterPackModuleFinder(object):
    def find_module(self, fullname, path=None):
        if imp.is_builtin(fullname):
            return None

        path = fullname.replace('.', '/')

        loader = self.__MakeLoader(path + ".py", False, False)
        if loader:
            print("Found and loaded module {0}".format(fullname))
            return loader

        loader = self.__MakeLoader(path + ".pyc", True, False)
        if loader:
            print("Found and loaded module {0}".format(fullname))
            return loader

        loader = self.__MakeLoader(path + "/__init__.py", False, True)
        if loader:
            print("Found and loaded package {0}".format(fullname))
            return loader

        loader = self.__MakeLoader(path + "/__init__.pyc", True, True)
        if loader:
            print("Found and loaded package {0}".format(fullname))
            return loader

        print("Failed to find {0}".format(fullname))
        return None

    def __MakeLoader(self, filename, is_compiled, is_package):
        data = GetVfsFile(filename)
        if data is None:
            print("Could not load file {}".format(filename))
            return None

        if is_compiled:
            if data[:4] != imp.get_magic():
                raise ImportError("Bad magic")

            code = marshal.loads(data[8:])
        else:
            code = compile(data, filename, "exec")

        return EterPackModuleLoader(filename, code, is_package)


if app.ENABLE_CYTHON:
    sys.meta_path = [EterPackModuleFinder()]
else:
    sys.meta_path.append(EterPackModuleFinder())


def execfile(fileName, dict=None):
    data = GetVfsFile(fileName)

    if fileName.endswith(".pyc") or fileName.endswith(".pyo"):
        if data[:4] != imp.get_magic():
            raise ImportError("Invalid magic")

        code = marshal.loads(data[8:])
    else:
        code = compile(data, fileName, "exec")

    exec (code, dict)


__builtin__.execfile = execfile

loginMark = "-cs"

import logging


class DbgHandler(logging.Handler):
    def emit(self, record):
        try:
            if record.levelno in (logging.ERROR, logging.CRITICAL):
                severity = dbg.SEVERITY_ERROR
            elif record.levelno == logging.WARNING:
                severity = dbg.SEVERITY_WARNING
            else:
                severity = dbg.SEVERITY_INFO

            dbg.Log(self.format(record), severity,
                    record.pathname, record.lineno,
                    record.funcName)
        except (KeyboardInterrupt, SystemExit):
            raise
        except TypeError:
            dbg.Log(record.msg, severity,
                    record.pathname, record.lineno,
                    record.funcName)
        except:
            self.handleError(record)


hdlr = DbgHandler()
hdlr.setFormatter(logging.Formatter("%(message)s", None))
logging.getLogger().addHandler(hdlr)

if __debug__:
    logging.getLogger().setLevel(logging.DEBUG)
else:
    logging.getLogger().setLevel(logging.INFO)

try:
    with open("lang.cfg", "r") as lfile:
        line = lfile.readline().strip()
        app.ForceSetLocale(line, "locale/" + line)
except IOError:
    app.ForceSetLocale("de", "locale/de")
    with open("lang.cfg", "w+") as lfile:
        lfile.write("de")

try:
    with open("ui_debug", "r") as lfile:
        import uiEx
except IOError:
    try:
        with open("ui_debug2", "r") as lfile:
            import uiEx2
    except IOError:
        import Prototype

