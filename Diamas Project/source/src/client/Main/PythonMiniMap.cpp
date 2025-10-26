#include "StdAfx.h"
#include "../eterLib/StateManager.h"
#include "../eterlib/Camera.h"
#include <pak/Vfs.hpp>

#include "PythonMiniMap.h"

#include "../EterBase/StepTimer.h"
#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "PythonGuild.h"

#include "../eterPythonLib/PythonWindowManager.h"
#include "../GameLib/NpcManager.h"
#include "../EterBase/Timer.h"

#ifdef ENABLE_ADMIN_MANAGER
#include "PythonAdmin.h"
#endif

void CPythonMiniMap::AddPartyMember(uint32_t dwPID, const char *szName)
{
    TPartyMember &rkPartyMember = m_kMap_dwPID_kPartyMember[dwPID];
    ZeroMemory(&rkPartyMember, sizeof(TPartyMember));
    rkPartyMember.pid = dwPID;
    strncpy(rkPartyMember.szName, szName, sizeof(rkPartyMember.szName));
    rkPartyMember.szName[CHARACTER_NAME_MAX_LEN] = '\0';
}

void CPythonMiniMap::MovePartyMember(uint32_t dwPID, long lX, long lY, float fRot)
{
    auto it = m_kMap_dwPID_kPartyMember.find(dwPID);
    if (it == m_kMap_dwPID_kPartyMember.end())
        return;
    if (lX > m_fAtlasMaxX)
        return;
    if (lY > m_fAtlasMaxY)
        return;
    /*char szBuf[256];    snprintf(szBuf, sizeof(szBuf), "MovePartyMember %u (%s) to %ldx%ld (rot %f)",    dwPID,
     * it->second.szName, lX, lY, fRot);    CPythonChat::Instance().AppendChat(CHAT_TYPE_INFO, szBuf);    */
    it->second.lX = lX;
    it->second.lY = lY;
    it->second.fRotation = fRot;
}

void CPythonMiniMap::RemovePartyMember(uint32_t dwPID)
{
    auto it = m_kMap_dwPID_kPartyMember.find(dwPID);
    if (it == m_kMap_dwPID_kPartyMember.end())
        return;
    it->second.lX = 0;
    it->second.lY = 0;
    it->second.fRotation = 0;
}

void CPythonMiniMap::ClearPartyMember() { m_kMap_dwPID_kPartyMember.clear(); }

void CPythonMiniMap::AddObserver(uint32_t dwVID, float fSrcX, float fSrcY)
{
    auto f = m_kMap_dwVID_kObserver.find(dwVID);
    if (m_kMap_dwVID_kObserver.end() == f)
    {
        SObserver kObserver;
        kObserver.dwSrcTime = ELTimer_GetMSec();
        kObserver.dwDstTime = kObserver.dwSrcTime + 1000;
        kObserver.fSrcX = fSrcX;
        kObserver.fSrcY = fSrcY;
        kObserver.fDstX = fSrcX;
        kObserver.fDstY = fSrcY;
        kObserver.fCurX = fSrcX;
        kObserver.fCurY = fSrcY;
        m_kMap_dwVID_kObserver.insert(std::make_pair(dwVID, kObserver));
    }
    else
    {
        SObserver &rkObserver = f->second;
        rkObserver.dwSrcTime = ELTimer_GetMSec();
        rkObserver.dwDstTime = rkObserver.dwSrcTime + 1000;
        rkObserver.fSrcX = fSrcX;
        rkObserver.fSrcY = fSrcY;
        rkObserver.fDstX = fSrcX;
        rkObserver.fDstY = fSrcY;
        rkObserver.fCurX = fSrcX;
        rkObserver.fCurY = fSrcY;
    }
}

void CPythonMiniMap::MoveObserver(uint32_t dwVID, float fDstX, float fDstY)
{
    auto f = m_kMap_dwVID_kObserver.find(dwVID);
    if (m_kMap_dwVID_kObserver.end() == f)
        return;

    SObserver &rkObserver = f->second;
    rkObserver.dwSrcTime = ELTimer_GetMSec();
    rkObserver.dwDstTime = rkObserver.dwSrcTime + 1000;
    rkObserver.fSrcX = rkObserver.fCurX;
    rkObserver.fSrcY = rkObserver.fCurY;
    rkObserver.fDstX = fDstX;
    rkObserver.fDstY = fDstY;
}

void CPythonMiniMap::RemoveObserver(uint32_t dwVID) { m_kMap_dwVID_kObserver.erase(dwVID); }

void CPythonMiniMap::SetCenterPosition(float fCenterX, float fCenterY)
{
    m_fCenterX = fCenterX;
    m_fCenterY = fCenterY;

    CMapOutdoor &rkMap = CPythonBackground::Instance().GetMapOutdoorRef();
    for (uint8_t byTerrainNum = 0; byTerrainNum < AROUND_AREA_NUM; ++byTerrainNum)
    {
        CTerrain *pTerrain;
        if (rkMap.GetTerrainPointer(byTerrainNum, &pTerrain))
            m_lpMiniMapTexture[byTerrainNum] = pTerrain->GetMiniMapTexture();
        else
            m_lpMiniMapTexture[byTerrainNum] = nullptr;
    }

    const TOutdoorMapCoordinate &rOutdoorMapCoord = rkMap.GetCurCoordinate();

    m_fCenterCellX = (m_fCenterX - (float)(rOutdoorMapCoord.m_sTerrainCoordX * CTerrainImpl::TERRAIN_XSIZE)) /
                     (float)(CTerrainImpl::CELLSCALE);
    m_fCenterCellY = (m_fCenterY - (float)(rOutdoorMapCoord.m_sTerrainCoordY * CTerrainImpl::TERRAIN_YSIZE)) /
                     (float)(CTerrainImpl::CELLSCALE);

    __SetPosition();
}

void CPythonMiniMap::Update(float fCenterX, float fCenterY)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    if (!rkBG.IsMapOutdoor())
        return;

    // ¹Ì´Ï¸Ê ±×¸² °»½Å
    if (m_fCenterX != fCenterX || m_fCenterY != fCenterY)
        SetCenterPosition(fCenterX, fCenterY);

    m_MinimapPosVector.clear();
    float fooCellScale = 1.0f / ((float)CTerrainImpl::CELLSCALE);

    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();

    CInstanceBase *pkInstMain = rkChrMgr.GetMainInstancePtr();
    if (!pkInstMain)
        return;

    CPythonCharacterManager::CharacterIterator i;
    for (i = rkChrMgr.CharacterInstanceBegin(); i != rkChrMgr.CharacterInstanceEnd(); ++i)
    {
        CInstanceBase *pkInstEach = *i;

        TPixelPosition kInstancePosition;
        pkInstEach->NEW_GetPixelPosition(&kInstancePosition);
        float fDistanceFromCenterX = (kInstancePosition.x - m_fCenterX) * fooCellScale * m_fScale;
        float fDistanceFromCenterY = (kInstancePosition.y - m_fCenterY) * fooCellScale * m_fScale;
        if (fabs(fDistanceFromCenterX) >= m_fMiniMapRadius || fabs(fDistanceFromCenterY) >= m_fMiniMapRadius)
            continue;

        float fDistanceFromCenter =
            sqrtf(fDistanceFromCenterX * fDistanceFromCenterX + fDistanceFromCenterY * fDistanceFromCenterY);
        if (fDistanceFromCenter >= m_fMiniMapRadius)
            continue;

        TMarkPosition aMarkPosition;

        if (pkInstEach->IsPC() && !pkInstEach->IsInvisibility())
        {
            if (pkInstEach == CPythonCharacterManager::Instance().GetMainInstancePtr())
                continue;

            aMarkPosition.m_bType = CActorInstance::TYPE_PC;
            aMarkPosition.m_fX = (m_fWidth - (float)m_WhiteMark.GetWidth()) / 2.0f + fDistanceFromCenterX + m_fScreenX;
            aMarkPosition.m_fY =
                (m_fHeight - (float)m_WhiteMark.GetHeight()) / 2.0f + fDistanceFromCenterY + m_fScreenY;
            aMarkPosition.m_eNameColor = pkInstEach->GetNameColorIndex();
            m_MinimapPosVector.push_back(aMarkPosition);
        }
        else if (pkInstEach->IsNPC())
        {
            aMarkPosition.m_bType = CActorInstance::TYPE_NPC;
            aMarkPosition.m_fX = (m_fWidth - (float)m_WhiteMark.GetWidth()) / 2.0f + fDistanceFromCenterX + m_fScreenX;
            aMarkPosition.m_fY =
                (m_fHeight - (float)m_WhiteMark.GetHeight()) / 2.0f + fDistanceFromCenterY + m_fScreenY;

            m_MinimapPosVector.push_back(aMarkPosition);
        }
        else if (pkInstEach->IsBuffBot())
        {
            aMarkPosition.m_bType = CActorInstance::TYPE_BUFFBOT;
            aMarkPosition.m_fX = (m_fWidth - (float)m_WhiteMark.GetWidth()) / 2.0f + fDistanceFromCenterX + m_fScreenX;
            aMarkPosition.m_fY =
                (m_fHeight - (float)m_WhiteMark.GetHeight()) / 2.0f + fDistanceFromCenterY + m_fScreenY;

            m_MinimapPosVector.push_back(aMarkPosition);
        }
        else if (pkInstEach->IsShop())
        {
            aMarkPosition.m_bType = CActorInstance::TYPE_SHOP;
            aMarkPosition.m_fX = (m_fWidth - (float)m_WhiteMark.GetWidth()) / 2.0f + fDistanceFromCenterX + m_fScreenX;
            aMarkPosition.m_fY =
                (m_fHeight - (float)m_WhiteMark.GetHeight()) / 2.0f + fDistanceFromCenterY + m_fScreenY;
            m_MinimapPosVector.push_back(aMarkPosition);
        }
        else if (pkInstEach->IsEnemy())
        {
            aMarkPosition.m_bType = CActorInstance::TYPE_ENEMY;
            aMarkPosition.m_fX = (m_fWidth - (float)m_WhiteMark.GetWidth()) / 2.0f + fDistanceFromCenterX + m_fScreenX;
            aMarkPosition.m_fY =
                (m_fHeight - (float)m_WhiteMark.GetHeight()) / 2.0f + fDistanceFromCenterY + m_fScreenY;

            m_MinimapPosVector.push_back(aMarkPosition);
        }
        else if (pkInstEach->IsWarp())
        {
            aMarkPosition.m_bType = CActorInstance::TYPE_WARP;
            aMarkPosition.m_fX = (m_fWidth - (float)m_WhiteMark.GetWidth()) / 2.0f + fDistanceFromCenterX + m_fScreenX;
            aMarkPosition.m_fY =
                (m_fHeight - (float)m_WhiteMark.GetHeight()) / 2.0f + fDistanceFromCenterY + m_fScreenY;

            m_MinimapPosVector.push_back(aMarkPosition);
        }
        else if (pkInstEach->IsStone())
        {
            aMarkPosition.m_bType = CActorInstance::TYPE_STONE;
            aMarkPosition.m_fX = (m_fWidth - (float)m_WhiteMark.GetWidth()) / 2.0f + fDistanceFromCenterX + m_fScreenX;
            aMarkPosition.m_fY =
                (m_fHeight - (float)m_WhiteMark.GetHeight()) / 2.0f + fDistanceFromCenterY + m_fScreenY;
            m_MinimapPosVector.push_back(aMarkPosition);
        }
    }

    {
        uint32_t dwCurTime = ELTimer_GetMSec();

        for (auto i = m_kMap_dwVID_kObserver.begin(); i != m_kMap_dwVID_kObserver.end(); ++i)
        {
            SObserver &rkObserver = i->second;

            float fPos = float(dwCurTime - rkObserver.dwSrcTime) / float(rkObserver.dwDstTime - rkObserver.dwSrcTime);
            if (fPos < 0.0f)
                fPos = 0.0f;
            else if (fPos > 1.0f)
                fPos = 1.0f;

            rkObserver.fCurX = (rkObserver.fDstX - rkObserver.fSrcX) * fPos + rkObserver.fSrcX;
            rkObserver.fCurY = (rkObserver.fDstY - rkObserver.fSrcY) * fPos + rkObserver.fSrcY;

            TPixelPosition kInstancePosition;
            kInstancePosition.x = rkObserver.fCurX;
            kInstancePosition.y = rkObserver.fCurY;
            kInstancePosition.z = 0.0f;

            float fDistanceFromCenterX = (kInstancePosition.x - m_fCenterX) * fooCellScale * m_fScale;
            float fDistanceFromCenterY = (kInstancePosition.y - m_fCenterY) * fooCellScale * m_fScale;
            if (fabs(fDistanceFromCenterX) >= m_fMiniMapRadius || fabs(fDistanceFromCenterY) >= m_fMiniMapRadius)
                continue;

            float fDistanceFromCenter =
                sqrtf(fDistanceFromCenterX * fDistanceFromCenterX + fDistanceFromCenterY * fDistanceFromCenterY);
            if (fDistanceFromCenter >= m_fMiniMapRadius)
                continue;

            TMarkPosition aMarkPosition;
            aMarkPosition.m_bType = CActorInstance::TYPE_PC;
            aMarkPosition.m_fX = (m_fWidth - (float)m_WhiteMark.GetWidth()) / 2.0f + fDistanceFromCenterX + m_fScreenX;
            aMarkPosition.m_fY =
                (m_fHeight - (float)m_WhiteMark.GetHeight()) / 2.0f + fDistanceFromCenterY + m_fScreenY;
            aMarkPosition.m_eNameColor = CInstanceBase::NAMECOLOR_PARTY;
            m_MinimapPosVector.push_back(aMarkPosition);
        }
    }

    std::sort(m_MinimapPosVector.begin(), m_MinimapPosVector.end(), [](TMarkPosition a, TMarkPosition b) {
        return a.m_bType > b.m_bType;
    }); // Sort minimap by TYPE => LESS GPU calls

    {
        TAtlasMarkInfoVector::iterator itor = m_AtlasWayPointInfoVector.begin();
        for (; itor != m_AtlasWayPointInfoVector.end(); ++itor)
        {
            TAtlasMarkInfo &rAtlasMarkInfo = *itor;

            if (TYPE_TARGET != rAtlasMarkInfo.m_byType)
                continue;

            if (0 != rAtlasMarkInfo.m_dwChrVID)
            {
                CInstanceBase *pInstance =
                    CPythonCharacterManager::Instance().GetInstancePtr(rAtlasMarkInfo.m_dwChrVID);
                if (pInstance)
                {
                    TPixelPosition kPixelPosition;
                    pInstance->NEW_GetPixelPosition(&kPixelPosition);
                    __UpdateWayPoint(&rAtlasMarkInfo, kPixelPosition.x, kPixelPosition.y);
                }
            }

            const float c_fMiniMapWindowRadius = 55.0f;

            float fDistanceFromCenterX = (rAtlasMarkInfo.m_fX - m_fCenterX) * fooCellScale * m_fScale;
            float fDistanceFromCenterY = (rAtlasMarkInfo.m_fY - m_fCenterY) * fooCellScale * m_fScale;
            float fDistanceFromCenter =
                sqrtf(fDistanceFromCenterX * fDistanceFromCenterX + fDistanceFromCenterY * fDistanceFromCenterY);

            if (fDistanceFromCenter >= c_fMiniMapWindowRadius)
            {
                float fRadianX = acosf(fDistanceFromCenterX / fDistanceFromCenter);
                float fRadianY = asinf(fDistanceFromCenterY / fDistanceFromCenter);
                fDistanceFromCenterX = 55.0f * cosf(fRadianX);
                fDistanceFromCenterY = 55.0f * sinf(fRadianY);
                rAtlasMarkInfo.m_fMiniMapX =
                    (m_fWidth - (float)m_WhiteMark.GetWidth()) / 2.0f + fDistanceFromCenterX + m_fScreenX + 2.0f;
                rAtlasMarkInfo.m_fMiniMapY =
                    (m_fHeight - (float)m_WhiteMark.GetHeight()) / 2.0f + fDistanceFromCenterY + m_fScreenY + 2.0f;
            }
            else
            {
                rAtlasMarkInfo.m_fMiniMapX =
                    (m_fWidth - (float)m_WhiteMark.GetWidth()) / 2.0f + fDistanceFromCenterX + m_fScreenX;
                rAtlasMarkInfo.m_fMiniMapY =
                    (m_fHeight - (float)m_WhiteMark.GetHeight()) / 2.0f + fDistanceFromCenterY + m_fScreenY;
            }
        }
    }
}

void CPythonMiniMap::Render(float fScreenX, float fScreenY)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    if (!rkBG.IsMapOutdoor())
        return;

    if (!m_bShow)
        return;

    if (!rkBG.IsMapReady())
        return;

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"**  CPythonMiniMap::Render **");

    if (m_fScreenX != fScreenX || m_fScreenY != fScreenY)
    {
        m_fScreenX = fScreenX;
        m_fScreenY = fScreenY;
        __SetPosition();
    }

    if (m_MiniMapFilterGraphicImageInstance.IsEmpty())
        return;

    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

    STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

    STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

    STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFF000000);

    STATEMANAGER.SetTexture(1, m_MiniMapFilterGraphicImageInstance.GetTexturePointer()->GetD3DTexture());
    STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &m_matMiniMapCover);
    Engine::GetDevice().SetFvF(D3DFVF_XYZ | D3DFVF_TEX1);

    Engine::GetDevice().SetVertexBuffer(m_VertexBuffer, 0, 0);
    Engine::GetDevice().SetIndexBuffer(m_IndexBuffer);

    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorld);

    if(!isValid(m_VertexBuffer) || !isValid(m_IndexBuffer))
        return;

    for (uint8_t byTerrainNum = 0; byTerrainNum < AROUND_AREA_NUM; ++byTerrainNum)
    {
        LPDIRECT3DTEXTURE9 pMiniMapTexture = m_lpMiniMapTexture[byTerrainNum];
        STATEMANAGER.SetTexture(0, pMiniMapTexture);
        if (pMiniMapTexture)
        {
            Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, byTerrainNum * 4, 4, byTerrainNum * 6, 2);
        }
        else
        {
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
            Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, byTerrainNum * 4, 4, byTerrainNum * 6, 2);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        }
    }

    STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAARG2);
    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAARG1);
    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAOP);
    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLORARG1);
    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLORARG2);
    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLOROP);

    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSU);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSV);
    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);
    STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSU);
    STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSV);

    SetDiffuseOperation();
    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matIdentity);

    STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);

    char stage = 0;
    bool scaleIsOK = m_fScale >= 2.0f;

    for (auto &i : m_MinimapPosVector)
    {
        if (!scaleIsOK && (i.m_bType == CActorInstance::TYPE_PC || i.m_bType == CActorInstance::TYPE_ENEMY))
            // Stop iterating if we cant draw
            continue;

        if (i.m_bType == CActorInstance::TYPE_PC) // Can have multiple colors no stage used
        {
            if (i.m_eNameColor == CInstanceBase::NAMECOLOR_PARTY)
            {
                const float now = DX::StepTimer::Instance().GetTotalSeconds();
                float v = (1.0f + std::sin(now * 6.0f)) / 5.0f + 0.6f;
                DirectX::SimpleMath::Color c(CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_PARTY));
                //(m_MarkTypeToColorMap[TYPE_PARTY]);
                DirectX::SimpleMath::Color d(v, v, v, 1);
                c = Color::Modulate(c, d);
                STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, c.BGRA().c);
            }
            else
            {
                STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR,
                                            CInstanceBase::GetIndexedNameColor(i.m_eNameColor).BGRA().c);
            }
        }
        else if (stage != 2 && i.m_bType == CActorInstance::TYPE_ENEMY)
        {
            STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR,
                                        CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_MOB).BGRA().c);
            // m_MarkTypeToColorMap[TYPE_MONSTER]);
            stage = 2;
        }
        else if (stage != 3 && i.m_bType == CActorInstance::TYPE_NPC)
        {
            STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR,
                                        CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_NPC).BGRA().c);
            stage = 3;
        }
        else if (stage != 4 && i.m_bType == CActorInstance::TYPE_WARP)
        {
            STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR,
                                        CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WARP).BGRA().c);
            stage = 4;
        }
        else if (stage != 5 && i.m_bType == CActorInstance::TYPE_SHOP)
        {
            STATEMANAGER.SetRenderState(
                D3DRS_TEXTUREFACTOR,
                CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_OFFLINE_SHOP).BGRA().c);
            stage = 5;
        }
        else if (stage != 6 && i.m_bType == CActorInstance::TYPE_BUFFBOT)
        {
            STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR,
                                        CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_BUFFBOT).BGRA().c);
            stage = 6;
        }
        else if (stage != 7 && i.m_bType == CActorInstance::TYPE_STONE)
        {
            STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR,
                                        CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_STONE).BGRA().c);
            stage = 7;
        }

        m_WhiteMark.SetPosition(i.m_fX, i.m_fY);
        m_WhiteMark.Render();
    }

    STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MIPFILTER);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);

    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // Ä³¸¯ÅÍ ¸¶Å©
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

    if (pkInst)
    {
        float fRotation;
        fRotation = (540.0f - pkInst->GetRotation());
        while (fRotation > 360.0f)
            fRotation -= 360.0f;
        while (fRotation < 0.0f)
            fRotation += 360.0f;

        m_PlayerMark.SetRotation(fRotation);
        m_PlayerMark.Render();
    }

    // Target
    {
        TAtlasMarkInfoVector::iterator itor = m_AtlasWayPointInfoVector.begin();
        for (; itor != m_AtlasWayPointInfoVector.end(); ++itor)
        {
            TAtlasMarkInfo &rAtlasMarkInfo = *itor;

            if (TYPE_TARGET != rAtlasMarkInfo.m_byType)
                continue;
            if (rAtlasMarkInfo.m_fMiniMapX <= 0.0f)
                continue;
            if (rAtlasMarkInfo.m_fMiniMapY <= 0.0f)
                continue;

            __RenderTargetMark(rAtlasMarkInfo.m_fMiniMapX, rAtlasMarkInfo.m_fMiniMapY);
        }
    }

    CCamera *pkCmrCur = CCameraManager::Instance().GetCurrentCamera();

    // Ä«¸Þ¶ó ¹æÇâ
    if (pkCmrCur)
    {
        m_MiniMapCameraraphicImageInstance.SetRotation(pkCmrCur->GetRoll());
        m_MiniMapCameraraphicImageInstance.Render();
    }
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);

    D3DPERF_EndEvent();
}

void CPythonMiniMap::SetScale(float fScale)
{
    if (fScale >= 4.0f)
        fScale = 4.0f;
    if (fScale <= 0.5f)
        fScale = 0.5f;
    m_fScale = fScale;

    __SetPosition();
}

void CPythonMiniMap::ScaleUp()
{
    m_fScale *= 2.0f;
    if (m_fScale >= 4.0f)
        m_fScale = 4.0f;
    __SetPosition();
}

void CPythonMiniMap::ScaleDown()
{
    m_fScale *= 0.5f;
    if (m_fScale <= 0.5f)
        m_fScale = 0.5f;
    __SetPosition();
}

void CPythonMiniMap::SetMiniMapSize(float fWidth, float fHeight)
{
    m_fWidth = fWidth;
    m_fHeight = fHeight;
}

#pragma pack(push)
#pragma pack(1)
typedef struct _MINIMAPVERTEX
{
    float x, y, z; // position
    float u, v;    // normal
} MINIMAPVERTEX, *LPMINIMAPVERTEX;
#pragma pack(pop)

bool CPythonMiniMap::Create()
{
    const std::string strImageRoot = "D:/ymir work/ui/";
    const std::string strImageFilter = strImageRoot + "minimap_image_filter.dds";
    const std::string strImageCamera = strImageRoot + "minimap_camera.dds";
    const std::string strPlayerMark = strImageRoot + "minimap/playermark.sub";
    const std::string strWhiteMark = strImageRoot + "minimap/whitemark.sub";

    auto &resMgr = CResourceManager::Instance();

    // 미니맵 커버
    m_MiniMapFilterGraphicImageInstance.SetImagePointer(resMgr.LoadResource<CGraphicImage>(strImageFilter));
    m_MiniMapCameraraphicImageInstance.SetImagePointer(resMgr.LoadResource<CGraphicImage>(strImageCamera));

    m_matMiniMapCover._11 = 1.0f / ((float)m_MiniMapFilterGraphicImageInstance.GetWidth());
    m_matMiniMapCover._22 = 1.0f / ((float)m_MiniMapFilterGraphicImageInstance.GetHeight());
    m_matMiniMapCover._33 = 0.0f;

    // Ä³¸¯ÅÍ ¸¶Å©
    auto pSubImage = CResourceManager::Instance().LoadResource<CGraphicImage>(strPlayerMark.c_str());
    m_PlayerMark.SetImagePointer(pSubImage);

    pSubImage = CResourceManager::Instance().LoadResource<CGraphicImage>(strWhiteMark.c_str());
    m_WhiteMark.SetImagePointer(pSubImage);

    char buf[256];
    for (int i = 0; i < MINI_WAYPOINT_IMAGE_COUNT; ++i)
    {
        sprintf(buf, "%sminimap/mini_waypoint%02d.sub", strImageRoot.c_str(), i + 1);
        m_MiniWayPointGraphicImageInstances[i].SetImagePointer(resMgr.LoadResource<CGraphicImage>(buf));
        m_MiniWayPointGraphicImageInstances[i].SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
    }
    for (int j = 0; j < WAYPOINT_IMAGE_COUNT; ++j)
    {
        sprintf(buf, "%sminimap/waypoint%02d.sub", strImageRoot.c_str(), j + 1);
        m_WayPointGraphicImageInstances[j].SetImagePointer(resMgr.LoadResource<CGraphicImage>(buf));
        m_WayPointGraphicImageInstances[j].SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
    }
    for (int k = 0; k < TARGET_MARK_IMAGE_COUNT; ++k)
    {
        sprintf(buf, "%sminimap/targetmark%02d.sub", strImageRoot.c_str(), k + 1);
        m_WayPointGraphicImageInstances[k].SetImagePointer(resMgr.LoadResource<CGraphicImage>(buf));
        m_TargetMarkGraphicImageInstances[k].SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
    }

    m_GuildAreaFlagImageInstance.SetImagePointer(
        resMgr.LoadResource<CGraphicImage>("d:/ymir work/ui/minimap/GuildArea01.sub"));

    // ±×·ÁÁú Æú¸®°ï ¼¼ÆÃ
#pragma pack(push)
#pragma pack(1)
    LPMINIMAPVERTEX lpMiniMapVertex;
    LPMINIMAPVERTEX lpOrigMiniMapVertex;
#pragma pack(pop)

    m_VertexBuffer = Engine::GetDevice().CreateVertexBuffer(36 * 20, 20, nullptr, D3DUSAGE_WRITEONLY);

    if (!isValid(m_VertexBuffer))
    {
        return false;
    }

    lpOrigMiniMapVertex = (LPMINIMAPVERTEX)Engine::GetDevice().LockVertexBuffer(m_VertexBuffer);
    if (lpOrigMiniMapVertex)
    {
        char *pchMiniMapVertex = (char *)lpOrigMiniMapVertex;
        memset(pchMiniMapVertex, 0, sizeof(char) * 720);
        lpMiniMapVertex = (LPMINIMAPVERTEX)pchMiniMapVertex;

        for (int iY = -3; iY <= 1; ++iY)
        {
            if (0 == iY % 2)
                continue;
            float fY = 0.5f * ((float)iY);
            for (int iX = -3; iX <= 1; ++iX)
            {
                if (0 == iX % 2)
                    continue;
                float fX = 0.5f * ((float)iX);
                lpMiniMapVertex = (LPMINIMAPVERTEX)pchMiniMapVertex;
                lpMiniMapVertex->x = fX;
                lpMiniMapVertex->y = fY;
                lpMiniMapVertex->z = 0.0f;
                lpMiniMapVertex->u = 0.0f;
                lpMiniMapVertex->v = 0.0f;
                pchMiniMapVertex += 20;
                lpMiniMapVertex = (LPMINIMAPVERTEX)pchMiniMapVertex;
                lpMiniMapVertex->x = fX;
                lpMiniMapVertex->y = fY + 1.0f;
                lpMiniMapVertex->z = 0.0f;
                lpMiniMapVertex->u = 0.0f;
                lpMiniMapVertex->v = 1.0f;
                pchMiniMapVertex += 20;
                lpMiniMapVertex = (LPMINIMAPVERTEX)pchMiniMapVertex;
                lpMiniMapVertex->x = fX + 1.0f;
                lpMiniMapVertex->y = fY;
                lpMiniMapVertex->z = 0.0f;
                lpMiniMapVertex->u = 1.0f;
                lpMiniMapVertex->v = 0.0f;
                pchMiniMapVertex += 20;
                lpMiniMapVertex = (LPMINIMAPVERTEX)pchMiniMapVertex;
                lpMiniMapVertex->x = fX + 1.0f;
                lpMiniMapVertex->y = fY + 1.0f;
                lpMiniMapVertex->z = 0.0f;
                lpMiniMapVertex->u = 1.0f;
                lpMiniMapVertex->v = 1.0f;
                pchMiniMapVertex += 20;
            }
        }

        Engine::GetDevice().UnlockVertexBuffer(m_VertexBuffer);
    }

    uint16_t pwIndices[54] = {0,  1,  2,  2,  1,  3,  4,  5,  6,  6,  5,  7,  8,  9,  10, 10, 9,  11,

                              12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23,

                              24, 25, 26, 26, 25, 27, 28, 29, 30, 30, 29, 31, 32, 33, 34, 34, 33, 35};

    m_IndexBuffer = Engine::GetDevice().CreateIndexBuffer(54, D3DFMT_INDEX16, pwIndices);
    if (!isValid(m_IndexBuffer))
        return false;

    return true;
}

void CPythonMiniMap::__SetPosition()
{
    m_fMiniMapRadius = fMIN(6400.0f / ((float)CTerrainImpl::CELLSCALE) * m_fScale, 64.0f);

    m_matWorld._11 = m_fWidth * m_fScale;
    m_matWorld._22 = m_fHeight * m_fScale;
    m_matWorld._41 = (1.0f + m_fScale) * m_fWidth * 0.5f - m_fCenterCellX * m_fScale + m_fScreenX;
    m_matWorld._42 = (1.0f + m_fScale) * m_fHeight * 0.5f - m_fCenterCellY * m_fScale + m_fScreenY;

    if (!m_MiniMapFilterGraphicImageInstance.IsEmpty())
    {
        m_matMiniMapCover._41 = -(m_fScreenX) / ((float)m_MiniMapFilterGraphicImageInstance.GetWidth());
        m_matMiniMapCover._42 = -(m_fScreenY) / ((float)m_MiniMapFilterGraphicImageInstance.GetHeight());
    }

    if (!m_PlayerMark.IsEmpty())
        m_PlayerMark.SetPosition((m_fWidth - (float)m_PlayerMark.GetWidth()) / 2.0f + m_fScreenX,
                                 (m_fHeight - (float)m_PlayerMark.GetHeight()) / 2.0f + m_fScreenY);

    if (!m_MiniMapCameraraphicImageInstance.IsEmpty())
        m_MiniMapCameraraphicImageInstance.SetPosition(
            (m_fWidth - (float)m_MiniMapCameraraphicImageInstance.GetWidth()) / 2.0f + m_fScreenX,
            (m_fHeight - (float)m_MiniMapCameraraphicImageInstance.GetHeight()) / 2.0f + m_fScreenY);
}

//////////////////////////////////////////////////////////////////////////
// Atlas

void CPythonMiniMap::ClearAtlasMarkInfo()
{
    m_AtlasNPCInfoVector.clear();
    m_AtlasWarpInfoVector.clear();
}

void CPythonMiniMap::ClearAtlasShopInfo() { m_AtlasShopInfoVector.clear(); }

void CPythonMiniMap::RegisterAtlasMark(uint8_t byType, const char *c_szName, long lx, long ly)
{
    TAtlasMarkInfo aAtlasMarkInfo;

    aAtlasMarkInfo.m_fX = float(lx);
    aAtlasMarkInfo.m_fY = float(ly);
    aAtlasMarkInfo.m_strText = c_szName;

    aAtlasMarkInfo.m_fScreenX =
        aAtlasMarkInfo.m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX - (float)m_WhiteMark.GetWidth() / 2.0f;
    aAtlasMarkInfo.m_fScreenY =
        aAtlasMarkInfo.m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY - (float)m_WhiteMark.GetHeight() / 2.0f;

    switch (byType)
    {
    case CActorInstance::TYPE_NPC:
        aAtlasMarkInfo.m_byType = TYPE_NPC;
        m_AtlasNPCInfoVector.push_back(aAtlasMarkInfo);
        break;
    case CActorInstance::TYPE_BUFFBOT:
        aAtlasMarkInfo.m_byType = TYPE_BUFFBOT;
        m_AtlasNPCInfoVector.push_back(aAtlasMarkInfo);
        break;
    case CActorInstance::TYPE_WARP:
        aAtlasMarkInfo.m_byType = TYPE_WARP;
        {
            int iPos = aAtlasMarkInfo.m_strText.find(' ');
            if (iPos >= 0)
                aAtlasMarkInfo.m_strText[iPos] = 0;
        }
        m_AtlasWarpInfoVector.push_back(aAtlasMarkInfo);
        break;
    case CActorInstance::TYPE_SHOP:
        aAtlasMarkInfo.m_byType = CActorInstance::TYPE_SHOP;
        aAtlasMarkInfo.m_strText.append("'s Shop ");
        m_AtlasShopInfoVector.push_back(aAtlasMarkInfo);
        break;
    }
}

void CPythonMiniMap::ClearGuildArea() { m_GuildAreaInfoVector.clear(); }

void CPythonMiniMap::UpdateGuildArea(uint32_t updateID, uint32_t updatedGuild)
{
    for (auto &rInfo : m_GuildAreaInfoVector)
    {
        if (rInfo.dwID == updateID)
            rInfo.dwGuildID = updatedGuild;
    }
}

void CPythonMiniMap::RegisterGuildArea(uint32_t dwID, uint32_t dwGuildID, long x, long y, long width, long height)
{
    TGuildAreaInfo kGuildAreaInfo;
    kGuildAreaInfo.dwID = dwID;
    kGuildAreaInfo.dwGuildID = dwGuildID;
    kGuildAreaInfo.lx = x;
    kGuildAreaInfo.ly = y;
    kGuildAreaInfo.lwidth = width;
    kGuildAreaInfo.lheight = height;
    m_GuildAreaInfoVector.push_back(kGuildAreaInfo);
}

uint32_t CPythonMiniMap::GetGuildAreaID(uint32_t x, uint32_t y)
{
    TGuildAreaInfoVectorIterator itor = m_GuildAreaInfoVector.begin();
    for (; itor != m_GuildAreaInfoVector.end(); ++itor)
    {
        TGuildAreaInfo &rAreaInfo = *itor;

        if (x >= rAreaInfo.lx)
            if (y >= rAreaInfo.ly)
                if (x <= rAreaInfo.lx + rAreaInfo.lwidth)
                    if (y <= rAreaInfo.ly + rAreaInfo.lheight)
                    {
                        return rAreaInfo.dwGuildID;
                    }
    }

    return 0xffffffff;
}

void CPythonMiniMap::CreateTarget(int iID, const char *c_szName)
{
    AddWayPoint(TYPE_TARGET, iID, 0.0f, 0.0f, c_szName);
}

void CPythonMiniMap::UpdateTarget(int iID, int ix, int iy)
{
    TAtlasMarkInfo *pkInfo;
    if (!__GetWayPoint(iID, &pkInfo))
        return;

    if (0 != pkInfo->m_dwChrVID)
    {
        if (CPythonCharacterManager::Instance().GetInstancePtr(pkInfo->m_dwChrVID))
            return;
    }

    if (ix < 0)
        return;
    if (iy < 0)
        return;
    if (ix > m_fAtlasMaxX)
        return;
    if (iy > m_fAtlasMaxY)
        return;

    __UpdateWayPoint(pkInfo, ix, iy);
}

void CPythonMiniMap::CreateTarget(int iID, const char *c_szName, uint32_t dwVID)
{
    AddWayPoint(TYPE_TARGET, iID, 0.0f, 0.0f, c_szName, dwVID);
}

void CPythonMiniMap::DeleteTarget(int iID) { RemoveWayPoint(iID); }

void CPythonMiniMap::__LoadAtlasMarkInfo()
{
    ClearAtlasMarkInfo();
    ClearGuildArea();

    CPythonBackground &rkBG = CPythonBackground::Instance();
    if (!rkBG.IsMapOutdoor())
        return;

    CMapOutdoor &rkMap = rkBG.GetMapOutdoorRef();

    // LOCALE
    char szAtlasMarkInfoFileName[64 + 1];
    _snprintf(szAtlasMarkInfoFileName, sizeof(szAtlasMarkInfoFileName), "%s/map/%s_point.txt",
              LocaleService_GetLocalePath(), rkMap.GetName().c_str());
    // END_OF_LOCALE

    CTokenVectorMap stTokenVectorMap;

    if (!LoadMultipleTextData(szAtlasMarkInfoFileName, stTokenVectorMap))
    {
        SPDLOG_DEBUG(" CPythonMiniMap::__LoadAtlasMarkInfo File Load %s ERROR\n", szAtlasMarkInfoFileName);
        return;
    }

    const std::string strType[TYPE_COUNT] = {"OPC", "OPCPVP", "OPCPVPSELF", "NPC", "MONSTER", "WARP", "WAYPOINT"};

    for (uint32_t i = 0; i < stTokenVectorMap.size(); ++i)
    {
        char szMarkInfoName[32 + 1];
        _snprintf(szMarkInfoName, sizeof(szMarkInfoName), "%d", i);

        if (stTokenVectorMap.end() == stTokenVectorMap.find(szMarkInfoName))
            continue;

        const CTokenVector &rVector = stTokenVectorMap[szMarkInfoName];

        if (rVector.size() == 3)
        {
            const std::string &c_rstrPositionX = rVector[0];
            const std::string &c_rstrPositionY = rVector[1];
            const std::string &c_rstrNPCVnum = rVector[2];

            NpcManager &rkNonPlayer = NpcManager::Instance();
            auto npcVnum = atoi(c_rstrNPCVnum.c_str());

            TAtlasMarkInfo aAtlasMarkInfo;

            aAtlasMarkInfo.m_fX = atof(c_rstrPositionX.c_str());
            aAtlasMarkInfo.m_fY = atof(c_rstrPositionY.c_str());
            aAtlasMarkInfo.m_strText = rkNonPlayer.GetMonsterName(npcVnum);

            rkNonPlayer.GetInstanceType(npcVnum, &aAtlasMarkInfo.m_byType);

            aAtlasMarkInfo.m_fScreenX =
                aAtlasMarkInfo.m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX - (float)m_WhiteMark.GetWidth() / 2.0f;
            aAtlasMarkInfo.m_fScreenY =
                aAtlasMarkInfo.m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY - (float)m_WhiteMark.GetHeight() / 2.0f;

            switch (aAtlasMarkInfo.m_byType)
            {
            case TYPE_NPC:
                m_AtlasNPCInfoVector.push_back(aAtlasMarkInfo);
                break;
            case TYPE_WARP:
                m_AtlasWarpInfoVector.push_back(aAtlasMarkInfo);
                break;
            }
        }
        else
        {
            const std::string &c_rstrType = rVector[0];
            const std::string &c_rstrPositionX = rVector[1];
            const std::string &c_rstrPositionY = rVector[2];
            const std::string &c_rstrText = rVector[3];

            TAtlasMarkInfo aAtlasMarkInfo;

            for (int i = 0; i < TYPE_COUNT; ++i)
            {
                if (0 == c_rstrType.compare(strType[i]))
                    aAtlasMarkInfo.m_byType = (uint8_t)i;
            }
            aAtlasMarkInfo.m_fX = atof(c_rstrPositionX.c_str());
            aAtlasMarkInfo.m_fY = atof(c_rstrPositionY.c_str());
            aAtlasMarkInfo.m_strText = c_rstrText;

            aAtlasMarkInfo.m_fScreenX =
                aAtlasMarkInfo.m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX - (float)m_WhiteMark.GetWidth() / 2.0f;
            aAtlasMarkInfo.m_fScreenY =
                aAtlasMarkInfo.m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY - (float)m_WhiteMark.GetHeight() / 2.0f;

            switch (aAtlasMarkInfo.m_byType)
            {
            case TYPE_NPC:
                m_AtlasNPCInfoVector.push_back(aAtlasMarkInfo);
                break;
            case TYPE_WARP:
                m_AtlasWarpInfoVector.push_back(aAtlasMarkInfo);
                break;
            }
        }
    }
}

bool CPythonMiniMap::LoadAtlas()
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    if (!rkBG.IsMapOutdoor())
        return false;

    CMapOutdoor &rkMap = rkBG.GetMapOutdoorRef();
    auto &resMgr = CResourceManager::Instance();
    const char *playerMarkFileName = "d:/ymir work/ui/minimap/playermark.sub";

    std::string atlasFileName = fmt::format("{}/atlas.sub", rkMap.GetName().c_str());
    if (!GetVfs().Exists(atlasFileName))
    {
        atlasFileName = fmt::format("d:/ymir work/ui/atlas/{}/atlas.sub", rkMap.GetName().c_str());
    }

    m_AtlasImageInstance.Destroy();
    m_AtlasPlayerMark.Destroy();
    m_AtlasPartyPlayerMark.Destroy();

    m_AtlasImageInstance.SetImagePointer(resMgr.LoadResource<CGraphicImage>(atlasFileName));

    if (m_AtlasImageInstance.IsEmpty())
        m_bAtlas = false;
    else
        m_bAtlas = true;

    m_AtlasPlayerMark.SetImagePointer(resMgr.LoadResource<CGraphicImage>(playerMarkFileName));
    m_AtlasPartyPlayerMark.SetImagePointer(resMgr.LoadResource<CGraphicImage>(playerMarkFileName));
    m_AtlasPartyPlayerMark.SetDiffuseColor(0.0f, 0.0f, 1.0f, 0.9f);

    short sTerrainCountX, sTerrainCountY;

    rkMap.GetTerrainCount(&sTerrainCountX, &sTerrainCountY);
    m_fAtlasMaxX = (float)sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE;
    m_fAtlasMaxY = (float)sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE;

    m_fAtlasImageSizeX = (float)m_AtlasImageInstance.GetWidth();
    m_fAtlasImageSizeY = (float)m_AtlasImageInstance.GetHeight();

    __LoadAtlasMarkInfo();

    if (m_bShowAtlas)
        OpenAtlasWindow();

    return true;
}

void CPythonMiniMap::__GlobalPositionToAtlasPosition(long lx, long ly, float *pfx, float *pfy)
{
    *pfx = lx / m_fAtlasMaxX * m_fAtlasImageSizeX;
    *pfy = ly / m_fAtlasMaxY * m_fAtlasImageSizeY;
}

void CPythonMiniMap::UpdateAtlas()
{
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

    if (pkInst)
    {
        TPixelPosition kInstPos;
        pkInst->NEW_GetPixelPosition(&kInstPos);

        float fRotation;
        fRotation = (540.0f - pkInst->GetRotation());
        while (fRotation > 360.0f)
            fRotation -= 360.0f;
        while (fRotation < 0.0f)
            fRotation += 360.0f;

        m_AtlasPlayerMark.SetPosition(
            kInstPos.x / m_fAtlasMaxX * m_fAtlasImageSizeX - (float)m_AtlasPlayerMark.GetWidth() / 2.0f,
            kInstPos.y / m_fAtlasMaxY * m_fAtlasImageSizeY - (float)m_AtlasPlayerMark.GetHeight() / 2.0f);
        m_AtlasPlayerMark.SetRotation(fRotation);
    }

    {
        TGuildAreaInfoVectorIterator itor = m_GuildAreaInfoVector.begin();
        for (; itor != m_GuildAreaInfoVector.end(); ++itor)
        {
            TGuildAreaInfo &rInfo = *itor;
            __GlobalPositionToAtlasPosition(rInfo.lx, rInfo.ly, &rInfo.fsxRender, &rInfo.fsyRender);
            __GlobalPositionToAtlasPosition(rInfo.lx + rInfo.lwidth, rInfo.ly + rInfo.lheight, &rInfo.fexRender,
                                            &rInfo.feyRender);
        }
    }

    {
        auto itor = m_kMap_dwPID_kPartyMember.begin();
        for (; itor != m_kMap_dwPID_kPartyMember.end(); ++itor)
        {
            TPartyMember &rInfo = itor->second;
            __GlobalPositionToAtlasPosition(rInfo.lX, rInfo.lY, &rInfo.fScreenX, &rInfo.fScreenY);
        }
    }
}

void CPythonMiniMap::RenderAtlas(float fScreenX, float fScreenY)
{
    if (!m_bShowAtlas)
        return;

    if (m_fAtlasScreenX != fScreenX || m_fAtlasScreenY != fScreenY)
    {
        m_matWorldAtlas._41 = fScreenX;
        m_matWorldAtlas._42 = fScreenY;
        m_fAtlasScreenX = fScreenX;
        m_fAtlasScreenY = fScreenY;
    }

    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldAtlas);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    m_AtlasImageInstance.Render();

    STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

    if (m_bAtlasRenderShops)
    {
        STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR,
                                    CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_OFFLINE_SHOP).BGRA().c);
        for (auto &rAtlasMarkInfo : m_AtlasShopInfoVector)
        {
            m_WhiteMark.SetPosition(rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
            m_WhiteMark.Render();
        }
    }

    if (m_bAtlasRenderNpc)
    {
        STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR,
                                    CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_NPC).BGRA().c);
        for (auto &rAtlasMarkInfo : m_AtlasNPCInfoVector)
        {
            m_WhiteMark.SetPosition(rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
            m_WhiteMark.Render();
        }
    }

    if (m_bAtlasRenderWarp)
    {
        STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR,
                                    CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WARP).BGRA().c);
        for (auto &rAtlasMarkInfo : m_AtlasWarpInfoVector)
        {
            m_WhiteMark.SetPosition(rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
            m_WhiteMark.Render();
        }
    }

    if (m_bAtlasRenderWaypoint)
    {
        STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR,
                                    CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WAYPOINT).BGRA().c);

        for (auto &rAtlasMarkInfo : m_AtlasWayPointInfoVector)
        {
            if (rAtlasMarkInfo.m_fScreenX <= 0.0f)
                continue;
            if (rAtlasMarkInfo.m_fScreenY <= 0.0f)
                continue;

            if (TYPE_TARGET == rAtlasMarkInfo.m_byType)
            {
                __RenderMiniWayPointMark(rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
            }
            else
            {
                __RenderWayPointMark(rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
            }
        }
    }

    STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

    if ((ELTimer_GetMSec() / 500) % 2)
    {
        for (auto itor = m_kMap_dwPID_kPartyMember.begin(); itor != m_kMap_dwPID_kPartyMember.end(); ++itor)
        {
            if (itor->second.lX == 0 && itor->second.lY == 0)
                continue;
            // TraceError("PartyPosition %ld %ld (screen %f %f)", itor->second.lX, itor->second.lY,
            // itor->second.fScreenX, itor->second.fScreenY);
            m_AtlasPartyPlayerMark.SetPosition(itor->second.fScreenX - (float)m_AtlasPartyPlayerMark.GetWidth() / 2.0f,
                                               itor->second.fScreenY -
                                                   (float)m_AtlasPartyPlayerMark.GetHeight() / 2.0f);
            m_AtlasPartyPlayerMark.SetRotation(itor->second.fRotation);
            m_AtlasPartyPlayerMark.Render();
        }
        m_AtlasPlayerMark.Render();
    }

    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matIdentity);

    {
        TGuildAreaInfoVectorIterator itor = m_GuildAreaInfoVector.begin();
        for (; itor != m_GuildAreaInfoVector.end(); ++itor)
        {
            TGuildAreaInfo &rInfo = *itor;

            m_GuildAreaFlagImageInstance.SetPosition(
                fScreenX + (rInfo.fsxRender + rInfo.fexRender) / 2.0f - m_GuildAreaFlagImageInstance.GetWidth() / 2,
                fScreenY + (rInfo.fsyRender + rInfo.feyRender) / 2.0f - m_GuildAreaFlagImageInstance.GetHeight() / 2);
            m_GuildAreaFlagImageInstance.Render();

            //			CScreen::RenderBar2d(fScreenX+rInfo.fsxRender,
            //								 fScreenY+rInfo.fsyRender,
            //								 fScreenX+rInfo.fexRender,
            //								 fScreenY+rInfo.feyRender);
        }
    }
}

bool CPythonMiniMap::GetPickedInstanceInfo(float fScreenX, float fScreenY, std::string &rReturnName, float *pReturnPosX,
                                           float *pReturnPosY, uint32_t *pdwTextColor)
{
    float fDistanceFromMiniMapCenterX = fScreenX - m_fScreenX - m_fWidth * 0.5f;
    float fDistanceFromMiniMapCenterY = fScreenY - m_fScreenY - m_fHeight * 0.5f;

    if (sqrtf(fDistanceFromMiniMapCenterX * fDistanceFromMiniMapCenterX +
              fDistanceFromMiniMapCenterY * fDistanceFromMiniMapCenterY) > m_fMiniMapRadius)
        return false;

    float fRealX = m_fCenterX + fDistanceFromMiniMapCenterX / m_fScale * ((float)CTerrainImpl::CELLSCALE);
    float fRealY = m_fCenterY + fDistanceFromMiniMapCenterY / m_fScale * ((float)CTerrainImpl::CELLSCALE);

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

    if (pkInst)
    {
        TPixelPosition kInstPos;
        pkInst->NEW_GetPixelPosition(&kInstPos);

        if (fabs(kInstPos.x - fRealX) < ((float)CTerrainImpl::CELLSCALE) * 6.0f / m_fScale &&
            fabs(kInstPos.y - fRealY) < ((float)CTerrainImpl::CELLSCALE) * 6.0f / m_fScale)
        {
            rReturnName = pkInst->GetNameString();
            *pReturnPosX = kInstPos.x;
            *pReturnPosY = kInstPos.y;
            *pdwTextColor = pkInst->GetNameColor().BGRA().c;
            return true;
        }
    }

    if (m_fScale < 1.0f)
        return false;

    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    CPythonCharacterManager::CharacterIterator i;
    for (i = rkChrMgr.CharacterInstanceBegin(); i != rkChrMgr.CharacterInstanceEnd(); ++i)
    {
        CInstanceBase *pkInstEach = *i;
        if (pkInstEach->IsInvisibility())
            continue;
        if (m_fScale < 2.0f && (pkInstEach->IsEnemy() || pkInstEach->IsPC()))
            continue;
        TPixelPosition kInstancePosition;
        pkInstEach->NEW_GetPixelPosition(&kInstancePosition);

        if (fabs(kInstancePosition.x - fRealX) < ((float)CTerrainImpl::CELLSCALE) * 3.0f / m_fScale &&
            fabs(kInstancePosition.y - fRealY) < ((float)CTerrainImpl::CELLSCALE) * 3.0f / m_fScale)
        {
            rReturnName = pkInstEach->GetNameString();
            *pReturnPosX = kInstancePosition.x;
            *pReturnPosY = kInstancePosition.y;
            *pdwTextColor = pkInstEach->GetNameColor().BGRA().c;
            return true;
        }
    }
    return false;
}

bool CPythonMiniMap::GetAtlasInfo(float fScreenX, float fScreenY, std::string &rReturnString, float *pReturnPosX,
                                  float *pReturnPosY, uint32_t *pdwTextColor, uint32_t *pdwGuildID)
{
    float fRealX = (fScreenX - m_fAtlasScreenX) * (m_fAtlasMaxX / m_fAtlasImageSizeX);
    float fRealY = (fScreenY - m_fAtlasScreenY) * (m_fAtlasMaxY / m_fAtlasImageSizeY);

    //((float) CTerrainImpl::CELLSCALE) * 10.0f
    float fCheckWidth = (m_fAtlasMaxX / m_fAtlasImageSizeX) * 5.0f;
    float fCheckHeight = (m_fAtlasMaxY / m_fAtlasImageSizeY) * 5.0f;

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

    if (pkInst)
    {
        TPixelPosition kInstPos;
        pkInst->NEW_GetPixelPosition(&kInstPos);

        if (kInstPos.x - fCheckWidth < fRealX && kInstPos.x + fCheckWidth > fRealX &&
            kInstPos.y - fCheckHeight < fRealY && kInstPos.y + fCheckHeight > fRealY)
        {
            rReturnString = pkInst->GetNameString();
            *pReturnPosX = kInstPos.x;
            *pReturnPosY = kInstPos.y;
            *pdwTextColor = pkInst->GetNameColor().BGRA().c;
            return true;
        }
    }

    if (m_bAtlasRenderShops)
    {
        for (auto &rAtlasMarkInfo : m_AtlasShopInfoVector)
        {
            if (rAtlasMarkInfo.m_fScreenX > 0.0f)
                if (rAtlasMarkInfo.m_fScreenY > 0.0f)
                    if (rAtlasMarkInfo.m_fX - fCheckWidth / 2 < fRealX && rAtlasMarkInfo.m_fX + fCheckWidth > fRealX &&
                        rAtlasMarkInfo.m_fY - fCheckWidth / 2 < fRealY && rAtlasMarkInfo.m_fY + fCheckHeight > fRealY)
                    {
                        rReturnString = rAtlasMarkInfo.m_strText;
                        *pReturnPosX = rAtlasMarkInfo.m_fX;
                        *pReturnPosY = rAtlasMarkInfo.m_fY;
                        *pdwTextColor =
                            CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_OFFLINE_SHOP).BGRA().c;
                        return true;
                    }
        }
    }

    if (m_bAtlasRenderNpc)
    {
        for (auto &rAtlasMarkInfo : m_AtlasNPCInfoVector)
        {
            if (rAtlasMarkInfo.m_fX - fCheckWidth / 2 < fRealX && rAtlasMarkInfo.m_fX + fCheckWidth > fRealX &&
                rAtlasMarkInfo.m_fY - fCheckWidth / 2 < fRealY && rAtlasMarkInfo.m_fY + fCheckHeight > fRealY)
            {
                rReturnString = rAtlasMarkInfo.m_strText;
                *pReturnPosX = rAtlasMarkInfo.m_fX;
                *pReturnPosY = rAtlasMarkInfo.m_fY;
                *pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_NPC).BGRA().c;
                // m_MarkTypeToColorMap[rAtlasMarkInfo.m_byType];
                return true;
            }
        }
    }

    if (m_bAtlasRenderWarp)
    {
        for (auto &rAtlasMarkInfo : m_AtlasWarpInfoVector)
        {
            if (rAtlasMarkInfo.m_fX - fCheckWidth / 2 < fRealX && rAtlasMarkInfo.m_fX + fCheckWidth > fRealX &&
                rAtlasMarkInfo.m_fY - fCheckWidth / 2 < fRealY && rAtlasMarkInfo.m_fY + fCheckHeight > fRealY)
            {
                rReturnString = rAtlasMarkInfo.m_strText;
                *pReturnPosX = rAtlasMarkInfo.m_fX;
                *pReturnPosY = rAtlasMarkInfo.m_fY;
                *pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WARP).BGRA().c;
                // m_MarkTypeToColorMap[rAtlasMarkInfo.m_byType];
                return true;
            }
        }
    }

    if (m_bAtlasRenderWaypoint)
    {
        for (auto &rAtlasMarkInfo : m_AtlasWayPointInfoVector)
        {
            if (rAtlasMarkInfo.m_fScreenX > 0.0f)
                if (rAtlasMarkInfo.m_fScreenY > 0.0f)
                    if (rAtlasMarkInfo.m_fX - fCheckWidth / 2 < fRealX && rAtlasMarkInfo.m_fX + fCheckWidth > fRealX &&
                        rAtlasMarkInfo.m_fY - fCheckWidth / 2 < fRealY && rAtlasMarkInfo.m_fY + fCheckHeight > fRealY)
                    {
                        rReturnString = rAtlasMarkInfo.m_strText;
                        *pReturnPosX = rAtlasMarkInfo.m_fX;
                        *pReturnPosY = rAtlasMarkInfo.m_fY;
                        *pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WAYPOINT).BGRA().c;
                        // m_MarkTypeToColorMap[rAtlasMarkInfo.m_byType];
                        return true;
                    }
        }
    }

    for (auto &rInfo : m_GuildAreaInfoVector)
    {
        if (fScreenX - m_fAtlasScreenX >= rInfo.fsxRender)
            if (fScreenY - m_fAtlasScreenY >= rInfo.fsyRender)
                if (fScreenX - m_fAtlasScreenX <= rInfo.fexRender)
                    if (fScreenY - m_fAtlasScreenY <= rInfo.feyRender)
                    {

                        rReturnString =
                            CPythonGuild::Instance().GetGuildName(rInfo.dwGuildID).value_or("empty_guild_area");

                        *pdwGuildID = rInfo.dwGuildID;

                        *pReturnPosX = rInfo.lx + rInfo.lwidth / 2;
                        *pReturnPosY = rInfo.ly + rInfo.lheight / 2;
                        *pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_PARTY).BGRA().c;
                        return true;
                    }
    }

    return false;
}

bool CPythonMiniMap::GetAtlasSize(float *pfSizeX, float *pfSizeY)
{
    CPythonBackground &rkBG = CPythonBackground::Instance();
    if (!rkBG.IsMapOutdoor())
        return false;

    *pfSizeX = m_fAtlasImageSizeX;
    *pfSizeY = m_fAtlasImageSizeY;

    return true;
}

// Atlas
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// WayPoint
void CPythonMiniMap::AddWayPoint(uint8_t byType, uint32_t dwID, float fX, float fY, std::string strText,
                                 uint32_t dwChrVID)
{
    m_AtlasMarkInfoListIterator = m_AtlasWayPointInfoVector.begin();
    while (m_AtlasMarkInfoListIterator != m_AtlasWayPointInfoVector.end())
    {
        TAtlasMarkInfo &rAtlasMarkInfo = *m_AtlasMarkInfoListIterator;
        if (rAtlasMarkInfo.m_dwID == dwID)
            return;
        ++m_AtlasMarkInfoListIterator;
    }

    TAtlasMarkInfo aAtlasMarkInfo;
    aAtlasMarkInfo.m_byType = byType;
    aAtlasMarkInfo.m_dwID = dwID;
    aAtlasMarkInfo.m_fX = fX;
    aAtlasMarkInfo.m_fY = fY;
    aAtlasMarkInfo.m_fScreenX = 0.0f;
    aAtlasMarkInfo.m_fScreenY = 0.0f;
    aAtlasMarkInfo.m_fMiniMapX = 0.0f;
    aAtlasMarkInfo.m_fMiniMapY = 0.0f;
    aAtlasMarkInfo.m_strText = std::move(strText);
    aAtlasMarkInfo.m_dwChrVID = dwChrVID;
    __UpdateWayPoint(&aAtlasMarkInfo, fX, fY);
    m_AtlasWayPointInfoVector.push_back(aAtlasMarkInfo);
}

void CPythonMiniMap::RemoveWayPoint(uint32_t dwID)
{
    m_AtlasMarkInfoListIterator = m_AtlasWayPointInfoVector.begin();
    while (m_AtlasMarkInfoListIterator != m_AtlasWayPointInfoVector.end())
    {
        TAtlasMarkInfo &rAtlasMarkInfo = *m_AtlasMarkInfoListIterator;
        if (rAtlasMarkInfo.m_dwID == dwID)
        {
            m_AtlasMarkInfoListIterator = m_AtlasWayPointInfoVector.erase(m_AtlasMarkInfoListIterator);
            return;
        }
        ++m_AtlasMarkInfoListIterator;
    }
}

bool CPythonMiniMap::__GetWayPoint(uint32_t dwID, TAtlasMarkInfo **ppkInfo)
{
    TAtlasMarkInfoVectorIterator itor = m_AtlasWayPointInfoVector.begin();
    for (; itor != m_AtlasWayPointInfoVector.end(); ++itor)
    {
        TAtlasMarkInfo &rInfo = *itor;
        if (dwID == rInfo.m_dwID)
        {
            *ppkInfo = &rInfo;
            return true;
        }
    }

    return false;
}

void CPythonMiniMap::__UpdateWayPoint(TAtlasMarkInfo *pkInfo, int ix, int iy)
{
    pkInfo->m_fX = float(ix);
    pkInfo->m_fY = float(iy);
    pkInfo->m_fScreenX = pkInfo->m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX;
    pkInfo->m_fScreenY = pkInfo->m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY;
}

// WayPoint
//////////////////////////////////////////////////////////////////////////

void CPythonMiniMap::__RenderWayPointMark(int ixCenter, int iyCenter)
{
    int iNum = (ELTimer_GetMSec() / 67) % WAYPOINT_IMAGE_COUNT;

    CGraphicImageInstance &rInstance = m_WayPointGraphicImageInstances[iNum];
    rInstance.SetPosition(ixCenter - rInstance.GetWidth() / 2, iyCenter - rInstance.GetHeight() / 2);
    rInstance.Render();
}

void CPythonMiniMap::__RenderMiniWayPointMark(int ixCenter, int iyCenter)
{
    int iNum = (ELTimer_GetMSec() / 67) % MINI_WAYPOINT_IMAGE_COUNT;

    CGraphicImageInstance &rInstance = m_MiniWayPointGraphicImageInstances[iNum];
    rInstance.SetPosition(ixCenter - rInstance.GetWidth() / 2, iyCenter - rInstance.GetHeight() / 2);
    rInstance.Render();
}

void CPythonMiniMap::__RenderTargetMark(int ixCenter, int iyCenter)
{
    int iNum = (ELTimer_GetMSec() / 80) % TARGET_MARK_IMAGE_COUNT;

    CGraphicImageInstance &rInstance = m_TargetMarkGraphicImageInstances[iNum];
    rInstance.SetPosition(ixCenter - rInstance.GetWidth() / 2, iyCenter - rInstance.GetHeight() / 2);
    rInstance.Render();
}

void CPythonMiniMap::AddSignalPoint(float fX, float fY)
{
    static unsigned int g_id = 255;

    TSignalPoint sp;
    sp.id = g_id;
    sp.v2Pos.x = fX;
    sp.v2Pos.y = fY;

    m_SignalPointVector.push_back(sp);

    AddWayPoint(TYPE_WAYPOINT, g_id, fX, fY, "");

    g_id++;
}

void CPythonMiniMap::ClearAllSignalPoint()
{
    std::vector<TSignalPoint>::iterator it;
    for (it = m_SignalPointVector.begin(); it != m_SignalPointVector.end(); ++it)
    {
        RemoveWayPoint(it->id);
    }
    m_SignalPointVector.clear();
}

void CPythonMiniMap::RegisterAtlasWindow(PyObject *poHandler) { m_poHandler = poHandler; }

void CPythonMiniMap::UnregisterAtlasWindow() { m_poHandler = 0; }

void CPythonMiniMap::OpenAtlasWindow()
{
    if (m_poHandler)
    {
        PyCallClassMemberFunc(m_poHandler, "Show", Py_BuildValue("()"));
    }
}

void CPythonMiniMap::SetAtlasCenterPosition(int x, int y)
{
    if (m_poHandler)
        PyCallClassMemberFunc(m_poHandler, "SetCenterPositionAdjust", Py_BuildValue("(ii)", x, y));
}

bool CPythonMiniMap::IsAtlas() { return m_bAtlas; }

void CPythonMiniMap::ShowAtlas() { m_bShowAtlas = true; }

void CPythonMiniMap::HideAtlas() { m_bShowAtlas = false; }

bool CPythonMiniMap::CanShowAtlas() { return m_bShowAtlas; }

bool CPythonMiniMap::CanShow() { return m_bShow; }

void CPythonMiniMap::Show() { m_bShow = true; }

void CPythonMiniMap::Hide() { m_bShow = false; }

bool CPythonMiniMap::ToggleAtlasMarker(int type)
{
    switch (type)
    {
    case CPythonMiniMap::TYPE_NPC:
        return m_bAtlasRenderNpc = !m_bAtlasRenderNpc;
    case CPythonMiniMap::TYPE_SHOP:
        return m_bAtlasRenderShops = !m_bAtlasRenderShops;
    case CPythonMiniMap::TYPE_WAYPOINT:
        return m_bAtlasRenderWaypoint = !m_bAtlasRenderWaypoint;
    case CPythonMiniMap::TYPE_WARP:
        return m_bAtlasRenderWarp = !m_bAtlasRenderWarp;
    }

    return false;
}

#ifdef ENABLE_ADMIN_MANAGER
bool CPythonMiniMap::LoadAdminManagerAtlas()
{
    const char *playerMarkFileName = "d:/ymir work/ui/minimap/playermark.sub";
    const char *otherPlayerMarkFileName = "d:/ymir work/ui/minimap/whitecirclemark.sub";

    const std::string &stBaseMapName = CPythonAdmin::Instance().MapViewer_GetMapName();
    std::string stMapName;

    if (stBaseMapName.find('/') != std::string::npos)
        stMapName = (stBaseMapName.c_str() + stBaseMapName.find_last_of('/') + 1);
    else
        stMapName = stBaseMapName;

    char atlasFileName[1024 + 1];
    snprintf(atlasFileName, sizeof(atlasFileName), "%s/atlas.sub", stMapName.c_str());
#ifdef __ENABLE_FOX_FS__
    if (!CFileSystem::Instance().isExist(atlasFileName, __FUNCTION__, false))
#else
    if (!GetVfs().Exists(atlasFileName))
#endif
    {
        snprintf(atlasFileName, sizeof(atlasFileName), "d:/ymir work/ui/atlas/%s/atlas.sub", stMapName.c_str());
    }

    m_AdminManagerAtlasImageInstance.Destroy();
    m_AdminManagerAtlasPlayerMark.Destroy();
    m_AdminManagerAtlasOtherPlayerMark.Destroy();
    auto pkGrpImgAtlas = CResourceManager::Instance().LoadResource<CGraphicImage>(atlasFileName);
    if (pkGrpImgAtlas)
    {
        m_AdminManagerAtlasImageInstance.SetImagePointer(pkGrpImgAtlas);

        if (pkGrpImgAtlas)
            m_bAdminManagerAtlas = true;
        else
            m_bAdminManagerAtlas = false;
    }
    else
    {
        SPDLOG_ERROR("cannot load atlas file %s", atlasFileName);
        m_bAdminManagerAtlas = false;
    }

    m_AdminManagerAtlasPlayerMark.SetImagePointer(
        CResourceManager::Instance().LoadResource<CGraphicImage>(playerMarkFileName));
    m_AdminManagerAtlasOtherPlayerMark.SetImagePointer(
        CResourceManager::Instance().LoadResource<CGraphicImage>(otherPlayerMarkFileName));

    m_fAdminManagerAtlasMaxX = (float)CPythonAdmin::Instance().MapViewer_GetSizeX() * CTerrainImpl::TERRAIN_XSIZE;
    m_fAdminManagerAtlasMaxY = (float)CPythonAdmin::Instance().MapViewer_GetSizeY() * CTerrainImpl::TERRAIN_YSIZE;

    m_fAdminManagerAtlasImageSizeX = (float)m_AdminManagerAtlasImageInstance.GetWidth();
    m_fAdminManagerAtlasImageSizeY = (float)m_AdminManagerAtlasImageInstance.GetHeight();
    m_fAdminManagerAtlasImageSizeScale = 1.0f;

    return true;
}

void CPythonMiniMap::ConvertGlobalPositionToAdminManagerAtlasData(long lX, long lY, float &fRetX, float &fRetY)
{
    ConvertPositionToAdminManagerAtlasData(lX, lY, fRetX, fRetY);
}

void CPythonMiniMap::ConvertPositionToAdminManagerAtlasData(long lX, long lY, float &fRetX, float &fRetY)
{
    float &fScale = m_fAdminManagerAtlasImageSizeScale;

    if (IsAdminManagerAtlasRotated())
    {
        float fXDif = m_matWorldAdminManagerAtlas._41 - m_fAdminManagerAtlasScreenX;
        float fYDif = m_matWorldAdminManagerAtlas._42 - m_fAdminManagerAtlasScreenY;

        fRetX = m_fAdminManagerAtlasImageSizeY * fScale -
                ((float)lY / m_fAdminManagerAtlasMaxY * m_fAdminManagerAtlasImageSizeY * fScale) - fXDif;
        fRetY = (float)lX / m_fAdminManagerAtlasMaxX * m_fAdminManagerAtlasImageSizeX * fScale - fYDif;
    }
    else
    {
        fRetX = (float)lX / m_fAdminManagerAtlasMaxX * m_fAdminManagerAtlasImageSizeX * fScale;
        fRetY = (float)lY / m_fAdminManagerAtlasMaxY * m_fAdminManagerAtlasImageSizeY * fScale;
    }
}

void CPythonMiniMap::ConvertAdminManagerAtlasDataToPosition(float fX, float fY, float &fRetX, float &fRetY)
{
    float &fScale = m_fAdminManagerAtlasImageSizeScale;

    if (IsAdminManagerAtlasRotated())
    {
        float fXDif = m_matWorldAdminManagerAtlas._41 - m_fAdminManagerAtlasScreenX;
        float fYDif = m_matWorldAdminManagerAtlas._42 - m_fAdminManagerAtlasScreenY;

        fRetY = (m_fAdminManagerAtlasImageSizeY * fScale - fX - fXDif) * m_fAdminManagerAtlasMaxY /
                m_fAdminManagerAtlasMaxY / fScale;
        fRetX = (fY + fYDif) * m_fAdminManagerAtlasMaxX / m_fAdminManagerAtlasImageSizeX / fScale;
    }
    else
    {
        fRetX = fX * m_fAdminManagerAtlasMaxX / m_fAdminManagerAtlasImageSizeX / fScale;
        fRetY = fY * m_fAdminManagerAtlasMaxY / m_fAdminManagerAtlasImageSizeY / fScale;
    }
}

void CPythonMiniMap::UpdateAdminManagerAtlas()
{
    // player mark
    if (IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_OBSERVING_PLAYER))
    {
        /*
        long lX, lY;
        float fRot;
        CPythonPlayer::Instance().GetPlayerAdminManagerPosition(lX, lY, fRot);

        if (IsAdminManagerAtlasRotated())
        fRot += 90.0f;

        float fRotation;
        fRotation = (540.0f - fRot);
        while (fRotation > 360.0f)
        fRotation -= 360.0f;
        while (fRotation < 0.0f)
        fRotation += 360.0f;

        float fMapX, fMapY;
        ConvertPositionToAdminManagerAtlasData(lX, lY, fMapX, fMapY);

        m_AdminManagerAtlasPlayerMark.SetPosition(fMapX - (float)m_AdminManagerAtlasPlayerMark.GetWidth() / 2.0f,
        fMapY - (float)m_AdminManagerAtlasPlayerMark.GetHeight() / 2.0f);
        m_AdminManagerAtlasPlayerMark.SetRotation(fRotation);
        */
    }
}

void CPythonMiniMap::RenderAdminManagerAtlas(float fScreenX, float fScreenY, float fMaxWidth, float fMaxHeight)
{

    if (!m_bShowAdminManagerAtlas)
        return;

    if (!m_bAdminManagerAtlas)
    {
        SPDLOG_ERROR("Cannot render AdminManager atlas");
        return;
    }

    float *pfSizeX, *pfSizeY;
    if (IsAdminManagerAtlasRotated())
    {
        pfSizeX = &m_fAdminManagerAtlasImageSizeY;
        pfSizeY = &m_fAdminManagerAtlasImageSizeX;
    }
    else
    {
        pfSizeX = &m_fAdminManagerAtlasImageSizeX;
        pfSizeY = &m_fAdminManagerAtlasImageSizeY;
    }

    float fScale = m_fAdminManagerAtlasImageSizeScale;
    if (*pfSizeX * fScale > fMaxWidth)
        fScale = fMaxWidth / *pfSizeX;
    if (*pfSizeY * fScale > fMaxHeight)
        fScale = fMaxHeight / *pfSizeY;
    if (fScale != m_fAdminManagerAtlasImageSizeScale)
    {
        m_fAdminManagerAtlasImageSizeScale = fScale;
        m_AdminManagerAtlasImageInstance.SetScale(fScale, fScale);
    }

    m_AdminManagerAtlasImageInstance.SetScale(m_fAdminManagerAtlasImageSizeScale, m_fAdminManagerAtlasImageSizeScale);

    if (m_fAdminManagerAtlasScreenX != fScreenX || m_fAdminManagerAtlasScreenY != fScreenY)
    {
        if (IsAdminManagerAtlasRotated())
        {
            m_matWorldAdminManagerAtlas._41 =
                fScreenX + m_fAdminManagerAtlasImageSizeY / 2 - m_fAdminManagerAtlasImageSizeX / 2;
            m_matWorldAdminManagerAtlas._42 =
                fScreenY + m_fAdminManagerAtlasImageSizeX / 2 - m_fAdminManagerAtlasImageSizeY / 2;
        }
        else
        {
            m_matWorldAdminManagerAtlas._41 = fScreenX;
            m_matWorldAdminManagerAtlas._42 = fScreenY;
        }
        m_fAdminManagerAtlasScreenX = fScreenX;
        m_fAdminManagerAtlasScreenY = fScreenY;
    }

    // main map
    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldAdminManagerAtlas);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

    if (IsAdminManagerAtlasRotated())
        m_AdminManagerAtlasImageInstance.SetRotation(90);
    m_AdminManagerAtlasImageInstance.Render();

    STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

    // mobs if shown
    if (IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_MOB | ADMIN_MANAGER_SHOW_STONE | ADMIN_MANAGER_SHOW_NPC))
    {
        const auto &r_monsterMap = CPythonAdmin::Instance().MapViewer_GetMonster();
        for (auto it = r_monsterMap.begin(); it != r_monsterMap.end(); ++it)
        {
            const auto &rMonsterInfo = it->second;
            const TMobTable *pProto = NpcManager::instance().GetTable(rMonsterInfo.dwRaceNum);
            UINT eNameColor = CInstanceBase::NAMECOLOR_DEAD;

            if (rMonsterInfo.is_alive && pProto)
            {
                switch (pProto->bType)
                {
                case CActorInstance::TYPE_ENEMY:
                    if (!IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_MOB))
                        continue;
                    eNameColor = CInstanceBase::NAMECOLOR_MOB;
                    break;

                case CActorInstance::TYPE_STONE:
                    if (!IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_STONE))
                        continue;
                    eNameColor = CInstanceBase::NAMECOLOR_STONE;
                    break;

                default:
                    if (!IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_NPC))
                        continue;
                    eNameColor = CInstanceBase::NAMECOLOR_NPC;
                    break;
                }
            }

            STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor(eNameColor));

            // draw
            float fX, fY;
            ConvertPositionToAdminManagerAtlasData(rMonsterInfo.x, rMonsterInfo.y, fX, fY);

            m_WhiteMark.SetPosition(fX - (float)m_WhiteMark.GetWidth() / 2.0f,
                                    fY - (float)m_WhiteMark.GetHeight() / 2.0f);
            m_WhiteMark.Render();
        }
    }

    // other pcs if shown
    if (IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_PC))
    {
        const auto &r_playerMap = CPythonAdmin::Instance().MapViewer_GetPlayer();
        for (auto it = r_playerMap.begin(); it != r_playerMap.end(); ++it)
        {
            const auto &rPlayerInfo = it->second;
            STATEMANAGER.SetRenderState(
                D3DRS_TEXTUREFACTOR,
                CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_EMPIRE_PC + rPlayerInfo.bEmpire));

            float fX, fY;
            ConvertPositionToAdminManagerAtlasData(rPlayerInfo.x, rPlayerInfo.y, fX, fY);

            m_AdminManagerAtlasOtherPlayerMark.SetPosition(
                fX - (float)m_AdminManagerAtlasOtherPlayerMark.GetWidth() / 2.0f,
                fY - (float)m_AdminManagerAtlasOtherPlayerMark.GetHeight() / 2.0f);
            m_AdminManagerAtlasOtherPlayerMark.Render();
        }
    }

    STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

    // observing player
    if (IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_OBSERVING_PLAYER) && (ELTimer_GetMSec() / 500) % 2)
        m_AdminManagerAtlasPlayerMark.Render();

    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matIdentity);
}

bool CPythonMiniMap::IsAdminManagerAtlasRotated() const { return false; }

bool CPythonMiniMap::GetAdminManagerAtlasInfo(float fScreenX, float fScreenY, std::string &rReturnString,
                                              float *pReturnPosX, float *pReturnPosY, uint32_t *pdwTextColor,
                                              uint32_t *pdwStoneDropVnum)
{
    // rotation dif
    float fBaseXDif = m_matWorldAdminManagerAtlas._41 - m_fAdminManagerAtlasScreenX;
    float fBaseYDif = m_matWorldAdminManagerAtlas._42 - m_fAdminManagerAtlasScreenY;
    // draw XY
    float fRealX = fScreenX - m_fAdminManagerAtlasScreenX - fBaseXDif;
    float fRealY = fScreenY - m_fAdminManagerAtlasScreenY - fBaseYDif;

    float fCheckWidth, fCheckHeight;
    float fMapX, fMapY;

    // check player mark
    if (IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_OBSERVING_PLAYER))
    {
        /*
        long lX, lY;
        float fRot;

        fCheckWidth = m_AdminManagerAtlasPlayerMark.GetWidth();
        fCheckHeight = m_AdminManagerAtlasPlayerMark.GetHeight();

        CPythonPlayer::Instance().GetPlayerAdminManagerPosition(lX, lY, fRot);
        ConvertPositionToAdminManagerAtlasData(lX, lY, fMapX, fMapY);

        if (fMapX - fCheckWidth / 2 <= fRealX && fMapX + fCheckWidth / 2 >= fRealX &&
        fMapY - fCheckHeight / 2 <= fRealY && fMapY + fCheckHeight / 2 >= fRealY)
        {
        rReturnString = CPythonPlayer::Instance().GetPlayerAdminManagerInfo()->name;
        *pReturnPosX = lX;
        *pReturnPosY = lY;
        return true;
        }
        */
    }

    // check other player if enabled
    if (IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_PC))
    {
        fCheckWidth = m_AdminManagerAtlasOtherPlayerMark.GetWidth();
        fCheckHeight = m_AdminManagerAtlasOtherPlayerMark.GetHeight();

        const auto &r_playerMap = CPythonAdmin::Instance().MapViewer_GetPlayer();
        for (auto it = r_playerMap.begin(); it != r_playerMap.end(); ++it)
        {
            const auto &rPlayerInfo = it->second;
            ConvertPositionToAdminManagerAtlasData(rPlayerInfo.x, rPlayerInfo.y, fMapX, fMapY);

            if (fMapX - fCheckWidth / 2 <= fRealX && fMapX + fCheckWidth / 2 >= fRealX &&
                fMapY - fCheckHeight / 2 <= fRealY && fMapY + fCheckHeight / 2 >= fRealY)
            {
                rReturnString = rPlayerInfo.szName;
                *pReturnPosX = rPlayerInfo.x;
                *pReturnPosY = rPlayerInfo.y;
                *pdwTextColor =
                    CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_EMPIRE_PC + rPlayerInfo.bEmpire);
                *pdwStoneDropVnum = 0;
                return true;
            }
        }
    }

    // check mob if enabled
    if (IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_MOB | ADMIN_MANAGER_SHOW_STONE | ADMIN_MANAGER_SHOW_NPC))
    {
        fCheckWidth = m_WhiteMark.GetWidth();
        fCheckHeight = m_WhiteMark.GetHeight();

        const auto &r_monsterMap = CPythonAdmin::Instance().MapViewer_GetMonster();
        if (r_monsterMap.size() > 0)
        {
            auto it = r_monsterMap.end();
            do
            {
                const auto &rMonsterInfo = (--it)->second;
                const TMobTable *pProto = NpcManager::Instance().GetTable(rMonsterInfo.dwRaceNum);
                if (pProto)
                {
                    switch (pProto->bType)
                    {
                    case CActorInstance::TYPE_ENEMY:
                        if (!IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_MOB))
                            continue;
                        *pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_MOB);
                        *pdwStoneDropVnum = 0;
                        break;

                    case CActorInstance::TYPE_STONE:
                        if (!IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_STONE))
                            continue;
                        *pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_STONE);
                        *pdwStoneDropVnum = rMonsterInfo.stone_drop_vnum;
                        break;

                    default:
                        if (!IsAdminManagerFlagShown(ADMIN_MANAGER_SHOW_NPC))
                            continue;
                        *pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_NPC);
                        *pdwStoneDropVnum = 0;
                        break;
                    }
                }

                ConvertPositionToAdminManagerAtlasData(rMonsterInfo.x, rMonsterInfo.y, fMapX, fMapY);

                if (fMapX - fCheckWidth / 2 <= fRealX && fMapX + fCheckWidth / 2 >= fRealX &&
                    fMapY - fCheckHeight / 2 <= fRealY && fMapY + fCheckHeight / 2 >= fRealY)
                {
                    rReturnString = NpcManager::Instance().GetMonsterName(rMonsterInfo.dwRaceNum);
                    *pReturnPosX = rMonsterInfo.x;
                    *pReturnPosY = rMonsterInfo.y;
                    return true;
                }
            } while (it != r_monsterMap.begin());
        }
    }

    return false;
}

bool CPythonMiniMap::GetAdminManagerAtlasInfoNew(float fScreenX, float fScreenY, float *fReturnPosX, float *fReturnPosY)
{
    // rotation dif
    float fBaseXDif = m_matWorldAdminManagerAtlas._41 - m_fAdminManagerAtlasScreenX;
    float fBaseYDif = m_matWorldAdminManagerAtlas._42 - m_fAdminManagerAtlasScreenY;
    // draw XY
    *fReturnPosX = fScreenX - m_fAdminManagerAtlasScreenX - fBaseXDif;
    *fReturnPosY = fScreenY - m_fAdminManagerAtlasScreenY - fBaseYDif;
    ConvertAdminManagerAtlasDataToPosition(*fReturnPosX, *fReturnPosY, *fReturnPosX, *fReturnPosY);

    if (*fReturnPosX > 0 && *fReturnPosX <= m_fAdminManagerAtlasMaxX && *fReturnPosY > 0 &&
        *fReturnPosY <= m_fAdminManagerAtlasMaxY)
        return true;

    return false;
}

void CPythonMiniMap::ShowAdminManagerAtlas() { m_bShowAdminManagerAtlas = true; }

void CPythonMiniMap::HideAdminManagerAtlas() { m_bShowAdminManagerAtlas = false; }

bool CPythonMiniMap::CanShowAdminManagerAtlas() const { return m_bShowAdminManagerAtlas; }

void CPythonMiniMap::ShowAdminManagerFlag(uint32_t dwFlag) { SET_BIT(m_dwShowAdminManagerFlag, dwFlag); }

void CPythonMiniMap::HideAdminManagerFlag(uint32_t dwFlag) { REMOVE_BIT(m_dwShowAdminManagerFlag, dwFlag); }

bool CPythonMiniMap::IsAdminManagerFlagShown(uint32_t dwFlag) const { return IS_SET(m_dwShowAdminManagerFlag, dwFlag); }
#endif

void CPythonMiniMap::__Initialize()
{
    m_poHandler = 0;

    SetMiniMapSize(128.0f, 128.0f);

    m_fScale = 2.0f;

    m_fCenterX = m_fWidth * 0.5f;
    m_fCenterY = m_fHeight * 0.5f;

    m_fScreenX = 0.0f;
    m_fScreenY = 0.0f;

    m_fAtlasScreenX = 0.0f;
    m_fAtlasScreenY = 0.0f;

    m_fAtlasMaxX = 0.0f;
    m_fAtlasMaxY = 0.0f;
#ifdef ENABLE_ADMIN_MANAGER
    m_fAdminManagerAtlasMaxX = 0.0f;
    m_fAdminManagerAtlasMaxY = 0.0f;
#endif
    m_fAtlasImageSizeX = 0.0f;
    m_fAtlasImageSizeY = 0.0f;
#ifdef ENABLE_ADMIN_MANAGER
    m_fAdminManagerAtlasImageSizeX = 0.0f;
    m_fAdminManagerAtlasImageSizeY = 0.0f;
    m_fAdminManagerAtlasImageSizeScale = 1.0f;
#endif
    m_bAtlas = false;
#ifdef ENABLE_ADMIN_MANAGER
    m_bAdminManagerAtlas = false;
#endif
    m_bShow = false;
    m_bShowAtlas = false;
#ifdef ENABLE_ADMIN_MANAGER
    m_bShowAdminManagerAtlas = false;
    m_dwShowAdminManagerFlag =
        ADMIN_MANAGER_SHOW_PC | ADMIN_MANAGER_SHOW_MOB | ADMIN_MANAGER_SHOW_STONE | ADMIN_MANAGER_SHOW_NPC;
#endif
    for (uint32_t i = 0; i < AROUND_AREA_NUM; ++i)
        m_lpMiniMapTexture[i] = nullptr;

    m_bAtlasRenderNpc = true;
    m_bAtlasRenderShops = true;
    m_bAtlasRenderWarp = true;
    m_bAtlasRenderWaypoint = true;

    m_matIdentity = Matrix::Identity;
    m_matWorld = Matrix::Identity;
    m_matMiniMapCover = Matrix::Identity;
    m_matWorldAtlas = Matrix::Identity;

    m_VertexBuffer = ENGINE_INVALID_HANDLE;
    m_IndexBuffer = ENGINE_INVALID_HANDLE;

#ifdef ENABLE_ADMIN_MANAGER
    DirectX::SimpleMath::MatrixIdentity(&m_matWorldAdminManagerAtlas);
#endif
}

void CPythonMiniMap::Destroy()
{
    ClearAllSignalPoint();
    m_poHandler = 0;

    Engine::GetDevice().DeleteVertexBuffer(m_VertexBuffer);
    Engine::GetDevice().DeleteIndexBuffer(m_IndexBuffer);
    m_VertexBuffer = ENGINE_INVALID_HANDLE;
    m_IndexBuffer = ENGINE_INVALID_HANDLE;

    m_PlayerMark.Destroy();

    m_MiniMapFilterGraphicImageInstance.Destroy();
    m_MiniMapCameraraphicImageInstance.Destroy();

    m_AtlasWayPointInfoVector.clear();
    m_AtlasImageInstance.Destroy();
    m_AtlasPlayerMark.Destroy();
    m_WhiteMark.Destroy();
#ifdef ENABLE_ADMIN_MANAGER
    m_AdminManagerAtlasImageInstance.Destroy();
    m_AdminManagerAtlasPlayerMark.Destroy();
    m_AdminManagerAtlasOtherPlayerMark.Destroy();
#endif
    for (auto &m_MiniWayPointGraphicImageInstance : m_MiniWayPointGraphicImageInstances)
        m_MiniWayPointGraphicImageInstance.Destroy();
    for (auto &m_WayPointGraphicImageInstance : m_WayPointGraphicImageInstances)
        m_WayPointGraphicImageInstance.Destroy();
    for (auto &m_TargetMarkGraphicImageInstance : m_TargetMarkGraphicImageInstances)
        m_TargetMarkGraphicImageInstance.Destroy();

    m_GuildAreaFlagImageInstance.Destroy();

    __Initialize();
}

CPythonMiniMap::CPythonMiniMap() : m_lpMiniMapTexture{} { __Initialize(); }

CPythonMiniMap::~CPythonMiniMap() { Destroy(); }
