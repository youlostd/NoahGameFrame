#ifndef METIN2_THECORE_SINGLETON_HPP
#define METIN2_THECORE_SINGLETON_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <cassert>


template <typename T>
class singleton
{
	public:
		singleton()
		{
			STORM_ASSERT(ms_singleton == nullptr, "Already initialized");
			ms_singleton = static_cast<T*>(this);
		}

		virtual ~singleton()
		{
			STORM_ASSERT(ms_singleton != nullptr, "Not initialized");
			ms_singleton = nullptr;
		}

		static T& Instance()
		{
			STORM_ASSERT(ms_singleton != nullptr, "Not initialized");
			return (*ms_singleton);
		}

		static T* InstancePtr()
		{
			return ms_singleton;
		}

		static T& instance()
		{
			STORM_ASSERT(ms_singleton != nullptr, "Not initialized");
			return *ms_singleton;
		}

	private:
		static T* ms_singleton;
};

template <typename T>
T* singleton<T>::ms_singleton = 0;


#endif
