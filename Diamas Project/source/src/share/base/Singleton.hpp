#ifndef METIN2_CLIENT_ETERBASE_SINGLETON_H
#define METIN2_CLIENT_ETERBASE_SINGLETON_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <cassert>

template <typename T> class CSingleton
{
	static T * ms_singleton;

public:

	CSingleton()
	{
		assert(!ms_singleton);
		ms_singleton = static_cast<T*>(this);
	}

	virtual ~CSingleton()
	{
		assert(ms_singleton);
		ms_singleton = 0;
	}

	BOOST_FORCEINLINE static T & Instance()
	{
		assert(ms_singleton);
		return (*ms_singleton);
	}

	BOOST_FORCEINLINE static T * InstancePtr()
	{
		return (ms_singleton);
	}

	BOOST_FORCEINLINE static T & instance()
	{
		assert(ms_singleton);
		return (*ms_singleton);
	}
};

template <typename T>
T* CSingleton <T>::ms_singleton = nullptr;

template<typename T>
using singleton = CSingleton<T>;

#endif /* METIN2_CLIENT_ETERBASE_SINGLETON_H */
