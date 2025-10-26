#include "StdAfx.h"
#define WORLD_EDITOR

#include <gamelib/ItemData.h>

METIN2_BEGIN_NS

///////////////////////////////////////////////////////////////////////////////////////////////////
// Model
const char * CObjectData::GetModelFileName()
{
	return m_strModelName.c_str();
}

void CObjectData::ClearModelData()
{
	m_AttachingDataVector.clear();
}

BOOL CObjectData::isModelThing()
{
	return NULL != m_pModelThing;
}

BOOL CObjectData::SetModelThing(const char * c_szFileName)
{
	m_pModelThing = CResourceManager::Instance().LoadResource<CGraphicThing>(c_szFileName);
	m_strModelName = c_szFileName;
	return true;
}

BOOL CObjectData::GetModelThing(CGraphicThing::Ptr* ppThing)
{
	*ppThing = m_pModelThing;
	return TRUE;
}

int & CObjectData::GetModelTypeReference()
{
	return m_iModelType;
}

uint32_t CObjectData::GetAttachingDataCount()
{
	return m_AttachingDataVector.size();
}

BOOL CObjectData::GetAttachingDataPointer(uint32_t dwIndex, NRaceData::TAttachingData ** ppAttachingData)
{
	if (dwIndex >= GetAttachingDataCount())
		return FALSE;

	*ppAttachingData = &m_AttachingDataVector[dwIndex];

	return TRUE;
}

BOOL CObjectData::GetCollisionDataPointer(uint32_t dwIndex, NRaceData::TAttachingData ** ppAttachingData)
{
	if (dwIndex >= GetAttachingDataCount())
		return FALSE;

	if (NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA != m_AttachingDataVector[dwIndex].dwType)
		return FALSE;

	*ppAttachingData = &m_AttachingDataVector[dwIndex];

	return TRUE;
}

BOOL CObjectData::GetObjectDataPointer(uint32_t dwIndex, NRaceData::TAttachingData ** ppAttachingData)
{
	if (dwIndex >= GetAttachingDataCount())
		return FALSE;

	//if (NRaceData::ATTACHING_DATA_TYPE_OBJECT != m_AttachingDataVector[dwIndex].dwType)
	//	return FALSE;
	switch(m_AttachingDataVector[dwIndex].dwType)
	{
		case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
		case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
			*ppAttachingData = &m_AttachingDataVector[dwIndex];
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

uint32_t CObjectData::CreateAttachingCollisionData()
{
	NRaceData::TAttachingData AttachingData;
	m_AttachingDataVector.push_back(AttachingData);

	NRaceData::TAttachingData & rData = m_AttachingDataVector[m_AttachingDataVector.size()-1];
	rData.dwType = NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA;
	rData.isAttaching = TRUE;
	rData.dwAttachingModelIndex = 0;
	rData.strAttachingBoneName = "Bip01";
	rData.pCollisionData = m_AttachingCollisionDataPool.Alloc();
	rData.pCollisionData->iCollisionType = NRaceData::COLLISION_TYPE_BODY;
	rData.pCollisionData->SphereDataVector.clear();

	// Make Initialize Data
	rData.pCollisionData->SphereDataVector.resize(1);

	for (uint32_t i = 0; i < rData.pCollisionData->SphereDataVector.size(); ++i)
	{
		TSphereData & r = rData.pCollisionData->SphereDataVector[i].GetAttribute();

		r.v3Position = Vector3(0.0f, 0.0f, 0.0f);
		r.fRadius = 50.0f;
	}
	// Make Initialize Data

	return m_AttachingDataVector.size() - 1;
}

uint32_t CObjectData::CreateAttachingObjectData()
{
	NRaceData::TAttachingData AttachingData;
	m_AttachingDataVector.push_back(AttachingData);

	NRaceData::TAttachingData & rData = m_AttachingDataVector[m_AttachingDataVector.size()-1];
	rData.dwType = NRaceData::ATTACHING_DATA_TYPE_OBJECT;
	rData.isAttaching = TRUE;
	rData.dwAttachingModelIndex = 0;
	rData.strAttachingBoneName = "Bip01";
	rData.pObjectData = m_AttachingObjectDataPool.Alloc();
	rData.pObjectData->strFileName = "";

	return m_AttachingDataVector.size() - 1;
}

uint32_t CObjectData::CreateAttachingEffectData()
{
	NRaceData::TAttachingData AttachingData;
	m_AttachingDataVector.push_back(AttachingData);

	NRaceData::TAttachingData & rData = m_AttachingDataVector[m_AttachingDataVector.size()-1];
	rData.dwType = NRaceData::ATTACHING_DATA_TYPE_EFFECT;
	rData.isAttaching = TRUE;
	rData.dwAttachingModelIndex = 0;
	rData.strAttachingBoneName = "Bip01";
	rData.pEffectData = m_AttachingEffectDataPool.Alloc();
	rData.pEffectData->strFileName = "";
	//rData.pEffectData->qRotation = Quaternion(0.0f,0.0f,0.0f,1.0f);
	rData.pEffectData->v3Rotation = Vector3(0.0f,0.0f,0.0f);
	rData.pEffectData->v3Position = Vector3(0.0f,0.0f,0.0f);

	return m_AttachingDataVector.size() - 1;
}


void CObjectData::DeleteOneAttachingData(uint32_t dwIndex)
{
	if (m_AttachingDataVector.empty())
		return;

	m_AttachingDataVector.erase(m_AttachingDataVector.begin() + dwIndex);
}
void CObjectData::DeleteAllAttachingData()
{
	m_AttachingDataVector.clear();
}

uint32_t & CObjectData::GetModelPartCountReference()
{
	return m_dwPartCount;
}

uint32_t & CObjectData::GetEquipmentTypeReference()
{
	return m_dwEquipmentType;
}

std::string & CObjectData::GetEquipmentIconImageFileNameReference()
{
	return m_strEquipmentIconImageFileName;
}

std::string & CObjectData::GetEquipmentDropModelFileNameReference()
{
	return m_strEquipmentDropModelFileName;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Motion
BOOL CObjectData::isMotionThing()
{
	return NULL != m_pMotionThing;
}

BOOL CObjectData::SetMotionThing(const char * c_szFileName)
{
	m_pMotionThing = CResourceManager::Instance().LoadResource<CGraphicThing>(c_szFileName);
	m_strMotionName = c_szFileName;
	return true;
}

BOOL CObjectData::GetMotionThing(CGraphicThing::Ptr* ppThing)
{
	*ppThing = m_pMotionThing;
	return TRUE;
}

CRaceMotionDataAccessor * CObjectData::GetMotionDataPointer()
{
	return &m_MotionData;
}

void CObjectData::Clear()
{
	DeleteAllAttachingData();

	m_dwSelectedLightIndex = 0;

	m_pModelThing = NULL;
	m_pMotionThing = NULL;

	m_LightData.Clear();
	m_MotionData.ClearAccessor();
}

void CObjectData::Initialize()
{
	ClearAllLightData();

	m_iModelType = MODEL_TYPE_CHARACTER;
	m_AttachingDataVector.clear();

	m_strShapeDataPathName = "";
	m_ShapeData.clear();

	/////
	m_dwPartCount = 1;
	/////
	m_dwEquipmentType = ITEM_WEAPON;
	m_strEquipmentIconImageFileName = "";
	m_strEquipmentDropModelFileName = "";
	/////

//#ifdef _DEBUG
	SetModelThing("D:\\Ymir Work\\pc\\assassin\\assassin_novice.GR2");
	SetMotionThing("D:\\Ymir Work\\pc\\assassin\\general\\walk.GR2");
//#endif
}

CObjectData::CObjectData()
{
	Clear();
}

CObjectData::~CObjectData()
{
}

METIN2_END_NS
