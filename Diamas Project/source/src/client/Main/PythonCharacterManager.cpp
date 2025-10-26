#include "pythoncharactermanager.h"
#include "../GameLib/NpcManager.h"
#include "PythonBackground.h"
#include "PythonPlayer.h"
#include "StdAfx.h"
#include <game/GamePacket.hpp>

#include "../EterLib/Camera.h"
#include "base/Remotery.h"
#include "game/AffectConstants.hpp"

#include <execution>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Frame Process

int CHAR_STAGE_VIEW_BOUND = 200 * 100;

void CPythonCharacterManager::AdjustCollisionWithOtherObjects(CActorInstance *pInst)
{
    if (!pInst->IsPC())
        return;

    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    for (auto i = rkChrMgr.CharacterInstanceBegin(); i != rkChrMgr.CharacterInstanceEnd(); ++i)
    {
        CInstanceBase *pkInstEach = *i;
        CActorInstance *rkActorEach = pkInstEach->GetGraphicThingInstancePtr();

        if (rkActorEach == pInst)
            continue;

        if (rkActorEach->IsPC() || rkActorEach->IsNPC() || rkActorEach->IsEnemy())
            continue;

        if (pInst->TestPhysicsBlendingCollision(*rkActorEach))
        {
            TPixelPosition curPos;
            pInst->GetPixelPosition(&curPos);
            pInst->SetBlendingPosition(curPos);
            break;
        }
    }
}

void CPythonCharacterManager::InsertPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
    CInstanceBase::InsertPVPKey(dwVIDSrc, dwVIDDst);

    CInstanceBase *pkInstSrc = GetInstancePtr(dwVIDSrc);
    if (pkInstSrc)
        pkInstSrc->RefreshTextTail();

    CInstanceBase *pkInstDst = GetInstancePtr(dwVIDDst);
    if (pkInstDst)
        pkInstDst->RefreshTextTail();
}

void CPythonCharacterManager::RemovePVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
    CInstanceBase::RemovePVPKey(dwVIDSrc, dwVIDDst);

    auto pkInstSrc = GetInstancePtr(dwVIDSrc);
    if (pkInstSrc)
        pkInstSrc->RefreshTextTail();

    auto pkInstDst = GetInstancePtr(dwVIDDst);
    if (pkInstDst)
        pkInstDst->RefreshTextTail();
}

void CPythonCharacterManager::ChangeGVG(uint32_t dwSrcGuildID, uint32_t dwDstGuildID)
{
    for (auto &itor : m_kAliveInstMap)
    {
        auto pInstance = itor.second;

        const auto dwInstanceGuildId = pInstance->GetGuildID();
        if (dwSrcGuildID == dwInstanceGuildId || dwDstGuildID == dwInstanceGuildId)
        {
            pInstance->RefreshTextTail();
        }
    }
}

void CPythonCharacterManager::SetPVPTeam(DWORD dwVID, int iTeam)
{
    auto it = m_map_pvpTeam.find(dwVID);
    if (it != m_map_pvpTeam.end())
    {
        if (iTeam == -1)
            m_map_pvpTeam.erase(it);
        else
            it->second = iTeam;
    }
    else if (iTeam != -1)
        m_map_pvpTeam.insert(std::pair<DWORD, int>(dwVID, iTeam));

    CInstanceBase *pkInst = GetInstancePtr(dwVID);
    if (pkInst)
        pkInst->RefreshTextTail();
}

int CPythonCharacterManager::GetPVPTeam(DWORD dwVID)
{
    auto it = m_map_pvpTeam.find(dwVID);
    if (it == m_map_pvpTeam.end())
        return -1;
    return it->second;
}

bool CPythonCharacterManager::HasPVPTeam(DWORD dwVID)
{
    return GetPVPTeam(dwVID) != -1;
}

void CPythonCharacterManager::ClearMainInstance()
{
    m_pkInstMain = nullptr;
}

bool CPythonCharacterManager::SetMainInstance(uint32_t dwVID)
{
    m_pkInstMain = GetInstancePtr(dwVID);

    return m_pkInstMain != nullptr;
}

CInstanceBase *CPythonCharacterManager::GetMainInstancePtr() const
{
    return m_pkInstMain;
}

void CPythonCharacterManager::GetInfo(std::string *pstInfo) const
{
    pstInfo->append(fmt::format("Container - Live {}, Dead {}", m_kAliveInstMap.size(), m_kDeadInstList.size()));
}

void CPythonCharacterManager::Update()
{
    CInstanceBase::ResetPerformanceCounter();

	for (const auto& p : m_kAliveInstMap)
		p.second->Update();

    UpdateTransform();
    UpdateDeleting();
    Pick();
}

void CPythonCharacterManager::ShowPointEffect(uint32_t ePoint, uint32_t dwVID)
{
    CInstanceBase *pkInstSel = (dwVID == 0xffffffff) ? GetMainInstancePtr() : GetInstancePtr(dwVID);

    if (!pkInstSel)
        return;

    switch (ePoint)
    {
    case POINT_LEVEL:
        pkInstSel->LevelUp();
        break;
    case POINT_LEVEL_STEP:
        pkInstSel->SkillUp();
        break;
    default:
        break;
    }
}

bool CPythonCharacterManager::RegisterPointEffect(uint32_t ePoint, const char *c_szFileName)
{
    if (ePoint >= POINT_MAX_NUM)
        return false;

    CEffectManager &rkEftMgr = CEffectManager::Instance();
    return rkEftMgr.RegisterEffect(c_szFileName, &m_adwPointEffect[ePoint]);
}

void CPythonCharacterManager::UpdateTransform()
{

	CInstanceBase * pMainInstance = GetMainInstancePtr();
	if (pMainInstance)
	{
		CPythonBackground& rkBG=CPythonBackground::Instance();
		for (TCharacterInstanceMap::iterator i = m_kAliveInstMap.begin(); i != m_kAliveInstMap.end(); ++i)
		{
			CInstanceBase * pSrcInstance = i->second;

			pSrcInstance->CheckAdvancing();

			// 2004.08.02.myevan.IsAttacked 일 경우 죽었을때도 체크하므로,
			// 실질적으로 거리가 변경되는 IsPushing일때만 체크하도록 한다
			if (pSrcInstance->IsPushing())
				rkBG.CheckAdvancing(pSrcInstance);
		}

		if (!m_pkInstMain->CanSkipCollision())
			rkBG.CheckAdvancing(m_pkInstMain);
	}

	{
		for (TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); ++itor)
		{
			CInstanceBase * pInstance = itor->second;
			pInstance->Transform();
		}
	}
}

void CPythonCharacterManager::UpdateDeleting()
{
    for (auto it = m_kDeadInstList.begin(); it != m_kDeadInstList.end();)
    {
        auto* pInstance = *it;

        if (pInstance->UpdateDeleting())
        {
            ++it;
        }
        else
        {
            DeleteInstance(pInstance);
            it = m_kDeadInstList.erase(it);
        }
    }
}

void CPythonCharacterManager::Deform()
{
    rmt_ScopedCPUSample(CPythonCharacterManager_Deform, 0);
    for (const auto &p : m_kAliveInstMap)
        p.second->Deform();

    for (const auto &p : m_kDeadInstList)
        p->Deform();
}

bool CPythonCharacterManager::OLD_GetPickedInstanceVID(uint32_t *pdwPickedActorID) const
{
    if (!m_pkInstPick)
        return false;

    *pdwPickedActorID = m_pkInstPick->GetVirtualID();
    return true;
}

CInstanceBase *CPythonCharacterManager::OLD_GetPickedInstancePtr() const
{
    return m_pkInstPick;
}

Vector2 &CPythonCharacterManager::OLD_GetPickedInstPosReference()
{
    return m_v2PickedInstProjPos;
}

bool CPythonCharacterManager::IsRegisteredVID(uint32_t dwVID)
{
    return !(m_kAliveInstMap.end() == m_kAliveInstMap.find(dwVID));
}

bool CPythonCharacterManager::IsAliveVID(uint32_t dwVID)
{
    return m_kAliveInstMap.find(dwVID) != m_kAliveInstMap.end();
}

bool CPythonCharacterManager::IsDeadVID(uint32_t dwVID)
{
    for (auto &f : m_kDeadInstList)
    {
        if (f->GetVirtualID() == dwVID)
            return true;
    }

    return false;
}

void CPythonCharacterManager::__RenderSortedAliveActorList()
{
    static std::vector<CInstanceBase *> s_kVct_pkInstAliveSort;
    s_kVct_pkInstAliveSort.clear();
    s_kVct_pkInstAliveSort.reserve(m_kAliveInstMap.size());

    auto &rkMapPkInstAlive = m_kAliveInstMap;
    for (auto &i : rkMapPkInstAlive)
        s_kVct_pkInstAliveSort.push_back(i.second);

    std::sort(std::execution::par, s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(),
              [](CInstanceBase *pkLeft, CInstanceBase *pkRight) { return pkLeft->LessRenderOrder(pkRight); });

    std::for_each(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(), [](CInstanceBase *pInstance) {
        pInstance->Render();
        pInstance->RenderTrace();
    });
}

void CPythonCharacterManager::__RenderSortedDeadActorList()
{
    static std::vector<CInstanceBase *> s_kVct_pkInstDeadSort;
    s_kVct_pkInstDeadSort.clear();
    s_kVct_pkInstDeadSort.reserve(m_kDeadInstList.size());

    for (auto &deadInst : m_kDeadInstList)
        s_kVct_pkInstDeadSort.push_back(deadInst);

    const auto f = [](CInstanceBase *pkLeft, CInstanceBase *pkRight) { return pkLeft->LessRenderOrder(pkRight); };

    std::sort(std::execution::par, s_kVct_pkInstDeadSort.begin(), s_kVct_pkInstDeadSort.end(), f);

    for (auto &&inst : s_kVct_pkInstDeadSort)
        inst->Render();
}

void CPythonCharacterManager::Render()
{
    rmt_ScopedCPUSample(RenderCharacters, 0);

    /*STATEMANAGER.SetTexture(0, nullptr);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

    STATEMANAGER.SetTexture(1, nullptr);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    */
    __RenderSortedAliveActorList();
    __RenderSortedDeadActorList();

    auto pkPickedInst = OLD_GetPickedInstancePtr();
    if (pkPickedInst)
    {
        const auto &c_rv3Position = pkPickedInst->GetGraphicThingInstanceRef().GetPosition();
        CPythonGraphic::Instance().ProjectPosition(c_rv3Position.x, c_rv3Position.y, c_rv3Position.z,
                                                   &m_v2PickedInstProjPos.x, &m_v2PickedInstProjPos.y);
    }
}

void CPythonCharacterManager::RenderShadowMainInstance() const
{
    auto pkInstMain = GetMainInstancePtr();
    if (pkInstMain)
        pkInstMain->RenderToShadowMap();
}

struct FCharacterManagerCharacterInstanceRenderToShadowMap
{
    void operator()(const std::pair<uint32_t, CInstanceBase *> &cr_Pair) const
    {
        cr_Pair.second->RenderToShadowMap();
    }
};

void CPythonCharacterManager::RenderShadowAllInstances()
{
    std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(),
                  FCharacterManagerCharacterInstanceRenderToShadowMap());
}

struct FCharacterManagerCharacterInstanceRenderCollision
{
    void operator()(const std::pair<uint32_t, CInstanceBase *> &cr_Pair) const
    {
        cr_Pair.second->RenderCollision();
    }
};

void CPythonCharacterManager::RenderCollision()
{
    std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), FCharacterManagerCharacterInstanceRenderCollision());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Managing Process

CInstanceBase *CPythonCharacterManager::CreateInstance(const CInstanceBase::SCreateData &c_rkCreateData)
{
    auto pCharacterInstance = RegisterInstance(c_rkCreateData.m_dwVID);
    if (!pCharacterInstance)
    {
        SPDLOG_ERROR("CPythonCharacterManager::CreateInstance: VID[{0}] - ALREADY EXIST", c_rkCreateData.m_dwVID);
        return nullptr;
    }

    if (!pCharacterInstance->Create(c_rkCreateData))
    {
        SPDLOG_ERROR("CPythonCharacterManager::CreateInstance VID[{0}] Race[{1}]", c_rkCreateData.m_dwVID,
                     c_rkCreateData.m_dwRace);
        DeleteInstance(c_rkCreateData.m_dwVID);
        return nullptr;
    }

    if (c_rkCreateData.m_isMain)
        SelectInstance(c_rkCreateData.m_dwVID);

    return (pCharacterInstance);
}

CInstanceBase *CPythonCharacterManager::RegisterInstance(uint32_t VirtualID)
{
    const auto itor = m_kAliveInstMap.find(VirtualID);
    if (m_kAliveInstMap.end() != itor)
        return nullptr;

    auto pCharacterInstance = new CInstanceBase();
    m_kAliveInstMap.insert(std::make_pair(VirtualID, pCharacterInstance));
    return pCharacterInstance;
}

void CPythonCharacterManager::DeleteInstance(uint32_t dwDelVID)
{
    const auto itor = m_kAliveInstMap.find(dwDelVID);
    if (m_kAliveInstMap.end() == itor)
    {
        SPDLOG_ERROR("DeleteCharacterInstance: no vid by {0}", dwDelVID);
        return;
    }

    DeleteInstance(itor->second);
    m_kAliveInstMap.erase(itor);
}

void CPythonCharacterManager::__DeleteBlendOutInstance(CInstanceBase *pkInstDel)
{
    pkInstDel->DeleteBlendOut();
    m_kDeadInstList.push_back(pkInstDel);

    auto &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.NotifyCharacterDead(pkInstDel->GetVirtualID());
}

void CPythonCharacterManager::DeleteInstanceByFade(uint32_t dwVID)
{
    const auto f = m_kAliveInstMap.find(dwVID);
    if (m_kAliveInstMap.end() == f)
        return;

    __DeleteBlendOutInstance(f->second);
    m_kAliveInstMap.erase(f);
}

void CPythonCharacterManager::SelectInstance(uint32_t VirtualID)
{
    const auto itor = m_kAliveInstMap.find(VirtualID);

    if (m_kAliveInstMap.end() == itor)
    {
        SPDLOG_ERROR("SelectCharacterInstance: no vid by {0}", VirtualID);
        return;
    }

    m_pkInstBind = itor->second;
}

CInstanceBase *CPythonCharacterManager::GetInstancePtr(uint32_t VirtualID) const
{
    const auto itor = m_kAliveInstMap.find(VirtualID);
    if (m_kAliveInstMap.end() == itor)
        return nullptr;

    return itor->second;
}

CInstanceBase *CPythonCharacterManager::GetInstancePtrByName(const char *name)
{
    for (auto &itor : m_kAliveInstMap)
    {
        const auto pInstance = itor.second;

        if (!strcmp(pInstance->GetNameString(), name))
            return pInstance;
    }

    return nullptr;
}

CInstanceBase *CPythonCharacterManager::GetSelectedInstancePtr() const
{
    return m_pkInstBind;
}

void CPythonCharacterManager::__UpdateSortPickedActorList()
{
    __UpdatePickedActorList();
    __SortPickedActorList();
}

void CPythonCharacterManager::__UpdatePickedActorList()
{
    m_kVct_pkInstPicked.clear();

    for (auto &i : m_kAliveInstMap)
    {
        auto pkInstEach = i.second;

        if (pkInstEach->CanPickInstance(false))
        {
            if (pkInstEach->IsDead())
            {
                if (pkInstEach->IntersectBoundingBox())
                    m_kVct_pkInstPicked.push_back(pkInstEach);
            }
            else
            {
                if (pkInstEach->IntersectDefendingSphere())
                    m_kVct_pkInstPicked.push_back(pkInstEach);
            }
        }
    }
}

struct CInstanceBase_SLessCameraDistance
{
    TPixelPosition m_kPPosEye;

    bool operator()(CInstanceBase *pkInstLeft, CInstanceBase *pkInstRight) const
    {
        const auto nLeftDeadPoint = pkInstLeft->IsDead();
        const auto nRightDeadPoint = pkInstRight->IsDead();

        if (nLeftDeadPoint < nRightDeadPoint)
            return true;

        if (pkInstLeft->CalculateDistanceSq3d(m_kPPosEye) < pkInstRight->CalculateDistanceSq3d(m_kPPosEye))
            return true;

        return false;
    }
};

void CPythonCharacterManager::__SortPickedActorList()
{
    const auto pCamera = CCameraManager::Instance().GetCurrentCamera();
    const auto &eyePos = pCamera->GetEye();

    CInstanceBase_SLessCameraDistance kLess;
    kLess.m_kPPosEye = TPixelPosition(+eyePos.x, -eyePos.y, +eyePos.z);

    std::sort(std::execution::par, m_kVct_pkInstPicked.begin(), m_kVct_pkInstPicked.end(), kLess);
}

void CPythonCharacterManager::Pick()
{
    __UpdateSortPickedActorList();

    auto pkInstMain = GetMainInstancePtr();

    {
        for (auto pkInstEach : m_kVct_pkInstPicked)
        {
            if (pkInstEach != pkInstMain && pkInstEach->IntersectBoundingBox())
            {
                if (m_pkInstPick)
                    if (m_pkInstPick != pkInstEach)
                        m_pkInstPick->OnUnselected();

                if (pkInstEach->CanPickInstance(false))
                {
                    m_pkInstPick = pkInstEach;
                    m_pkInstPick->OnSelected();
                    return;
                }
            }
        }
    }

    {
        for (auto pkInstEach : m_kVct_pkInstPicked)
        {
            if (pkInstEach != pkInstMain)
            {
                if (m_pkInstPick)
                    if (m_pkInstPick != pkInstEach)
                        m_pkInstPick->OnUnselected();

                if (pkInstEach->CanPickInstance(false))
                {
                    m_pkInstPick = pkInstEach;
                    m_pkInstPick->OnSelected();
                    return;
                }
            }
        }
    }

    if (pkInstMain)
        if (pkInstMain->CanPickInstance(false))
            if (m_kVct_pkInstPicked.end() !=
                std::find(m_kVct_pkInstPicked.begin(), m_kVct_pkInstPicked.end(), pkInstMain))
            {
                if (m_pkInstPick)
                    if (m_pkInstPick != pkInstMain)
                        m_pkInstPick->OnUnselected();

                m_pkInstPick = pkInstMain;
                m_pkInstPick->OnSelected();
                return;
            }

    if (m_pkInstPick)
    {
        m_pkInstPick->OnUnselected();
        m_pkInstPick = nullptr;
    }
}

int CPythonCharacterManager::PickAll()
{
    for (auto &itor : m_kAliveInstMap)
    {
        auto pInstance = itor.second;

        if (pInstance->IntersectDefendingSphere())
            return pInstance->GetVirtualID();
    }

    return -1;
}

CInstanceBase *CPythonCharacterManager::GetTabNextTargetPointer(CInstanceBase *pkInstMain)
{
    if (!pkInstMain)
    {
        ResetTabNextTargetVectorIndex();
        return nullptr;
    }

    struct FCharacterManagerInstanceTarget
    {
        CInstanceBase *pkInstMain;

        FCharacterManagerInstanceTarget(CInstanceBase *pInstance) : pkInstMain(pInstance)
        {
        }

        inline void operator()(const std::pair<uint32_t, CInstanceBase *> &itor)
        {
            const auto pkInstTarget = itor.second;
            if (!pkInstTarget || pkInstTarget == pkInstMain || !pkInstTarget->IsEnemy())
                return;

            const auto fRadiusDistance = pkInstMain->GetDistance(pkInstTarget);
            if (fRadiusDistance < 1500.0f)
                m_vecTargetInstance.push_back(pkInstTarget);
        }

        std::vector<CInstanceBase *> m_vecTargetInstance;
    };

    FCharacterManagerInstanceTarget f(pkInstMain);
    f = std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), f);

    const auto kTargetCount = f.m_vecTargetInstance.size();
    if (kTargetCount == 0)
    {
        ResetTabNextTargetVectorIndex();
        return nullptr;
    }

    if (GetTabNextTargetVectorIndex() >= kTargetCount - 1)
        ResetTabNextTargetVectorIndex();

    return f.m_vecTargetInstance.at(++m_adwVectorIndexTabNextTarget);
}

CInstanceBase *CPythonCharacterManager::GetCloseInstance(CInstanceBase *pInstance)
{
    auto fMinDistance = 10000.0f;
    CInstanceBase *pCloseInstance = nullptr;

    auto itor = m_kAliveInstMap.begin();
    for (; itor != m_kAliveInstMap.end(); ++itor)
    {
        auto pTargetInstance = itor->second;

        if (pTargetInstance == pInstance)
            continue;

        const auto fDistance = pInstance->GetDistance(pTargetInstance);
        if (fDistance < fMinDistance)
        {
            fMinDistance = fDistance;
            pCloseInstance = pTargetInstance;
        }
    }

    return pCloseInstance;
}

void CPythonCharacterManager::RefreshAllPCTextTail()
{
    auto itor = CharacterInstanceBegin();
    const auto itorEnd = CharacterInstanceEnd();
    for (; itor != itorEnd; ++itor)
    {
        auto pInstance = *itor;
        if (!pInstance->IsPC())
            continue;

        pInstance->RefreshTextTail();
    }
}

void CPythonCharacterManager::RefreshAllGuildMark()
{
    auto itor = CharacterInstanceBegin();
    const auto itorEnd = CharacterInstanceEnd();
    for (; itor != itorEnd; ++itor)
    {
        auto pInstance = *itor;
        if (!pInstance->IsPC())
            continue;

        pInstance->ChangeGuild(pInstance->GetGuildID());
        pInstance->RefreshTextTail();
    }
}

void CPythonCharacterManager::DeleteAllInstances()
{
    DestroyAliveInstanceMap();
    DestroyDeadInstanceList();
}

void CPythonCharacterManager::DestroyAliveInstanceMap()
{
    for (const auto &p : m_kAliveInstMap)
        DeleteInstance(p.second);

    m_kAliveInstMap.clear();
}

void CPythonCharacterManager::DestroyDeadInstanceList()
{
    for (const auto &ch : m_kDeadInstList)
        DeleteInstance(ch);

    m_kDeadInstList.clear();
}

void CPythonCharacterManager::Destroy()
{
    DeleteAllInstances();

    __Initialize();
}

void CPythonCharacterManager::__Initialize()
{
    memset(m_adwPointEffect, 0, sizeof(m_adwPointEffect));
    m_pkInstMain = nullptr;
    m_pkInstBind = nullptr;
    m_pkInstPick = nullptr;
    m_v2PickedInstProjPos = Vector2(0.0f, 0.0f);
    ResetTabNextTargetVectorIndex();
}

CPythonCharacterManager::CPythonCharacterManager()
{
    __Initialize();
}

CPythonCharacterManager::~CPythonCharacterManager()
{
    CPythonCharacterManager::Destroy();
}

void CPythonCharacterManager::DeleteInstance(CInstanceBase *ch)
{
    if (ch == m_pkInstBind)
        m_pkInstBind = nullptr;

    if (ch == m_pkInstMain)
        m_pkInstMain = nullptr;

    if (ch == m_pkInstPick)
        m_pkInstPick = nullptr;

    delete ch;
}
