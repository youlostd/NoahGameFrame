#include "stdafx.h"
#include "..\WorldEditor.h"
#include "MapEnvironmentPage.h"

#include "MainFrm.h"
#include "WorldEditorDoc.h"

#include "../DataCtrl/MapAccessorOutdoor.h"

#include <GameLib/EnvironmentData.hpp>

#include <EterBase/Utils.h>

METIN2_BEGIN_NS

BOOL CMapEnvironmentPage::ms_isCallBackFlag = TRUE;
CMapEnvironmentPage * CMapEnvironmentPage::ms_pThis = NULL;

CMapOutdoorAccessor* GetMap()
{
	auto app = (CWorldEditorApp *)AfxGetApp();
	const auto mgr = app->GetMapManagerAccessor();
	if (!mgr)
		return nullptr;

	return mgr->GetMapOutdoorPtr();
}

/////////////////////////////////////////////////////////////////////////////
// CMapEnvironmentPage dialog

CMapEnvironmentPage::CMapEnvironmentPage(CWnd* pParent /*=NULL*/)
	: CPageCtrl(CMapEnvironmentPage::IDD, pParent)
{
	ms_pThis = this;
	ms_isCallBackFlag = TRUE;
	//{{AFX_DATA_INIT(CMapEnvironmentPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CMapEnvironmentPage::DoDataExchange(CDataExchange* pDX)
{
	CPageCtrl::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapEnvironmentPage)
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_LENSFLARE_MAIN_FLARE_SIZE, m_ctrlMainFlareSize);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_LENSFLARE_MAX_BRIGHTNESS, m_ctrlLensFlareMaxBrightness);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_LENSFLARE_COLOR, m_ctrlLensFlareBrightnessColor);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_GRADIENT_SECOND_COLOR, m_ctrlSkyBoxGradientSecondColor);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_GRADIENT_FIRST_COLOR, m_ctrlSkyBoxGradientFirstColor);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_GRADIENT_LIST, m_ctrlSkyBoxGradientList);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_FILTERING_ALPHA_DEST, m_ctrlFilteringAlphaDest);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_FILTERING_ALPHA_SRC, m_ctrlFilteringAlphaSrc);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_FILTERING_ALPHA, m_ctrlFilteringAlpha);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_FILTERING_COLOR, m_ctrlFilteringColor);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_WIND_RANDOM, m_ctrlWindRandom);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_WIND_STRENGTH, m_ctrlWindStrength);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_FOG_NEAR_DISTANCE, m_ctrlFogNearDistance);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_FOG_FAR_DISTANCE, m_ctrlFogFarDistance);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_MATERIAL_EMISSIVE, m_ctrlMaterialEmissive);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_LIGHT_AMBIENT, m_ctrlLightAmbient);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_LIGHT_DIFFUSE, m_ctrlLightDiffuse);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_MATERIAL_AMBIENT, m_ctrlMaterialAmbient);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_MATERIAL_DIFFUSE, m_ctrlMaterialDiffuse);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_FOG_COLOR, m_ctrlFogColor);
	DDX_Control(pDX, IDC_MAP_ENVIRONMENT_SCRIPT_NAME_PRINT, m_EnviromentScriptName);
	//}}AFX_DATA_MAP

	m_ctrlWindStrength.SetRangeMin(0);
	m_ctrlWindStrength.SetRangeMax(200);
	m_ctrlWindStrength.SetPos(0);
	m_ctrlWindRandom.SetRangeMin(0);
	m_ctrlWindRandom.SetRangeMax(100);
	m_ctrlWindRandom.SetPos(0);
	m_ctrlFilteringAlpha.SetRangeMin(0);
	m_ctrlFilteringAlpha.SetRangeMax(100);
	m_ctrlFilteringAlpha.SetPos(0);
	m_ctrlLensFlareMaxBrightness.SetRangeMin(0);
	m_ctrlLensFlareMaxBrightness.SetRangeMax(100);
	m_ctrlMainFlareSize.SetRangeMin(0);
	m_ctrlMainFlareSize.SetRangeMax(100);
}

BEGIN_MESSAGE_MAP(CMapEnvironmentPage, CPageCtrl)
	//{{AFX_MSG_MAP(CMapEnvironmentPage)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_FOG_ENABLE, OnEnableFog)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LIGHT_ENABLE, OnEnableLight)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_FILTERING_ENABLE, OnEnableFiltering)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LOAD_SCRIPT, OnLoadEnvironmentScript)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_SAVE_SCRIPT, OnSaveEnvironmentScript)
	ON_BN_CLICKED(IDC_CHECK_LIGHTPOSITIONEDIT, OnCheckLightpositionEdit)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LOAD_CLOUD_TEXTURE, OnLoadCloudTexture)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_INSERT_UPPER_GRADIENT, OnInsertGradientUpper)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_INSERT_LOWER_GRADIENT, OnOnInsertGradientLower)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_DELETE_GRADIENT, OnDeleteGradient)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LENSFLARE_ENABLE, OnCheckLensFlareEnable)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LOAD_LENSFLARE_TEXTURE, OnLoadLensFlareTexture)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_MAINFLARE_ENABLE, OnCheckMainFlareEnable)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LENSFLARE_ENABLE2, &CMapEnvironmentPage::OnBnClickedSkyBoxModeTexture)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LOAD_CLOUD_TEXTURE2, &CMapEnvironmentPage::OnSetSkyBoxFrontTexture)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LOAD_CLOUD_TEXTURE4, &CMapEnvironmentPage::OnSetSkyBoxBackTexture)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LOAD_CLOUD_TEXTURE5, &CMapEnvironmentPage::OnSetSkyBoxLeftTexture)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LOAD_CLOUD_TEXTURE6, &CMapEnvironmentPage::OnSetSkyBoxRightTexture)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LOAD_CLOUD_TEXTURE3, &CMapEnvironmentPage::OnSetSkyBoxTopTexture)
	ON_BN_CLICKED(IDC_MAP_ENVIRONMENT_LOAD_CLOUD_TEXTURE8, &CMapEnvironmentPage::OnSetSkyBoxBottomTexture)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapEnvironmentPage normal functions

BOOL CMapEnvironmentPage::Create(CWnd * pParent)
{
	if (!CPageCtrl::Create(CMapEnvironmentPage::IDD, pParent))
		return FALSE;

	if (!m_ctrlMaterialDiffuse.Create(this))
		return FALSE;
	if (!m_ctrlMaterialAmbient.Create(this))
		return FALSE;
	if (!m_ctrlMaterialEmissive.Create(this))
		return FALSE;
	if (!m_ctrlLightDiffuse.Create(this))
		return FALSE;
	if (!m_ctrlLightAmbient.Create(this))
		return FALSE;
	if (!m_ctrlFogColor.Create(this))
		return FALSE;
	if (!m_ctrlFilteringColor.Create(this))
		return FALSE;
	if (!m_ctrlFilteringAlphaDest.Create())
		return FALSE;
	if (!m_ctrlFilteringAlphaSrc.Create())
		return FALSE;
	if (!m_ctrlSkyBoxGradientFirstColor.Create(this))
		return FALSE;
	if (!m_ctrlSkyBoxGradientSecondColor.Create(this))
		return FALSE;
	if (!m_ctrlLensFlareBrightnessColor.Create(this))
		return FALSE;

	m_ctrlMaterialDiffuse.pfnCallBack = CallBack;
	m_ctrlMaterialAmbient.pfnCallBack = CallBack;
	m_ctrlMaterialEmissive.pfnCallBack = CallBack;
	m_ctrlLightDiffuse.pfnCallBack = CallBack;
	m_ctrlLightAmbient.pfnCallBack = CallBack;
	m_ctrlFogColor.pfnCallBack = CallBack;
	m_ctrlFilteringColor.pfnCallBack = CallBack;
	m_ctrlSkyBoxGradientFirstColor.pfnCallBack = CallBack;
	m_ctrlSkyBoxGradientSecondColor.pfnCallBack = CallBack;
	m_ctrlLensFlareBrightnessColor.pfnCallBack = CallBack;

	m_iSliderIndex = 0;
	return TRUE;
}

void CMapEnvironmentPage::Initialize()
{
	UpdateUI();
}

void CMapEnvironmentPage::UpdateUI()
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	TurnOffCallBack();

	COLORREF MaterialDiffuseColor = RGB(WORD(data->Material.Diffuse.r*255.0f),
	                                    WORD(data->Material.Diffuse.g*255.0f),
	                                    WORD(data->Material.Diffuse.b*255.0f));
	COLORREF MaterialAmbientColor = RGB(WORD(data->Material.Ambient.r*255.0f),
	                                    WORD(data->Material.Ambient.g*255.0f),
	                                    WORD(data->Material.Ambient.b*255.0f));
	COLORREF MaterialEmissiveColor = RGB(WORD(data->Material.Emissive.r*255.0f),
	                                     WORD(data->Material.Emissive.g*255.0f),
	                                     WORD(data->Material.Emissive.b*255.0f));
	COLORREF LightDiffuseColor = RGB(WORD(data->DirLights[ENV_DIRLIGHT_BACKGROUND].Diffuse.r*255.0f),
	                                 WORD(data->DirLights[ENV_DIRLIGHT_BACKGROUND].Diffuse.g*255.0f),
	                                 WORD(data->DirLights[ENV_DIRLIGHT_BACKGROUND].Diffuse.b*255.0f));
	COLORREF LightAmbientColor = RGB(WORD(data->DirLights[ENV_DIRLIGHT_BACKGROUND].Ambient.r*255.0f),
	                                 WORD(data->DirLights[ENV_DIRLIGHT_BACKGROUND].Ambient.g*255.0f),
	                                 WORD(data->DirLights[ENV_DIRLIGHT_BACKGROUND].Ambient.b*255.0f));
	COLORREF FogColor = RGB(WORD(data->FogColor.r*255.0f),
	                        WORD(data->FogColor.g*255.0f),
	                        WORD(data->FogColor.b*255.0f));
	COLORREF FilteringColor = RGB(WORD(data->FilteringColor.r*255.0f),
	                              WORD(data->FilteringColor.g*255.0f),
	                              WORD(data->FilteringColor.b*255.0f));

	m_ctrlMaterialDiffuse.SetColor(MaterialDiffuseColor);
	m_ctrlMaterialAmbient.SetColor(MaterialAmbientColor);
	m_ctrlMaterialEmissive.SetColor(MaterialEmissiveColor);
	m_ctrlLightDiffuse.SetColor(LightDiffuseColor);
	m_ctrlLightAmbient.SetColor(LightAmbientColor);
	m_ctrlFogColor.SetColor(FogColor);
	m_ctrlFilteringColor.SetColor(FilteringColor);
	m_ctrlMaterialDiffuse.Update();
	m_ctrlMaterialAmbient.Update();
	m_ctrlMaterialEmissive.Update();
	m_ctrlLightDiffuse.Update();
	m_ctrlLightAmbient.Update();
	m_ctrlFogColor.Update();
	m_ctrlFilteringColor.Update();
	m_ctrlWindStrength.SetPos(long(data->fWindStrength));
	m_ctrlWindRandom.SetPos(long(data->fWindRandom));
	m_ctrlFilteringAlpha.SetPos(long(data->FilteringColor.a * 100.0f));

	m_ctrlFogNearDistance.SetLimitText(16);
	m_ctrlFogFarDistance.SetLimitText(16);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_FOG_FAR_DISTANCE, data->m_fFogFarDistance);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_FOG_NEAR_DISTANCE, data->m_fFogNearDistance);

	CheckDlgButton(IDC_MAP_ENVIRONMENT_FOG_ENABLE, data->bFogEnable);
	CheckDlgButton(IDC_MAP_ENVIRONMENT_LIGHT_ENABLE, data->bDirLightsEnable[ENV_DIRLIGHT_BACKGROUND]);
	CheckDlgButton(IDC_MAP_ENVIRONMENT_FILTERING_ENABLE, data->bFilteringEnable);

	m_ctrlFilteringAlphaSrc.SelectBlendType(data->byFilteringAlphaSrc);
	m_ctrlFilteringAlphaDest.SelectBlendType(data->byFilteringAlphaDest);

	// SkyBox
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_SCALE_X, data->v3SkyBoxScale.x);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_SCALE_Y, data->v3SkyBoxScale.y);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_SCALE_Z, data->v3SkyBoxScale.z);

	//Check
	CheckDlgButton(IDC_MAP_ENVIRONMENT_LENSFLARE_ENABLE2, data->bSkyBoxTextureRenderMode);
	SetDlgItemText(IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_FILE_NAME2, data->strSkyBoxFaceFileName[0].c_str()); // front
	SetDlgItemText(IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_FILE_NAME4, data->strSkyBoxFaceFileName[1].c_str()); // back
	SetDlgItemText(IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_FILE_NAME5, data->strSkyBoxFaceFileName[2].c_str()); // left
	SetDlgItemText(IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_FILE_NAME6, data->strSkyBoxFaceFileName[3].c_str()); // right
	SetDlgItemText(IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_FILE_NAME3, data->strSkyBoxFaceFileName[4].c_str()); // top
	SetDlgItemText(IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_FILE_NAME8, data->strSkyBoxFaceFileName[5].c_str()); // bottom

	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_CLOUD_SCALE_X, data->v2CloudScale.x);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_CLOUD_SCALE_Y, data->v2CloudScale.y);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_SCALE_X, data->v2CloudTextureScale.x);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_SCALE_Y, data->v2CloudTextureScale.y);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_CLOUD_SPEED_X, data->v2CloudSpeed.x);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_CLOUD_SPEED_Y, data->v2CloudSpeed.y);
	SetDialogFloatText(GetSafeHwnd(), IDC_MAP_ENVIRONMENT_CLOUD_HEIGHT, data->fCloudHeight);
	SetDlgItemText(IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_FILE_NAME, data->strCloudTextureFileName.c_str());

	CheckDlgButton(IDC_MAP_ENVIRONMENT_LENSFLARE_ENABLE, data->bLensFlareEnable);
	m_ctrlLensFlareBrightnessColor.SetColor(data->LensFlareBrightnessColor);
	m_ctrlLensFlareMaxBrightness.SetPos(int(data->fLensFlareMaxBrightness*100.0f));
	SetDlgItemText(IDC_MAP_ENVIRONMENT_LENSFLARE_TEXTURE_FILE_NAME, data->strMainFlareTextureFileName.c_str());

	CheckDlgButton(IDC_MAP_ENVIRONMENT_MAINFLARE_ENABLE, data->bMainFlareEnable);
	m_ctrlMainFlareSize.SetPos(int(data->fMainFlareSize*100.0f));

	COLORREF BrightnessColor = RGB(WORD(data->LensFlareBrightnessColor.r*255.0f),
	                               WORD(data->LensFlareBrightnessColor.g*255.0f),
	                               WORD(data->LensFlareBrightnessColor.b*255.0f));
	m_ctrlLensFlareBrightnessColor.SetColor(BrightnessColor);

	RebuildGradientList();

	std::string path;
	GetOnlyFileName(map->GetEnvironmentPath(map->GetSelectedEnvironment()),
	                path);
	m_EnviromentScriptName.SetWindowText(path.c_str());

	TurnOnCallBack();
}

void CMapEnvironmentPage::RebuildGradientList()
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	char szGradientName[32+1];
	uint32_t dwControlIndex = 0;

	m_ctrlSkyBoxGradientList.ResetContent();
	for (uint32_t i = 0; i < data->bySkyBoxGradientLevelUpper; ++i)
	{
		_snprintf(szGradientName, 32, "Upper %02d", i);
		m_ctrlSkyBoxGradientList.InsertString(dwControlIndex++, szGradientName);
	}
	for (uint32_t j = 0; j < data->bySkyBoxGradientLevelLower; ++j)
	{
		_snprintf(szGradientName, 32, "Lower %02d", j);
		m_ctrlSkyBoxGradientList.InsertString(dwControlIndex++, szGradientName);
	}

	m_ctrlSkyBoxGradientList.InsertString(dwControlIndex++, "Cloud");

	if (m_ctrlSkyBoxGradientList.GetCount() > 0)
	{
		CString strGradientName;
		m_ctrlSkyBoxGradientList.GetLBText(0, strGradientName);
		m_ctrlSkyBoxGradientList.SelectString(0, strGradientName);
		SelectGradient(0);
	}
	else
	{
		m_ctrlSkyBoxGradientList.InsertString(0, "None");
		m_ctrlSkyBoxGradientList.SelectString(0, "None");
	}
}

void CMapEnvironmentPage::CallBack()
{
	if (!ms_isCallBackFlag)
		return;

	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	COLORREF MaterialDiffuse;
	COLORREF MaterialAmbient;
	COLORREF MaterialEmissive;
	COLORREF LightDiffuse;
	COLORREF LightAmbient;
	COLORREF FogColor;
	COLORREF FilteringColor;
	COLORREF FirstGradientColor;
	COLORREF SecondGradientColor;
	COLORREF BrightnessColor;

	ms_pThis->m_ctrlMaterialDiffuse.GetColor(&MaterialDiffuse);
	ms_pThis->m_ctrlMaterialAmbient.GetColor(&MaterialAmbient);
	ms_pThis->m_ctrlMaterialEmissive.GetColor(&MaterialEmissive);
	ms_pThis->m_ctrlLightDiffuse.GetColor(&LightDiffuse);
	ms_pThis->m_ctrlLightAmbient.GetColor(&LightAmbient);
	ms_pThis->m_ctrlFogColor.GetColor(&FogColor);
	ms_pThis->m_ctrlFilteringColor.GetColor(&FilteringColor);
	ms_pThis->m_ctrlSkyBoxGradientFirstColor.GetColor(&FirstGradientColor);
	ms_pThis->m_ctrlSkyBoxGradientSecondColor.GetColor(&SecondGradientColor);
	ms_pThis->m_ctrlLensFlareBrightnessColor.GetColor(&BrightnessColor);

	data->Material.Diffuse.r = GetRValue(MaterialDiffuse) / 255.0f;
	data->Material.Diffuse.g = GetGValue(MaterialDiffuse) / 255.0f;
	data->Material.Diffuse.b = GetBValue(MaterialDiffuse) / 255.0f;

	data->Material.Ambient.r = GetRValue(MaterialAmbient) / 255.0f;
	data->Material.Ambient.g = GetGValue(MaterialAmbient) / 255.0f;
	data->Material.Ambient.b = GetBValue(MaterialAmbient) / 255.0f;

	data->Material.Emissive.r = GetRValue(MaterialEmissive) / 255.0f;
	data->Material.Emissive.g = GetGValue(MaterialEmissive) / 255.0f;
	data->Material.Emissive.b = GetBValue(MaterialEmissive) / 255.0f;

	data->DirLights[ENV_DIRLIGHT_BACKGROUND].Diffuse.r = GetRValue(LightDiffuse) / 255.0f;
	data->DirLights[ENV_DIRLIGHT_BACKGROUND].Diffuse.g = GetGValue(LightDiffuse) / 255.0f;
	data->DirLights[ENV_DIRLIGHT_BACKGROUND].Diffuse.b = GetBValue(LightDiffuse) / 255.0f;

	data->DirLights[ENV_DIRLIGHT_BACKGROUND].Ambient.r = GetRValue(LightAmbient) / 255.0f;
	data->DirLights[ENV_DIRLIGHT_BACKGROUND].Ambient.g = GetGValue(LightAmbient) / 255.0f;
	data->DirLights[ENV_DIRLIGHT_BACKGROUND].Ambient.b = GetBValue(LightAmbient) / 255.0f;

	data->FogColor.r = GetRValue(FogColor) / 255.0f;
	data->FogColor.g = GetGValue(FogColor) / 255.0f;
	data->FogColor.b = GetBValue(FogColor) / 255.0f;

	data->FilteringColor.r = GetRValue(FilteringColor) / 255.0f;
	data->FilteringColor.g = GetGValue(FilteringColor) / 255.0f;
	data->FilteringColor.b = GetBValue(FilteringColor) / 255.0f;

	std::pair<DirectX::SimpleMath::Color, DirectX::SimpleMath::Color> gradientColor;
	gradientColor.first = DirectX::SimpleMath::Color(GetRValue(SecondGradientColor) / 255.0f,
	                                GetGValue(SecondGradientColor) / 255.0f,
	                                GetBValue(SecondGradientColor) / 255.0f, 1.0f);
	gradientColor.second = DirectX::SimpleMath::Color(GetRValue(FirstGradientColor) / 255.0f,
	                                 GetGValue(FirstGradientColor) / 255.0f,
	                                 GetBValue(FirstGradientColor) / 255.0f, 1.0f);

	data->SetSkyBoxGradientColor(ms_pThis->m_ctrlSkyBoxGradientList.GetCurSel(),
	                             gradientColor);

	data->LensFlareBrightnessColor.r = GetRValue(BrightnessColor) / 255.0f;
	data->LensFlareBrightnessColor.g = GetGValue(BrightnessColor) / 255.0f;
	data->LensFlareBrightnessColor.b = GetBValue(BrightnessColor) / 255.0f;

	map->RefreshEnvironment();
}

void CMapEnvironmentPage::TurnOnCallBack()
{
	ms_isCallBackFlag = TRUE;
}

void CMapEnvironmentPage::TurnOffCallBack()
{
	ms_isCallBackFlag = FALSE;
}

void CMapEnvironmentPage::SelectGradient(uint32_t dwIndex)
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	EnvironmentData::GradientColor gradientColor;
	if (!data->GetSkyBoxGradientColor(dwIndex, gradientColor))
		return;

	COLORREF FirstColor = RGB(WORD(gradientColor.first.r*255.0f),
	                          WORD(gradientColor.first.g*255.0f),
	                          WORD(gradientColor.first.b*255.0f));
	COLORREF SecondColor = RGB(WORD(gradientColor.second.r*255.0f),
	                           WORD(gradientColor.second.g*255.0f),
	                           WORD(gradientColor.second.b*255.0f));

	m_ctrlSkyBoxGradientFirstColor.SetColor(FirstColor);
	m_ctrlSkyBoxGradientFirstColor.Update();
	m_ctrlSkyBoxGradientSecondColor.SetColor(SecondColor);
	m_ctrlSkyBoxGradientSecondColor.Update();
}

/////////////////////////////////////////////////////////////////////////////
// CMapEnvironmentPage message handlers

void CMapEnvironmentPage::OnEnableFog()
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	data->bFogEnable = IsDlgButtonChecked(IDC_MAP_ENVIRONMENT_FOG_ENABLE);
	map->RefreshEnvironment();
}

void CMapEnvironmentPage::OnEnableLight()
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	data->bDirLightsEnable[ENV_DIRLIGHT_BACKGROUND] =
		IsDlgButtonChecked(IDC_MAP_ENVIRONMENT_LIGHT_ENABLE);
	map->RefreshEnvironment();
}

void CMapEnvironmentPage::OnEnableFiltering()
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	data->bFilteringEnable = IsDlgButtonChecked(IDC_MAP_ENVIRONMENT_FILTERING_ENABLE);
	map->RefreshEnvironment();
}

BOOL CMapEnvironmentPage::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	m_iSliderIndex = wParam;

	return CPageCtrl::OnNotify(wParam, lParam, pResult);
}

BOOL CMapEnvironmentPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	const auto map = GetMap();
	if (!map)
		return CPageCtrl::OnCommand(wParam, lParam);

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return CPageCtrl::OnCommand(wParam, lParam);

	switch(LOWORD(wParam)) {
		// Fog
		case IDC_MAP_ENVIRONMENT_FOG_NEAR_DISTANCE: {
			char szDistance[256];

			m_ctrlFogNearDistance.GetWindowText(szDistance, 256);
			float fNear = atof(szDistance);

			m_ctrlFogFarDistance.GetWindowText(szDistance, 256);
			float fFar = atof(szDistance);

			if (fNear >= fFar)
				fNear = fFar;

			data->m_fFogNearDistance = fNear;
			break;
		}
		case IDC_MAP_ENVIRONMENT_FOG_FAR_DISTANCE: {
			char szDistance[256];
			m_ctrlFogFarDistance.GetWindowText(szDistance, 256);

			float fFar = atof(szDistance);

			m_ctrlFogNearDistance.GetWindowText(szDistance, 256);

			float fNear = atof(szDistance);

			if (fNear >= fFar)
				fFar = fNear;

			data->m_fFogFarDistance = fFar;
			break;
		}

		// Filtering
		case IDC_MAP_ENVIRONMENT_FILTERING_ALPHA_SRC:
			data->byFilteringAlphaSrc = m_ctrlFilteringAlphaSrc.GetBlendType();
			break;
		case IDC_MAP_ENVIRONMENT_FILTERING_ALPHA_DEST:
			data->byFilteringAlphaDest = m_ctrlFilteringAlphaDest.GetBlendType();
			break;

		// SkyBox
		case IDC_MAP_ENVIRONMENT_SCALE_X:
			data->v3SkyBoxScale.x = GetDialogFloatText(GetSafeHwnd(), LOWORD(wParam));
			break;
		case IDC_MAP_ENVIRONMENT_SCALE_Y:
			data->v3SkyBoxScale.y = GetDialogFloatText(GetSafeHwnd(), LOWORD(wParam));
			break;
		case IDC_MAP_ENVIRONMENT_SCALE_Z:
			data->v3SkyBoxScale.z = GetDialogFloatText(GetSafeHwnd(), LOWORD(wParam));
			break;

		case IDC_MAP_ENVIRONMENT_CLOUD_SCALE_X:
			data->v2CloudScale.x = GetDialogFloatText(GetSafeHwnd(), LOWORD(wParam));
			break;
		case IDC_MAP_ENVIRONMENT_CLOUD_SCALE_Y:
			data->v2CloudScale.y = GetDialogFloatText(GetSafeHwnd(), LOWORD(wParam));
			break;

		case IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_SCALE_X:
			data->v2CloudTextureScale.x = GetDialogFloatText(GetSafeHwnd(), LOWORD(wParam));
			break;
		case IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_SCALE_Y:
			data->v2CloudTextureScale.y = GetDialogFloatText(GetSafeHwnd(), LOWORD(wParam));
			break;

		case IDC_MAP_ENVIRONMENT_CLOUD_SPEED_X:
			data->v2CloudSpeed.x = GetDialogFloatText(GetSafeHwnd(), LOWORD(wParam));
			break;
		case IDC_MAP_ENVIRONMENT_CLOUD_SPEED_Y:
			data->v2CloudSpeed.y = GetDialogFloatText(GetSafeHwnd(), LOWORD(wParam));
			break;

		case IDC_MAP_ENVIRONMENT_CLOUD_HEIGHT:
			data->fCloudHeight = GetDialogFloatText(GetSafeHwnd(), LOWORD(wParam));
			break;

		case IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_FILE_NAME: {
			CString strTextureFileName;
			GetDlgItemText(IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_FILE_NAME, strTextureFileName);

			data->strCloudTextureFileName = strTextureFileName;
			break;
		}

		case IDC_MAP_ENVIRONMENT_GRADIENT_LIST:
			SelectGradient(m_ctrlSkyBoxGradientList.GetCurSel());
			break;

		// LensFlare
		case IDC_MAP_ENVIRONMENT_LENSFLARE_TEXTURE_FILE_NAME: {
			CString strTextureFileName;
			GetDlgItemText(IDC_MAP_ENVIRONMENT_LENSFLARE_TEXTURE_FILE_NAME, strTextureFileName);

			data->strMainFlareTextureFileName = strTextureFileName;
			break;
		}
	}

	map->RefreshEnvironment();

	return CPageCtrl::OnCommand(wParam, lParam);
}

void CMapEnvironmentPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	switch(m_iSliderIndex)
	{
		case IDC_MAP_ENVIRONMENT_WIND_STRENGTH:
			data->fWindStrength = m_ctrlWindStrength.GetPos() / 100.0f;
			break;

		case IDC_MAP_ENVIRONMENT_WIND_RANDOM:
			data->fWindRandom = float(m_ctrlWindRandom.GetPos()) / 100.0f;
			break;

		case IDC_MAP_ENVIRONMENT_FILTERING_ALPHA:
			data->FilteringColor.a = float(m_ctrlFilteringAlpha.GetPos()) / 100.0f;
			break;

		case IDC_MAP_ENVIRONMENT_LENSFLARE_MAX_BRIGHTNESS:
			data->fLensFlareMaxBrightness = float(m_ctrlLensFlareMaxBrightness.GetPos()) / 100.0f;
			break;

		case IDC_MAP_ENVIRONMENT_LENSFLARE_MAIN_FLARE_SIZE:
			data->fMainFlareSize = float(m_ctrlMainFlareSize.GetPos()) / 100.0f;
			break;
	}

	map->RefreshEnvironment();

	CPageCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMapEnvironmentPage::OnLoadEnvironmentScript()
{
	uint32_t dwFlag = OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	const char * c_szFilter = "Metin2 Environment Script Files (*.msenv)|*.msenv|All Files (*.*)|*.*|";

	CFileDialog FileOpener(TRUE, "Load", "", dwFlag, c_szFilter, this);

	if (FileOpener.DoModal()) {
		const auto map = GetMap();
		if (!map)
			return;


		map->RegisterEnvironment(map->GetSelectedEnvironment(), FileOpener.GetPathName().GetString());
		map->RefreshEnvironment(); // Since we overwrote the current env.

		UpdateUI();
	}
}

/* (수)
*  환경변수 저장 후 저장된 이름으로 UI 갱신
*/
void CMapEnvironmentPage::OnSaveEnvironmentScript()
{
	uint32_t dwFlag = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	const char * c_szFilter = "Metin2 Environment Script Files (*.msenv)|*.msenv|All Files (*.*)|*.*|";

	CFileDialog FileOpener(FALSE, "Save", "", dwFlag, c_szFilter, this);

	if (FileOpener.DoModal()) {
		const auto map = GetMap();
		if (!map)
			return;

		const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
		if (!data)
			return;

		data->Save(FileOpener.GetPathName().GetString());
	}
}

void CMapEnvironmentPage::OnCheckLightpositionEdit()
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CSceneMap * pSceneMap = pApplication->GetSceneMap();

	pSceneMap->SetLightPositionEditing(IsDlgButtonChecked(IDC_CHECK_LIGHTPOSITIONEDIT));
}

void CMapEnvironmentPage::OnLoadCloudTexture()
{
	uint32_t dwFlag = OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	const char * c_szFilter = "Texture Files (*.tga)|*.tga|All Files (*.*)|*.*|";

	CFileDialog FileOpener(TRUE, "Load", "", dwFlag, c_szFilter, this);

	if (FileOpener.DoModal()) {
		const auto map = GetMap();
		if (!map)
			return;

		const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
		if (!data)
			return;

		data->strCloudTextureFileName = FileOpener.GetPathName();
		map->RefreshEnvironment();

		SetDlgItemText(IDC_MAP_ENVIRONMENT_CLOUD_TEXTURE_FILE_NAME, FileOpener.GetPathName());
	}
}

void CMapEnvironmentPage::OnInsertGradientUpper()
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	data->InsertGradientUpper();
	RebuildGradientList();
	map->RefreshEnvironment();
}

void CMapEnvironmentPage::OnOnInsertGradientLower()
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	data->InsertGradientLower();
	RebuildGradientList();
	map->RefreshEnvironment();
}

void CMapEnvironmentPage::OnDeleteGradient()
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	data->DeleteGradient(m_ctrlSkyBoxGradientList.GetCurSel());
	RebuildGradientList();
	map->RefreshEnvironment();
}

void CMapEnvironmentPage::OnCheckLensFlareEnable()
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	data->bLensFlareEnable = IsDlgButtonChecked(IDC_MAP_ENVIRONMENT_LENSFLARE_ENABLE);
	map->RefreshEnvironment();
}

void CMapEnvironmentPage::OnCheckMainFlareEnable()
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	data->bMainFlareEnable = IsDlgButtonChecked(IDC_MAP_ENVIRONMENT_MAINFLARE_ENABLE);
	map->RefreshEnvironment();
}

void CMapEnvironmentPage::OnLoadLensFlareTexture()
{
	uint32_t dwFlag = OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	const char * c_szFilter = "Texture Files (*.tga)|*.tga|All Files (*.*)|*.*|";

	CFileDialog FileOpener(TRUE, "Load", "", dwFlag, c_szFilter, this);

	if (FileOpener.DoModal()) {
		const auto map = GetMap();
		if (!map)
			return;

		const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
		if (!data)
			return;

		data->strMainFlareTextureFileName = FileOpener.GetPathName();
		map->RefreshEnvironment();

		SetDlgItemText(IDC_MAP_ENVIRONMENT_LENSFLARE_TEXTURE_FILE_NAME, FileOpener.GetPathName());
	}
}

void CMapEnvironmentPage::OnBnClickedSkyBoxModeTexture()
{
	const auto map = GetMap();
	if (!map)
		return;

	const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
	if (!data)
		return;

	data->bSkyBoxTextureRenderMode = IsDlgButtonChecked(IDC_MAP_ENVIRONMENT_LENSFLARE_ENABLE2);
	map->RefreshEnvironment();
}

void CMapEnvironmentPage::SetSkyBoxTexture(int iFaceIndex)
{
	uint32_t dwFlag = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	const char * c_szFilter = "Texture Files (*.dds;*.jpg)|*.dds;*.jpg|All Files (*.*)|*.*|";
	CFileDialog FileOpener(TRUE, "Load", "", dwFlag, c_szFilter, this);

	if (FileOpener.DoModal())
	{
		const auto map = GetMap();
		if (!map)
			return;

		const auto data = map->GetEnvironment(map->GetSelectedEnvironment());
		if (!data)
			return;

		data->strSkyBoxFaceFileName[iFaceIndex] = FileOpener.GetPathName();
		map->RefreshEnvironment();

		// Let's not duplicate the filename label update code here...
		UpdateUI();
	}
}

void CMapEnvironmentPage::OnSetSkyBoxFrontTexture()
{
	SetSkyBoxTexture(0);
}

void CMapEnvironmentPage::OnSetSkyBoxBackTexture()
{
	SetSkyBoxTexture(1);
}

void CMapEnvironmentPage::OnSetSkyBoxLeftTexture()
{
	SetSkyBoxTexture(2);
}

void CMapEnvironmentPage::OnSetSkyBoxRightTexture()
{
	SetSkyBoxTexture(3);
}

void CMapEnvironmentPage::OnSetSkyBoxTopTexture()
{
	SetSkyBoxTexture(4);
}

void CMapEnvironmentPage::OnSetSkyBoxBottomTexture()
{
	SetSkyBoxTexture(5);
}

METIN2_END_NS
