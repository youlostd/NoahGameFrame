#pragma once

#pragma warning(disable:4786)	// character 255 �Ѿ�°� ���

#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#include <vector>
#include <map>

#include "Stl.h"

class CMemoryTextFileLoader
{
	public:
		CMemoryTextFileLoader();
		virtual ~CMemoryTextFileLoader();

		void				Bind(std::string_view data);
		uint32_t				GetLineCount() const;
		bool				CheckLineIndex(uint32_t dwLine) const;
		bool				SplitLine(uint32_t dwLine, CTokenVector * pstTokenVector, const char * c_szDelimeter = " \t");
		int					SplitLine2(uint32_t dwLine, CTokenVector * pstTokenVector, const char * c_szDelimeter = " \t");
		bool				SplitLineByTab(uint32_t dwLine, CTokenVector* pstTokenVector);
		const std::string &	GetLineString(uint32_t dwLine);

	protected:
		std::vector<std::string> m_stLineVector;
};

class CMemoryFileLoader
{
	public:
		CMemoryFileLoader(int size, const void * c_pvMemoryFile);
		virtual ~CMemoryFileLoader();

		bool Read(int size, void* pvDst);

		int				GetPosition() const;		
		int				GetSize() const;
		
	protected:
		bool			IsReadableSize(int size) const;
		const char *	GetCurrentPositionPointer() const;

	protected:
		const char *	m_pcBase;
		int				m_size;
		int				m_pos;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
class CDiskFileLoader
{
	public:
		CDiskFileLoader();
		virtual ~CDiskFileLoader();

		void Close();
		bool Open(const char * c_szFileName);
		bool Read(int size, void * pvDst) const;

		int GetSize() const;

	protected:
		void Initialize();

	protected:
		FILE *	m_fp{};
		int		m_size{};
};

typedef std::map<std::string, std::string> TStringMap;