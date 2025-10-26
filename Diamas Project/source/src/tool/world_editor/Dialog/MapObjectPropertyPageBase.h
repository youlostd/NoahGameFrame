#ifndef METIN2_TOOL_WORLDEDITOR_DIALOG_MAPOBJECTPROPERTYPAGEBASE_HPP
#define METIN2_TOOL_WORLDEDITOR_DIALOG_MAPOBJECTPROPERTYPAGEBASE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CMapObjectPropertyPageBase : public CDialog
{
	public:
		CMapObjectPropertyPageBase(UINT nIDTemplate, CWnd* pParentWnd = NULL);
		virtual ~CMapObjectPropertyPageBase();

		void UpdateUI(CProperty * pProperty);
		void UpdatePropertyData(const char * c_szPropertyName);
		bool Save(const char * c_szPathName, CProperty * pProperty);
		void Render(HWND hWnd);

		uint32_t GetPropertyCRC32();

	protected:
		virtual void OnUpdateUI(CProperty * pProperty) = 0;
		virtual void OnUpdatePropertyData(const char * c_szPropertyName) = 0;
		virtual void OnRender(HWND hWnd) = 0;
		virtual bool OnSave(const char * c_szPathName, CProperty * pProperty) = 0;

	protected:
		uint32_t m_dwCRC;
};

METIN2_END_NS

#endif
