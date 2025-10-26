#ifndef METIN2_THECORE_SEMAPHORE_HPP
#define METIN2_THECORE_SEMAPHORE_HPP


#pragma once

#if defined(_WIN32)
#include <windows.h>
#else
#include <semaphore.h>
#endif


class CSemaphore
{
	public:
		CSemaphore();
		~CSemaphore();

		bool Initialize();
		void Clear();
		void Destroy();

		bool Wait();
		bool Release(int count = 1);

	private:
#if defined(_WIN32)
		void* m_sem;
#else
		sem_t m_sem;
#endif

};


#endif
