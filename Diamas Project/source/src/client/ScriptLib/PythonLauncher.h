#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <frameobject.h>

class CPythonLauncher 
{
public:
	CPythonLauncher();
	virtual ~CPythonLauncher();
	static void InitializeLogging();

	bool Run();

protected:
	py::module_ m_mainModule;
};
