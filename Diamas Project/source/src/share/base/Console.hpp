#ifndef METIN2_BASE_CONSOLE_HPP
#define METIN2_BASE_CONSOLE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <functional>



#if VSTD_PLATFORM_WINDOWS

bool CreateConsoleWindow();

class ScopedConsoleCloseHandler
{
	public:
		ScopedConsoleCloseHandler(std::function<void()> h);
		~ScopedConsoleCloseHandler();

		void operator()();

	private:
		std::function<void()> m_handler;
};

#endif



#endif
