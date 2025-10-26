#include "StdAfx.h"
#include "RaceData.h"
#include "RaceMotionData.h"

#include "../EterLib/ResourceManager.h"
#include "../EterLib/AttributeInstance.h"
#include "../EterBase/Utils.h"
#include "../EterBase/Filename.h"
#include <game/MotionConstants.hpp>
#include <game/MotionTypes.hpp>

#include "../eterGrnLib/ThingInstance.h"

const std::string &CRaceData::GetSmokeBone() const { return m_strSmokeBoneName; }

uint32_t CRaceData::GetSmokeEffectID(UINT eSmoke)
{
    if (eSmoke >= SMOKE_NUM)
    {
        SPDLOG_ERROR("CRaceData::GetSmokeEffectID(eSmoke={0})", eSmoke);
        return 0;
    }

    return m_adwSmokeEffectID[eSmoke];
}

CRaceData::SHair *CRaceData::FindHair(UINT eHair)
{
    auto f = m_kMap_dwHairKey_kHair.find(eHair);
    if (m_kMap_dwHairKey_kHair.end() == f)
    {
        SPDLOG_ERROR("Hair number {0} is not exist.", eHair);
        return nullptr;
    }

    return &f->second;
}

void CRaceData::SetHairSkin(UINT eHair, UINT ePart, const char *c_szModelFileName, const char *c_szSrcFileName,
                            const char *c_szDstFileName)
{
    SSkin kSkin;
    kSkin.m_ePart = ePart;
    kSkin.m_stSrcFileName = c_szSrcFileName;
    kSkin.m_stDstFileName = c_szDstFileName;

    CFileNameHelper::ChangeDosPath(kSkin.m_stSrcFileName);
    m_kMap_dwHairKey_kHair[eHair].m_kVct_kSkin.push_back(kSkin);
    m_kMap_dwHairKey_kHair[eHair].m_stModelFileName = c_szModelFileName;
}

void CRaceData::SetHairModel(UINT eHair, const char *c_szModelFileName)
{
    m_kMap_dwHairKey_kHair[eHair].m_stModelFileName = c_szModelFileName;
}

CRaceData::SShape *CRaceData::FindShape(UINT eShape)
{
    auto f = m_kMap_dwShapeKey_kShape.find(eShape);
    if (m_kMap_dwShapeKey_kShape.end() == f)
        return NULL;

    return &f->second;
}

void CRaceData::SetShapeModel(UINT eShape, const char *c_szModelFileName)
{
    m_kMap_dwShapeKey_kShape[eShape].m_stModelFileName = c_szModelFileName;
}

void CRaceData::AppendShapeSkin(UINT eShape, UINT ePart, const char *c_szSrcFileName, const char *c_szDstFileName)
{
    SSkin kSkin;
    kSkin.m_ePart = ePart;
    kSkin.m_stSrcFileName = c_szSrcFileName;
    kSkin.m_stDstFileName = c_szDstFileName;

    CFileNameHelper::ChangeDosPath(kSkin.m_stSrcFileName);
    m_kMap_dwShapeKey_kShape[eShape].m_kVct_kSkin.push_back(kSkin);
}

const CRaceData::TMotionModeDataMap &CRaceData::GetMotionModes() const { return m_pMotionModeDataMap; }

bool CRaceData::GetMotionKey(uint16_t wMotionModeIndex, uint16_t wMotionIndex, uint32_t *pMotionKey)
{
    TMotionModeData *pMotionModeData;
    if (!GetMotionModeDataPointer(wMotionModeIndex, &pMotionModeData))
        return false;

    if (pMotionModeData->MotionVectorMap.end() == pMotionModeData->MotionVectorMap.find(wMotionIndex))
    {
        uint16_t wGeneralMode = MOTION_MODE_GENERAL;

        switch (wMotionModeIndex)
        {
        case MOTION_MODE_HORSE_ONEHAND_SWORD:
        case MOTION_MODE_HORSE_TWOHAND_SWORD:
        case MOTION_MODE_HORSE_DUALHAND_SWORD:
        case MOTION_MODE_HORSE_FAN:
        case MOTION_MODE_HORSE_BELL:
        case MOTION_MODE_HORSE_BOW:
        case MOTION_MODE_HORSE_CLAW:
            wGeneralMode = MOTION_MODE_HORSE;
            break;

        default:
            wGeneralMode = MOTION_MODE_GENERAL;
            break;
        }

        TMotionModeData *pMotionModeGeneralData;
        if (!GetMotionModeDataPointer(wGeneralMode, &pMotionModeGeneralData))
            return false;

        if (pMotionModeGeneralData->MotionVectorMap.end() == pMotionModeGeneralData->MotionVectorMap.find(wMotionIndex))
            return false;

        *pMotionKey = MakeMotionKey(wGeneralMode, wMotionIndex);
    }
    else
    {
        *pMotionKey = MakeMotionKey(wMotionModeIndex, wMotionIndex);
    }

    return true;
}

bool CRaceData::GetMotionModeDataPointer(uint16_t wMotionMode, TMotionModeData **ppMotionModeData)
{
    auto itor = m_pMotionModeDataMap.find(wMotionMode);
    if (itor == m_pMotionModeDataMap.end())
        return false;

    *ppMotionModeData = &itor->second;
    return true;
}

bool CRaceData::GetMotionVectorPointer(uint16_t wMotionMode, uint16_t wMotionIndex, TMotionVector **ppMotionVector)
{
    TMotionModeData *pMotionModeData;
    if (!GetMotionModeDataPointer(wMotionMode, &pMotionModeData))
        return false;

    const auto itor = pMotionModeData->MotionVectorMap.find(wMotionIndex);
    if (pMotionModeData->MotionVectorMap.end() == itor)
        return false;

    *ppMotionVector = &itor->second;
    return true;
}

bool CRaceData::GetMotionDataPointer(uint16_t wMotionMode, uint16_t wMotionIndex, uint16_t wMotionSubIndex,
                                     CRaceMotionData **c_ppMotionData)
{
    const TMotionVector *c_pMotionVector;
    if (!GetMotionVectorPointer(wMotionMode, wMotionIndex, &c_pMotionVector))
        return false;

    if (wMotionSubIndex >= c_pMotionVector->size())
        return false;

    const TMotion &c_rMotion = c_pMotionVector->at(wMotionSubIndex);
    if (!c_rMotion.pMotionData)
        return false;

    *c_ppMotionData = c_rMotion.pMotionData.get();
    return true;
}

bool CRaceData::GetMotionDataPointer(uint32_t dwMotionKey, CRaceMotionData **c_ppMotionData)
{
    MotionId id = MakeMotionId(dwMotionKey);
    return GetMotionDataPointer(id.mode, id.index, id.subIndex, c_ppMotionData);
}

bool CRaceData::GetMotionVectorPointer(uint16_t wMotionMode, uint16_t wMotionIndex,
                                       const TMotionVector **c_ppMotionVector)
{
    TMotionVector *pMotionVector;
    if (!GetMotionVectorPointer(wMotionMode, wMotionIndex, &pMotionVector))
        return false;

    *c_ppMotionVector = pMotionVector;
    return true;
}

const NRaceData::TAttachingDataVector &CRaceData::GetAttachingData() const { return m_AttachingDataVector; }

void CRaceData::SetRace(uint32_t dwRaceIndex) { m_dwRaceIndex = dwRaceIndex; }

void CRaceData::RegisterAttachingBoneName(uint32_t dwPartIndex, const char *c_szBoneName)
{
    m_AttachingBoneNameMap.insert(std::make_pair(dwPartIndex, c_szBoneName));
}

void CRaceData::RegisterMotionMode(uint16_t wMotionModeIndex)
{
    TMotionModeData motionModeData;
    motionModeData.wMotionModeIndex = wMotionModeIndex;
    m_pMotionModeDataMap.emplace(wMotionModeIndex, std::move(motionModeData));
}

CGraphicThing::Ptr CRaceData::RegisterMotion(std::unique_ptr<CRaceMotionData> pkMotionData, uint16_t wMotionModeIndex,
                                             uint16_t wMotionIndex, const char *c_szFileName, uint8_t byPercentage)
{
    auto pMotionThing = CResourceManager::Instance().LoadResource<CGraphicThing>(c_szFileName);
    if (!pMotionThing)
    {
        SPDLOG_ERROR("Could not load motion mode {0} index {1} file {2}", wMotionModeIndex, wMotionIndex, c_szFileName);
        return nullptr;
    }

    TMotionModeData *pMotionModeData;
    if (!GetMotionModeDataPointer(wMotionModeIndex, &pMotionModeData))
    {
        AssertLog("Failed getting motion mode data!");
        return NULL;
    }

    TMotion kMotion;
    kMotion.byPercentage = byPercentage;
    kMotion.pMotion = pMotionThing;
    kMotion.pMotionData = std::move(pkMotionData);
    pMotionModeData->MotionVectorMap[wMotionIndex].push_back(std::move(kMotion));

    auto motionId = MakeMotionId(wMotionModeIndex, wMotionIndex);

    m_registeredMotionCache.emplace(motionId.key, c_szFileName);

    return pMotionThing;
}

CGraphicThing::Ptr CRaceData::RegisterMotionData(uint16_t wMotionMode, uint16_t wMotionIndex,
                                                 const std::string &c_szFileName, uint8_t byPercentage)
{
    auto id = MakeMotionId(wMotionMode, wMotionIndex);
    auto it = m_registeredMotionCache.find(id.key);
    if (it != m_registeredMotionCache.end())
    {
        SPDLOG_INFO("Motion data is already registered (motion-mode {}, motion-index {}, filename {})", wMotionMode,
                    wMotionIndex, c_szFileName);
        auto pMotionThing = CResourceManager::Instance().LoadResource<CGraphicThing>(c_szFileName);
        return pMotionThing;
    }

    SPDLOG_DEBUG("CRaceData::RegisterMotionData");
    auto pRaceMotionData = std::make_unique<CRaceMotionData>();
    if (!pRaceMotionData->LoadMotionData(c_szFileName.c_str()))
    {
        SPDLOG_ERROR("Failed to load motion-data {0}", c_szFileName);
        return nullptr;
    }

    // 2004. 3. 15. myevan. Originally there should be index information in motion.
    pRaceMotionData->SetName(wMotionIndex);

    const auto motionFileName = pRaceMotionData->GetMotionFileName();

    // FIXME : What should I do if I already have GR2 loaded?
    // Now, one more thing is added to the same thing. - [levites]
    return RegisterMotion(std::move(pRaceMotionData), wMotionMode, wMotionIndex, motionFileName, byPercentage);
}

bool CRaceData::SetMotionRandomWeight(uint16_t wMotionModeIndex, uint16_t wMotionIndex, uint16_t wMotionSubIndex,
                                      uint8_t byPercentage)
{
    TMotionModeData *pMotionModeData;
    if (!GetMotionModeDataPointer(wMotionModeIndex, &pMotionModeData))
    {
        // AssertLog("Failed getting motion mode data!");
        return false;
    }

    auto itor = pMotionModeData->MotionVectorMap.find(wMotionIndex);
    if (pMotionModeData->MotionVectorMap.end() != itor)
    {
        TMotionVector &rMotionVector = itor->second;
        if (wMotionSubIndex < rMotionVector.size())
            rMotionVector[wMotionSubIndex].byPercentage = byPercentage;
        else
            return false;
    }
    else
    {
        SPDLOG_ERROR("Failed to find motion {0}:{1}", wMotionModeIndex, wMotionIndex);
        return false;
    }

    return true;
}

void CRaceData::RegisterNormalAttack(uint16_t wMotionModeIndex, uint16_t wMotionIndex)
{
    m_NormalAttackIndexMap.emplace(wMotionModeIndex, wMotionIndex);
}

bool CRaceData::GetNormalAttackIndex(uint16_t wMotionModeIndex, uint16_t *pwMotionIndex)
{
    auto itor = m_NormalAttackIndexMap.find(wMotionModeIndex);
    if (m_NormalAttackIndexMap.end() == itor)
        return false;

    *pwMotionIndex = itor->second;
    return true;
}

void CRaceData::ReserveComboAttack(uint16_t wMotionModeIndex, uint16_t wComboType, uint32_t dwComboCount)
{
    TComboData ComboData;
    ComboData.ComboIndexVector.clear();
    ComboData.ComboIndexVector.resize(dwComboCount);
    m_ComboAttackDataMap.insert(std::make_pair(MAKE_COMBO_KEY(wMotionModeIndex, wComboType), ComboData));
}

void CRaceData::RegisterComboAttack(uint16_t wMotionModeIndex, uint16_t wComboType, uint32_t dwComboIndex,
                                    uint16_t wMotionIndex)
{
    auto itor = m_ComboAttackDataMap.find(MAKE_COMBO_KEY(wMotionModeIndex, wComboType));
    if (m_ComboAttackDataMap.end() == itor)
        return;

    TComboIndexVector &rComboIndexVector = itor->second.ComboIndexVector;
    if (dwComboIndex >= rComboIndexVector.size())
    {
        AssertLog("CRaceData::RegisterCombo - Strange combo index!");
        return;
    }

    rComboIndexVector[dwComboIndex] = wMotionIndex;
}

bool CRaceData::GetComboDataPointer(uint16_t wMotionModeIndex, uint16_t wComboType, TComboData **ppComboData)
{
    auto itor = m_ComboAttackDataMap.find(MAKE_COMBO_KEY(wMotionModeIndex, wComboType));
    if (m_ComboAttackDataMap.end() == itor)
        return false;

    *ppComboData = &itor->second;
    return true;
}

const char *CRaceData::GetBaseModelFileName() const { return m_strBaseModelFileName.c_str(); }

const char *CRaceData::GetAttributeFileName() const { return m_strAttributeFileName.c_str(); }

const char *CRaceData::GetMotionListFileName() const { return m_strMotionListFileName.c_str(); }

CGraphicThing::Ptr CRaceData::GetBaseModelThing()
{
    if (!m_pBaseModelThing)
        m_pBaseModelThing = CResourceManager::Instance().LoadResource<CGraphicThing>(m_strBaseModelFileName);

    return m_pBaseModelThing;
}

CAttributeData::Ptr CRaceData::GetAttributeDataPtr()
{
    if (m_strAttributeFileName.empty())
        return NULL;

    return CResourceManager::Instance().LoadResource<CAttributeData>(m_strAttributeFileName);
}

bool CRaceData::CreateMotionModeIterator(TMotionModeDataIterator &itor)
{
    if (m_pMotionModeDataMap.empty())
        return false;

    itor = m_pMotionModeDataMap.begin();

    return true;
}

bool CRaceData::NextMotionModeIterator(TMotionModeDataIterator &itor)
{
    ++itor;

    return m_pMotionModeDataMap.end() != itor;
}

void CRaceData::LoadMotions()
{
    if (!m_motionsLoaded)
    {
        for (const auto &p : GetMotionModes())
        {
            for (const auto &p2 : p.second.MotionVectorMap)
            {
                uint32_t subIndex = 0;
                for (const auto &motion : p2.second)
                {
                    const auto key = MakeMotionKey(p.first, p2.first, subIndex++);
                    CGraphicThingInstance::RegisterMotionThing(m_dwRaceIndex, key, motion.pMotion);
                }
            }
        }
        m_motionsLoaded = true;
    }
}

bool CRaceData::GetAttachingBoneName(uint32_t dwPartIndex, const char **c_pszBoneName)
{
    TAttachingBoneNameMap::iterator itor = m_AttachingBoneNameMap.find(dwPartIndex);
    if (itor == m_AttachingBoneNameMap.end())
        return false;

    const std::string &c_rstrBoneName = itor->second;

    *c_pszBoneName = c_rstrBoneName.c_str();
    return true;
}

bool CRaceData::IsTree() { return !m_strTreeFileName.empty(); }

const char *CRaceData::GetTreeFileName() { return m_strTreeFileName.c_str(); }

CRaceData::CRaceData() : m_dwRaceIndex(0), m_strMotionListFileName("motlist.txt")
{
    std::memset(m_adwSmokeEffectID, 0, sizeof(m_adwSmokeEffectID));
}
