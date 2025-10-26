#include "stdafx.h"
#include "EffectTabPage.h"

#include "../WorldEditor.h"
#include "../MainFrm.h"
#include "../Dialog/EffectBoundingSphere.h"

#include <EterBase/Filename.h>
#include <EterBase/Utils.h>

#include <storm/WindowsPlatform.hpp>
#include <fmt/ostream.h>
#include <SpdLog.hpp>


#include <fmt/format.h>


METIN2_BEGIN_NS

CEffectTabPage::CEffectTabPage(CWnd* pParent /*=NULL*/)
	: CPageCtrl(CEffectTabPage::IDD, pParent)
{
	m_iSelectedIndex = 0;
	m_strFileName = "";

	//{{AFX_DATA_INIT(CEffectTabPage)
	m_bUsingSceneObject = FALSE;
	m_bShowCursor = TRUE;
	m_bShowCharacter = TRUE;
	m_bShowBoundingSphere = TRUE;
	//}}AFX_DATA_INIT
}


void CEffectTabPage::DoDataExchange(CDataExchange* pDX)
{
	CPageCtrl::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEffectTabPage)
	DDX_Control(pDX, IDC_EFFECT_LIST, m_ctrlEffectList);
	DDX_Check(pDX, IDC_EFFECT_USING_SCENEOBJECT, m_bUsingSceneObject);
	DDX_Check(pDX, IDC_EFFECT_SHOW_CURSOR, m_bShowCursor);
	DDX_Check(pDX, IDC_EFFECT_SHOW_CHARACTER, m_bShowCharacter);
	DDX_Check(pDX, IDC_EFFECT_SHOW_BOUDNING_SPHERE, m_bShowBoundingSphere);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEffectTabPage, CPageCtrl)
	//{{AFX_MSG_MAP(CEffectTabPage)
	ON_WM_CREATE()
	ON_LBN_SELCHANGE(IDC_EFFECT_LIST, OnChangeEffectList)
	ON_BN_CLICKED(IDC_EFFECT_TAB_CREATE_PARTICLE, OnCreateParticle)
	ON_BN_CLICKED(IDC_EFFECT_TAB_CREATE_MESH, OnCreateMesh)
	ON_BN_CLICKED(IDC_EFFECT_LOAD_SCRIPT, OnLoadScript)
	ON_BN_CLICKED(IDC_EFFECT_SAVE_SCRIPT, OnSaveScript)
	ON_BN_CLICKED(IDC_EFFECT_CLEAR_ONE, OnClearEffectElementOne)
	ON_BN_CLICKED(IDC_EFFECT_CLEAR_ALL, OnClearEffectElementAll)
	ON_BN_CLICKED(IDC_EFFECT_TAB_TIME_BAR_ENABLE, OnTimeBarEnable)
	ON_BN_CLICKED(IDC_EFFECT_TAB_CREATE_LIGHT, OnCreateLight)
	ON_BN_CLICKED(IDC_EFFECT_DUPLICATE, OnEffectDuplicate)
	ON_BN_CLICKED(IDC_EFFECT_MERGE_SCRIPT, OnEffectMergeScript)
	ON_BN_CLICKED(IDC_EFFECT_SHOW_CURSOR, OnEffectShowCursor)
	ON_BN_CLICKED(IDC_EFFECT_SHOW_CHARACTER, OnEffectShowCharacter)
	ON_BN_CLICKED(IDC_EFFECT_SHOW_BOUDNING_SPHERE, OnEffectShowBoundingSphere)
	ON_BN_CLICKED(IDC_EFFECT_SHOW, OnShow)
	ON_BN_CLICKED(IDC_EFFECT_HIDE, OnHide)
	ON_BN_CLICKED(IDC_EFFECT_SHOW_ALL, OnShowAll)
	ON_BN_CLICKED(IDC_EFFECT_HIDE_ALL, OnHideAll)
	ON_BN_CLICKED(IDC_EFFECT_SAVE_AS_SCRIPT, OnSaveAsScript)
	ON_BN_CLICKED(IDC_EFFECT_TAB_BOUNDING_SPHERE, OnOpenBoundSphereDialog)
	ON_BN_CLICKED(IDC_EFFECT_MOVE_UP, OnEffectMoveUp)
	ON_BN_CLICKED(IDC_EFFECT_MOVE_DOWN, OnEffectMoveDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEffectTabPage message handlers

BOOL CEffectTabPage::Create(CWnd * pParent)
{
	if (!CPageCtrl::Create(CEffectTabPage::IDD, pParent))
		return FALSE;

	CreateBitmapButton((CButton*)GetDlgItem(IDC_EFFECT_TAB_CREATE_PARTICLE), IDB_EFFECT_PARTICLE, m_BitmapParticle);
	CreateBitmapButton((CButton*)GetDlgItem(IDC_EFFECT_TAB_CREATE_MESH), IDB_EFFECT_MESH, m_BitmapMesh);
	CreateBitmapButton((CButton*)GetDlgItem(IDC_EFFECT_TAB_CREATE_LIGHT), IDB_EFFECT_LIGHT, m_BitmapLight);

	return TRUE;
}

void CEffectTabPage::UpdateUI()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	uint32_t dwOldCount = m_ctrlEffectList.GetCount();
	for (uint32_t j = 0; j < dwOldCount; ++j)
		m_ctrlEffectList.DeleteString(0);

	char szName[32+1];
	uint32_t iCount = pEffectAccessor->GetElementCount();
	for (uint32_t i = 0; i < iCount; ++i)
	{
		CEffectAccessor::TEffectElement * pElement;
		if (!pEffectAccessor->GetElement(i, &pElement))
			continue;

		if (pElement->bVisible)
			_snprintf(szName, 32, "[%02d] [SHOW] %s", i+1, pElement->strName.c_str());
		else
			_snprintf(szName, 32, "[%02d] [HIDE] %s", i+1, pElement->strName.c_str());

		m_ctrlEffectList.InsertString(i, szName);

		if (i == m_iSelectedIndex)
			m_ctrlEffectList.SelectString(-1, szName);
	}
}

void CEffectTabPage::UpdateUIWithNewItem()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();
	m_iSelectedIndex = pEffectAccessor->GetElementCount()-1;
	UpdateUI();
}

void CEffectTabPage::Initialize()
{
	UpdateUI();
}

int CEffectTabPage::GetCurrentTab()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	uint32_t dwSelectedIndex = GetSelectedIndex();

	CEffectAccessor::TEffectElement * pElement;
	if (!pEffectAccessor->GetElement(dwSelectedIndex, &pElement))
		return PAGE_TYPE_NONE;

	return pElement->iType;
}

int CEffectTabPage::GetSelectedIndex()
{
	static int lastSelect=-2;
	if (lastSelect!=m_ctrlEffectList.GetCurSel())
	{
		lastSelect = m_ctrlEffectList.GetCurSel();
		CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
		CSceneEffect * pSceneEffect = pApplication->GetSceneEffect();
		pSceneEffect->SelectEffectElement(m_ctrlEffectList.GetCurSel());
		OnChangeEffectList();
	}
	return m_ctrlEffectList.GetCurSel();
}

void CEffectTabPage::OnLoadScript(const char * c_szFileName)
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	pEffectAccessor->Clear();
	pEffectAccessor->LoadScript(c_szFileName);
	UpdateUI();

	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	pFrame->UpdateEffectControlBar();

	//pFrame->PlayEffect();
}

void CEffectTabPage::OnSaveScript(const char * c_szFileName)
{


	storm::File File;

	bsys::error_code ec;
	File.Open(c_szFileName, ec,
	          storm::AccessMode::kWrite,
	          storm::CreationDisposition::kCreateAlways,
	          storm::ShareMode::kNone,
	          storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          c_szFileName, ec);
		return;
	}

	// Getting PathName
	std::string strGlobalPathName;
	StringPath(GetOnlyPathName(c_szFileName), strGlobalPathName);
	// Getting PathName

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	Vector3 & c_rv3BoundingSpherePosition = pEffectAccessor->GetBoundingSpherePosition();
	PrintfTabs(File, 0, "BoundingSphereRadius   %f\n", pEffectAccessor->GetBoundingSphereRadius());
	PrintfTabs(File, 0, "BoundingSpherePosition %f %f %f\n", c_rv3BoundingSpherePosition.x, c_rv3BoundingSpherePosition.y, c_rv3BoundingSpherePosition.z);
	PrintfTabs(File, 0, "\n");

	for (uint32_t i = 0; i < pEffectAccessor->GetElementCount(); ++i)
	{
		CEffectAccessor::TEffectElement * pElement;
		if (!pEffectAccessor->GetElement(i, &pElement))
			continue;

		switch (pElement->iType)
		{
			case CEffectAccessor::EFFECT_ELEMENT_TYPE_PARTICLE:
				PrintfTabs(File, 0, "Group Particle\n");
				PrintfTabs(File, 0, "{\n");
				pElement->pBase->SaveScript(1, File);
				pElement->pParticle->SaveScript(1, File, strGlobalPathName.c_str());
				PrintfTabs(File, 0, "}\n");
				break;

			case CEffectAccessor::EFFECT_ELEMENT_TYPE_MESH:
				PrintfTabs(File, 0, "Group Mesh\n");
				PrintfTabs(File, 0, "{\n");
				pElement->pBase->SaveScript(1, File);
				pElement->pMesh->SaveScript(1, File, strGlobalPathName.c_str());
				PrintfTabs(File, 0, "}\n");
				break;

			case CEffectAccessor::EFFECT_ELEMENT_TYPE_LIGHT:
				PrintfTabs(File, 0, "Group Light\n");
				PrintfTabs(File, 0, "{\n");
				pElement->pBase->SaveScript(1, File);
				pElement->pLight->SaveScript(1, File);
				PrintfTabs(File, 0, "}\n");
				break;
		}
	}
}

BOOL CEffectTabPage::IsOpenTimeBar()
{
	return IsDlgButtonChecked(IDC_EFFECT_TAB_TIME_BAR_ENABLE);
}

BOOL CEffectTabPage::IsOpenBoundingSphereDialog()
{
	return IsDlgButtonChecked(IDC_EFFECT_TAB_BOUNDING_SPHERE);
}

/////////////////////////////////////////////////////////////////////////////
// CEffectTabPage message handlers

int CEffectTabPage::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPageCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CEffectTabPage::OnChangeEffectList()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	pFrame->UpdateEffectControlBar();

	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CSceneEffect * pSceneEffect = pApplication->GetSceneEffect();
	pSceneEffect->RefreshTranslationDialog();

	m_iSelectedIndex = GetSelectedIndex();
}

void CEffectTabPage::OnCreateParticle()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	int iIndex = pEffectAccessor->GetElementCount();
	CParticleAccessor * pParticle = (CParticleAccessor *)pEffectAccessor->AllocParticle();
	pParticle->Clear();
	pParticle->SetDefaultData();

	OnChangeEffectList();

	CheckDlgButton(IDC_EFFECT_TAB_CREATE_PARTICLE, FALSE);

	UpdateUIWithNewItem();
}

void CEffectTabPage::OnCreateMesh()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	int iIndex = pEffectAccessor->GetElementCount();
	CMeshAccessor * pMesh = (CMeshAccessor *)pEffectAccessor->AllocMesh();
	pMesh->Clear();

	OnChangeEffectList();

	CheckDlgButton(IDC_EFFECT_TAB_CREATE_MESH, FALSE);

	UpdateUIWithNewItem();
}

void CEffectTabPage::OnCreateLight()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	int iIndex = pEffectAccessor->GetElementCount();

	CLightAccessor * pLight = (CLightAccessor *)pEffectAccessor->AllocLight();
	pLight->Clear();

	OnChangeEffectList();

	CheckDlgButton(IDC_EFFECT_TAB_CREATE_LIGHT, FALSE);

	UpdateUIWithNewItem();
}

void CEffectTabPage::OnTimeBarEnable()
{
	OnChangeEffectList();
}

void CEffectTabPage::OnLoadScript()
{
	uint32_t dwFlag = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	const char * c_szFilter = "Metin2 Effect Script Files (*.mse)|*.mse|All Files (*.*)|*.*|";
	CFileDialog FileOpener(TRUE, "Load", "", dwFlag, c_szFilter, this);

	if (TRUE == FileOpener.DoModal())
	{
		// NOTE : Load 하기전에 Play 중인 Effect를 Stop
		CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
		CSceneEffect * pSceneEffect = pApplication->GetSceneEffect();
		pSceneEffect->Stop();

		/////

		SetFileName(FileOpener.GetPathName().GetString());
	}
}

void CEffectTabPage::OnSaveScript()
{
	if (m_strFileName.empty())
	{
		OnSaveAsScript();
		return;
	}

	OnSaveScript(m_strFileName.c_str());
}

void CEffectTabPage::SetFileName(const char * c_szFileName)
{
	m_strFileName = c_szFileName;

	OnLoadScript(m_strFileName.c_str());
	SetDlgItemText(IDC_EFFECT_SCRIPT_NAME, CFileNameHelper::NoPath(m_strFileName).c_str());
	//SetDlgItemText(IDC_EFFECT_SCRIPT_NAME, m_strFileName.NoPath().c_str());
}

void CEffectTabPage::OnSaveAsScript()
{
	uint32_t dwFlag = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	const char * c_szFilter = "Metin2 Effect Script Files (*.mse)|*.mse|All Files (*.*)|*.*|";

	CFileDialog FileOpener(FALSE, "mse", NULL, dwFlag, c_szFilter, this);

	if (TRUE == FileOpener.DoModal())
	{
		OnSaveScript(FileOpener.GetPathName());
		SetFileName(FileOpener.GetPathName());
	}
}

void CEffectTabPage::OnClearEffectElementOne()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CSceneEffect * pSceneEffect = pApplication->GetSceneEffect();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	pSceneEffect->Stop();

	pEffectAccessor->DeleteElement(GetSelectedIndex());
	UpdateUI();

	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	pFrame->UpdateEffectControlBar();
}

void CEffectTabPage::OnClearEffectElementAll()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CSceneEffect * pSceneEffect = pApplication->GetSceneEffect();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	pSceneEffect->Stop();

	pEffectAccessor->Clear();
	UpdateUI();

	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	pFrame->UpdateEffectControlBar();
}


void CEffectTabPage::OnEffectDuplicate()
{
	if (GetSelectedIndex()<0 || GetSelectedIndex()>= m_ctrlEffectList.GetCount())
		return;
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CSceneEffect * pSceneEffect = pApplication->GetSceneEffect();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	CEffectAccessor::TEffectElement * pEffectElement, effectelement;
	pEffectAccessor->GetElement(GetSelectedIndex(), &pEffectElement);
	effectelement = *pEffectElement;

	int iIndex = pEffectAccessor->GetElementCount();

	char szName[32+1];

	switch(pEffectElement->iType)
	{
		case CEffectAccessor::EFFECT_ELEMENT_TYPE_PARTICLE:
			{
				CParticleAccessor * pParticleAccessor = (CParticleAccessor *) pEffectAccessor->AllocParticle();
				*pParticleAccessor = *(effectelement.pParticle);
				_snprintf(szName, 32, "Index %d : Particle", iIndex);
			}
			break;
		case CEffectAccessor::EFFECT_ELEMENT_TYPE_MESH:
			{
				CMeshAccessor * pMeshAccessor = (CMeshAccessor *) pEffectAccessor->AllocMesh();
				*pMeshAccessor = *(effectelement.pMesh);
				_snprintf(szName, 32, "Index %d : Mesh", iIndex);
			}
			break;
		case CEffectAccessor::EFFECT_ELEMENT_TYPE_LIGHT:
			{
				CLightAccessor * pLightAccessor = (CLightAccessor *) pEffectAccessor->AllocLight();
				*pLightAccessor = *(effectelement.pLight);
				_snprintf(szName, 32, "Index %d : Light", iIndex);
			}
			break;
	}

	m_ctrlEffectList.InsertString(iIndex, szName);
	m_ctrlEffectList.SelectString(-1, szName);
	OnChangeEffectList();
}

void CEffectTabPage::OnEffectMergeScript()
{
	uint32_t dwFlag = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	const char * c_szFilter = "Metin2 Effect Script Files (*.mse)|*.mse|All Files (*.*)|*.*|";
	CFileDialog FileOpener(TRUE, "Load", "", dwFlag, c_szFilter, this);

	if (TRUE == FileOpener.DoModal())
	{
		// NOTE : Load 하기전에 Play 중인 Effect를 Stop
		CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
		CSceneEffect * pSceneEffect = pApplication->GetSceneEffect();
		pSceneEffect->Stop();

		/////

		CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

		pEffectAccessor->LoadScript(FileOpener.GetPathName());
		UpdateUI();

		CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
		pFrame->UpdateEffectControlBar();
	}
}


void CEffectTabPage::OnEffectUsingSceneobject()
{
	UpdateData();
	((CWorldEditorApp * )AfxGetApp())->GetSceneEffect()->UsingSceneObject(m_bUsingSceneObject);
}

void CEffectTabPage::OnEffectShowCursor()
{
	UpdateData();
	((CWorldEditorApp * )AfxGetApp())->GetSceneEffect()->SetCursorVisibility(m_bShowCursor);
}

void CEffectTabPage::OnEffectShowCharacter()
{
	UpdateData();
	((CWorldEditorApp * )AfxGetApp())->GetSceneEffect()->SetCharacterVisibility(m_bShowCharacter);
}

void CEffectTabPage::OnEffectShowBoundingSphere()
{
	UpdateData();
	((CWorldEditorApp * )AfxGetApp())->GetSceneEffect()->SetBoundingSphereVisibility(m_bShowBoundingSphere);
}

void CEffectTabPage::OnShow()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CSceneEffect * pSceneEffect = pApplication->GetSceneEffect();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	pEffectAccessor->SetVisible(GetSelectedIndex(), TRUE);
	UpdateUI();
}

void CEffectTabPage::OnHide()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CSceneEffect * pSceneEffect = pApplication->GetSceneEffect();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	pEffectAccessor->SetVisible(GetSelectedIndex(), FALSE);
	UpdateUI();
}

void CEffectTabPage::OnShowAll()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CSceneEffect * pSceneEffect = pApplication->GetSceneEffect();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	for (int i = 0; i < pEffectAccessor->GetElementCount(); ++i)
		pEffectAccessor->SetVisible(i, TRUE);
	UpdateUI();
}

void CEffectTabPage::OnHideAll()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CSceneEffect * pSceneEffect = pApplication->GetSceneEffect();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	for (int i = 0; i < pEffectAccessor->GetElementCount(); ++i)
		pEffectAccessor->SetVisible(i, FALSE);
	UpdateUI();
}

void CEffectTabPage::OnLoadEnvironment()
{
	uint32_t dwFlag = OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	const char * c_szFilter = "Metin2 Environment Script Files (*.msenv)|*.msenv|All Files (*.*)|*.*|";
	COpenFileDialog FileOpener(this, dwFlag, NULL, c_szFilter, "D:\\Ymir Work\\environment");

	if (TRUE == FileOpener.DoModal())
	{
		// TODO(tim): hmm
	}
}

void CEffectTabPage::OnOpenBoundSphereDialog()
{
	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	pFrame->UpdateEffectControlBar();
}

void CEffectTabPage::OnEffectMoveUp()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	uint32_t dwSelectedIndex = GetSelectedIndex();

	if (dwSelectedIndex > 0)
	{
		pEffectAccessor->SwapElement(dwSelectedIndex-1, dwSelectedIndex);
		m_iSelectedIndex = dwSelectedIndex-1;
		UpdateUI();
	}
}

void CEffectTabPage::OnEffectMoveDown()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CEffectAccessor * pEffectAccessor = pApplication->GetEffectAccessor();

	uint32_t dwSelectedIndex = GetSelectedIndex();

	if (dwSelectedIndex+1 < pEffectAccessor->GetElementCount())
	{
		pEffectAccessor->SwapElement(dwSelectedIndex, dwSelectedIndex+1);
		m_iSelectedIndex = dwSelectedIndex+1;
		UpdateUI();
	}
}


METIN2_END_NS
