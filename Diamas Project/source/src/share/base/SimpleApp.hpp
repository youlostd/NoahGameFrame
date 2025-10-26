#ifndef METIN2_BASE_SIMPLEAPP_HPP
#define METIN2_BASE_SIMPLEAPP_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <storm/memory/NewAllocator.hpp>



class SimpleApp
{
	public:
		SimpleApp();
		~SimpleApp();

		// int Run(int argc, const char** argv);

	protected:
		storm::NewAllocator m_allocator;
};

#define METIN2_IMPLEMENT_SIMPLEAPP_MAIN(ImplName) \
extern "C" int main(int argc, const char** argv) \
{ \
	return ImplName().Run(argc, argv); \
}



#endif
