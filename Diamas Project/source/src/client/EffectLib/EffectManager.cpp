#include "StdAfx.h"
#include "EffectManager.h"
#include "../EterLib/Engine.hpp"
#include "../EterBase/Utils.h"
#include "base/Remotery.h"

void CEffectManager::GetInfo(std::string *pstInfo) const
{
    char szInfo[256];

    sprintf(szInfo,
            "Effect: Inst - ED %llu, EI %llu Pool - PSI %d, MI %d, LI %d, PI %d, EI %d, ED %d, PSD %d, EM %d, LD %d",
            m_kEftDataMap.size(),
            m_kEftInstMap.size(),
            CParticleSystemInstance::ms_kPool.GetCapacity(),
            CEffectMeshInstance::ms_kPool.GetCapacity(),
            CLightInstance::ms_kPool.GetCapacity(),
            CParticleInstance::ms_kPool.GetCapacity(),
            CEffectInstance::ms_kPool.GetCapacity(),
            CEffectData::ms_kPool.GetCapacity(),
            CParticleSystemData::ms_kPool.GetCapacity(),
            CEffectMeshScript::ms_kPool.GetCapacity(),
            CLightData::ms_kPool.GetCapacity()
        );
    pstInfo->append(szInfo);
}

bool CEffectManager::IsAliveEffect(uint32_t dwInstanceIndex)
{
    auto f = m_kEftInstMap.find(dwInstanceIndex);
    if (m_kEftInstMap.end() == f)
        return false;

    return f->second->isAlive() != 0;
}

void CEffectManager::Update()
{
    for (auto itor = m_kEftInstMap.begin(); itor != m_kEftInstMap.end();)
    {
        auto pEffectInstance = itor->second;
        pEffectInstance->Update();

        if (!pEffectInstance->isAlive())
        {
            itor = m_kEftInstMap.erase(itor);
            CEffectInstance::Delete(pEffectInstance);
        }
        else
        {
            ++itor;
        }
    }
}

void CEffectManager::Render()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(251, 50, 50, 0), L"** CEffectManager::Render **");
    rmt_ScopedCPUSample(RenderEffects, 0);

    STATEMANAGER.SetTexture(0, NULL);
    STATEMANAGER.SetTexture(1, NULL);

    if (m_isDisableSortRendering)
    {
        for (auto itor = m_kEftInstMap.begin(); itor != m_kEftInstMap.end();)
        {
            auto pEffectInstance = itor->second;
            pEffectInstance->Render();
            ++itor;
        }
    }
    else
    {
        static std::vector<CEffectInstance *> s_kVct_pkEftInstSort;
        s_kVct_pkEftInstSort.reserve(m_kEftInstMap.size());
        s_kVct_pkEftInstSort.clear();

        for (auto &i : m_kEftInstMap)
        {
            if (!Engine::Engine::GetSettings().IsShowGeneralEffects() && i.second->GetEffectKind() !=
                EFFECT_KIND_SPECIAL_EFFECT && i.second->GetEffectKind() != EFFECT_KIND_SHINING_PERSIST)
                continue;

            if (!Engine::Engine::GetSettings().IsShowSkillEffects())
            {
                if (i.second->GetEffectKind() == EFFECT_KIND_MOTION_EFFECT ||
                    i.second->GetEffectKind() == EFFECT_KIND_MOTION_EFFECT_TO_TARGET)
                    continue;
            }

            if (!Engine::Engine::GetSettings().IsShowBuffEffects())
            {
                if (i.second->GetEffectKind() == EFFECT_KIND_AFFECT)
                    continue;
            }

            s_kVct_pkEftInstSort.push_back(i.second);
        }

        std::sort(std::execution::par, s_kVct_pkEftInstSort.begin(), s_kVct_pkEftInstSort.end(),
                  [](CEffectInstance *pkLeft, CEffectInstance *pkRight)
                  {
                      return pkLeft->LessRenderOrder(pkRight);
                  });

        std::for_each(s_kVct_pkEftInstSort.begin(), s_kVct_pkEftInstSort.end(), [](CEffectInstance *inst)
        {
            inst->Render();
        });
    }
    D3DPERF_EndEvent();
}

bool CEffectManager::RegisterEffect(const char *filename, uint32_t *id, bool overwrite)
{
    std::string strFileName;
    StringPath(filename, strFileName);
    uint32_t crc = ComputeCrc32(0, strFileName.c_str(), strFileName.length());

    if (id)
        *id = crc;

    const auto it = m_kEftDataMap.find(crc);
    if (m_kEftDataMap.end() != it)
    {
        if (overwrite)
        {
            CEffectData::Delete(it->second);
            m_kEftDataMap.erase(it);
        }
        else
        {
            return true;
        }
    }

    auto * data = CEffectData::New();
    if (!data->LoadScript(strFileName.c_str()))
    {
        SPDLOG_ERROR("RegisterEffect - LoadScript({0}) Error", strFileName);
        CEffectData::Delete(data);
        return false;
    }

    m_kEftDataMap.emplace(crc, data);
    return true;
}

// CEffectData 를 포인터형으로 리턴하게 하고..
// CEffectData에서 CRC를 얻을수 있게 한다
int CEffectManager::CreateEffect(const std::string &filename)
{
    auto path = filename;
    StringPath(path);
    const auto id = ComputeCrc32(0, path.data(), path.size());
    return CreateEffect(id);
}

int CEffectManager::CreateEffect(uint32_t id, float scale, uint32_t effectKind)
{
    CEffectData *pEffect;
    if (!GetEffectData(id, &pEffect))
    {
        SPDLOG_WARN("Failed to find effect with ID {0}", id);
        return false;
    }
    int index = GetEmptyIndex();

    auto pEffectInstance = CEffectInstance::New();
    pEffectInstance->SetScale(scale);
    pEffectInstance->SetBaseEffectId(id);
    pEffectInstance->SetEffectKind(effectKind);
    pEffectInstance->SetEffectDataPointer(pEffect);

    m_kEftInstMap.emplace(index, pEffectInstance);
    return index;
}

bool CEffectManager::CreateEffectInstance(uint32_t index, uint32_t id)
{
    if (id == 0)
        return false;

    CEffectData *pEffect;
    if (!GetEffectData(id, &pEffect))
    {
        SPDLOG_WARN(
            "No effect with ID {0} found for instance {1}",
            id, index);
        return false;
    }

    CEffectInstance *pEffectInstance = CEffectInstance::New();
    pEffectInstance->SetEffectDataPointer(pEffect);

    m_kEftInstMap.emplace(index, pEffectInstance);
    return true;
}

bool CEffectManager::DestroyEffectInstance(uint32_t dwInstanceIndex)
{
    const auto itor = m_kEftInstMap.find(dwInstanceIndex);

    if (itor == m_kEftInstMap.end())
        return false;

    CEffectInstance *pEffectInstance = itor->second;
    m_kEftInstMap.erase(itor);

    CEffectInstance::Delete(pEffectInstance);

    return true;
}

void CEffectManager::DeactiveEffectInstance(uint32_t dwInstanceIndex)
{
    TEffectInstanceMap::iterator itor = m_kEftInstMap.find(dwInstanceIndex);

    if (itor == m_kEftInstMap.end())
        return;

    CEffectInstance *pEffectInstance = itor->second;
    pEffectInstance->SetDeactive();
}

void CEffectManager::CreateUnsafeEffectInstance(uint32_t dwEffectDataID, CEffectInstance **ppEffectInstance)
{
    CEffectData *pEffect;
    if (!GetEffectData(dwEffectDataID, &pEffect))
    {
        SPDLOG_INFO("CEffectManager::CreateEffectInstance - NO DATA :%d\n", dwEffectDataID);
        return;
    }

    CEffectInstance *pkEftInstNew = CEffectInstance::New();
    pkEftInstNew->SetEffectDataPointer(pEffect);

    *ppEffectInstance = pkEftInstNew;
}

bool CEffectManager::DestroyUnsafeEffectInstance(CEffectInstance *pEffectInstance)
{
    if (!pEffectInstance)
        return false;

    CEffectInstance::Delete(pEffectInstance);

    return true;
}

CEffectInstance *CEffectManager::GetEffectInstance(uint32_t index)
{
    const auto it = m_kEftInstMap.find(index);
    if (it == m_kEftInstMap.end())
        return nullptr;

    return it->second;
}

bool CEffectManager::SelectEffectInstance(uint32_t dwInstanceIndex)
{
    TEffectInstanceMap::iterator itor = m_kEftInstMap.find(dwInstanceIndex);

    m_pSelectedEffectInstance = NULL;

    if (m_kEftInstMap.end() == itor)
        return FALSE;

    m_pSelectedEffectInstance = itor->second;

    return TRUE;
}

void CEffectManager::SetEffectTextures(uint32_t dwID, const std::vector<std::string> &textures)
{
    CEffectData *pEffectData;
    if (!GetEffectData(dwID, &pEffectData))
    {
        SPDLOG_INFO("CEffectManager::CreateEffectInstance - NO DATA :%d\n", dwID);
        return;
    }

    for (uint32_t i = 0; i < textures.size(); i++)
    {
        auto pParticle = pEffectData->GetParticlePointer(i);
        pParticle->ChangeTexture(textures.at(i).c_str());
    }
}

void CEffectManager::SetEffectInstancePosition(const Vector3 &c_rv3Position)
{
    if (!m_pSelectedEffectInstance)
    {
        //		assert(!"Instance to use is not yet set!");
        return;
    }

    m_pSelectedEffectInstance->SetPosition(c_rv3Position);
}

void CEffectManager::SetEffectInstanceRotation(const Vector3 &c_rv3Rotation)
{
    if (!m_pSelectedEffectInstance)
    {
        //		assert(!"Instance to use is not yet set!");
        return;
    }

    m_pSelectedEffectInstance->SetRotation(c_rv3Rotation.x, c_rv3Rotation.y, c_rv3Rotation.z);
}

void CEffectManager::ShowEffect()
{
    if (!m_pSelectedEffectInstance)
        return;

    m_pSelectedEffectInstance->Show();
}

void CEffectManager::HideEffect()
{
    if (!m_pSelectedEffectInstance)
        return;

    m_pSelectedEffectInstance->Hide();
}

bool CEffectManager::GetEffectData(uint32_t dwID, CEffectData **ppEffect)
{
    TEffectDataMap::iterator itor = m_kEftDataMap.find(dwID);

    if (itor == m_kEftDataMap.end())
        return false;

    *ppEffect = itor->second;

    return true;
}

bool CEffectManager::GetEffectData(uint32_t dwID, const CEffectData **c_ppEffect)
{
    const auto itor = m_kEftDataMap.find(dwID);

    if (itor == m_kEftDataMap.end())
        return false;

    *c_ppEffect = itor->second;

    return true;
}

int CEffectManager::GetEmptyIndex()
{
    static int iMaxIndex = 1;

    if (iMaxIndex > 2100000000)
        iMaxIndex = 1;

    int iNextIndex = iMaxIndex++;
    while (m_kEftInstMap.find(iNextIndex) != m_kEftInstMap.end())
        iNextIndex++;

    return iNextIndex;
}

void CEffectManager::DeleteAllInstances()
{
    __DestroyEffectInstanceMap();
}

void CEffectManager::__DestroyEffectInstanceMap()
{
    for (auto &i : m_kEftInstMap)
    {
        CEffectInstance *pkEftInst = i.second;
        CEffectInstance::Delete(pkEftInst);
    }

    m_kEftInstMap.clear();
}

void CEffectManager::__DestroyEffectCacheMap()
{
    for (auto &i : m_kEftCacheMap)
    {
        CEffectInstance *pkEftInst = i.second;
        CEffectInstance::Delete(pkEftInst);
    }

    m_kEftCacheMap.clear();
}

void CEffectManager::__DestroyEffectDataMap()
{
    for (auto &i : m_kEftDataMap)
    {
        CEffectData *pData = i.second;
        CEffectData::Delete(pData);
    }

    m_kEftDataMap.clear();
}

void CEffectManager::Destroy()
{
    __DestroyEffectInstanceMap();
    __DestroyEffectCacheMap();
    __DestroyEffectDataMap();

    __Initialize();
}

void CEffectManager::__Initialize()
{
    m_pSelectedEffectInstance = NULL;
    m_isDisableSortRendering = false;
}

CEffectManager::CEffectManager()
{
    __Initialize();
}

CEffectManager::~CEffectManager()
{
    Destroy();
}

// just for map effect
