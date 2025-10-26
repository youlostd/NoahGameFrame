#include "stdafx.h"
#include "..\worldeditor.h"
#include "MapObjectProperty.h"
#include "MapObjectPropertyPageEffect.h"

#include <GameLib/Property.h>

#include <EterBase/Utils.h>

#include <SpdLog.hpp>

METIN2_BEGIN_NS

CMapObjectPropertyPageEffect::CMapObjectPropertyPageEffect(CWnd* pParent /*=NULL*/)
	: CMapObjectPropertyPageBase(CMapObjectPropertyPageEffect::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapObjectPropertyPageEffect)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMapObjectPropertyPageEffect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapObjectPropertyPageEffect)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapObjectPropertyPageEffect, CDialog)
	//{{AFX_MSG_MAP(CMapObjectPropertyPageEffect)
	ON_BN_CLICKED(IDC_MAP_OBJECT_PROPERTY_EFFECT_LOAD, OnLoadEffectFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapObjectPropertyPageEffect message handlers

BOOL CMapObjectPropertyPageEffect::Create(CMapObjectProperty * pParent, const CRect & c_rRect)
{
	if (!CDialog::Create(CMapObjectPropertyPageEffect::IDD, (CWnd*)pParent))
		return FALSE;

	SetWindowPos(NULL, c_rRect.left, c_rRect.top, c_rRect.Width(), c_rRect.Height(), TRUE);
	ShowWindow(SW_HIDE);

	m_pParent = pParent;

	m_propertyEffect.strName = "";
	m_propertyEffect.strFileName = "";
	return TRUE;
}

void CMapObjectPropertyPageEffect::OnUpdateUI(CProperty * pProperty)
{
	const char * c_szPropertyType;
	const char * c_szPropertyName;
	if (!pProperty->GetString("PropertyType", &c_szPropertyType))
		return;
	if (!pProperty->GetString("PropertyName", &c_szPropertyName))
		return;
	if (PROPERTY_TYPE_EFFECT != GetPropertyType(c_szPropertyType))
		return;

	const char * c_szEffectFile;
	if (pProperty->GetString("EffectFile",&c_szEffectFile))
	{
		SetDlgItemText(IDC_MAP_OBJECT_PROPERTY_EFFECT_FILE, c_szEffectFile);
	}

	OnUpdatePropertyData(c_szPropertyName);
}

void CMapObjectPropertyPageEffect::OnUpdatePropertyData(const char * c_szPropertyName)
{
	CString strEffectFileName;
	GetDlgItemText(IDC_MAP_OBJECT_PROPERTY_EFFECT_FILE, strEffectFileName);

	m_propertyEffect.strName = c_szPropertyName;
	m_propertyEffect.strFileName = storm::String(strEffectFileName);
}

bool CMapObjectPropertyPageEffect::OnSave(const char * c_szPathName, CProperty * pProperty)
{
	storm::String strFileName = c_szPathName;
	strFileName += "/";
	strFileName += m_propertyEffect.strName;
	strFileName += ".pre";

	if (m_propertyEffect.strName.empty())
	{
		spdlog::error("이름을 입력 하셔야 합니다.");
		return false;
	}

	if (m_propertyEffect.strFileName.empty())
	{
		spdlog::error("이펙트 파일이름을 입력 하셔야 합니다.");
		return false;
	}

	auto& mgr = CPropertyManager::Instance();
	if (!pProperty && !mgr.Get(strFileName, &pProperty)) {
		pProperty = new CProperty(strFileName);
		mgr.Put(pProperty);
	} else if (pProperty->GetFileName() != strFileName) {
		SPDLOG_WARN("Property path {0} != {1}",
		          pProperty->GetFileName(), strFileName);
	}

	PropertyEffectDataToString(&m_propertyEffect, pProperty);

	pProperty->Save();

	m_dwCRC = pProperty->GetCRC();
	return true;

}

void CMapObjectPropertyPageEffect::OnRender(HWND hWnd)
{
	m_Screen.Begin();
	m_Screen.SetClearColor(0.0f, 0.0f, 0.0f);
	m_Screen.Clear();

	// TODO : Please writing here code that renders preview

	m_Screen.Show(hWnd);
	m_Screen.End();
}

void CMapObjectPropertyPageEffect::OnOK()
{
	m_pParent->OnOk();
}
void CMapObjectPropertyPageEffect::OnCancel()
{
	m_pParent->OnCancel();
}

void CMapObjectPropertyPageEffect::OnLoadEffectFile()
{
	uint32_t dwFlag = OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	const char * c_szFilter = "Effect File (*.mse) |*.mse|All Files (*.*)|*.*|";

	CFileDialog FileOpener(TRUE, "Load", "", dwFlag, c_szFilter, this);

	if (FileOpener.DoModal())
	{
		std::string strFullFileName;
		StringPath(FileOpener.GetPathName(), strFullFileName);

		SetDlgItemText(IDC_MAP_OBJECT_PROPERTY_EFFECT_FILE, strFullFileName.c_str());
	}
}


METIN2_END_NS
