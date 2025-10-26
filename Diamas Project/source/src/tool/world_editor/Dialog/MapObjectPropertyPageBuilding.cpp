#include "stdafx.h"
#include "..\WorldEditor.h"
#include "MapObjectPropertyPageBuilding.h"
#include "MapObjectProperty.h"

#include <GameLib/Property.h>

#include <EterBase/Utils.h>

#include <SpdLog.hpp>

METIN2_BEGIN_NS

CMapObjectPropertyPageBuilding::CMapObjectPropertyPageBuilding(CWnd* pParent /*=NULL*/)
	: CMapObjectPropertyPageBase(CMapObjectPropertyPageBuilding::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapObjectPropertyPageBuilding)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMapObjectPropertyPageBuilding::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapObjectPropertyPageBuilding)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapObjectPropertyPageBuilding, CDialog)
	//{{AFX_MSG_MAP(CMapObjectPropertyPageBuilding)
	ON_BN_CLICKED(IDC_MAP_OBJECT_PROPERTY_BUILDING_LOAD, OnLoadBuildingFile)
	ON_BN_CLICKED(IDC_MAP_OBJECT_PROPERTY_BUILDING_SHADOW_FLAG, OnCheckShadowFlag)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapObjectPropertyPageBuilding normal functions

BOOL CMapObjectPropertyPageBuilding::Create(CMapObjectProperty * pParent, const CRect & c_rRect)
{
	if (!CDialog::Create(CMapObjectPropertyPageBuilding::IDD, (CWnd*)pParent))
		return FALSE;

	SetWindowPos(NULL, c_rRect.left, c_rRect.top, c_rRect.Width(), c_rRect.Height(), TRUE);
	ShowWindow(SW_HIDE);

	m_pParent = pParent;

	m_propertyBuilding.strName = "";
	m_propertyBuilding.strFileName = "";
	m_propertyBuilding.strAttributeDataFileName = "";
	m_propertyBuilding.isShadowFlag = TRUE;
	return TRUE;
}

void CMapObjectPropertyPageBuilding::OnUpdateUI(CProperty * pProperty)
{
	const char * c_szPropertyType;
	const char * c_szPropertyName;
	if (!pProperty->GetString("PropertyType", &c_szPropertyType))
		return;
	if (!pProperty->GetString("PropertyName", &c_szPropertyName))
		return;
	if (PROPERTY_TYPE_BUILDING != GetPropertyType(c_szPropertyType))
		return;

	const char * c_szBuildingFile;
	if (pProperty->GetString("BuildingFile", &c_szBuildingFile))
	{
		SetDlgItemText(IDC_MAP_OBJECT_PROPERTY_BUILDING_FILE, c_szBuildingFile);
	}

	const char * c_szShadowFlag;
	if (!pProperty->GetString("ShadowFlag", &c_szShadowFlag))
	{
		CheckDlgButton(IDC_MAP_OBJECT_PROPERTY_BUILDING_SHADOW_FLAG, TRUE);
	}
	else
	{
		if (atoi(c_szShadowFlag))
			CheckDlgButton(IDC_MAP_OBJECT_PROPERTY_BUILDING_SHADOW_FLAG, TRUE);
		else
			CheckDlgButton(IDC_MAP_OBJECT_PROPERTY_BUILDING_SHADOW_FLAG, FALSE);
	}

	OnUpdatePropertyData(c_szPropertyName);
}

void CMapObjectPropertyPageBuilding::OnUpdatePropertyData(const char * c_szPropertyName)
{
	CString strBuildingFileName;
	GetDlgItemText(IDC_MAP_OBJECT_PROPERTY_BUILDING_FILE, strBuildingFileName);
	storm::String file;
	m_propertyBuilding.strFileName = strBuildingFileName;
	m_propertyBuilding.strName = c_szPropertyName;
}

bool CMapObjectPropertyPageBuilding::OnSave(const char * c_szPathName, CProperty * pProperty)
{
	storm::String strFileName = c_szPathName;
	strFileName += "/";
	strFileName += m_propertyBuilding.strName;
	strFileName += ".prb";

	if (m_propertyBuilding.strName.empty())
	{
		spdlog::error("No name");
		return false;
	}

	if (m_propertyBuilding.strFileName.empty())
	{
		spdlog::error("No filename");
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

	PropertyBuildingDataToString(&m_propertyBuilding, pProperty);

	pProperty->Save();

	m_dwCRC = pProperty->GetCRC();
	return true;
}

void CMapObjectPropertyPageBuilding::OnRender(HWND hWnd)
{
	m_Screen.Begin();
	m_Screen.SetClearColor(0.0f, 0.0f, 0.0f);
	m_Screen.Clear();

	// TODO : Please writing here code that renders preview

	m_Screen.Show(hWnd);
	m_Screen.End();
}

/////////////////////////////////////////////////////////////////////////////
// CMapObjectPropertyPageBuilding message handlers

void CMapObjectPropertyPageBuilding::OnOK()
{
	m_pParent->OnOk();
}
void CMapObjectPropertyPageBuilding::OnCancel()
{
	m_pParent->OnCancel();
}

void CMapObjectPropertyPageBuilding::OnLoadBuildingFile()
{
	uint32_t dwFlag = OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	const char * c_szFilter = "Granny File (*.gr2) |*.gr2|All Files (*.*)|*.*|";

	CFileDialog FileOpener(TRUE, "Load", "", dwFlag, c_szFilter, this);

	if (FileOpener.DoModal())
	{
		std::string strFullFileName;
		StringPath(FileOpener.GetPathName(), strFullFileName);

		SetDlgItemText(IDC_MAP_OBJECT_PROPERTY_BUILDING_FILE, strFullFileName.c_str());
	}
}

void CMapObjectPropertyPageBuilding::OnCheckShadowFlag()
{
	m_propertyBuilding.isShadowFlag = IsDlgButtonChecked(IDC_MAP_OBJECT_PROPERTY_BUILDING_SHADOW_FLAG);
}

METIN2_END_NS
