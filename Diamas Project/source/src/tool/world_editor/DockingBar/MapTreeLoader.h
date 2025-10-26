#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPTREELOADER_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_MAPTREELOADER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <EterBase/FileDir.h>

METIN2_BEGIN_NS

class CTreeLoader : public CDir
{
	public:
		CTreeLoader();
		~CTreeLoader();

		void SetTreeControler(CPropertyTreeControler * pPropertyTree);
		void SetParentItem(HTREEITEM hItem);

		void DownFolder(const char* c_szFilter, const char* c_szPathName);
		bool OnFolder(const char* c_szFilter, const char* c_szPathName, const char* c_szFileName);
		bool OnFile(const char* c_szPathName, const char* c_szFileName);

	protected:
		CPropertyTreeControler * m_pTreeControler;
		HTREEITEM	m_hParentItem;
		HTREEITEM	m_hNextItem;
};

METIN2_END_NS

#endif
