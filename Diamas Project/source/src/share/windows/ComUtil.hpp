#ifndef METIN2_WINDOWS_COMUTIL_HPP
#define METIN2_WINDOWS_COMUTIL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/WindowsPlatform.hpp>

#include <comdef.h>
#include <wbemidl.h>

METIN2_BEGIN_NS

class ScopedComInitialization
{
	public:
		ScopedComInitialization();
		~ScopedComInitialization();
	private:
		bool m_initialized;
};

class WmiQuery
{
	public:
		WmiQuery();
		~WmiQuery();

		bool Setup();

		IEnumWbemClassObject* WmiQuery::Execute(const char* query);

	private:
		IWbemLocator* m_loc;
		IWbemServices* m_svc;
};

METIN2_END_NS

#endif
