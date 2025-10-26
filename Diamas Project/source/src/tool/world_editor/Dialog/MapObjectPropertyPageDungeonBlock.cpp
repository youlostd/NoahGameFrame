#include "stdafx.h"
#include "..\worldeditor.h"
#include "MapObjectProperty.h"
#include "MapObjectPropertyPageDungeonBlock.h"

#include <GameLib/Property.h>

#include <EterBase/Utils.h>

#include <SpdLog.hpp>

METIN2_BEGIN_NS

CMapObjectPropertyPageDungeonBlock::CMapObjectPropertyPageDungeonBlock(CWnd* pParent /*=NULL*/)
	: CMapObjectPropertyPageBase(CMapObjectPropertyPageDungeonBlock::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapObjectPropertyPageDungeonBlock)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMapObjectPropertyPageDungeonBlock::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapObjectPropertyPageDungeonBlock)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapObjectPropertyPageDungeonBlock, CDialog)
	//{{AFX_MSG_MAP(CMapObjectPropertyPageDungeonBlock)
	ON_BN_CLICKED(IDC_MAP_OBJECT_PROPERTY_DUNGEON_BLOCK_LOAD, OnLoadDungeonBlockFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapObjectPropertyPageDungeonBlock normal functions

BOOL CMapObjectPropertyPageDungeonBlock::Create(CMapObjectProperty * pParent, const CRect & c_rRect)
{
	if (!CDialog::Create(CMapObjectPropertyPageDungeonBlock::IDD, (CWnd*)pParent))
		return FALSE;

	SetWindowPos(NULL, c_rRect.left, c_rRect.top, c_rRect.Width(), c_rRect.Height(), TRUE);
	ShowWindow(SW_HIDE);

	m_pParent = pParent;

	return TRUE;
}

void CMapObjectPropertyPageDungeonBlock::OnUpdateUI(CProperty * pProperty)
{
	const char * c_szPropertyType;
	const char * c_szPropertyName;
	if (!pProperty->GetString("PropertyType", &c_szPropertyType))
		return;
	if (!pProperty->GetString("PropertyName", &c_szPropertyName))
		return;
	if (PROPERTY_TYPE_DUNGEON_BLOCK != GetPropertyType(c_szPropertyType))
		return;

	const char * c_szBuildingFile;
	if (pProperty->GetString("DungeonBlockFile", &c_szBuildingFile))
	{
		SetDlgItemText(IDC_MAP_OBJECT_PROPERTY_DUNGEON_BLOCK_FILE, c_szBuildingFile);
	}

	OnUpdatePropertyData(c_szPropertyName);
}

void CMapObjectPropertyPageDungeonBlock::OnUpdatePropertyData(const char * c_szPropertyName)
{
	CString strDungeonBlockFileName;
	GetDlgItemText(IDC_MAP_OBJECT_PROPERTY_DUNGEON_BLOCK_FILE, strDungeonBlockFileName);

	m_propertyDungeonBlock.strName = c_szPropertyName;
	m_propertyDungeonBlock.strFileName = strDungeonBlockFileName;
}

void CMapObjectPropertyPageDungeonBlock::OnRender(HWND hWnd)
{
	m_Screen.Begin();
	m_Screen.SetClearColor(0.0f, 0.0f, 0.0f);
	m_Screen.Clear();

	// TODO : Please writing here code that renders preview

	m_Screen.Show(hWnd);
	m_Screen.End();
}

bool CMapObjectPropertyPageDungeonBlock::OnSave(const char * c_szPathName, CProperty * pProperty)
{
	storm::String strFileName = c_szPathName;
	strFileName += "/";
	strFileName += m_propertyDungeonBlock.strName;
	strFileName += c_szPropertyExtension[PROPERTY_TYPE_DUNGEON_BLOCK];

	if (m_propertyDungeonBlock.strName.empty())
	{
		spdlog::error("이름을 입력 하셔야 합니다.");
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

	PropertyDungeonBlockDataToString(&m_propertyDungeonBlock, pProperty);

	pProperty->Save();

	m_dwCRC = pProperty->GetCRC();
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CMapObjectPropertyPageDungeonBlock message handlers

void CMapObjectPropertyPageDungeonBlock::OnOK()
{
	m_pParent->OnOk();
}
void CMapObjectPropertyPageDungeonBlock::OnCancel()
{
	m_pParent->OnCancel();
}

void CMapObjectPropertyPageDungeonBlock::OnLoadDungeonBlockFile()
{
	uint32_t dwFlag = OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	const char * c_szFilter = "Granny File (*.gr2) |*.gr2|All Files (*.*)|*.*|";

	CFileDialog FileOpener(TRUE, "Load", "", dwFlag, c_szFilter, this);

	if (FileOpener.DoModal())
	{
		std::string strFullFileName;
		StringPath(FileOpener.GetPathName(), strFullFileName);

		SetDlgItemText(IDC_MAP_OBJECT_PROPERTY_DUNGEON_BLOCK_FILE, strFullFileName.c_str());
	}
}

METIN2_END_NS
