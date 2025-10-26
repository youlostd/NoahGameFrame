#include "StdAfx.h"
#include "Util.h"

#include <SpdLog.hpp>

METIN2_BEGIN_NS

char					g_szProgramPath[PROGRAM_PATH_LENGTH + 1];
char					g_szProgramWindowPath[PROGRAM_PATH_LENGTH + 1];

bool					g_isTileData = FALSE;
bool					g_isTileFile = TRUE;
bool					g_isCharacterData = FALSE;
bool					g_isCharacterFile = TRUE;
CGraphicThing::Ptr		g_pTileThing = NULL;
CTileInstance			g_TileInstance;
CGraphicThingInstance	g_CharacterInstance;

void CreateBitmapButton(CButton * pButton, int iBitmapID, CBitmap & rBitmap)
{
	rBitmap.DeleteObject();
	rBitmap.LoadBitmap(iBitmapID);
	pButton->SetBitmap(rBitmap);
}

void RepositioningWindow(CPageCtrl & rPageCtrl, int ixTemporarySpace, int iyTemporarySpace)
{
	CRect Rect;
	rPageCtrl.GetClientRect(Rect);
	rPageCtrl.MoveWindow(Rect.left+ixTemporarySpace, Rect.top+iyTemporarySpace, Rect.Width(), Rect.Height());
}

void CreateHighColorImageList(int iBitmapID, CImageList * pImageList)
{
	CBitmap Bitmap;

	HINSTANCE hInstance = ::AfxFindResourceHandle(MAKEINTRESOURCE(iBitmapID), RT_BITMAP);
	HANDLE hImage = ::LoadImage(hInstance, MAKEINTRESOURCE(iBitmapID), IMAGE_BITMAP, 0, 0, (LR_DEFAULTSIZE | LR_CREATEDIBSECTION));
	Bitmap.Attach(hImage);

	// Replace
	BITMAP bmInfo;
	const RGBTRIPLE	kBackgroundColor = {192, 192, 192};

	VERIFY(Bitmap.GetBitmap(&bmInfo));

	const UINT numPixels (bmInfo.bmHeight * bmInfo.bmWidth);
	DIBSECTION ds;

	VERIFY (Bitmap.GetObject (sizeof (DIBSECTION), &ds) == sizeof (DIBSECTION));

	RGBTRIPLE * pixels = reinterpret_cast<RGBTRIPLE*>(ds.dsBm.bmBits);
	VERIFY (pixels != NULL);

	const COLORREF buttonColor (::GetSysColor(COLOR_BTNFACE));
	const RGBTRIPLE userBackgroundColor = { GetBValue (buttonColor), GetGValue (buttonColor), GetRValue (buttonColor) };

	for (UINT i = 0; i < numPixels; ++i)
	{
		if (pixels [i].rgbtBlue == kBackgroundColor.rgbtBlue &&
			pixels [i].rgbtGreen == kBackgroundColor.rgbtGreen &&
			pixels [i].rgbtRed == kBackgroundColor.rgbtRed)
		{
			pixels [i] = userBackgroundColor;
		}
	}

	// Create
	pImageList->Create(16, 16, ILC_COLOR24, 5, 0);
	pImageList->Add(&Bitmap, RGB(0, 0, 0));
}

void SetDialogFloatText(HWND hWnd, int iID, float fData)
{
	char szString[64 + 1];
	_snprintf(szString, 64, "%.3f", fData);
	SetDlgItemText(hWnd, iID, szString);
}

void SetDialogIntegerText(HWND hWnd, int iID, int iData)
{
	char szString[64 + 1];
	_snprintf(szString, 64, "%i", iData);
	SetDlgItemText(hWnd, iID, szString);
}

float GetDialogFloatText(HWND hWnd, int iID)
{
	char szString[64 + 1];
	GetDlgItemText(hWnd, iID, szString, 64);
	return atof(szString);
}

int GetDialogIntegerText(HWND hWnd, int iID)
{
	char szString[64 + 1];
	GetDlgItemText(hWnd, iID, szString, 64);
	return atoi(szString);
}

void SelectComboBoxItem(CComboBox & rComboBox, uint32_t dwIndex)
{
	CString strItem;
	rComboBox.GetLBText(dwIndex, strItem);
	rComboBox.SelectString(0, strItem);
}

const char * GetProgramExcutingPath(const char * c_szFullFileName)
{
	if (strlen(c_szFullFileName) < strlen(g_szProgramPath) + 1)
		return c_szFullFileName;

	for (auto i = 0; i < strlen(g_szProgramPath); ++i)
	{
		char src, dst;

		src = tolower(c_szFullFileName[i]);
		dst = tolower(g_szProgramPath[i]);

		if (src == '/')
			src = '\\';

		if (dst == '/')
			dst = '\\';

		if (src != dst)
			return &c_szFullFileName[i];
	}

	return &c_szFullFileName[strlen(g_szProgramPath) + 1];
}

void RenderBackGroundCharacter(float fx, float fy, float fz)
{
	if (!g_isCharacterFile)
		return;

	if (!g_isCharacterData)
	{
		const storm::StringRef charPath("D:\\Ymir Work\\pc\\sura\\sura_novice.GR2");
		auto pCharacter = CResourceManager::Instance().LoadResource<CGraphicThing>(charPath);
		if (!pCharacter)
		{
			g_isCharacterFile = false;
			spdlog::error("{0} is not exist", charPath);
			return;
		}

		const storm::StringRef motionPath("D:\\Ymir Work\\pc\\sura\\onehand_sword\\wait.GR2");
		auto pMotion = CResourceManager::Instance().LoadResource<CGraphicThing>(motionPath);
		if (!pMotion)
		{
			g_isCharacterFile = false;
			spdlog::error("{0} is not exist", motionPath);
			return;
		}

		g_CharacterInstance.Clear();
		g_CharacterInstance.ReserveModelThing(1);
		g_CharacterInstance.ReserveModelInstance(1);
		g_CharacterInstance.RegisterModelThing(0, pCharacter);
		g_CharacterInstance.RegisterMotionThing(MAIN_RACE_SURA_M, 1, pMotion);

		g_CharacterInstance.SetModelInstance(0, 0, 0);
		g_CharacterInstance.SetMotion(1);
		g_isCharacterData = true;
	}

	g_CharacterInstance.SetPosition(fx, fy, fz);
	g_CharacterInstance.SetRotation(45.0f);
	g_CharacterInstance.Update();
	g_CharacterInstance.Transform();
	g_CharacterInstance.Deform();
	g_CharacterInstance.Render();
}

void RenderBackGroundTile()
{
	if (!g_isTileFile)
		return;

	if (!g_isTileData)
	{
		if (g_pTileThing)
			return;

		g_pTileThing = CResourceManager::Instance().LoadResource<CGraphicThing>("background_stone.GR2");
		if (!g_pTileThing) {
			g_isTileFile = false;
			return;
		}

		g_TileInstance.Clear();


		g_TileInstance.SetLinkedModelPointer(g_pTileThing->GetModelPointer(0),
		                                     nullptr);
		g_TileInstance.LoadTexture();

		g_isTileData = true;
	}

	g_TileInstance.RenderTile();
}

void DestroyBackGroundData()
{
	if (g_isCharacterData)
	{
		g_CharacterInstance.Clear();
		g_isCharacterData = false;
	}

	if (g_isTileData)
	{
		g_TileInstance.Clear();
		g_pTileThing = nullptr;
		g_isTileData = false;
	}
}

void CTileInstance::LoadTexture()
{
	auto r = CResourceManager::Instance().LoadResource<CGraphicImage>("D:/Ymir Work/terrainmaps/b/field/field 01.dds");
	m_ImageInstance.SetImagePointer(r);
}

void CTileInstance::RenderTile()
{
	const auto& rigidVertexBuffer  = m_model->GetVertexBuffer();
	if (!rigidVertexBuffer.IsEmpty())
		rigidVertexBuffer.Bind(0);

	STATEMANAGER.SetIndices(m_model->GetD3DIndexBuffer());
	STATEMANAGER.SetTransform(D3DTS_WORLD, &CScreen::GetIdentityMatrix());
	STATEMANAGER.SetTexture(0, m_ImageInstance.GetTexturePointer()->GetD3DTexture());

	const CGrannyModel::TMeshNode * pMeshNode = m_model->GetMeshNodeList(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
	while (pMeshNode) {
		const CGrannyMesh* pMesh = pMeshNode->pMesh;
		const CGrannyMesh::TTriGroupNode* pTriGroupNode = pMesh->GetTriGroupNodeList(CGrannyMaterial::TYPE_DIFFUSE_PNT);
		while (pTriGroupNode) {
			ms_faceCount += pTriGroupNode->triCount;

			Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,
			                                  pMesh->GetVertexCount(),
			                                  pTriGroupNode->idxPos,
			                                  pTriGroupNode->triCount,
			                                  pMesh->GetVertexBasePosition());

			pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
		}

		pMeshNode = pMeshNode->pNextMeshNode;
	}
}

std::string RemoveStartString(const std::string & strOriginal, const std::string & strBegin)
{
	if (strOriginal.substr(0,strBegin.length()) == strBegin)
		return strOriginal.substr(strBegin.length(),strOriginal.npos);

	return strOriginal;
}

METIN2_END_NS
