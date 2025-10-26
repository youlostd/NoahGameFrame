#include "StdAfx.h"
#include "MapObjectPropertyPageBase.h"

METIN2_BEGIN_NS

void CMapObjectPropertyPageBase::UpdateUI(CProperty * pProperty)
{
	OnUpdateUI(pProperty);
}

void CMapObjectPropertyPageBase::UpdatePropertyData(const char * c_szPropertyName)
{
	OnUpdatePropertyData(c_szPropertyName);
}

bool CMapObjectPropertyPageBase::Save(const char * c_szPathName, CProperty * pProperty)
{
	return OnSave(c_szPathName, pProperty);
}

void CMapObjectPropertyPageBase::Render(HWND hWnd)
{
	OnRender(hWnd);
}

uint32_t CMapObjectPropertyPageBase::GetPropertyCRC32()
{
	return m_dwCRC;
}

CMapObjectPropertyPageBase::CMapObjectPropertyPageBase(UINT nIDTemplate, CWnd* pParentWnd)
	: CDialog(nIDTemplate, pParentWnd)
{
	m_dwCRC = 0;
}

CMapObjectPropertyPageBase::~CMapObjectPropertyPageBase()
{
}

METIN2_END_NS
