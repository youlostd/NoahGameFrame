#include "StdAfx.h"
#include "MapAccessorArea.h"
#include "MapAccessorOutdoor.h"

#include <EffectLib/EffectManager.h>
#include <GameLib/Property.h>
#include <EterBase/Utils.h>

#include <storm/io/Path.hpp>

METIN2_BEGIN_NS

/*
*  Effect Object 와 Tree Object 도 움직일수 있도록 충돌검사 추가.
*/
int CAreaAccessor::GetPickedObjectIndex()
{
	for (uint32_t i = 0; i < m_ObjectInstanceVector.size(); ++i)
	{
		const TObjectInstance& instance = m_ObjectInstanceVector[i];

		if (PROPERTY_TYPE_BUILDING == instance.dwType)
		{
			if (instance.pThingInstance)
			{
				float fu, fv, ft;
				if (instance.pThingInstance->Intersect(&fu, &fv, &ft))
					return i;
			}
		}
		else if (PROPERTY_TYPE_DUNGEON_BLOCK == instance.dwType)
		{
			if (instance.pDungeonBlock)
			{
				float fu, fv, ft;
				//Matrix & rmatTransform = instance.pDungeonBlock->GetTransform();
				if (instance.pDungeonBlock->Intersect(&fu, &fv, &ft))
					return i;
			}
		}
		else if (PROPERTY_TYPE_AMBIENCE == instance.dwType)
		{
			if (instance.pAmbienceInstance->Picking())
				return i;
		}
		else if (PROPERTY_TYPE_TREE == instance.dwType)
		{
			CSpeedTreeWrapper * pTree = instance.pTree;
			UINT uiCollisionObjectCount = pTree->GetCollisionObjectCount();

			for (UINT j = 0; j < uiCollisionObjectCount; ++j)
			{
				CSpeedTreeRT::ECollisionObjectType ObjectType;
				Vector3 v3Position(0.0f, 0.0f, 0.0f);
				float fDimensional[3];

#ifdef _SPEEDTREE_42_
				float fEulerAngles[3];
				pTree->GetCollisionObject(i, ObjectType, (float *)&v3Position, fDimensional, fEulerAngles);
#else
				pTree->GetCollisionObject(j, ObjectType, (float *)&v3Position, fDimensional,);
#endif

				if (ObjectType == CSpeedTreeRT::CO_SPHERE)
				{
					CSphereCollisionInstance* pSphereCollisionInstance = NULL;
					pSphereCollisionInstance = static_cast<CSphereCollisionInstance*>(pTree->GetCollisionInstanceData(j));

					if(pSphereCollisionInstance)
					{
						TSphereData& tSphereData = pSphereCollisionInstance->GetAttribute();
						if( pTree->IntersectSphere(tSphereData.v3Position, tSphereData.fRadius) )
							return i;
					}
				}
			}
		}
		else if (PROPERTY_TYPE_EFFECT == instance.dwType)
		{
			if (!instance.effect)
				continue;

			TObjectData * pObjectData;
			if (!GetObjectPointer(i, &pObjectData))
				continue;

			Vector3 v3Center = pObjectData->Position;
			v3Center.z += pObjectData->m_fHeightBias;

			//Effect 의 Radius 가 너무 제각각이라서 고정수치 하드코딩합니다.
			float fRadius = 200.0f;

			if (instance.effect->IntersectSphere(v3Center, fRadius))
				return i;
		}
	}

	return -1;
}

BOOL CAreaAccessor::Picking()
{
	for (uint32_t i = 0; i < m_ObjectInstanceVector.size(); ++i)
	{
		const TObjectInstance& instance = m_ObjectInstanceVector[i];

		if (PROPERTY_TYPE_TREE == instance.dwType)
		{
			CSpeedTreeWrapper * pTree = instance.pTree;
			UINT uiCollisionObjectCount = pTree->GetCollisionObjectCount();

			for (UINT i = 0; i < uiCollisionObjectCount; ++i)
			{
				CSpeedTreeRT::ECollisionObjectType ObjectType;
				Vector3 v3Position(0.0f, 0.0f, 0.0f);
				float fDimensional[3], fEulerAngles[3];

#ifdef _SPEEDTREE_42_
				pTree->GetCollisionObject(i, ObjectType, (float *)&v3Position, fDimensional, fEulerAngles);
#else
				pTree->GetCollisionObject(i, ObjectType, (float *)&v3Position, fDimensional,);
#endif

				if (ObjectType == CSpeedTreeRT::CO_SPHERE)
				{
				}
				else if (ObjectType == CSpeedTreeRT::CO_BOX)
				{
				}
			}
		}
		else if (PROPERTY_TYPE_BUILDING == instance.dwType)
		{
			if (instance.pThingInstance)
			{
				float fu, fv, ft;
				if (instance.pThingInstance->Intersect(&fu, &fv, &ft))
				{
					__ClickObject(i);
					return TRUE;
				}
			}
		}
		else if (PROPERTY_TYPE_DUNGEON_BLOCK == instance.dwType)
		{
			if (instance.pDungeonBlock)
			{
				float fu, fv, ft;
				Matrix & rmatTransform = instance.pDungeonBlock->GetTransform();
				if (instance.pDungeonBlock->Intersect(&fu, &fv, &ft))
				{
					__ClickObject(i);
					return TRUE;
				}
			}
		}
		else if (PROPERTY_TYPE_AMBIENCE == instance.dwType)
		{
			if (instance.pAmbienceInstance->Picking())
			{
				if (!IsSelectedObject(i))
				{
					CancelSelect();
					SelectObject(i);
				}
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CAreaAccessor::IsSelected()
{
	return (!m_SelectObjectList.empty());
}

typedef struct SSelectObjectRenderer : public CScreen
{
	SSelectObjectRenderer(CAreaAccessor::TObjectData & rObjectData, CAreaAccessor::TObjectInstance & rObjectInstance)
		: m_rObjectData(rObjectData)
		, m_rObjectInstance(rObjectInstance)
	{
		m_fRadius = 100.0f;
		m_bBoundBox = FALSE;

		CProperty * pProperty;
		if (!CPropertyManager::Instance().Get(m_rObjectData.dwCRC, &pProperty))
			return;

		const char * c_szPropertyType;
		if (!pProperty->GetString("PropertyType", &c_szPropertyType))
			return;

		switch (GetPropertyType(c_szPropertyType))
		{
			case PROPERTY_TYPE_BUILDING:
			{
				m_rObjectInstance.pThingInstance->GetBoundBox(&m_v3Min, &m_v3Max);
				m_fRotation = m_rObjectInstance.pThingInstance->GetRotation();
				m_bBoundBox = TRUE;
				break;
			}
			case PROPERTY_TYPE_DUNGEON_BLOCK:
			{
				m_rObjectInstance.pDungeonBlock->GetBoundBox(&m_v3Min, &m_v3Max);
				m_fRotation = m_rObjectInstance.pDungeonBlock->GetRotation();
				m_bBoundBox = TRUE;
				break;
			}
			case PROPERTY_TYPE_TREE:
			{
				m_fRadius = 50.0f;
				break;
			}
			case PROPERTY_TYPE_AMBIENCE:
			{
				m_fRadius = m_rObjectData.dwRange;
				break;
			}
		}
	}
	void Render()
	{
		/* light 관련 코드 추가
		*  특정 상황에서 의도치 않는 색깔로 그려지는 버그가 있었음.
		*  Light 영향으로 그런거라 무조건 light 끄고 Render 되도록 수정.
		*/
		uint32_t dwLightState = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
		STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

		SetDiffuseColor(0.0f, 1.0f, 0.0f);

		if (m_bBoundBox)
		{
			ms_lpd3dMatStack->Push();
			ms_lpd3dMatStack->RotateAxis(&Vector3(0.0f, 0.0f, 1.0f), DirectX::XMConvertToRadians(m_fRotation));
			ms_lpd3dMatStack->Translate(m_rObjectData.Position.x, m_rObjectData.Position.y, m_rObjectData.Position.z + m_rObjectData.m_fHeightBias);
			RenderLineCube(m_v3Min.x, m_v3Min.y, m_v3Min.z,
						   m_v3Max.x, m_v3Max.y, m_v3Max.z);
			ms_lpd3dMatStack->Pop();
		}
		else
		{
			STATEMANAGER.SetTransform(D3DTS_WORLD, &GetIdentityMatrix());
			RenderCircle2d(m_rObjectData.Position.x, m_rObjectData.Position.y, m_rObjectData.Position.z + m_rObjectData.m_fHeightBias, m_fRadius);
			RenderCircle2d(m_rObjectData.Position.x, m_rObjectData.Position.y, m_rObjectData.Position.z + m_rObjectData.m_fHeightBias, m_fRadius*2.0f);
			RenderCircle2d(m_rObjectData.Position.x, m_rObjectData.Position.y, m_rObjectData.Position.z + m_rObjectData.m_fHeightBias, m_fRadius*3.0f);
		}

		STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightState);
	}

	BOOL m_bBoundBox;
	Vector3 m_v3Min;
	Vector3 m_v3Max;

	float m_fRotation;
	float m_fRadius;
	CAreaAccessor::TObjectData & m_rObjectData;
	CAreaAccessor::TObjectInstance & m_rObjectInstance;
} TSelectObjectRenderer;

void CAreaAccessor::RenderSelectedObject()
{
	TSelectObjectList::iterator itor = m_SelectObjectList.begin();
	for (; itor != m_SelectObjectList.end(); ++itor)
	{
		SSelectObject & rSelectObject = *itor;

		TObjectData * pObjectData;
		if (!GetObjectPointer(rSelectObject.dwIndex, &pObjectData))
			continue;

		TObjectInstance * pObjectInstance;
		if (!GetInstancePointer(rSelectObject.dwIndex, &pObjectInstance))
			continue;

		TSelectObjectRenderer ObjectData(*pObjectData, *pObjectInstance);
		ObjectData.Render();
	}
}

/* 함수 원형 변경
* SelectObject(float fxStart, float fyStart, float fxEnd, float fyEnd) 에서
* SelectObject(float fxStart, float fyStart, float fxEnd, float fyEnd, SHORT sSpecialKeys) 로 변경
* 나중에 특수키가 뭘로 바뀔지 모르는 상황에서 VK_LCONTROL 을 코드에 강제로 넣기 싫었다.
* 특수키(sSpecialKeys)가 생긴 이유 : 오브젝트를 추가적으로 더 선택할수 있는 기능을 추가
* m_bAddedSelectObject 가 생긴 이유: 추가된 오브젝트가 있는지만 판별할 필요가 있었다.
*/
BOOL CAreaAccessor::SelectObject(float fxStart, float fyStart, float fxEnd, float fyEnd, SHORT sSpecialKeys)
{
	BOOL bNeedChange = FALSE;

	// 추가된 오브젝트 확인
	for (uint32_t i = 0; i < m_ObjectDataVector.size(); ++i)
	{
		TObjectData & rObjectData = m_ObjectDataVector[i];

		if (rObjectData.Position.x > fxStart)
		if (rObjectData.Position.y > fyStart)
		if (rObjectData.Position.x < fxEnd)
		if (rObjectData.Position.y < fyEnd)
		{
			if (!IsSelectedObject(i))
			{
				SelectObject(i);

				bNeedChange = TRUE;
				m_bAddedSelectObject = TRUE;

				if(sSpecialKeys)
					m_AddedSelectObjectList.push_back(i);
			}
		}
	}

	// 제외된 오브젝트 확인
	std::list<uint32_t> kList_iDeletingObjectList;

	if (0 == sSpecialKeys)
	{
		for (TSelectObjectList::iterator itor=m_SelectObjectList.begin(); itor!=m_SelectObjectList.end(); ++itor)
		{
			SSelectObject & rSelectObject = *itor;
			uint32_t dwNum = rSelectObject.dwIndex;
			TObjectData & rObjectData = m_ObjectDataVector[dwNum];

			if (rObjectData.Position.x < fxStart ||
				rObjectData.Position.y < fyStart ||
				rObjectData.Position.x > fxEnd ||
				rObjectData.Position.y > fyEnd)
			{
				if (IsSelectedObject(dwNum))
				{
					kList_iDeletingObjectList.push_back(dwNum);
					bNeedChange = TRUE;
				}
			}
		}
	}
	else
	{
		for (std::list<uint32_t>::iterator itor=m_AddedSelectObjectList.begin(); itor!=m_AddedSelectObjectList.end(); ++itor)
		{
			uint32_t dwNum = *itor;
			TObjectData & rObjectData = m_ObjectDataVector[dwNum];

			if (rObjectData.Position.x < fxStart ||
				rObjectData.Position.y < fyStart ||
				rObjectData.Position.x > fxEnd ||
				rObjectData.Position.y > fyEnd)
			{
				if (IsAddedSelectObject(dwNum))
				{
					kList_iDeletingObjectList.push_back(dwNum);
					bNeedChange = TRUE;
				}
			}
		}
	}

	for (std::list<uint32_t>::iterator itorDel=kList_iDeletingObjectList.begin(); itorDel!=kList_iDeletingObjectList.end(); ++itorDel)
	{
		DeselectObject(*itorDel);
	}

	return bNeedChange;
}

/* DeselectObject 추가
* 선택된 오브젝트 중에서 특정 오브젝트들을 해제하는 기능을 추가
*/
BOOL CAreaAccessor::DeselectObject(float fxStart, float fyStart, float fxEnd, float fyEnd)
{
	BOOL bNeedChange = FALSE;

	// 추가된 오브젝트 확인
	for (uint32_t i = 0; i < m_ObjectDataVector.size(); ++i)
	{
		TObjectData & rObjectData = m_ObjectDataVector[i];

		if (rObjectData.Position.x > fxStart)
		if (rObjectData.Position.y > fyStart)
		if (rObjectData.Position.x < fxEnd)
		if (rObjectData.Position.y < fyEnd)
		{
			if (IsSelectedObject(i))
			{
				DeselectObject(i);

				m_DeselectObjectList.push_back(i);

				bNeedChange = TRUE;
			}
		}
	}

	// 제외된 오브젝트 확인
	for (std::list<uint32_t>::iterator itor=m_DeselectObjectList.begin(); itor!=m_DeselectObjectList.end(); ++itor)
	{
		uint32_t dwNum = *itor;
		TObjectData & rObjectData = m_ObjectDataVector[dwNum];

		if (rObjectData.Position.x < fxStart ||
			rObjectData.Position.y < fyStart ||
			rObjectData.Position.x > fxEnd ||
			rObjectData.Position.y > fyEnd)
		{
			if (IsDeselectObject(dwNum))
			{
				SelectObject(dwNum);
				bNeedChange = TRUE;
			}
		}
	}

	return bNeedChange;
}

bool CAreaAccessor::IsDeselectObject(uint32_t dwNum)
{
	for(std::list<uint32_t>::iterator iter = m_DeselectObjectList.begin();
		iter != m_DeselectObjectList.end(); ++iter)
	{
		if(dwNum == *iter)
			return true;
	}

	return false;
}

void CAreaAccessor::DeleteSelectedObject()
{
	assert(m_ObjectInstanceVector.size() == m_ObjectDataVector.size());

	SPDLOG_WARN("DELETING {0} OBJECTS",
	           m_SelectObjectList.size(),
	           m_ObjectInstanceVector.size());

	for (auto& x : m_SelectObjectList) {
		SPDLOG_WARN("Index {0}", x.dwIndex);
	}

	// Iterate backwards to preserve indices as long as possible.
	for (std::size_t i = 0, s = m_ObjectDataVector.size(); i <= s; ++i) {
		// Cheap way to ensure that 0 is included.
		std::size_t d = s - i;

		if (!IsSelectedObject(d))
			continue;

		m_ObjectInstanceVector.erase(m_ObjectInstanceVector.begin() + d);
		m_ObjectDataVector.erase(m_ObjectDataVector.begin() + d);
	}

	// Re-build clone lists
	Refresh();

	CancelSelect();
}

void CAreaAccessor::MoveSelectedObject(float fx, float fy)
{
	__RefreshObjectPosition(fx, fy, 0.0f);
}

void CAreaAccessor::MoveSelectedObjectHeight(float fHeight)
{
	__RefreshObjectPosition(0.0f, 0.0f, fHeight);
}

void CAreaAccessor::AddSelectedAmbienceScale(int iAddScale)
{
	TSelectObjectList::reverse_iterator ritor;
	for (ritor = m_SelectObjectList.rbegin(); ritor != m_SelectObjectList.rend(); ++ritor)
	{
		SSelectObject & rSelectObject = *ritor;

		TObjectData * pObjectData;
		if (!GetObjectPointer(rSelectObject.dwIndex, &pObjectData))
			continue;

		TObjectInstance * pObjectInstance;
		if (!GetInstancePointer(rSelectObject.dwIndex, &pObjectInstance))
			continue;

		if (PROPERTY_TYPE_AMBIENCE != pObjectInstance->dwType)
			continue;

		auto pAmbienceInstance = pObjectInstance->pAmbienceInstance.get();
		if (!pAmbienceInstance)
			continue;

		pObjectData->dwRange = std::max(10, int(pObjectData->dwRange) + iAddScale);
		pAmbienceInstance->dwRange = std::max(10, int(pAmbienceInstance->dwRange) + iAddScale);
	}
}

void CAreaAccessor::AddSelectedAmbienceMaxVolumeAreaPercentage(float fPercentage)
{
	TSelectObjectList::reverse_iterator ritor;
	for (ritor = m_SelectObjectList.rbegin(); ritor != m_SelectObjectList.rend(); ++ritor)
	{
		SSelectObject & rSelectObject = *ritor;

		TObjectData * pObjectData;
		if (!GetObjectPointer(rSelectObject.dwIndex, &pObjectData))
			continue;

		TObjectInstance * pObjectInstance;
		if (!GetInstancePointer(rSelectObject.dwIndex, &pObjectInstance))
			continue;

		if (PROPERTY_TYPE_AMBIENCE != pObjectInstance->dwType)
			continue;

		auto pAmbienceInstance = pObjectInstance->pAmbienceInstance.get();
		if (!pAmbienceInstance)
			continue;

		pObjectData->fMaxVolumeAreaPercentage += fPercentage;
		pObjectData->fMaxVolumeAreaPercentage = fMIN(1.0f, pObjectData->fMaxVolumeAreaPercentage);
		pObjectData->fMaxVolumeAreaPercentage = fMAX(0.0f, pObjectData->fMaxVolumeAreaPercentage);
		pAmbienceInstance->fMaxVolumeAreaPercentage = pObjectData->fMaxVolumeAreaPercentage;
	}
}

void CAreaAccessor::AddSelectedObjectRotation(float fYaw, float fPitch, float fRoll)
{
	TSelectObjectList::iterator itor;
	for (itor = m_SelectObjectList.begin(); itor != m_SelectObjectList.end(); ++itor)
	{
		SSelectObject & rSelectObject = *itor;

		TObjectData * pObjectData;
		if (!GetObjectPointer(rSelectObject.dwIndex, &pObjectData))
			continue;

		pObjectData->m_fYaw = fmod(pObjectData->m_fYaw + fYaw + 360.0f, 360.0f);
		pObjectData->m_fPitch = fmod(pObjectData->m_fPitch + fPitch + 360.0f, 360.0f);
		pObjectData->m_fRoll = fmod(pObjectData->m_fRoll + fRoll + 360.0f, 360.0f);

		////////////////////////////
		// SetPosition
		TObjectInstance * pObjectInstance;
		if (!GetInstancePointer(rSelectObject.dwIndex, &pObjectInstance))
			continue;

		if (pObjectInstance->effect) {
			Matrix mat;
			DirectX::SimpleMath::MatrixRotationYawPitchRoll(&mat,
				DirectX::XMConvertToRadians(pObjectData->m_fYaw),
				DirectX::XMConvertToRadians(pObjectData->m_fPitch),
				DirectX::XMConvertToRadians(pObjectData->m_fRoll)
			);

			mat._41 = pObjectData->Position.x;
			mat._42 = pObjectData->Position.y;
			mat._43 = pObjectData->Position.z + pObjectData->m_fHeightBias;

			pObjectInstance->effect->SetGlobalMatrix(std::move(mat));
		}

		if (pObjectInstance->pTree)
		{
			/*pObjectInstance->pTree->SetRotation(pObjectData->m_fYaw, pObjectData->m_fPitch, pObjectData->m_fRoll);
			pObjectInstance->pTree->Update();
			pObjectInstance->pTree->Transform();
			pObjectInstance->pTree->Deform();*/
		}
		if (pObjectInstance->pThingInstance)
		{
			pObjectInstance->pThingInstance->SetRotation(pObjectData->m_fYaw, pObjectData->m_fPitch, pObjectData->m_fRoll);
			pObjectInstance->pThingInstance->Update();
			pObjectInstance->pThingInstance->Transform();
			pObjectInstance->pThingInstance->Deform();
		}
		if (pObjectInstance->pDungeonBlock)
		{
			pObjectInstance->pDungeonBlock->SetRotation(pObjectData->m_fYaw, pObjectData->m_fPitch, pObjectData->m_fRoll);
			pObjectInstance->pDungeonBlock->Update();
			pObjectInstance->pDungeonBlock->Transform();
			pObjectInstance->pDungeonBlock->Deform();
		}
	}
}

void CAreaAccessor::SetSelectedObjectPortalNumber(int iID)
{
	TSelectObjectList::reverse_iterator ritor;
	for (ritor = m_SelectObjectList.rbegin(); ritor != m_SelectObjectList.rend(); ++ritor)
	{
		SSelectObject & rSelectObject = *ritor;

		TObjectData * pObjectData;
		if (!GetObjectPointer(rSelectObject.dwIndex, &pObjectData))
			continue;

		TObjectInstance * pObjectInstance;
		if (!GetInstancePointer(rSelectObject.dwIndex, &pObjectInstance))
			continue;

		BOOL hasSameID = FALSE;
		int i;
		for (i = 0; i < PORTAL_ID_MAX_NUM; ++i)
		{
			if (iID == pObjectData->abyPortalID[i])
			{
				hasSameID = TRUE;
				break;
			}
			if (0 == pObjectData->abyPortalID[i])
			{
				break;
			}
		}

		if (hasSameID)
			continue;

		if (i >= PORTAL_ID_MAX_NUM)
		{
			assert(!"포탈 번호 슬롯 개수를 초과했습니다");
			continue;
		}

		pObjectData->abyPortalID[i] = iID;
	}

	__RefreshSelectedInfo();
}

void CAreaAccessor::DelSelectedObjectPortalNumber(int iID)
{
	TSelectObjectList::reverse_iterator ritor;
	for (ritor = m_SelectObjectList.rbegin(); ritor != m_SelectObjectList.rend(); ++ritor)
	{
		SSelectObject & rSelectObject = *ritor;

		TObjectData * pObjectData;
		if (!GetObjectPointer(rSelectObject.dwIndex, &pObjectData))
			continue;

		TObjectInstance * pObjectInstance;
		if (!GetInstancePointer(rSelectObject.dwIndex, &pObjectInstance))
			continue;

		for (int i = 0; i < PORTAL_ID_MAX_NUM; ++i)
		{
			if (iID == pObjectData->abyPortalID[i])
			{
				for (int j = i; j < PORTAL_ID_MAX_NUM-1; ++j)
				{
					pObjectData->abyPortalID[j] = pObjectData->abyPortalID[j+1];
				}

				pObjectData->abyPortalID[PORTAL_ID_MAX_NUM-1] = 0;
				break;
			}
		}
	}

	__RefreshSelectedInfo();
}

void CAreaAccessor::CollectPortalNumber(std::set<int> * pkSet_iPortalNumber)
{
	for (uint32_t i = 0; i < m_ObjectDataVector.size(); ++i)
	{
		TObjectData & rObjectData = m_ObjectDataVector[i];
		for (int j = 0; j < PORTAL_ID_MAX_NUM; ++j)
		{
			if (0 == rObjectData.abyPortalID[j])
				break;

			pkSet_iPortalNumber->insert(rObjectData.abyPortalID[j]);
		}
	}
}

void CAreaAccessor::AddObject(const TObjectData* c_pObjectData)
{
	auto it = std::upper_bound(m_ObjectDataVector.begin(), m_ObjectDataVector.end(),
	                           *c_pObjectData, ObjectDataComp());
	it = m_ObjectDataVector.insert(it, *c_pObjectData);

	auto it2 = m_ObjectInstanceVector.begin() + (it - m_ObjectDataVector.begin());
	it2 = m_ObjectInstanceVector.emplace(it2);

	__SetObjectInstance(&*it2, &*it);

	Refresh();
	CancelSelect();
}

bool CAreaAccessor::GetObjectPointer(uint32_t dwIndex, TObjectData ** ppObjectData)
{
	if (!CheckObjectIndex(dwIndex))
	{
		assert(!"CAreaAccessor::GetObjectPointer() : Object Index overflow!");
		return false;
	}

	*ppObjectData = &m_ObjectDataVector[dwIndex];
	return true;
}

bool CAreaAccessor::Save(const std::string & c_rstrMapName)
{

	uint32_t dwID = (uint32_t) (m_wX) * 1000L + (uint32_t) (m_wY);

	char szFileName[256];
	sprintf(szFileName, "%s\\%06u\\AreaData.txt", c_rstrMapName.c_str(), dwID);
	char szAmbiencePathName[256+1];
	char szAmbienceFileName[256+1];

	if (!__SaveObjects(szFileName))
		return false;

	_snprintf(szAmbiencePathName, 256, "%s\\%06u\\", c_rstrMapName.c_str(), dwID);
	_snprintf(szAmbienceFileName, 256, "%s\\%06u\\AreaAmbienceData.txt", c_rstrMapName.c_str(), dwID);
	if (!__SaveAmbiences(szAmbiencePathName, szAmbienceFileName))
		return false;

	return true;
}

bool CAreaAccessor::__SaveObjects(const char * filename)
{
	FILE * File = fopen(filename, "w");

	if (!File) {
		spdlog::error("Could not open object file {0}", filename);
		return false;
	}

	fprintf(File, "AreaDataFile\n");
	fprintf(File, "\n");

	uint32_t dwObjectDataCount = 0;

	for (uint32_t i = 0; i < GetObjectDataCount(); ++i)
	{
		const TObjectData * c_pObjectData;

		if (!GetObjectDataPointer(i, &c_pObjectData))
			continue;

		CProperty * pProperty;
		if (!CPropertyManager::Instance().Get(c_pObjectData->dwCRC, &pProperty))
			continue;
		const char * c_szPropertyType;
		if (!pProperty->GetString("PropertyType", &c_szPropertyType))
			continue;

		uint32_t dwPropertyType = GetPropertyType(c_szPropertyType);

		if (PROPERTY_TYPE_AMBIENCE == dwPropertyType)
			continue;

		fprintf(File, "Start Object%03d\n", dwObjectDataCount);
		fprintf(File, "    %f %f %f\n", c_pObjectData->Position.x, c_pObjectData->Position.y, c_pObjectData->Position.z);
		fprintf(File, "    %u\n", c_pObjectData->dwCRC);
		fprintf(File, "    %f#%f#%f\n", c_pObjectData->m_fYaw, c_pObjectData->m_fPitch, c_pObjectData->m_fRoll);
		fprintf(File, "    %f\n", c_pObjectData->m_fHeightBias);

		if (0 != c_pObjectData->abyPortalID[0])
		{
			std::set<int> kSet_iPortalID;

			fprintf(File, "   ");
			for (int p = 0; p < PORTAL_ID_MAX_NUM; ++p)
			{
				BYTE byPortalID = c_pObjectData->abyPortalID[p];
				if (0 == byPortalID)
					break;
				if (kSet_iPortalID.end() != kSet_iPortalID.find(byPortalID))
					continue;

				fprintf(File, " %d", byPortalID);
				kSet_iPortalID.insert(byPortalID);
			}
			fprintf(File, "\n");
		}

		fprintf(File, "End Object\n");

		++dwObjectDataCount;
	}

	fprintf(File, "\n");
	fprintf(File, "ObjectCount %d\n", dwObjectDataCount);

	fclose(File);
	return true;
}

bool CAreaAccessor::__SaveAmbiences(const char * c_szPathName, const char * c_szFileName)
{
	MyCreateDirectory(c_szPathName);

	//////////////////////////////////////////////////////

	FILE * File = fopen(c_szFileName, "w");

	if (!File)
		return false;

	fprintf(File, "AreaAmbienceDataFile\n");
	fprintf(File, "\n");

	uint32_t dwObjectDataCount = 0;

	for (uint32_t i = 0; i < GetObjectDataCount(); ++i)
	{
		const TObjectData * c_pObjectData;

		if (!GetObjectDataPointer(i, &c_pObjectData))
			continue;

		CProperty * pProperty;
		if (!CPropertyManager::Instance().Get(c_pObjectData->dwCRC, &pProperty))
			continue;
		const char * c_szPropertyType;
		if (!pProperty->GetString("PropertyType", &c_szPropertyType))
			continue;

		uint32_t dwPropertyType = GetPropertyType(c_szPropertyType);

		if (PROPERTY_TYPE_AMBIENCE == dwPropertyType)
		{
			fprintf(File, "Start Object%03d\n", dwObjectDataCount);
			fprintf(File, "    %f %f %f\n", c_pObjectData->Position.x, c_pObjectData->Position.y, c_pObjectData->Position.z);
			fprintf(File, "    %u\n", c_pObjectData->dwCRC);
			fprintf(File, "    %u\n", c_pObjectData->dwRange);
			fprintf(File, "    %f\n", c_pObjectData->fMaxVolumeAreaPercentage);
			fprintf(File, "End Object\n");

			++dwObjectDataCount;
		}
	}

	fprintf(File, "\n");
	fprintf(File, "ObjectCount %d\n", dwObjectDataCount);

	fclose(File);
	return true;
}

void CAreaAccessor::RefreshArea()
{
	SPDLOG_DEBUG("REFRESHING AREA");
	__Load_BuildObjectInstances();
	Refresh();
}

void CAreaAccessor::UpdateObject(uint32_t dwIndex, const TObjectData * c_pObjectData)
{
	TObjectInstance * pObjectInstance;
	if (!GetInstancePointer(dwIndex, &pObjectInstance))
	{
		assert(!"Too large index to update!");
		return;
	}

	if (PROPERTY_TYPE_TREE == pObjectInstance->dwType)
	{
		//pObjectInstance->pTree->SetPosition(c_pObjectData->Position.x, c_pObjectData->Position.y, c_pObjectData->Position.z + c_pObjectData->m_fHeightBias);
	}
	else if (PROPERTY_TYPE_BUILDING == pObjectInstance->dwType)
	{
		pObjectInstance->pThingInstance->SetPosition(c_pObjectData->Position.x, c_pObjectData->Position.y, c_pObjectData->Position.z + c_pObjectData->m_fHeightBias);
		pObjectInstance->pThingInstance->Transform();
		pObjectInstance->pThingInstance->Update();
		pObjectInstance->pThingInstance->Deform();
	}
}

bool CAreaAccessor::CheckInstanceIndex(uint32_t dwIndex)
{
	if (dwIndex >= m_ObjectInstanceVector.size())
		return false;

	return true;
}

bool CAreaAccessor::GetInstancePointer(uint32_t dwIndex, TObjectInstance ** ppObjectInstance)
{
	if (!CheckInstanceIndex(dwIndex))
		return false;

	*ppObjectInstance = &m_ObjectInstanceVector[dwIndex];
	return true;
}

struct FCompareSelectObjectList
{
	uint32_t m_dwCheckIndex;
	FCompareSelectObjectList(uint32_t dwCheckIndex) : m_dwCheckIndex(dwCheckIndex)
	{
	}
	int operator () (const CAreaAccessor::SSelectObject & c_rSelectObject)
	{
		return c_rSelectObject.dwIndex == m_dwCheckIndex;
	}
};

BOOL CAreaAccessor::IsSelectedObject(uint32_t dwIndex)
{
	auto itor = std::find_if(m_SelectObjectList.begin(),
	                         m_SelectObjectList.end(),
	                         FCompareSelectObjectList(dwIndex));
	return itor != m_SelectObjectList.end();
}

bool CAreaAccessor::IsAddedSelectObject(uint32_t dwNum)
{
	for(std::list<uint32_t>::iterator iter = m_AddedSelectObjectList.begin();
		iter != m_AddedSelectObjectList.end(); ++iter)
	{
		if(dwNum == *iter)
			return true;
	}

	return false;
}

void CAreaAccessor::ClearAddedSelectObject()
{
	m_bAddedSelectObject = FALSE;
	m_AddedSelectObjectList.clear();
}

void CAreaAccessor::ClearDeselectObject()
{
	m_DeselectObjectList.clear();
}

void CAreaAccessor::__ClickObject(uint32_t dwIndex)
{
	if (IsSelectedObject(dwIndex))
	{
		DeselectObject(dwIndex);
		return;
	}

	SelectObject(dwIndex);
}

int CAreaAccessor::GetSelectedObjectCount()
{
	return m_SelectObjectList.size();
}

void CAreaAccessor::SelectObject(uint32_t dwIndex)
{
	if (dwIndex >= m_ObjectDataVector.size())
		return;

	if (IsSelectedObject(dwIndex))
		return;

	TObjectData & rObjectData = m_ObjectDataVector[dwIndex];

	SSelectObject SelectObject;
	SelectObject.dwIndex = dwIndex;
	SelectObject.dwCRC32 = rObjectData.dwCRC;
	m_SelectObjectList.push_back(SelectObject);

	__RefreshSelectedInfo();
}

void CAreaAccessor::DeselectObject(uint32_t dwIndex)
{
	for (TSelectObjectList::iterator itor=m_SelectObjectList.begin(); itor!=m_SelectObjectList.end(); ++itor)
	{
		SSelectObject & rSelectObject = *itor;
		if (dwIndex == rSelectObject.dwIndex)
		{
			if (1 == m_SelectObjectList.size())
			{
				CancelSelect();
				return;
			}

			m_SelectObjectList.erase(itor);
			__RefreshSelectedInfo();
			return;
		}
	}
}

void CAreaAccessor::CancelSelect()
{
	m_SelectObjectList.clear();
	__RefreshSelectedInfo();
}

void CAreaAccessor::__RefreshSelectedInfo()
{
	CMapOutdoorAccessor * pMapAccessor = (CMapOutdoorAccessor *)GetOwner();
	pMapAccessor->ClearSelectedPortalNumber();
	if (m_SelectObjectList.empty())
	{
		pMapAccessor->SetSelectedObjectName("None");
	}
	else
	{
		// Name
		if (1 == m_SelectObjectList.size())
		{
			SSelectObject & rSelectObject = *m_SelectObjectList.begin();
			TObjectData & rObjectData = m_ObjectDataVector[rSelectObject.dwIndex];

			CProperty * pProperty;
			if (CPropertyManager::Instance().Get(rObjectData.dwCRC, &pProperty))
			{
				const char * c_szObjectName = _getf("[%u] %s", pProperty->GetCRC(), pProperty->GetFileName());
				pMapAccessor->SetSelectedObjectName(c_szObjectName);
			}
		}
		else
		{
			const char * c_szObjectName = _getf("Multiple objects");
			pMapAccessor->SetSelectedObjectName(c_szObjectName);
		}

		// Portal Number
		std::map<uint32_t, uint32_t> kMap_iPortalNumber;
		for (TSelectObjectList::iterator itor=m_SelectObjectList.begin(); itor!=m_SelectObjectList.end(); ++itor)
		{
			SSelectObject & rSelectObject = *itor;
			TObjectData & rObjectData = m_ObjectDataVector[rSelectObject.dwIndex];

			CProperty * pProperty;
			if (!CPropertyManager::Instance().Get(rObjectData.dwCRC, &pProperty))
				continue;
			for (int i = 0; i < PORTAL_ID_MAX_NUM; ++i)
			{
				BYTE byNumber = rObjectData.abyPortalID[i];
				if (0 == byNumber)
					continue;

				if (kMap_iPortalNumber.end() == kMap_iPortalNumber.find(byNumber))
				{
					kMap_iPortalNumber[byNumber] = 0;
				}

				++kMap_iPortalNumber[byNumber];
			}
		}

		std::map<uint32_t, uint32_t>::iterator itorNum = kMap_iPortalNumber.begin();
		for (; itorNum != kMap_iPortalNumber.end(); ++itorNum)
		{
			if (itorNum->second >= m_SelectObjectList.size())
			{
				BYTE byNumber = itorNum->first;
				pMapAccessor->AddSelectedObjectPortalNumber(byNumber);
			}
		}
	}
}

// 가장 마지막에 선택한 오브젝트의 정보(위치, 회전 등..)를 리턴
const CArea::TObjectData* CAreaAccessor::GetLastSelectedObjectData() const
{
	if (m_SelectObjectList.empty())
		return NULL;

	const SSelectObject& rSelectObject = m_SelectObjectList.back();

	return &m_ObjectDataVector[rSelectObject.dwIndex];
}

/*
*  오브젝트의 위치 갱신과 함께 충돌정보도 갱신되게 수정.
*/
void CAreaAccessor::__RefreshObjectPosition(float fx, float fy, float fz)
{
	TSelectObjectList::iterator itor;
	for (itor = m_SelectObjectList.begin(); itor != m_SelectObjectList.end(); ++itor)
	{
		SSelectObject & rSelectObject = *itor;

		TObjectData * pObjectData;
		if (!GetObjectPointer(rSelectObject.dwIndex, &pObjectData))
			continue;

		Vector3 & rv3Position = pObjectData->Position;
		rv3Position.x += fx;
		rv3Position.y += fy;
		//rv3Position.z += fz;
		pObjectData->m_fHeightBias += fz;

		////////////////////////////
		// SetPosition
		TObjectInstance * pObjectInstance;
		if (!GetInstancePointer(rSelectObject.dwIndex, &pObjectInstance))
			continue;

		if (pObjectInstance->effect) {
			Matrix mat;
			DirectX::SimpleMath::MatrixRotationYawPitchRoll(&mat,
				DirectX::XMConvertToRadians(pObjectData->m_fYaw),
				DirectX::XMConvertToRadians(pObjectData->m_fPitch),
				DirectX::XMConvertToRadians(pObjectData->m_fRoll)
			);

			mat._41 = pObjectData->Position.x;
			mat._42 = pObjectData->Position.y;
			mat._43 = pObjectData->Position.z + pObjectData->m_fHeightBias;

			pObjectInstance->effect->SetGlobalMatrix(std::move(mat));
		}

		if (pObjectInstance->pTree)
		{
			Vector3 temp(rv3Position);
			temp.z += pObjectData->m_fHeightBias;

		}

		if (pObjectInstance->pThingInstance)
		{
			pObjectInstance->pThingInstance->SetPosition(rv3Position.x, rv3Position.y, rv3Position.z + pObjectData->m_fHeightBias);
			pObjectInstance->pThingInstance->Update();
			pObjectInstance->pThingInstance->Transform();
			pObjectInstance->pThingInstance->Deform();
			pObjectInstance->pThingInstance->UpdateBoundingSphere();

			if (pObjectInstance->pAttributeInstance)
			{
				pObjectInstance->pThingInstance->UpdateCollisionData(&pObjectInstance->pAttributeInstance->GetObjectPointer()->GetCollisionDataVector());
				pObjectInstance->pAttributeInstance->RefreshObject(pObjectInstance->pThingInstance->GetTransform());
				pObjectInstance->pThingInstance->UpdateHeightInstance(pObjectInstance->pAttributeInstance.get());
			}
		}

		if (pObjectInstance->pDungeonBlock)
		{
			pObjectInstance->pDungeonBlock->SetPosition(rv3Position.x, rv3Position.y, rv3Position.z + pObjectData->m_fHeightBias);
			pObjectInstance->pDungeonBlock->Update();
			pObjectInstance->pDungeonBlock->Transform();
			pObjectInstance->pDungeonBlock->Deform();
			pObjectInstance->pDungeonBlock->UpdateBoundingSphere();

			if(pObjectInstance->pAttributeInstance)
			{
				pObjectInstance->pDungeonBlock->UpdateCollisionData(&pObjectInstance->pAttributeInstance->GetObjectPointer()->GetCollisionDataVector());
				pObjectInstance->pAttributeInstance->RefreshObject(pObjectInstance->pDungeonBlock->GetTransform());
				pObjectInstance->pDungeonBlock->UpdateHeightInstance(pObjectInstance->pAttributeInstance.get());
			}
		}

		if (pObjectInstance->pAmbienceInstance)
		{
			pObjectInstance->pAmbienceInstance->fx = rv3Position.x;
			pObjectInstance->pAmbienceInstance->fy = rv3Position.y;
			pObjectInstance->pAmbienceInstance->fz = rv3Position.z + pObjectData->m_fHeightBias;
		}
	}
}

/*
*  외부에서도 오브젝트의 Height 값을 알수있도록 인터페이스 제공.
*  여기서 Height 값은 오브젝트의 밑면 높이 정보를 말한다.
*/
bool CAreaAccessor::GetObjectHeight(float fX, float fY, float* fHeight)
{
	for (auto& instance : m_ObjectInstanceVector)
	{
		if (!instance.pAttributeInstance)
			continue;

		if (instance.pAttributeInstance->GetHeight(fX, fY, fHeight))
			return true;
	}

	return false;
}

template <typename Function>
void ProcessLine(int sx, int sy, int ex, int ey, Function&& f)
{
	int         mode = 0;
	int         w, h;
	int         x = 0, y = 0;
	int         d;
	int         px, py;

	// 0: dx > 0 && dy > 0
	// 1: dx < 0 && dy > 0
	// 2: dx > 0 && dy < 0
	// 3: dx < 0 && dy < 0
	if (sx > ex)
	{
		mode |= 1;
		w = sx - ex;
	}
	else
		w = ex - sx;

	if (sy > ey)
	{
		mode |= 2;
		h = sy - ey;
	}
	else
		h = ey - sy;

	// distance = (w * w) + (h * h); // °Å¸®
	if (w > h)
	{
		d = (h << 1) - w;

		while (x < w)
		{
			if (d < 0)
			{
				d += (h << 1);
				x++;
			}
			else
			{
				d += ((h - w) << 1);

				x++;
				y++;
			}

			switch (mode)
			{
				case 0: px = sx + x; py = sy + y; break;
				case 1: px = sx - x; py = sy + y; break;
				case 2: px = sx + x; py = sy - y; break;
				case 3: px = sx - x; py = sy - y; break;
				default: return;
			}

			f(px, py);
		}
	}
	else
	{
		d = h - (w << 1);

		while (y < h)
		{
			if (d > 0)
			{
				d += -(w << 1);
				y++;
			}
			else
			{
				d += ((h - w) << 1);
				x++;
				y++;
			}

			switch (mode)
			{
				case 0: px = sx + x; py = sy + y; break;
				case 1: px = sx - x; py = sy + y; break;
				case 2: px = sx + x; py = sy - y; break;
				case 3: px = sx - x; py = sy - y; break;
				default: return;
			}

			f(px, py);
		}
	}
}

template <typename Function>
void ProcessSphere(int x, int y, float fRadius, Function&& f)
{
	int r = (int)fRadius + 1;

	for (int rx = -r; rx < r; ++rx) {
		for (int ry = -r; ry < r; ++ry) {
			if (rx*rx + ry*ry >= r*r)
				continue;

			f(rx + x, ry + y);
		}
	}
}

void PlaceCollisionOnAttr(CBaseCollisionInstance* base, uint8_t* attr)
{
	auto f = [attr] (int32_t x, int32_t y) {
		if (x < 0 || y < 0)
			return;

		x /= CTerrainImpl::HALF_CELLSCALE;
		y /= CTerrainImpl::HALF_CELLSCALE;

		x %= CTerrainImpl::ATTRMAP_XSIZE;
		y %= CTerrainImpl::ATTRMAP_YSIZE;

		attr[y * CTerrainImpl::ATTRMAP_XSIZE + x] &= CTerrainImpl::ATTRIBUTE_BLOCK;
	};

	if (auto sphere = dynamic_cast<CSphereCollisionInstance*>(base)) {
		ProcessSphere(sphere->GetAttribute().v3Position.x,
		              -sphere->GetAttribute().v3Position.y,
		              sphere->GetAttribute().fRadius,
		              f);
	} else if (auto plane = dynamic_cast<CPlaneCollisionInstance*>(base)) {
		int pairs[6][2] = {
			{ 0, 3 },
			{ 0, 1 },
			{ 0, 2 },
			{ 1, 2 },
			{ 1, 3 },
			{ 2, 3 },
		};

		for (int iPairIndex = 0; iPairIndex < 6; iPairIndex++) {
			int iFrom = pairs[iPairIndex][0];
			int iTo = pairs[iPairIndex][1];

			ProcessLine(plane->GetAttribute().v3QuadPosition[iFrom].x,
			            -plane->GetAttribute().v3QuadPosition[iFrom].y,
			            plane->GetAttribute().v3QuadPosition[iTo].x,
			            -plane->GetAttribute().v3QuadPosition[iTo].y,
			            f);
		}
	} else if (auto cylinder = dynamic_cast<CCylinderCollisionInstance*>(base)) {
		ProcessSphere(cylinder->GetAttribute().v3Position.x,
		              -cylinder->GetAttribute().v3Position.y,
		              cylinder->GetAttribute().fRadius,
		              f);
	}
}

void CopySingleObjectAttr(CGraphicObjectInstance* obj, uint8_t* attr)
{
	const auto count = obj->GetCollisionInstanceCount();
	for (uint32_t i = 0; i != count; ++i)
		PlaceCollisionOnAttr(obj->GetCollisionInstanceData(i), attr);
}

void CAreaAccessor::CopyObjectAttr(uint8_t* attr)
{
	for (const auto& objectInstance : m_ObjectInstanceVector) {
		if (objectInstance.pDungeonBlock)
			CopySingleObjectAttr(objectInstance.pDungeonBlock.get(), attr);

		if (objectInstance.pThingInstance)
			CopySingleObjectAttr(objectInstance.pThingInstance, attr);
	}
}

//////////////////////////////////////////////////////////////////////////
// ShadowMap
void CAreaAccessor::RenderToShadowMap()
{
	for (uint32_t i = 0; i < m_ShadowThingCloneInstaceVector.size(); ++i)
		m_ShadowThingCloneInstaceVector[i]->RenderToShadowMap();
}
// ShadowMap
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void CAreaAccessor::ReloadBuildingTexture()
{
	CGraphicThingInstance* pkThingInst;
	TThingInstanceVector::iterator i=m_ThingCloneInstaceVector.begin();
	while (i!=m_ThingCloneInstaceVector.end())
	{
		pkThingInst=*i++;
		pkThingInst->ReloadTexture();
	}
}

/*
*  map tool 에서 Texture Reload 기능을 위해 추가
*/
void CAreaAccessor::ReloadDungeonBlockTexture()
{
	CDungeonBlock* pDungeonBlock = NULL;
	TDungeonBlockInstanceVector::iterator iter = m_DungeonBlockCloneInstanceVector.begin();
	while(iter!= m_DungeonBlockCloneInstanceVector.end())
	{
		pDungeonBlock = *iter++;
		pDungeonBlock->ReloadTexture();
	}
}
//////////////////////////////////////////////////////////////////////////
void CAreaAccessor::UndoClear()
{
	CArea::Clear();
}

void CAreaAccessor::Clear()
{
	m_DeselectObjectList.clear();
	m_AddedSelectObjectList.clear();
	m_SelectObjectList.clear();
	CArea::Clear();
}

CAreaAccessor::CAreaAccessor() : CArea()
{
}

CAreaAccessor::~CAreaAccessor()
{
}

METIN2_END_NS
