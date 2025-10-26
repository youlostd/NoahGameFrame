#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include "../eterLib/StdAfx.h"

#include <python.h>
#include <node.h>
#include <grammar.h>
#include <token.h>
#include <parsetok.h>
#include <errcode.h>
#include <compile.h>
typedef struct _mod *mod_ty;
#include <symtable.h>
#include <eval.h>
#include <marshal.h>
#include <modsupport.h>

#include <pybind11/pybind11.h>
#include <pybind11/eval.h>
namespace py = pybind11;

#include "PythonUtils.h"
#include "PythonLauncher.h"
#include "PythonMarshal.h"

