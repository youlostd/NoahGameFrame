#include "StdAfx.h"
#include "MapManagerAccessor.h"
#include "ShadowRenderHelper.h"
#include "MiniMapRenderHelper.h"
#include "MapAccessorOutdoor.h"
#include "MapAccessorTerrain.h"
#include "../MainFrm.h"
#include "../DockingBar/MapTreeLoader.h"
#define WORLD_EDITOR

#include <GameLib/Property.h>

#include <EterBase/Utils.h>

#include <SpdLog.hpp>

#include <il/il.h>
#include <il/ilu.h>

#include <sys/stat.h>
#include <fcntl.h>

METIN2_BEGIN_NS

float CMapManagerAccessor::CHeightObserver::GetHeight(float fx, float fy)
{
	return m_pMap->GetHeight(fx, fy);
}

CMapManagerAccessor::CMapManagerAccessor()
	: m_pHeightObserver(NULL)
{
	m_v3EditingCursorPosition = Vector3(0.0f, 0.0f, 0.0f);

	Initialize();
}

CMapManagerAccessor::~CMapManagerAccessor()
{
	Destroy();
}

void CMapManagerAccessor::Initialize()
{
	Clear();
	CMapManager::Initialize();
}

void CMapManagerAccessor::Destroy()
{
	if (m_pHeightObserver)
	{
		delete m_pHeightObserver;
		m_pHeightObserver = NULL;
	}
}

void CMapManagerAccessor::DestroyShadowTexture()
{
	if (!m_pMapAccessor)
		return;

	m_pMapAccessor->ReleaseCharacterShadowTexture();
}

void CMapManagerAccessor::RecreateShadowTexture()
{
	if (!m_pMapAccessor || !m_pMapAccessor->IsReady())
		return;

	m_pMapAccessor->CreateCharacterShadowTexture();
}

void CMapManagerAccessor::Clear()
{
	m_pMapAccessor = NULL;

	m_byBrushSize = 1;
	m_byBrushStrength = 30;

	m_ixEdit = 0;
	m_iyEdit = 0;
	m_bySubCellX = 0;
	m_bySubCellY = 0;
	m_wEditTerrainNumX = 0;
	m_wEditTerrainNumY = 0;

	m_bEditingInProgress = false;
	m_bHeightEditingOn = false;
	m_bTextureEditingOn = false;
	m_bWaterEditingOn = false;
	m_bAttrEditingOn = false;
	m_bEraseTexture = false;
	m_bDrawOnlyOnBlankTile = false;
	m_bEraseWater = false;
	m_bEraseAttr = false;

	m_bySelectedAttrFlag = 0;

	m_lBrushWaterHeight = 32768; // 현재 수치에 맞춰 하드코딩..ㅜㅜ 32768 로 해 놓으면 연산을 통해 water height 수치 0.0 미터로 표기됨.

	CMapManager::Clear();
}

CMapOutdoor* CMapManagerAccessor::AllocMap()
{
	m_pMapAccessor = new CMapOutdoorAccessor;
	m_pMapAccessor->SetDrawShadow(true);			// 기본값이 false 라 라이트를 안먹으므로 .. 여기서 한다.
	m_pMapAccessor->SetDrawCharacterShadow(true);	// 기본값이 false 라 라이트를 안먹으므로 .. 여기서 한다.
	m_pHeightObserver = new CHeightObserver(m_pMapAccessor);
	return m_pMapAccessor;
}

void CMapManagerAccessor::UpdateEditing()
{
	// Move To SceneMap because it's code for editing
	if (m_bEditingInProgress)
	{
		if (m_bHeightEditingOn)
		{
			EditTerrain();

			// 고쳐야 함... 심리스 되면서 EditX, EditY 만으로는 해결 안됨..
			RefreshObjectHeight(m_v3EditingCursorPosition.x,
								m_v3EditingCursorPosition.y,
								float(m_byBrushSize) * CTerrainImpl::CELLSCALE);
		}
		else if (m_bTextureEditingOn)
		{
			EditTerrainTexture();
		}
		else if (m_bWaterEditingOn)
		{
			EditWater();
		}
		else if (m_bAttrEditingOn)
		{
			EditAttr();
		}
	}
}

void CMapManagerAccessor::SetBrushShape(uint32_t dwShape)
{
	m_dwBrushShape = dwShape;
}

void CMapManagerAccessor::SetBrushType(uint32_t dwType)
{
	m_dwBrushType = dwType;
}

void CMapManagerAccessor::SetBrushSize(unsigned char ucBrushSize)
{
	if (ucBrushSize > m_byMAXBrushSize)
		m_byBrushSize = m_byMAXBrushSize;
	else
		m_byBrushSize = ucBrushSize;
}

void CMapManagerAccessor::SetMaxBrushSize(unsigned char ucMaxBrushSize)
{
	m_byMAXBrushSize = ucMaxBrushSize;
}

void CMapManagerAccessor::SetBrushStrength(unsigned char ucBrushStrength)
{
	if (ucBrushStrength < 1)
		ucBrushStrength = 1;
	else if (ucBrushStrength > m_byMAXBrushStrength)
		ucBrushStrength = m_byMAXBrushStrength;
	m_byBrushStrength = ucBrushStrength;
}

/*  min, max 검사 추가
*  기존에 있던 water height 를 값을 건드는 단축키가 있는데
*  의도된 수치보다 커지거나 작어져서 min, max 추가
*/
void CMapManagerAccessor::SetBrushWaterHeight(long lBrushWaterHeight)
{
	lBrushWaterHeight = std::min<long>(lBrushWaterHeight, 65536);
	lBrushWaterHeight = std::max<long>(lBrushWaterHeight, 0);

	m_lBrushWaterHeight = lBrushWaterHeight;
}

void CMapManagerAccessor::SetTextureBrushVector(std::vector<BYTE> & rTextureBrushNumberVector)
{
	if (rTextureBrushNumberVector.empty())
	{
		m_TextureBrushNumberVector.clear();
		return;
	}

	m_TextureBrushNumberVector.resize(rTextureBrushNumberVector.size());

	std::copy(rTextureBrushNumberVector.begin(),
	          rTextureBrushNumberVector.end(),
	          m_TextureBrushNumberVector.begin());
}

void CMapManagerAccessor::SetInitTextureBrushVector(std::vector<BYTE> & rTextureBrushNumberVector)
{
	if (rTextureBrushNumberVector.empty())
	{
		m_InitTextureBrushNumberVector.clear();
		return;
	}

	m_InitTextureBrushNumberVector.resize(rTextureBrushNumberVector.size());

	std::copy(rTextureBrushNumberVector.begin(),
	          rTextureBrushNumberVector.end(),
	          m_InitTextureBrushNumberVector.begin());
}

bool CMapManagerAccessor::InitBaseTexture(const char * c_szMapName)
{
	if (m_InitTextureBrushNumberVector.empty())
		return false;

	if (!m_pMapAccessor->RAW_InitBaseTexture(m_InitTextureBrushNumberVector))
	{
		spdlog::error(
		          "{0}: Failed to initialize the default texture",
		          m_pMapAccessor->GetName());
		return false;
	}

	return true;
}

void CMapManagerAccessor::SetMaxBrushStrength(unsigned char ucMaxBrushStrength)
{
	m_byMAXBrushStrength = ucMaxBrushStrength;
	if(m_pMapAccessor)
		m_pMapAccessor->SetMaxBrushStrength(m_byMAXBrushStrength);
}

void CMapManagerAccessor::EditTerrain()
{
	m_pMapAccessor->DrawHeightBrush(m_dwBrushShape, m_dwBrushType, m_wEditTerrainNumX, m_wEditTerrainNumY, m_ixEdit, m_iyEdit, m_byBrushSize, m_byBrushStrength);
}

void CMapManagerAccessor::EditingStart()
{
	m_bEditingInProgress = true;
}

void CMapManagerAccessor::EditingEnd()
{
	m_bEditingInProgress = false;
}

void CMapManagerAccessor::EditTerrainTexture()
{
	m_pMapAccessor->DrawTextureBrush(m_dwBrushShape, m_TextureBrushNumberVector, m_wEditTerrainNumX, m_wEditTerrainNumY, m_ixEdit, m_iyEdit, m_bySubCellX, m_bySubCellY, m_byBrushSize, m_bEraseTexture, m_bDrawOnlyOnBlankTile);
}

void CMapManagerAccessor::EditWater()
{
	m_pMapAccessor->DrawWaterBrush(m_dwBrushShape, m_wEditTerrainNumX, m_wEditTerrainNumY, m_ixEdit, m_iyEdit, m_byBrushSize, m_lBrushWaterHeight, m_bEraseWater);
}

void CMapManagerAccessor::PreviewEditWater()
{
	m_pMapAccessor->PreviewWaterBrush(m_dwBrushShape, m_wEditTerrainNumX, m_wEditTerrainNumY, m_ixEdit, m_iyEdit, m_byBrushSize, m_lBrushWaterHeight, m_bEraseWater);
}

/*  현재 cell 의 water height 값을 알수 있도록 하기 위해 추가
*/
void CMapManagerAccessor::CurCellEditWater()
{
	m_pMapAccessor->CurCellEditWater(m_wEditTerrainNumX, m_wEditTerrainNumY, m_ixEdit, m_iyEdit);
}

void CMapManagerAccessor::EditAttr()
{
	m_pMapAccessor->DrawAttrBrush(m_dwBrushShape, m_bySelectedAttrFlag, m_wEditTerrainNumX, m_wEditTerrainNumY, m_ixEdit, m_iyEdit, m_bySubCellX, m_bySubCellY, m_byBrushSize, m_bEraseAttr);
}

void CMapManagerAccessor::UpdateHeightFieldEditingPt(Vector3 * v3IntersectPt)
{
	m_pMapAccessor->GetPickingCoordinate(v3IntersectPt, &m_ixEdit, &m_iyEdit, &m_bySubCellX, &m_bySubCellY, &m_wEditTerrainNumX, &m_wEditTerrainNumY);
}

CMapManagerAccessor::CHeightObserver * CMapManagerAccessor::GetHeightObserverPointer()
{
	return m_pHeightObserver;
}

void CMapManagerAccessor::InsertObject(float fx, float fy, float fz, int iRotation, uint32_t dwCRC)
{
	SPDLOG_INFO("Deprecated : CMapManagerAccessor::InsertObject(x,y,z,rotation,crc)");
	InsertObject(fx,fy,fz,0,0,iRotation,0,dwCRC);
}

/* (화)
*  오브젝트를 찍고나서 바로 선택이 풀리게 해달라는 요청에 의해.
*  pAreaAccessor->SetAddedSelectObject(TRUE); 추가
*/
void CMapManagerAccessor::InsertObject(float fx, float fy, float fz, float fYaw, float fPitch, float fRoll, uint32_t dwScale, uint32_t dwCRC)
{
	WORD wIndex = GetTerrainNum(fx, fy);

	CTerrainAccessor * pTerrainAccessor;
	if (!GetTerrain(wIndex, &pTerrainAccessor))
		return;

	CAreaAccessor * pAreaAccessor;
	if (!GetArea(wIndex, &pAreaAccessor))
		return;

	CProperty * pProperty;
	if (!CPropertyManager::Instance().Get(dwCRC, &pProperty))
		return;

	const char * c_szPropertyType;
	if (!pProperty->GetString("PropertyType", &c_szPropertyType))
		return;

	CArea::TObjectData ObjectData;
	ObjectData.Position.x = fx;
	ObjectData.Position.y = fy;

	ObjectData.Position.z = pTerrainAccessor->GetHeight((int)fx, -(int)fy);
	ObjectData.dwCRC = dwCRC;
	while( fYaw   >= 360.0f ) fYaw-=360.0f;
	while( fYaw   <    0.0f ) fYaw+=360.0f;
	while( fPitch >= 360.0f ) fPitch-=360.0f;
	while( fPitch <    0.0f ) fPitch+=360.0f;
	while( fRoll  >= 360.0f ) fRoll-=360.0f;
	while( fRoll  <    0.0f ) fRoll+=360.0f;
	ObjectData.m_fYaw = fYaw;
	ObjectData.m_fPitch = fPitch;
	ObjectData.m_fRoll = fRoll;
	ObjectData.m_fHeightBias = fz;
	ObjectData.dwRange = 0;
	ObjectData.fMaxVolumeAreaPercentage = 0.0f;
	ZeroMemory(ObjectData.abyPortalID, sizeof(ObjectData.abyPortalID));

	if (PROPERTY_TYPE_AMBIENCE == GetPropertyType(c_szPropertyType))
	{
		ObjectData.dwRange = dwScale;
		ObjectData.fMaxVolumeAreaPercentage = 0.0f;

		const char * c_szPercentage;
		if (pProperty->GetString("MaxVolumeAreaPercentage", &c_szPercentage))
		{
			ObjectData.fMaxVolumeAreaPercentage = atof(c_szPercentage);
		}
	}

	pAreaAccessor->AddObject(&ObjectData);
	pAreaAccessor->SetAddedSelectObject(TRUE);
}

void CMapManagerAccessor::RefreshObjectHeight(float fx, float fy, float fHalfSize)
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pAreaAccessor;
		if (!GetArea(i, &pAreaAccessor))
			continue;

		uint32_t dwObjectCount = pAreaAccessor->GetObjectDataCount();
		for (uint32_t j = 0; j < dwObjectCount; ++j)
		{
			CArea::TObjectData * pObjectData;
			if (!pAreaAccessor->GetObjectPointer(j, &pObjectData))
				continue;

			// Dungeon Block 은 높이를 수정하지 않는다.
			CProperty * pProperty;
			if (CPropertyManager::Instance().Get(pObjectData->dwCRC, &pProperty))
			{
				const char * c_szPropertyType;
				if (pProperty->GetString("PropertyType", &c_szPropertyType))
				{
					if (PROPERTY_TYPE_DUNGEON_BLOCK == GetPropertyType(c_szPropertyType))
						continue;
				}
			}

			float fxDistance = fabs(pObjectData->Position.x - fx);
			float fyDistance = fabs(pObjectData->Position.y - fy);
			if (fxDistance < fHalfSize && fyDistance < fHalfSize)
			{
				pObjectData->Position.z = m_pMapAccessor->GetTerrainHeight(pObjectData->Position.x, -pObjectData->Position.y);
				pAreaAccessor->UpdateObject(j, pObjectData);
			}
		}
	}
}


//오브젝트 선택과 관련
// down ~ up 시점까지 추가된 오브젝트가 있는지 검사하기 위한용도
bool CMapManagerAccessor::IsAddedSelectObject()
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		bool bAddedSelectObject = pArea->IsAddedSelectObject();
		if(bAddedSelectObject)
			return TRUE;
	}

	return FALSE;
}

//오브젝트 선택과 관련
// down ~ up 시점까지 추가된 오브젝트가 있는지 검사하기 위한용도
// down 시점에서 false 셋팅
void CMapManagerAccessor::ClearAddedSelectObject()
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->ClearAddedSelectObject();
	}
}

// 선택된 오브젝트 중에서 해제를 하는 기능과 관련하여 추가
void CMapManagerAccessor::ClearDeselectObject()
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->ClearDeselectObject();
	}
}

bool CMapManagerAccessor::GetObjectDataPointer(const AreaObjectRef& ref,
                                               const CArea::TObjectData ** ppObjectData) const
{
	CAreaAccessor * pAreaAccessor;
	if (!m_pMapAccessor->GetAreaAccessor(ref.area, &pAreaAccessor))
		return NULL;

	return pAreaAccessor->GetObjectDataPointer(ref.index, ppObjectData);
}

bool CMapManagerAccessor::GetObjectInstancePointer(const AreaObjectRef& ref,
                                                   const CArea::TObjectInstance** ppObjectInstance) const
{
	CAreaAccessor* pAreaAccessor;
	if (!m_pMapAccessor->GetAreaAccessor(ref.area, &pAreaAccessor))
		return NULL;

	*ppObjectInstance = pAreaAccessor->GetObjectInstancePointer(ref.index);
	return nullptr != *ppObjectInstance;
}

/* (월)
*  외부에서 오브젝트의 Height 값을 알수있도록 인터페이스 제공을 위해 추가.
*  여기서 Height 값은 오브젝트의 밑면 높이 정보를 말한다.
*/
bool CMapManagerAccessor::GetObjectHeight(float fX, float fY, float* fHeight)
{
	CAreaAccessor * pArea;
	if (!m_pMapAccessor->GetAreaPointer(4, (CArea **)&pArea))
		return false;

	return pArea->GetObjectHeight(fX, fY, fHeight);
}

bool CMapManagerAccessor::AddTerrainTexture(const char * pFilename)
{
	CTextureSet * pTextureSet = CTerrain::GetTextureSet();

	if (!pTextureSet->AddTexture(pFilename, 4.0f, 4.0f, 0.0f, 0.0f, true, 0, 0, 1.0f / (float) (CTerrainImpl::PATCH_XSIZE * CTerrainImpl::CELLSCALE)))
		return false;

	ResetTerrainTexture();
	return true;
}

bool CMapManagerAccessor::RemoveTerrainTexture(long lTexNum)
{
	CTextureSet * pTextureSet = CTerrain::GetTextureSet();

	if (!pTextureSet->RemoveTexture(lTexNum))
	{
		spdlog::error("Failed to remove texture {0}", lTexNum);
		return false;
	}

	ResetTerrainTexture();
	return true;
}

void CMapManagerAccessor::ResetTerrainTexture()
{
	m_pMapAccessor->ResetTextures();
}

/* (목)
*  현재 Terrain Texture List 를 TextureSet 으로 지정된 파일에 저장한다.
*/
bool CMapManagerAccessor::SaveTerrainTextureSet()
{
	CTextureSet* pTextureSet = CTerrain::GetTextureSet();
	if (!pTextureSet->Save(pTextureSet->GetFileName()))
		return false;

	return true;
}

void CMapManagerAccessor::RenderSelectedObject()
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->RenderSelectedObject();
	}
}

void CMapManagerAccessor::CancelSelect()
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->CancelSelect();
	}
}

bool CMapManagerAccessor::IsSelected()
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		if (pArea->IsSelected())
			return TRUE;
	}

	return FALSE;
}

/* 함수 원형 변경
* CAreaAccessor 의 SelectObject 함수 원형 변경에 따른
* SelectObject(float fxStart, float fyStart, float fxEnd, float fyEnd) 에서
* SelectObject(float fxStart, float fyStart, float fxEnd, float fyEnd, SHORT sSpecialKeys) 로 변경
*/
bool CMapManagerAccessor::SelectObject(float fxStart, float fyStart, float fxEnd, float fyEnd, SHORT sSpecialKeys)
{
	bool bNeedChange = FALSE;

	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		bNeedChange = pArea->SelectObject(fxStart, fyStart, fxEnd, fyEnd, sSpecialKeys);
	}

	return bNeedChange;
}

/* Deselect 추가
*  선택된 오브젝트 중에서 특정 오브젝트들을 해제하는 기능 추가
*/
bool CMapManagerAccessor::DeselectObject(float fxStart, float fyStart, float fxEnd, float fyEnd)
{
	bool bNeedChange = FALSE;

	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		bNeedChange = pArea->DeselectObject(fxStart, fyStart, fxEnd, fyEnd);
	}

	return bNeedChange;
}

void CMapManagerAccessor::DeleteSelectedObject()
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->DeleteSelectedObject();
	}
}

void CMapManagerAccessor::MoveSelectedObject(float fx, float fy)
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->MoveSelectedObject(fx, fy);
	}
}

void CMapManagerAccessor::MoveSelectedObjectHeight(float fHeight)
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->MoveSelectedObjectHeight(fHeight);
	}
}

void CMapManagerAccessor::AddSelectedAmbienceScale(int iAddScale)
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->AddSelectedAmbienceScale(iAddScale);
	}
}

void CMapManagerAccessor::AddSelectedAmbienceMaxVolumeAreaPercentage(float fPercentage)
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->AddSelectedAmbienceMaxVolumeAreaPercentage(fPercentage);
	}
}

void CMapManagerAccessor::AddSelectedObjectRotation(float fYaw, float fPitch, float fRoll)
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->AddSelectedObjectRotation(fYaw, fPitch, fRoll);
	}
}

void CMapManagerAccessor::SetSelectedObjectPortalNumber(int iID)
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->SetSelectedObjectPortalNumber(iID);
	}
}

void CMapManagerAccessor::DelSelectedObjectPortalNumber(int iID)
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->DelSelectedObjectPortalNumber(iID);
	}
}

void CMapManagerAccessor::CollectPortalNumber(std::set<int> * pkSet_iPortalNumber)
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->CollectPortalNumber(pkSet_iPortalNumber);
	}
}

void CMapManagerAccessor::EnablePortal(bool bFlag)
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CAreaAccessor * pArea;
		if (!GetArea(i, &pArea))
			continue;

		pArea->EnablePortal(bFlag);
	}
}

AreaObjectRef CMapManagerAccessor::GetPickedObjectIndex()
{
	for (uint32_t i = 0; i != AROUND_AREA_NUM; ++i) {
		CAreaAccessor* pAreaAccessor;
		if (!m_pMapAccessor->GetAreaAccessor(i, &pAreaAccessor))
			continue;

		const int picked = pAreaAccessor->GetPickedObjectIndex();
		if (-1 != picked)
			return {i, picked};
	}

	return {0, -1};
}

bool CMapManagerAccessor::IsSelectedObject(const AreaObjectRef& ref)
{
	CAreaAccessor* pAreaAccessor;
	if (!m_pMapAccessor->GetAreaAccessor(ref.area, &pAreaAccessor))
		return false;

	return pAreaAccessor->IsSelectedObject(ref.index);
}

int CMapManagerAccessor::GetSelectedObjectCount()
{
	int count = 0;
	for (int i = 0; i != AROUND_AREA_NUM; ++i) {
		CAreaAccessor* pAreaAccessor;
		if (!m_pMapAccessor->GetAreaAccessor(i, &pAreaAccessor))
			continue;

		count += pAreaAccessor->GetSelectedObjectCount();
	}

	return count;
}

void CMapManagerAccessor::SelectObject(const AreaObjectRef& ref)
{
	CAreaAccessor* pAreaAccessor;
	if (!m_pMapAccessor->GetAreaAccessor(ref.area, &pAreaAccessor))
		return;

	pAreaAccessor->SelectObject(ref.index);
}

void CMapManagerAccessor::DeselectObject(const AreaObjectRef& ref)
{
	CAreaAccessor* pAreaAccessor;
	if (!m_pMapAccessor->GetAreaAccessor(ref.area, &pAreaAccessor))
		return;

	pAreaAccessor->DeselectObject(ref.index);
}

bool CMapManagerAccessor::Picking()
{
	for (int i = 0; i != AROUND_AREA_NUM; ++i) {
		CAreaAccessor* pAreaAccessor;
		if (!m_pMapAccessor->GetAreaAccessor(i, &pAreaAccessor))
			continue;

		if (pAreaAccessor->Picking())
			return true;
	}

	return false;
}

void CMapManagerAccessor::LoadProperty(CPropertyTreeControler * pTreeControler)
{
	CTreeLoader TreeLoader;
	TreeLoader.SetTreeControler(pTreeControler);

	TreeLoader.Create("*.*", "Property");

	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->UpdateMapControlBar();
}

void CMapManagerAccessor::SetSelectedAttrFlag(BYTE bFlag)
{
	m_bySelectedAttrFlag = bFlag;
	m_pMapAccessor->ResetAttrSplats();
}

void CMapManagerAccessor::GetEditArea(int * iEditX, int * iEditY, unsigned char * wSubEditX, unsigned char * wSubEditY, unsigned short * wTerrainNumX,  unsigned short * wTerrainNumY)
{
	*iEditX = m_ixEdit;
	*iEditY = m_iyEdit;
	*wSubEditX = m_bySubCellX;
	*wSubEditY = m_bySubCellY;
	*wTerrainNumX = m_wEditTerrainNumX;
	*wTerrainNumY = m_wEditTerrainNumY;
}

bool CMapManagerAccessor::NewMap(const std::string& name,
                                 uint32_t w, uint32_t h,
                                 const std::string& textureSet)
{
	storm::String path = name;

	struct _stat sb;
	if (0 != _stat(path.c_str(), &sb))
	{
		// Make The Directory
		if (!CreateDirectory(path.c_str(), NULL))
		{
			spdlog::error("Save Failed: Unable to create directory {0}", path);
			return false;
		}

		if (0 != _stat(path.c_str(), &sb))
		{
			spdlog::error("Save failed: Disk error {0}", path);
			return false;
		}

		if (!(sb.st_mode & _S_IFDIR))
		{
			spdlog::error("Save failed: Not a directory {0}", path);
			return false;
		}
	}
	else
	{
		char szWarningMessage[1024];
		sprintf(szWarningMessage, "Directory [%s] already exists. Overwrite?.", name.c_str());
		if (IDCANCEL == MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
		                           szWarningMessage,
		                           "Overwrite warning",
		                           MB_OKCANCEL))
			return false;
	}

	auto pNewMap = std::make_unique<CMapOutdoorAccessor>();

	if (!pNewMap->NewMap(name, w, h, textureSet))
		return false;

	if (!pNewMap->SaveProperty(path))
	{
		spdlog::error("{0} Failed to save Property", path);
		return false;
	}

	if (!pNewMap->SaveSetting(path))
	{
		spdlog::error("{0} Failed to save Settings", path);
		return false;
	}

	for (uint32_t usY = 0; usY < h; ++usY)
	{
		for (uint32_t usX = 0; usX < w; ++usX)
		{
			if (!pNewMap->CreateNewTerrainSector(usX, usY))
				return false;
		}
	}

	return true;
}

bool CMapManagerAccessor::SaveMap(const char * c_szMapName)
{
	storm::String strMapFolder;

	if (NULL == c_szMapName)
	{
		if ("" == m_pMapAccessor->GetName())
		{
			spdlog::error("Save Failed: No map name");
			return false;
		}
		strMapFolder = m_pMapAccessor->GetName().c_str();
	}
	else
	{
		strMapFolder = c_szMapName;
	}

	SPDLOG_INFO("Saving map to {0}", strMapFolder);

	struct _stat sb;

	if (0 != _stat(strMapFolder.c_str(), &sb))
	{
		spdlog::error("{0} Disk Error", strMapFolder);
		return false;
	}

	if (!(sb.st_mode & _S_IFDIR))
	{
		spdlog::error("{0} Not a directory", strMapFolder);
		return false;
	}

	// Map Property
	if (!SaveMapProperty(strMapFolder))
	{
		spdlog::error("{0} Failed to save Property", strMapFolder);
		return false;
	}

	// Map Setting
	if (!SaveMapSetting(strMapFolder))
	{
		spdlog::error("{0} Failed to save Settings", strMapFolder);
		return false;
	}

	if (!SaveTerrains())
	{
		spdlog::error("{0} Failed to save Terrain", strMapFolder);
		return false;
	}

	if (!SaveAreas())
	{
		spdlog::error("{0} Failed to save Areas", strMapFolder);
		return false;
	}

	if (!m_pMapAccessor->SaveEnvironments())
	{
		spdlog::error("{0} Failed to save Environment Script", strMapFolder);
		return false;
	}

	return true;
}

bool CMapManagerAccessor::SaveMapProperty(storm::String c_rstrFolder)
{
	return m_pMapAccessor->SaveProperty(std::move(c_rstrFolder));
}

bool CMapManagerAccessor::SaveMapSetting(storm::String c_rstrFolder)
{
	return m_pMapAccessor->SaveSetting(std::move(c_rstrFolder));
}

bool CMapManagerAccessor::SaveTerrains()
{
	return m_pMapAccessor->SaveTerrains();
}

bool CMapManagerAccessor::SaveAreas()
{
	return m_pMapAccessor->SaveAreas();
}

bool CMapManagerAccessor::GetEditArea(CAreaAccessor ** ppAreaAccessor)
{
	unsigned char ucAreaNum = (m_wEditTerrainNumY - m_pMapAccessor->GetCurCoordinate().m_sTerrainCoordY + 1) * 3 +
							  (m_wEditTerrainNumX - m_pMapAccessor->GetCurCoordinate().m_sTerrainCoordX + 1);

	if (ucAreaNum < 0 || ucAreaNum > AROUND_AREA_NUM - 1)
		return FALSE;

	CArea * pArea;
	if (!m_pMapAccessor->GetAreaPointer(ucAreaNum, &pArea))
		return FALSE;

	*ppAreaAccessor = static_cast<CAreaAccessor *>(pArea);

	return TRUE;
}

bool CMapManagerAccessor::GetArea(uint8_t c_ucAreaNum, CAreaAccessor ** ppAreaAccessor)
{
	if (c_ucAreaNum < 0 || c_ucAreaNum > AROUND_AREA_NUM - 1)
		return FALSE;

	if(!m_pMapAccessor)
		return false;
	
	return m_pMapAccessor->GetAreaPointer(c_ucAreaNum, (CArea **) &*ppAreaAccessor);
}

bool CMapManagerAccessor::GetEditTerrain(CTerrainAccessor ** ppTerrainAccessor)
{
	unsigned char ucTerrainNum = (m_wEditTerrainNumY - m_pMapAccessor->GetCurCoordinate().m_sTerrainCoordY + 1) * 3 +
								 (m_wEditTerrainNumX - m_pMapAccessor->GetCurCoordinate().m_sTerrainCoordX + 1);
	return GetTerrain(ucTerrainNum, ppTerrainAccessor);
}

bool CMapManagerAccessor::GetTerrain(uint8_t c_ucTerrainNum, CTerrainAccessor ** ppTerrainAccessor)
{
	if (c_ucTerrainNum < 0 || c_ucTerrainNum > AROUND_AREA_NUM - 1)
		return FALSE;

	return m_pMapAccessor->GetTerrainPointer(c_ucTerrainNum, (CTerrain **) ppTerrainAccessor);
}

const unsigned char CMapManagerAccessor::GetTerrainNum(float fx, float fy)
{
	BYTE byTerrainNum;
	m_pMapAccessor->GetTerrainNum(fx, fy, &byTerrainNum);
	return byTerrainNum;
}

const unsigned char CMapManagerAccessor::GetEditTerrainNum()
{
	return (m_wEditTerrainNumY - m_pMapAccessor->GetCurCoordinate().m_sTerrainCoordY + 1) * 3 +
		   (m_wEditTerrainNumX - m_pMapAccessor->GetCurCoordinate().m_sTerrainCoordX + 1);
}

void CMapManagerAccessor::RefreshArea()
{
	CAreaAccessor * pArea;
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		if (!m_pMapAccessor->GetAreaPointer(i, (CArea **)&pArea))
			continue;

		pArea->RefreshArea();
	}
}

void CMapManagerAccessor::RenderAttr()
{
	m_pMapAccessor->RenderAccessorTerrain(CMapOutdoorAccessor::RENDER_ATTR, m_bySelectedAttrFlag);
}

bool CMapManagerAccessor::ResetToDefaultAttr()
{
	std::string strMapFolder = m_pMapAccessor->GetName();

	struct _stat sb;

	if (0 != _stat(strMapFolder.c_str(), &sb))
	{
		spdlog::error("{0} Folder doesn't exist", strMapFolder);
		return false;
	}

	if (!(sb.st_mode & _S_IFDIR))
	{
		spdlog::error("{0} Not a folder", strMapFolder);
		return false;
	}

	if (!m_pMapAccessor->ResetToDefaultAttr())
	{
		spdlog::error("{0} Failed to reset attr", strMapFolder);
		return false;
	}

	return true;
}

void CMapManagerAccessor::RenderObjectCollision()
{
	if (m_pMapAccessor)
		m_pMapAccessor->RenderObjectCollision();
}

void CMapManagerAccessor::SetTerrainModified()
{
	if (m_pMapAccessor)
		m_pMapAccessor->SetTerrainModified();
}

void CMapManagerAccessor::SetWireframe(bool isWireframe)
{
	if (m_pMapAccessor)
		m_pMapAccessor->SetWireframe(isWireframe);
}

void CMapManagerAccessor::ToggleWireframe()
{
	if (m_pMapAccessor)
		m_pMapAccessor->SetWireframe(!m_pMapAccessor->IsWireframe());
}

void CMapManagerAccessor::SaveMiniMap()
{
	if (!IsMapReady())
		return;

	CTerrainAccessor * pTerrainAccessor;
	if (!m_pMapAccessor->GetTerrainPointer(4, (CTerrain **) &pTerrainAccessor))
		return;

	CMiniMapRenderHelper::Instance().CreateTextures();

	CMiniMapRenderHelper::Instance().SetMapOutdoorAccssorPointer(m_pMapAccessor);

	WORD wCoordX, wCoordY;
	pTerrainAccessor->GetCoordinate(&wCoordX, &wCoordY);
	CMiniMapRenderHelper::Instance().SetTargetTerrainCoord(wCoordX, wCoordY);

	if (CMiniMapRenderHelper::Instance().StartRendering())
	{
		m_pMapAccessor->RenderMiniMap();
	}
	else
	{
		spdlog::error("CSceneMap::SaveMiniMap() Rendering Failed");
	}

	CMiniMapRenderHelper::Instance().EndRendering();

	pTerrainAccessor->SaveMiniMap(m_pMapAccessor->GetName(),
	                              CMiniMapRenderHelper::Instance().GetMiniMapTexture());

	CMiniMapRenderHelper::Instance().ReleaseTextures();
}


/* 전체지도저장 기능 정상화
*  1. ilGenImages 메소드 잘못사용되고 있던 부분 수정.
*  2. ilShutDown 메소드를 추가하여 메모리 해제기능 추가.
*    (메모리를 정삭적으로 해제를 해주지 않으면
*     SaveAtlas() 함수가 2번째 호출되었을때 문제가 발생한다 - 현재 오리지널 버전이 그럼)
*/
void CMapManagerAccessor::SaveAtlas()
{
	short sCountX, sCountY;
	m_pMapAccessor->GetTerrainCount(&sCountX, &sCountY);

	storm::String realFilename;
	char szFileName[256];

	ilInit();
	ilEnable(IL_FILE_OVERWRITE);

	ILuint dstImage;
	ilGenImages(1, &dstImage);

	ilBindImage(dstImage);
	ilClearImage();
	ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
	iluScale(256 * sCountX, 256 * sCountY, 1);

	bool bSuccessed = true;

	for (short sY = 0; sY < sCountY; ++sY)
	{
		for (short sX = 0; sX < sCountX; ++sX)
		{
			uint32_t wID = (uint32_t) (sX) * 1000L + (uint32_t)(sY);

			_snprintf(szFileName, MAX_PATH, "%s\\%06u\\minimap.bmp", m_pMapAccessor->GetName().c_str(), wID);
			realFilename = szFileName;

			ILuint srcImage;
			ilGenImages(1, &srcImage);
			ilBindImage(srcImage);

			ilLoadImage(realFilename.c_str());
			iluFlipImage(); // BMP는 이미지가 거꾸로 되어 있다.

			ILubyte* pData = ilGetData();
			ILint iImageWidth = ilGetInteger(IL_IMAGE_WIDTH);
			ILint iImageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
			ILint iImageFormat = ilGetInteger(IL_IMAGE_FORMAT);
			ILint iImageType = ilGetInteger(IL_IMAGE_TYPE);

			ilBindImage(dstImage);

			ilSetPixels((ILint) sX * 256, (ILint) sY * 256, 0,
			            iImageWidth, iImageHeight,
			            1, iImageFormat, iImageType, pData);

			ilDeleteImages(1, &srcImage);
		}
	}

	sprintf(szFileName, "%s_atlas.bmp", m_pMapAccessor->GetName().c_str());
	realFilename= szFileName;

	iluFlipImage(); // BMP는 이미지가 거꾸로 되어 있다.
	ilSaveImage(realFilename.c_str());

	ilDeleteImages(1, &dstImage);

	int e;
	while ((e = ilGetError()) != NO_ERROR) {
		SPDLOG_WARN("ERROR: {0}", iluErrorString(e));
	}

	ilShutDown();	//추가

	if (bSuccessed)
		SPDLOG_INFO("Atlas creation succeeded.");
	else
		spdlog::error("Atlas creation failed.");
}

void CMapManagerAccessor::UpdateTerrainShadowMap()
{
	if (!IsMapReady())
		return;

	CTerrainAccessor * pTerrainAccessor;
	if (!m_pMapAccessor->GetTerrainPointer(4, (CTerrain **) &pTerrainAccessor))
		return;

	CShadowRenderHelper::Instance().CreateTextures();

	CShadowRenderHelper::Instance().SetMapOutdoorAccssorPointer(m_pMapAccessor);

	WORD wCoordX, wCoordY;
	pTerrainAccessor->GetCoordinate(&wCoordX, &wCoordY);
	CShadowRenderHelper::Instance().SetTargetTerrainCoord(wCoordX, wCoordY);

	if (CShadowRenderHelper::Instance().StartRenderingPhase(1))
	{
		CSpeedTreeForest::Instance().Render(Forest_RenderAll | Forest_RenderToShadow);
		m_pMapAccessor->RenderToShadowMap();
	}
	else
		spdlog::error("CSceneMap::OnRenderToShadowMap() RenderingPhase 1 Failed");

	CShadowRenderHelper::Instance().EndRenderingPhase(1);

	if (CShadowRenderHelper::Instance().StartRenderingPhase(2))
		m_pMapAccessor->RenderShadow();
	else
		spdlog::error("CSceneMap::OnRenderToShadowMap() RenderingPhase 2 Failed");

	CShadowRenderHelper::Instance().EndRenderingPhase(2);

	pTerrainAccessor->SaveShadowMap(m_pMapAccessor->GetName(),
	                                CShadowRenderHelper::Instance().GetShadowTexture());

	CShadowRenderHelper::Instance().ReleaseTextures();
}

/* 수정
*  매개변수 추가 bAllArea 추가
*  실제 shadowmap 이 갱신되는 영역은 한부분인데 주변영역까지 리로드가 일어남.
*  bAllArea 변수를 통하여 현재구역만 리로드가 될수 있게끔 수정.
*  기존에 쓰던부분에서 영향을 안받게끔 디폴트 매개변수로 처리.
*/
void CMapManagerAccessor::ReloadTerrainShadowTexture(bool bAllArea)
{
	if (!IsMapReady())
		return;

	if(bAllArea)
	{
		for (int i = 0; i < AROUND_AREA_NUM; ++i)
		{
			CTerrainAccessor * pTerrainAccessor;
			if (!m_pMapAccessor->GetTerrainPointer(i, (CTerrain **) &pTerrainAccessor))
				continue;
			pTerrainAccessor->ReloadShadowTexture(m_pMapAccessor->GetName());
 		}
	}
	else
	{
		CTerrainAccessor * pTerrainAccessor;
		if (!m_pMapAccessor->GetTerrainPointer(4, (CTerrain **) &pTerrainAccessor))
			return;
		pTerrainAccessor->ReloadShadowTexture(m_pMapAccessor->GetName());
	}
}

/*
* LoadMap 수정
* 1. CResourceManager::Instance().ReloadAll(); 추가함
*    이유 : CMapFilePage::OnLoadMap() 부분에 주석을 달아놓음.
*
* (수) ReloadEnvironmentScript 부분 코드 추가.
* (목) 폰트리소스(.fnt)는 Reload 에서 제외.
*/
bool CMapManagerAccessor::LoadMap(const std::string& c_rstrMapName)
{
	m_pMapAccessor->Clear();
	m_pMapAccessor->Leave();
	m_pMapAccessor->SetName(c_rstrMapName);
	m_pMapAccessor->LoadProperty();

	//
	// [tim] We need to destroy the characters we created now,
	// otherwise they begin referring to invalid data.
	// Of course the real solution would be an event-system that
	// notifies character-instances about resource-reloads etc.
	//

	//DestroyBackGroundData();
	//CResourceManager::Instance().ReloadAll();

	__RefreshMapID(c_rstrMapName.c_str());

	if (!m_pMapAccessor->Load(0.0f, 0.0f, 0.0f))
			return false;

	m_pMapAccessor->Enter();
	return true;
}

void CMapManagerAccessor::UpdateMapInfo()
{
	if (!IsMapReady())
		return;

}

void CMapManagerAccessor::__RefreshMapID(const char * c_szMapName)
{
	std::string strMapName = c_szMapName;
	stl_lowers(strMapName);
	if (m_kMap_strMapName_iID.end() != m_kMap_strMapName_iID.find(strMapName))
	{
		int iID = m_kMap_strMapName_iID[strMapName];
		m_pMapAccessor->SetMapID(iID);
	}
}

CMapOutdoorAccessor * CMapManagerAccessor::GetMapOutdoorPtr()
{
	return m_pMapAccessor;
}

void CMapManagerAccessor::SetEditingCursorPosition(const Vector3 & c_rv3Position)
{
	m_v3EditingCursorPosition = c_rv3Position;
}

void CMapManagerAccessor::RenderGuildArea()
{
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.RenderMarkedArea();
}

METIN2_END_NS
