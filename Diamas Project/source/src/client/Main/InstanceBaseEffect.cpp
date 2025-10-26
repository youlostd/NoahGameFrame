#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonTextTail.h"
#include "PythonPlayer.h"
#include "../EterLib/Engine.hpp"
#include "PythonApplication.h"

#include "../EffectLib/EffectManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/CharacterEffectRegistry.hpp"
#include <game/AffectsHolderUtil.hpp>
#include <game/AffectConstants.hpp>

float CInstanceBase::ms_fDustGap;
float CInstanceBase::ms_fHorseDustGap;

#define BYTE_COLOR_TO_D3DX_COLOR(r, g, b) DirectX::SimpleMath::Color(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f, 1.0f)

/*
Color CInstanceBase::ms_kD3DXClrPC(0xFFFFD84D);//1.0f, 0.8470f, 0.3f, 1.0f
Color CInstanceBase::ms_kD3DXClrNPC(0xFF7BE85E);//0.4823f, 0.9098f, 0.3686f, 1.0f
Color CInstanceBase::ms_kD3DXClrMOB(0xFFEC170a);//0.9254f, 0.0901f, 0.0392f, 1.0f
Color CInstanceBase::ms_kD3DXClrPVP(0xFF8532D9);
Color CInstanceBase::ms_kD3DXClrPVPSelf(0xFFEE36DF);
Color CInstanceBase::ms_kD3DXClrKiller = BYTE_COLOR_TO_D3DX_COLOR(180, 100, 0);
Color CInstanceBase::ms_kD3DXClrTitle[CInstanceBase::TITLE_MAX_NUM] =
{
	BYTE_COLOR_TO_D3DX_COLOR(  0, 204, 255),
	BYTE_COLOR_TO_D3DX_COLOR(  0, 144, 255),
	BYTE_COLOR_TO_D3DX_COLOR( 92, 110, 255),
	BYTE_COLOR_TO_D3DX_COLOR(155, 155, 255),
	0xFFFFFFFF, // None
	BYTE_COLOR_TO_D3DX_COLOR(207, 117,   0),
	BYTE_COLOR_TO_D3DX_COLOR(235,  83,   0),
	BYTE_COLOR_TO_D3DX_COLOR(227,   0,   0),
	BYTE_COLOR_TO_D3DX_COLOR(255,   0,   0),
};
*/

DirectX::SimpleMath::Color g_akD3DXClrTitle[TITLE_NUM];
DirectX::SimpleMath::Color g_akD3DXClrName[CInstanceBase::NAMECOLOR_NUM];

std::map<int, std::string> g_TitleNameMap;
std::map<int, std::string> g_TitleNameMapFemale;

std::map<int, std::string> g_NamePrefixMap;

std::set<uint32_t> g_kSet_dwPVPReadyKey;
std::set<uint32_t> g_kSet_dwPVPKey;
std::set<uint32_t> g_kSet_dwGVGKey;
std::set<uint32_t> g_kSet_dwDUELKey;

bool g_isEmpireNameMode = false;

void CInstanceBase::SetEmpireNameMode(bool isEnable)
{
    g_isEmpireNameMode = isEnable;

    if (isEnable)
    {
        g_akD3DXClrName[NAMECOLOR_MOB] = g_akD3DXClrName[NAMECOLOR_EMPIRE_MOB];
        g_akD3DXClrName[NAMECOLOR_NPC] = g_akD3DXClrName[NAMECOLOR_EMPIRE_NPC];
        g_akD3DXClrName[NAMECOLOR_PC] = g_akD3DXClrName[NAMECOLOR_NORMAL_PC];

        for (UINT uEmpire = 1; uEmpire < EMPIRE_MAX_NUM; ++uEmpire)
            g_akD3DXClrName[NAMECOLOR_PC + uEmpire] = g_akD3DXClrName[NAMECOLOR_EMPIRE_PC + uEmpire];
    }
    else
    {
        g_akD3DXClrName[NAMECOLOR_MOB] = g_akD3DXClrName[NAMECOLOR_NORMAL_MOB];
        g_akD3DXClrName[NAMECOLOR_NPC] = g_akD3DXClrName[NAMECOLOR_NORMAL_NPC];

        for (UINT uEmpire = 0; uEmpire < EMPIRE_MAX_NUM; ++uEmpire)
            g_akD3DXClrName[NAMECOLOR_PC + uEmpire] = g_akD3DXClrName[NAMECOLOR_NORMAL_PC];
    }
}

const DirectX::SimpleMath::Color &CInstanceBase::GetIndexedNameColor(UINT eNameColor)
{
    if (eNameColor == NAMECOLOR_OFFLINE_SHOP)
    {
        static DirectX::SimpleMath::Color shopColor(D3DCOLOR_XRGB(255, 102, 0));
        return shopColor;
    }

    if (eNameColor >= NAMECOLOR_NUM)
    {
        static DirectX::SimpleMath::Color s_kD3DXClrNameDefault(0xffffffff);
        return s_kD3DXClrNameDefault;
    }

    return g_akD3DXClrName[eNameColor];
}

void CInstanceBase::AddDamageEffect(uint32_t damage, uint8_t flag, bool bSelf, bool bTarget)
{
    if (Engine::GetSettings().IsShowDamage())
    {
        SEffectDamage sDamage{};
        sDamage.bSelf = bSelf;
        sDamage.bTarget = bTarget;
        sDamage.damage = damage;
        sDamage.flag = flag;
        m_DamageQueue.push_back(sDamage);
    }
}

void CInstanceBase::ProcessDamage()
{
    if (m_DamageQueue.empty())
        return;

    const SEffectDamage sDamage = m_DamageQueue.front();

    m_DamageQueue.pop_front();

    uint32_t damage = sDamage.damage;
    uint8_t flag = sDamage.flag;
    bool bSelf = sDamage.bSelf;
    bool bTarget = sDamage.bTarget;

    CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
    float cameraAngle = DirectX::XMConvertToRadians(GetDegreeFromPosition2(pCamera->GetTarget().x,
        pCamera->GetTarget().y,
        pCamera->GetEye().x,
        pCamera->GetEye().y));

    uint32_t FONT_WIDTH = 30;

    auto &mgr = CEffectManager::Instance();

    Vector3 v3Pos = m_GraphicThingInstance.GetPosition();

    v3Pos.z += 200;
  

    if (flag & (DAMAGE_DODGE | DAMAGE_BLOCK))
    {
        const uint32_t id = bSelf ? EFFECT_DAMAGE_MISS : EFFECT_DAMAGE_TARGETMISS;

        const auto info = gCharacterEffectRegistry->FindEffect(id);
        if (!info)
        {
            SPDLOG_ERROR("Failed to find block/crit effect {0}", id);
            return;
        }

        auto index = mgr.CreateEffect(info->id, 1, EFFECT_KIND_BATTLE);

        Matrix mat = Matrix::CreateRotationZ(cameraAngle);
        mat._41 = v3Pos.x;
        mat._42 = v3Pos.y;
        mat._43 = v3Pos.z;
        auto* effect = mgr.GetEffectInstance(index);
        if (effect)
            effect->SetGlobalMatrix(mat);

        return;
    }
    else if (flag & DAMAGE_CRITICAL)
    {
        //rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_CRITICAL],v3Pos,v3Rot);
        //return; 숫자도 표시.
    }

    std::string strDamageType;
    uint32_t rdwCRCEft = 0;
    /*
    if ( (flag & DAMAGE_POISON) )
    {
        strDamageType = "poison_";
        rdwCRCEft = EFFECT_DAMAGE_POISON;
    }
    else
    */
    {
        if (bSelf)
        {
            strDamageType = "damage_";
            if (m_bDamageEffectType == 0)
                rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE;
            else
                rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE2;
            m_bDamageEffectType = !m_bDamageEffectType;
        }
        else if (bTarget == false)
        {
            strDamageType = "nontarget_";
            rdwCRCEft = EFFECT_DAMAGE_NOT_TARGET;
            return; //현재 적용 안됨.
        }
        else
        {
            strDamageType = (flag & DAMAGE_CRITICAL) ? "target_crit_" : "target_";
            rdwCRCEft = EFFECT_DAMAGE_TARGET;
        }
    }

    uint32_t num = 0;
    std::vector<std::string> textures;
    textures.reserve(7);

    for (uint32_t i = 0; damage != 0; ++i)
    {
        if (i > 7)
        {
            SPDLOG_ERROR("ProcessDamage infinite loop possible");
            break;
        }

        const uint32_t number = damage % 10;
        textures.push_back(fmt::format("d:/ymir work/effect/affect/damagevalue/{0}{1}.dds", strDamageType, number));

        const auto info = gCharacterEffectRegistry->FindEffect(rdwCRCEft);
        if (!info)
        {
            SPDLOG_ERROR("Failed to find dmg effect {0}", rdwCRCEft);
            break;
        }

        Matrix matrix = Matrix::CreateTranslation(v3Pos);
        matrix = pCamera->GetInverseViewMatrix() * matrix;

        Matrix matTrans = Matrix::CreateTranslation(FONT_WIDTH * i, 0.0f, 0.0f);
        matTrans._41 = -matTrans._41;

        matrix = matTrans * matrix;
        matrix = pCamera->GetViewMatrix() * matrix;

        mgr.SetEffectTextures(info->id, textures);
        auto index = mgr.CreateEffect(info->id, 1, EFFECT_KIND_BATTLE);

        Matrix mat = Matrix::CreateRotationZ(cameraAngle);
        mat._41 = matrix._41;
        mat._42 = matrix._42;
        mat._43 = matrix._43;

        auto* effect = mgr.GetEffectInstance(index);
        if (effect)
        {
            effect->SetAlwaysRender(true);
            effect->SetGlobalMatrix(mat);
        }

        textures.clear();
        damage /= 10;
    }
}

uint32_t CInstanceBase::CreateEffect(uint32_t effect, uint32_t color, float scale, uint32_t effectKind)
{
    auto &mgr = CEffectManager::Instance();

    const auto info = gCharacterEffectRegistry->FindEffect(effect);
    if (!info)
    {
        //SPDLOG_ERROR( "Failed to find effect %d", effect);
        return 0;
    }

    auto index = mgr.CreateEffect(info->id, scale, effectKind);
    if (index == 0)
        return 0;

    auto* inst = mgr.GetEffectInstance(index);
    if (inst)
    {
        Matrix transform = m_GraphicThingInstance.GetTransform();
        inst->SetGlobalMatrix(transform);
        inst->SetAddColor(color);
    }

    return index;
}

uint32_t CInstanceBase::CreateEffectWithPositionAndRotation(uint32_t effect, Vector3 &pos, Vector3 &rot,
                                                            uint32_t color, float scale, uint32_t effectKind)
{
    auto &mgr = CEffectManager::Instance();

    const auto info = gCharacterEffectRegistry->FindEffect(effect);
    if (!info)
    {
        //SPDLOG_ERROR( "Failed to find effect %d", effect);
        return 0;
    }

    auto index = mgr.CreateEffect(info->id, scale, effectKind);
    if (index == 0)
        return 0;

    auto* inst = mgr.GetEffectInstance(index);
    if (inst)
    {
        Matrix c_rmatGlobal = Matrix::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(rot.x), DirectX::XMConvertToRadians(rot.y), DirectX::XMConvertToRadians(rot.z));
        c_rmatGlobal._41 = pos.x;
        c_rmatGlobal._42 = pos.y;
        c_rmatGlobal._43 = pos.z;
        inst->SetGlobalMatrix(c_rmatGlobal);
        inst->SetAddColor(color);
    }

    return index;
}

uint32_t CInstanceBase::AttachEffect(uint32_t effect, uint32_t color, float scale, uint32_t effectKind)
{
    if (IsInvisibility() && !__IsMainInstance())
        return 0;

    const auto info = gCharacterEffectRegistry->FindEffect(effect);
    if (!info)
    {
        //SPDLOG_ERROR( "Failed to find effect %d", effect);
        return 0;
    }

    return AttachEffect(*info, color, scale, effectKind);
}

uint32_t CInstanceBase::AttachAffectEffect(uint32_t affectType, uint32_t color)
{
    const auto info = gCharacterEffectRegistry->FindAffect(affectType);
    if (!info)
    {
        SPDLOG_DEBUG("Failed to find affect {0} effect info", affectType);
        return 0;
    }

    return AttachEffect(*info, color, 1, EFFECT_KIND_AFFECT);
}

uint32_t CInstanceBase::AttachEffect(const CharacterEffectInfo &info, uint32_t color, float scale, uint32_t effectKind)
{
    if (info.attachingBoneName.empty())
        return m_GraphicThingInstance.AttachEffectByID(0, nullptr, info.id, nullptr, color, scale, effectKind);

    const char *boneName = info.attachingBoneName.c_str();
    return m_GraphicThingInstance.AttachEffectByID(0, boneName, info.id, nullptr, color, scale, effectKind);
}

void CInstanceBase::DetachEffect(uint32_t index)
{
    m_GraphicThingInstance.DettachEffect(index);
}

void CInstanceBase::HideEffect(uint32_t index)
{
    m_GraphicThingInstance.HideEffect(index);
}

void CInstanceBase::ShowEffect(uint32_t index)
{
    m_GraphicThingInstance.ShowEffect(index);
}

void CInstanceBase::AttacAcceEffect()
{
    if (!m_acceRefineEffect)
        m_acceRefineEffect = AttachEffect(EFFECT_ACCE_BACK, 0, 1, EFFECT_KIND_REFINED);
}

void CInstanceBase::LevelUp()
{
    AttachEffect(EFFECT_LEVELUP, 0, 1, EFFECT_KIND_MISC);
}

void CInstanceBase::SkillUp()
{
    AttachEffect(EFFECT_SKILLUP, 0, 1, EFFECT_KIND_MISC);
}

void CInstanceBase::__EffectContainer_Destroy()
{
    for (const auto &p : m_attachedEffects)
        DetachEffect(p.second);

    m_attachedEffects.clear();
}

void CInstanceBase::__EffectContainer_Initialize()
{
    m_attachedEffects.clear();
}

uint32_t CInstanceBase::__EffectContainer_AttachEffect(uint32_t id)
{
    const auto it = m_attachedEffects.find(id);
    if (m_attachedEffects.end() != it)
        return 0;

    const auto index = AttachEffect(id, 0, 1, EFFECT_KIND_SPECIAL_EFFECT);
    m_attachedEffects.insert(std::make_pair(id, index));
    return index;
}

void CInstanceBase::__EffectContainer_DeactiveteEffect(uint32_t id)
{
    const auto it = m_attachedEffects.find(id);
    if (m_attachedEffects.end() == it)
        return;

    auto index = it->second;

    auto inst = CEffectManager::instance().GetEffectInstance(index);
    if (inst)
    {
        inst->SetDeactive();
    }
}

void CInstanceBase::__EffectContainer_ActiveEffect(uint32_t id)
{
    const auto it = m_attachedEffects.find(id);
    if (m_attachedEffects.end() == it)
        return;

    auto index = it->second;

    auto inst = CEffectManager::instance().GetEffectInstance(index);
    if (inst)
    {
        inst->SetActive();
    }
}

void CInstanceBase::__EffectContainer_DetachEffect(uint32_t id)
{
    const auto it = m_attachedEffects.find(id);
    if (m_attachedEffects.end() == it)
        return;

    DetachEffect(it->second);
    m_attachedEffects.erase(it);
}

void CInstanceBase::__AttachEmpireEffect(uint32_t eEmpire)
{
    if (!__IsExistMainInstance())
        return;

    CInstanceBase *pkInstMain = __GetMainInstancePtr();

    if (IsWarp())
        return;
    if (IsObject())
        return;
    if (IsFlag())
        return;
    if (IsPet())
        return;
    if (IsAttackPet())
        return;
    if (IsMount())
        return;
    if (IsShop())
        return;
    if (IsNPC())
        return;
    if (IsResource())
        return;
    if (IsInvisibility()) //Assasins spawned guys etc
        return;
    if (!pkInstMain->IsGameMaster() && pkInstMain->IsSameEmpire(*this))
        return;

    if (!pkInstMain->IsGameMaster())
    {
        if (pkInstMain->IsSameEmpire(*this))
            return;

        // HIDE_OTHER_EMPIRE_EUNHYEONG_ASSASSIN
        if (IsInvisibility())
            return;
        // END_OF_HIDE_OTHER_EMPIRE_EUNHYEONG_ASSASSIN
    }

    if (IsGameMaster())
        return;

    auto index = __EffectContainer_AttachEffect(EFFECT_EMPIRE + eEmpire);

}

void CInstanceBase::__AttachSelectEffect()
{
    auto index = __EffectContainer_AttachEffect(EFFECT_SELECT);

}

void CInstanceBase::__DetachSelectEffect()
{
    __EffectContainer_DetachEffect(EFFECT_SELECT);
}

void CInstanceBase::__AttachTargetEffect()
{
    auto index = __EffectContainer_AttachEffect(EFFECT_TARGET);

}

void CInstanceBase::__DetachTargetEffect()
{
    __EffectContainer_DetachEffect(EFFECT_TARGET);
}

void CInstanceBase::__StoneSmoke_Destroy()
{
    if (m_stoneSmokeIndex == 0)
        return;

    DetachEffect(m_stoneSmokeIndex);
    m_stoneSmokeIndex = 0;
}

void CInstanceBase::__StoneSmoke_Create(uint32_t id)
{
    m_stoneSmokeIndex = m_GraphicThingInstance.AttachSmokeEffect(id);
}

void CInstanceBase::SetAlpha(float fAlpha)
{
    __SetBlendRenderingMode();
    __SetAlphaValue(fAlpha);
}

bool CInstanceBase::UpdateDeleting()
{
    Update();
    Transform();

    auto &rApp = CPythonApplication::AppInst();

    float fAlpha = __GetAlphaValue() - (rApp.GetGlobalElapsedTime() * 1.5f);
    __SetAlphaValue(fAlpha);

    if (fAlpha < 0.0f)
        return false;

    return true;
}

void CInstanceBase::DeleteBlendOut()
{
    __SetBlendRenderingMode();
    __SetAlphaValue(1.0f);
    DetachTextTail();

    CPythonPlayer::Instance().NotifyDeletingCharacterInstance(GetVirtualID());
}

void CInstanceBase::ClearPVPKeySystem()
{
    g_kSet_dwPVPReadyKey.clear();
    g_kSet_dwPVPKey.clear();
    g_kSet_dwGVGKey.clear();
    g_kSet_dwDUELKey.clear();
}

void CInstanceBase::InsertPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
    uint32_t dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

    g_kSet_dwPVPKey.insert(dwPVPKey);
}

void CInstanceBase::InsertPVPReadyKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
    uint32_t dwPVPReadyKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

    g_kSet_dwPVPKey.insert(dwPVPReadyKey);
}

void CInstanceBase::RemovePVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
    uint32_t dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

    g_kSet_dwPVPKey.erase(dwPVPKey);
}

void CInstanceBase::InsertGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID)
{
    uint32_t dwGVGKey = __GetPVPKey(dwSrcGuildVID, dwDstGuildVID);
    g_kSet_dwGVGKey.insert(dwGVGKey);
}

void CInstanceBase::RemoveGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID)
{
    uint32_t dwGVGKey = __GetPVPKey(dwSrcGuildVID, dwDstGuildVID);
    g_kSet_dwGVGKey.erase(dwGVGKey);
}

void CInstanceBase::InsertDUELKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
    uint32_t dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

    g_kSet_dwDUELKey.insert(dwPVPKey);
}

uint32_t CInstanceBase::__GetPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
    if (dwVIDSrc > dwVIDDst)
        std::swap(dwVIDSrc, dwVIDDst);

    uint32_t awSrc[2];
    awSrc[0] = dwVIDSrc;
    awSrc[1] = dwVIDDst;

    const uint8_t *s = (const uint8_t *)awSrc;
    const uint8_t *end = s + sizeof(awSrc);
    uint32_t h = 0;

    while (s < end)
    {
        h *= 16777619;
        h ^= (uint8_t)*(uint8_t *)(s++);
    }

    return h;
}

bool CInstanceBase::__FindPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
    uint32_t dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

    if (g_kSet_dwPVPKey.end() == g_kSet_dwPVPKey.find(dwPVPKey))
        return false;

    return true;
}

bool CInstanceBase::__FindPVPReadyKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
    uint32_t dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

    if (g_kSet_dwPVPReadyKey.end() == g_kSet_dwPVPReadyKey.find(dwPVPKey))
        return false;

    return true;
}

//±æµåÀü½Ã »ó´ë ±æµåÀÎÁö È®ÀÎÇÒ¶§.
bool CInstanceBase::__FindGVGKey(uint32_t dwSrcGuildID, uint32_t dwDstGuildID)
{
    uint32_t dwGVGKey = __GetPVPKey(dwSrcGuildID, dwDstGuildID);

    if (g_kSet_dwGVGKey.end() == g_kSet_dwGVGKey.find(dwGVGKey))
        return false;

    return true;
}

//´ë·Ã ¸ðµå¿¡¼­´Â ´ë·Ã »ó´ë¸¸ °ø°ÝÇÒ ¼ö ÀÖ´Ù.
bool CInstanceBase::__FindDUELKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
    uint32_t dwDUELKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

    if (g_kSet_dwDUELKey.end() == g_kSet_dwDUELKey.find(dwDUELKey))
        return false;

    return true;
}

bool CInstanceBase::IsPVPInstance(CInstanceBase &rkInstSel)
{
    uint32_t dwVIDSrc = GetVirtualID();
    uint32_t dwVIDDst = rkInstSel.GetVirtualID();

    uint32_t dwGuildIDSrc = GetGuildID();
    uint32_t dwGuildIDDst = rkInstSel.GetGuildID();

    if (GetDuelMode()) //´ë·Ã ¸ðµåÀÏ¶§´Â ~_~
        return true;

    return __FindPVPKey(dwVIDSrc, dwVIDDst) || __FindGVGKey(dwGuildIDSrc, dwGuildIDDst);
    //__FindDUELKey(dwVIDSrc, dwVIDDst);
}

const DirectX::SimpleMath::Color &CInstanceBase::GetNameColor()
{
    return GetIndexedNameColor(GetNameColorIndex());
}

UINT CInstanceBase::GetNameColorIndex()
{
    if (IsPC())
    {
        if (m_isKiller)
            return NAMECOLOR_PK;

        if (__IsExistMainInstance() && !__IsMainInstance())
        {
            CInstanceBase *pkInstMain = __GetMainInstancePtr();
            if (!pkInstMain)
            {
                SPDLOG_ERROR("CInstanceBase::GetNameColorIndex - MainInstance is NULL");
                return NAMECOLOR_PC;
            }
            uint32_t dwVIDMain = pkInstMain->GetVirtualID();
            uint32_t dwVIDSelf = GetVirtualID();

            int iPVPTeam = CPythonPlayer::Instance().GetPVPTeam();
            {
                int iMyPVPTeam = CPythonCharacterManager::Instance().GetPVPTeam(dwVIDSelf);
                if (iMyPVPTeam != -1)
                {
                    if (iMyPVPTeam == iPVPTeam)
                        return NAMECOLOR_PC;
                    else
                        return NAMECOLOR_PVP;
                }
            }

            if (pkInstMain->GetDuelMode())
            {
                switch (pkInstMain->GetDuelMode())
                {
                case DUEL_CANNOTATTACK:
                    return NAMECOLOR_PC + GetEmpireID();
                case DUEL_START:
                    if (__FindDUELKey(dwVIDMain, dwVIDSelf))
                        return NAMECOLOR_PVP;
                    else
                        return NAMECOLOR_PC + GetEmpireID();
                }
            }
            const auto index = CPythonBackground::Instance().GetMapIndex();
            if (pkInstMain->IsSameEmpire(*this) || CPythonBackground::instance().IsDuelMap(index))
            {
                if (__FindPVPKey(dwVIDMain, dwVIDSelf))
                    return NAMECOLOR_PVP;

                uint32_t dwGuildIDMain = pkInstMain->GetGuildID();
                uint32_t dwGuildIDSelf = GetGuildID();
                if (__FindGVGKey(dwGuildIDMain, dwGuildIDSelf))
                    return NAMECOLOR_PVP;
            }
            else
                return NAMECOLOR_PVP;
        }

        auto &rPlayer = CPythonPlayer::Instance();
        if (rPlayer.IsPartyMemberByVID(GetVirtualID()))
            return NAMECOLOR_PARTY;

        return NAMECOLOR_PC + GetEmpireID();
    }

    if (IsNPC())
        return NAMECOLOR_NPC;

    if (IsEnemy())
        return NAMECOLOR_MOB;

    if (IsPoly())
        return NAMECOLOR_MOB;

    if (IsBuffBot())
        return NAMECOLOR_BUFFBOT;

    return NAMECOLOR_NUM;
}

const DirectX::SimpleMath::Color &CInstanceBase::GetTitleColor()
{
    UINT uGrade = GetAlignmentGrade();
    if (uGrade >= TITLE_NUM)
    {
        static DirectX::SimpleMath::Color s_kD3DXClrTitleDefault(0xffffffff);
        return s_kD3DXClrTitleDefault;
    }

    return g_akD3DXClrTitle[uGrade];
}

void CInstanceBase::AttachTextTail()
{
    if (m_isTextTail)
    {
        return;
    }

    m_isTextTail = true;

    uint32_t dwVID = GetVirtualID();

    float fTextTailHeight = IsMountingHorse() ? 110.0f : 10.0f;

    static DirectX::SimpleMath::Color s_kD3DXClrTextTail = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f);
    CPythonTextTail::Instance().RegisterCharacterTextTail(m_dwGuildID, dwVID, s_kD3DXClrTextTail, fTextTailHeight);

    // CHARACTER_LEVEL
    if (m_dwLevel)
        UpdateTextTailLevel(m_dwLevel);
}

void CInstanceBase::DetachTextTail()
{
    if (!m_isTextTail)
        return;

    m_isTextTail = false;
    CPythonTextTail::Instance().DeleteCharacterTextTail(GetVirtualID());
}

void CInstanceBase::UpdateTextTailLevel(uint32_t level)
{
 //   static Color s_kLevelColor = DirectX::SimpleMath::Color(119.0f/255.0f, 246.0f/255.0f, 168.0f/255.0f, 1.0f);
    static DirectX::SimpleMath::Color s_kPlayerLevelColor = DirectX::SimpleMath::Color(152.0f / 255.0f, 255.0f / 255.0f, 51.0f / 255.0f, 1.0f);
#if defined(WJ_SHOW_MOB_INFO)
    static DirectX::SimpleMath::Color s_kPetLevelColor = DirectX::SimpleMath::Color(255.0f / 255.0f, 255.0f / 255.0f, 0.0f, 1.0f);
    static DirectX::SimpleMath::Color s_kMobLevelColor = DirectX::SimpleMath::Color(119.0f / 255.0f, 246.0f / 255.0f, 168.0f / 255.0f, 1.0f);
#endif

//std::string szLevel = fmt::format("Lv. {}", level);
//    std::string szLevel = " Diamas";
    std::string szLevel = "";

#if defined(WJ_SHOW_MOB_INFO)
    if (IsPC())
        CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szLevel.c_str(), s_kPlayerLevelColor);
    else if (IsAttackPet())
        CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szLevel.c_str(), s_kPetLevelColor);
    else if (IsEnemy() || IsStone())
        CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szLevel.c_str(), s_kMobLevelColor);
    else
        CPythonTextTail::Instance().DetachLevel(GetVirtualID());
#else
	CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kPlayerLevelColor);
#endif
}

void CInstanceBase::RefreshTextTail()
{
    CPythonTextTail::Instance().SetCharacterTextTailColor(GetVirtualID(), GetNameColor());

    int iAlignmentGrade = GetAlignmentGrade();
    if (TITLE_NONE == iAlignmentGrade)
    {
        CPythonTextTail::Instance().DetachTitle(GetVirtualID());
    }
    else
    {
        if (GetSexByRace(GetRace()) == SEX_MALE)
        {
            if (auto itor = g_TitleNameMap.find(iAlignmentGrade); g_TitleNameMap.end() != itor)
            {
                const std::string &c_rstrTitleName = itor->second;

                CPythonTextTail::Instance().AttachTitle(GetVirtualID(), c_rstrTitleName.c_str(), GetTitleColor());
            }
        }
        else
        {
            if (auto itor = g_TitleNameMapFemale.find(iAlignmentGrade); g_TitleNameMapFemale.end() != itor)
            {
                const std::string &c_rstrTitleName = itor->second;

                CPythonTextTail::Instance().AttachTitle(GetVirtualID(), c_rstrTitleName.c_str(), GetTitleColor());
            }
        }
    }

    if (0 == m_prefixNum)
    {
        CPythonTextTail::Instance().DetachPrefix(GetVirtualID());
    }
    else
    {
        if (auto itor = g_NamePrefixMap.find(m_prefixNum); g_NamePrefixMap.end() != itor)
        {
            const std::string &c_rstrTitleName = itor->second;
            CPythonTextTail::Instance().AttachPrefix(GetVirtualID(), c_rstrTitleName.c_str(),
                                                     GetNameColor());
        }
    }

    if (!m_titlePreview.empty())
    {
        CPythonTextTail::Instance().AttachCustomTitle(GetVirtualID(), m_titlePreview.c_str(), Color(m_titleColorPreview));
    }
    else
    {
        if (m_title.empty())
        {
            CPythonTextTail::Instance().DetachCustomTitle(GetVirtualID());
        }
        else
        {
            CPythonTextTail::Instance().AttachCustomTitle(GetVirtualID(), m_title.c_str(), Color(m_titleColor));
        }
    }
}

void CInstanceBase::RefreshTextTailTitle()
{
    RefreshTextTail();
}

void CInstanceBase::ClearAffects()
{
    for (const auto type : GetAffectTypes(m_affects))
        SetAffectActive(type, false);

    m_affects.clear();
}

void CInstanceBase::AddAffect(const AffectData &affect)
{
    if (!SearchAffect(m_affects, affect.type))
        SetAffectActive(affect.type, true, affect.color);

    InsertAffect(m_affects, affect);
}

void CInstanceBase::RemoveAffect(uint32_t type, uint32_t pointType)
{
    // Construct a minimal temporary for comparision.
    AffectData cmpAffect = {};
    cmpAffect.type = type;
    cmpAffect.pointType = pointType;

    const auto range = std::equal_range(m_affects.begin(), m_affects.end(),
                                        cmpAffect);
    m_affects.erase(range.first, range.second);

    // If all effects with this type are gone, deactivate any effect
    // we might have used.
#ifndef _DEBUG
	if (range.first != range.second && !HasAffect(type))
#else
    if (!HasAffect(type))
#endif
        SetAffectActive(type, false);
}

bool CInstanceBase::HasAffect(uint32_t type, uint32_t pointType)
{
    return SearchAffect(m_affects, type, pointType);
}

void CInstanceBase::SetAffectActive(uint32_t type, bool activate, uint32_t color)
{
    switch (type)
    {
    case SKILL_CHUNKEON:
        m_GraphicThingInstance.SetResistFallen(activate);
        break;

    case SKILL_GEOMKYUNG:
        __Warrior_SetGeomgyeongAffect(activate, color);
        return;

    case SKILL_EUNHYUNG:
    case AFFECT_REVIVE_INVISIBLE:
        __Assassin_SetEunhyeongAffect(activate);
        return;

    case SKILL_GYEONGGONG:
    case SKILL_KWAESOK:
        // 경공술, 쾌속은 뛸때만 Attaching 시킵니다. - [levites]
        if (activate)
            if (!IsWalking())
                return;
        break;

    case AFFECT_INVISIBILITY:
        // 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
        if (activate)
        {
            // m_GraphicThingInstance.ClearAttachingEffect();
            // __EffectContainer_Destroy();
            // DetachTextTail();
            HideEffect(m_armorShiningEffects);
            HideEffect(m_armorCostumeShiningEffects);
            HideEffect(m_armorCostumeEffectShiningEffects);
            HideEffect(m_wingCostumeShiningEffect);

            HideEffect(m_swordRefineEffectRight);
            HideEffect(m_swordRefineEffectLeft);
            HideEffect(m_armorRefineEffect);
            HideEffect(m_acceRefineEffect);
            __HideWeaponRefineEffect();
            m_GraphicThingInstance.HideAllAttachingEffect();
        }
        else
        {
            m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
            m_GraphicThingInstance.ShowAllAttachingEffect();
            ShowEffect(m_armorShiningEffects);
            ShowEffect(m_armorCostumeShiningEffects);
            ShowEffect(m_armorCostumeEffectShiningEffects);
            ShowEffect(m_wingCostumeShiningEffect);
            ShowEffect(m_swordRefineEffectRight);
            ShowEffect(m_swordRefineEffectLeft);
            ShowEffect(m_armorRefineEffect);
            ShowEffect(m_acceRefineEffect);
            __ShowWeaponRefineEffect();

            // AttachTextTail();
            // RefreshTextTail();
        }
        return;

    case AFFECT_STUN:
        m_GraphicThingInstance.SetSleep(activate);
        break;
    }

    if (activate)
    {
        auto &index = m_affectEffects[type];
        if (index != 0)
            return;

        index = AttachAffectEffect(type, color);
    }
    else
    {
        const auto it = m_affectEffects.find(type);
        if (it != m_affectEffects.end())
        {
            DetachEffect(it->second);
            m_affectEffects.erase(it);
        }
    }
}

void CInstanceBase::__Assassin_SetEunhyeongAffect(bool isVisible)
{
    if (isVisible)
    {
        // NOTE : Dress ¸¦ ÀÔ°í ÀÖÀ¸¸é Alpha ¸¦ ³ÖÁö ¾Ê´Â´Ù.
        if (IsWearingDress())
            return;

        if (__IsMainInstance())
        {
            m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
            if (m_weaponShiningEffects[0])
            {
                HideEffect(m_weaponShiningEffects[0]);
            }

            if (m_weaponShiningEffects[1])
            {
                HideEffect(m_weaponShiningEffects[1]);
            }

            if (m_weaponCostumeShiningEffects[0])
            {
                HideEffect(m_weaponCostumeShiningEffects[0]);
            }

            if (m_weaponCostumeShiningEffects[1])
            {
                HideEffect(m_weaponCostumeShiningEffects[1]);
            }

            if (m_weaponCostumeEffectShiningEffects[0])
            {
                HideEffect(m_weaponCostumeEffectShiningEffects[0]);
            }

            if (m_weaponCostumeEffectShiningEffects[1])
            {
                HideEffect(m_weaponCostumeEffectShiningEffects[1]);
            }

            HideEffect(m_armorShiningEffects);
            HideEffect(m_armorCostumeShiningEffects);
            HideEffect(m_armorCostumeEffectShiningEffects);
            HideEffect(m_wingCostumeShiningEffect);

            HideEffect(m_swordRefineEffectRight);
            HideEffect(m_swordRefineEffectLeft);
            HideEffect(m_armorRefineEffect);
            HideEffect(m_acceRefineEffect);

            __HideWeaponRefineEffect();
            OnUnselected();
        }
        else
        {
            // 2004.10.16.myevan.ÀºÇü¹ý ¿ÏÀü Åõ¸í
            m_GraphicThingInstance.BlendAlphaValue(0.0f, 1.0f);
            m_GraphicThingInstance.HideAllAttachingEffect();
            if (m_weaponShiningEffects[0])
            {
                HideEffect(m_weaponShiningEffects[0]);
            }

            if (m_weaponShiningEffects[1])
            {
                HideEffect(m_weaponShiningEffects[1]);
            }

            if (m_weaponCostumeShiningEffects[0])
            {
                HideEffect(m_weaponCostumeShiningEffects[0]);
            }

            if (m_weaponCostumeShiningEffects[1])
            {
                HideEffect(m_weaponCostumeShiningEffects[1]);
            }

            if (m_weaponCostumeEffectShiningEffects[0])
            {
                HideEffect(m_weaponCostumeEffectShiningEffects[0]);
            }

            if (m_weaponCostumeEffectShiningEffects[1])
            {
                HideEffect(m_weaponCostumeEffectShiningEffects[1]);
            }

            HideEffect(m_armorShiningEffects);
            HideEffect(m_armorCostumeShiningEffects);
            HideEffect(m_armorCostumeEffectShiningEffects);
            HideEffect(m_wingCostumeShiningEffect);

            HideEffect(m_swordRefineEffectRight);
            HideEffect(m_swordRefineEffectLeft);
            HideEffect(m_armorRefineEffect);
            HideEffect(m_acceRefineEffect);
            __HideWeaponRefineEffect();
            OnUnselected();
            CPythonPlayer::instance().NotifyCharacterDead(GetVirtualID());
        }
    }
    else
    {
        m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
        m_GraphicThingInstance.ShowAllAttachingEffect();
        if (m_weaponShiningEffects[0])
        {
            ShowEffect(m_weaponShiningEffects[0]);
        }

        if (m_weaponShiningEffects[1])
        {
            ShowEffect(m_weaponShiningEffects[1]);
        }

        if (m_weaponCostumeShiningEffects[0])
        {
            ShowEffect(m_weaponCostumeShiningEffects[0]);
        }

        if (m_weaponCostumeShiningEffects[1])
        {
            ShowEffect(m_weaponCostumeShiningEffects[1]);
        }

        if (m_weaponCostumeEffectShiningEffects[0])
        {
            ShowEffect(m_weaponCostumeEffectShiningEffects[0]);
        }

        if (m_weaponCostumeEffectShiningEffects[1])
        {
            ShowEffect(m_weaponCostumeEffectShiningEffects[1]);
        }

        ShowEffect(m_armorShiningEffects);
        ShowEffect(m_armorCostumeShiningEffects);
        ShowEffect(m_armorCostumeEffectShiningEffects);
        ShowEffect(m_wingCostumeShiningEffect);

        ShowEffect(m_swordRefineEffectRight);
        ShowEffect(m_swordRefineEffectLeft);
        ShowEffect(m_armorRefineEffect);
        ShowEffect(m_acceRefineEffect);

        __ShowWeaponRefineEffect();
    }
}

void CInstanceBase::__Warrior_SetGeomgyeongAffect(bool isVisible, uint32_t color)
{
    if (isVisible)
    {
        if (IsWearingDress())
            return;

        auto &index = m_affectEffects[SKILL_GEOMKYUNG];

        if (index)
            DetachEffect(index);

        m_GraphicThingInstance.SetReachScale(1.5f);

        if (m_GraphicThingInstance.IsTwoHandMode())
            index = AttachEffect(EFFECT_WEAPON_TWOHANDED, color, 1, EFFECT_KIND_AFFECT);
        else
            index = AttachEffect(EFFECT_WEAPON_ONEHANDED, color, 1, EFFECT_KIND_AFFECT);

        if (!m_hasWeapon)
        {
            auto effectGeom = CEffectManager::Instance().GetEffectInstance(index);
            if (effectGeom)
                effectGeom->SetDeactive();
        }
    }
    else
    {
        m_GraphicThingInstance.SetReachScale(1.0f);

        const auto it = m_affectEffects.find(SKILL_GEOMKYUNG);
        if (it != m_affectEffects.end())
        {
            DetachEffect(it->second);
            m_affectEffects.erase(it);
        }
    }
}

bool CInstanceBase::IsPossibleEmoticon()
{
    CEffectManager &rkEftMgr = CEffectManager::Instance();
    for (uint32_t emoticon = 0; emoticon != EMOTICON_NUM; ++emoticon)
    {
        const auto it = m_attachedEffects.find(EFFECT_EMOTICON + emoticon);
        if (it == m_attachedEffects.end())
            continue;

        if (it->second && rkEftMgr.IsAliveEffect(it->second))
            return false;
    }

    if (ELTimer_GetMSec() - m_dwEmoticonTime < 1000)
        return false;

    return true;
}

void CInstanceBase::SetFishEmoticon()
{
    SetEmoticon(EMOTICON_FISH);
}

void CInstanceBase::SetEmoticon(UINT eEmoticon)
{
    if (eEmoticon >= EMOTICON_NUM)
    {
        SPDLOG_ERROR("Invalid emoticon ID {0} > {1}",
                      eEmoticon, EMOTICON_NUM);
        return;
    }

    if (!IsPossibleEmoticon())
        return;

    const auto effect = EFFECT_EMOTICON + eEmoticon;
    Vector3 posD(0.0f, 0.0f, m_GraphicThingInstance.GetHeight());

    const auto info = gCharacterEffectRegistry->FindEffect(effect);
    if (!info)
    {
        //SPDLOG_ERROR( "Failed to find effect %d for emoticon %d",
        //	effect, eEmoticon);
        return;
    }

    m_attachedEffects[effect] = m_GraphicThingInstance.AttachEffectByID(0, NULL,
                                                                        info->id,
                                                                        &posD, 0, 1, EFFECT_KIND_EMOTION);
    m_dwEmoticonTime = ELTimer_GetMSec();
}

void CInstanceBase::SetDustGap(float fDustGap)
{
    ms_fDustGap = fDustGap;
}

void CInstanceBase::SetHorseDustGap(float fDustGap)
{
    ms_fHorseDustGap = fDustGap;
}

void CInstanceBase::__ComboProcess()
{
    /*
    uint32_t dwcurComboIndex = m_GraphicThingInstance.GetComboIndex();

    if (0 != dwcurComboIndex)
    {
        if (m_dwLastComboIndex != m_GraphicThingInstance.GetComboIndex())
        {
            if (!m_GraphicThingInstance.IsHandMode() & IsAffect(AFFECT_HWAYEOM))
            {
                __AttachEffect(EFFECT_FLAME_ATTACK);
            }
        }
    }

    m_dwLastComboIndex = dwcurComboIndex;
    */
}

void RegisterTitleName(int iIndex, const char *c_szTitleName, const char *c_szTitleNameFemale)
{
    g_TitleNameMap.emplace(iIndex, c_szTitleName);
    g_TitleNameMapFemale.emplace(iIndex, c_szTitleNameFemale);
}

void RegisterNamePrefix(int iIndex, const char *prefix)
{
    g_NamePrefixMap.emplace(iIndex, prefix);
}

DirectX::SimpleMath::Color __RGBToColoru(UINT r, UINT g, UINT b)
{
    uint32_t dwColor = 0xff;
    dwColor <<= 8;
    dwColor |= r;
    dwColor <<= 8;
    dwColor |= g;
    dwColor <<= 8;
    dwColor |= b;

    return DirectX::SimpleMath::Color(dwColor);
}

bool RegisterNameColor(UINT uIndex, UINT r, UINT g, UINT b)
{
    if (uIndex >= CInstanceBase::NAMECOLOR_NUM)
        return false;

    g_akD3DXClrName[uIndex] = __RGBToColoru(r, g, b);
    return true;
}

bool RegisterTitleColor(UINT uIndex, UINT r, UINT g, UINT b)
{
    if (uIndex >= TITLE_NUM)
        return false;

    g_akD3DXClrTitle[uIndex] = __RGBToColoru(r, g, b);
    return true;
}
