#ifndef __INC_ETERBASE_FILEBASE_H__
#define __INC_ETERBASE_FILEBASE_H__

#include <string>

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <WinSock2.h>
#include <Windows.h>

class CFileBase
{
	public:
		enum EFileMode
		{
			FILEMODE_READ = (1 << 0),
			FILEMODE_WRITE = (1 << 1)
		};

		CFileBase();
		virtual	~CFileBase();

		void Destroy();
		void Close();
		
		bool Create(const std::string& filename, EFileMode mode);
		uint32_t Size() const;
		void SeekCur(uint32_t size);
		void Seek(uint32_t offset);
		uint32_t GetPosition() const;

		bool Write(const void* src, uint32_t bytes);
		bool Read(void* dest, uint32_t bytes) const;

		const std::string& GetFileName() const;
		
	protected:
		int m_mode;
		std::string m_filename;
		HANDLE m_hFile;
		uint32_t m_dwSize;
};

#endif
