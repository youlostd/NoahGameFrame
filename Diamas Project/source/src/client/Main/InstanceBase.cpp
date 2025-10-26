#include "InstanceBase.h"
#include "../GameLib/NpcManager.h"
#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"
#include "StdAfx.h"
#include <game/GamePacket.hpp>

#include "../EterLib/CharacterEffectRegistry.hpp"
#include "../EterLib/Engine.hpp"
#include "../EterLib/StateManager.h"
#include "../GameLib/ItemManager.h"
#include "PythonApplication.h"
#include <game/AffectConstants.hpp>
#include <game/AffectsHolderUtil.hpp>
#include <game/MotionConstants.hpp>
#include <game/MotionTypes.hpp>
#include <unordered_map>

#include "../EterLib/lineintersect_utils.h"
#include "base/Remotery.h"

namespace
{
std::pair<CItemData*, uint32_t> GetPartData(const Part& part)
{
    std::pair<CItemData*, uint32_t> data;

    if (part.vnum == part.appearance || part.appearance == 0)
        data = std::make_pair(CItemManager::Instance().GetProto(part.vnum),
                              part.vnum);
    else
        data =
            std::make_pair(CItemManager::Instance().GetProto(part.appearance),
                           part.appearance);

    if (Engine::GetSettings().IsShowCostume() ||
        CItemData::IsWeddingDress(part.costume)) {
        if (part.costume != 0 && (part.costume == part.costume_appearance ||
                                  part.costume_appearance == 0))
            data = std::make_pair(
                CItemManager::Instance().GetProto(part.costume), part.costume);
        else if (part.costume_appearance)
            data = std::make_pair(
                CItemManager::Instance().GetProto(part.costume_appearance),
                part.costume_appearance);
    }

    return data;
}
} // namespace

static const float c_fDefaultRotationSpeed = 1200.0f;
static const float c_fDefaultHorseRotationSpeed = 1800.0f;

bool IsWall(unsigned race)
{
    switch (race) {
        case 14201:
        case 14202:
        case 14203:
        case 14204:
            return true;
        default:;
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////

CInstanceBase::SHORSE::SHORSE()
{
    __Initialize();
}

CInstanceBase::SHORSE::~SHORSE()
{
    assert(m_pkActor == NULL);
}

void CInstanceBase::SHORSE::__Initialize()
{
    m_isMounting = false;
    m_pkActor = nullptr;
}

void CInstanceBase::SHORSE::SetAttackSpeed(UINT uAtkSpd) const
{
    if (!IsMounting())
        return;

    CActorInstance& rkActor = GetActorRef();
    rkActor.SetAttackSpeed(uAtkSpd / 100.0f);
}

void CInstanceBase::SHORSE::SetMoveSpeed(UINT uMovSpd) const
{
    if (!IsMounting())
        return;

    CActorInstance& rkActor = GetActorRef();
    rkActor.SetMoveSpeed(uMovSpd / 100.0f);
}

void CInstanceBase::SHORSE::Create(const TPixelPosition& c_rkPPos, UINT eRace,
                                   UINT eHitEffect)
{
    assert(NULL == m_pkActor && "CInstanceBase::SHORSE::Create - ALREADY "
                                "MOUNT");

    m_pkActor = new CActorInstance;

    auto& rkActor = GetActorRef();
    rkActor.SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());
    if (!rkActor.SetRace(eRace)) {
        delete m_pkActor;
        m_pkActor = nullptr;
        return;
    }

    rkActor.SetShape(static_cast<uint32_t>(0), 0.0f);
    rkActor.SetBattleHitEffect(eHitEffect);
    rkActor.SetAlphaValue(0.0f);
    rkActor.BlendAlphaValue(1.0f, 0.5f);
    rkActor.SetMoveSpeed(1.0f);
    rkActor.SetAttackSpeed(1.0f);
    rkActor.SetMotionMode(MOTION_MODE_GENERAL);
    rkActor.Stop();

    rkActor.SetCurPixelPosition(c_rkPPos);

    m_isMounting = true;
}

void CInstanceBase::SHORSE::Destroy()
{
    if (m_pkActor) {
        delete m_pkActor;
        m_pkActor = nullptr;
    }

    __Initialize();
}

CActorInstance& CInstanceBase::SHORSE::GetActorRef() const
{
    assert(NULL != m_pkActor && "CInstanceBase::SHORSE::GetActorRef");
    return *m_pkActor;
}

CActorInstance* CInstanceBase::SHORSE::GetActorPtr() const
{
    return m_pkActor;
}

UINT CInstanceBase::SHORSE::GetLevel()
{
#ifdef HORSE_NORMAL_ATTACK
    if (m_pkActor) {
        uint32_t mount = m_pkActor->GetRace();
        switch (mount) {
            case 20101:
            case 20102:
            case 20103:
            case 20223: //	Àü°©¼®·æÀÚ
                return 1;
            case 20104:
            case 20105:
            case 20106:
            case 20224: //³­ÆøÇÑ Àü°©¼®·æÀÚ
                return 2;
            case 20107:
            case 20108:
            case 20109:
            case 20110: // #0000673: [M2EU] »õ·Î¿î Å»°Í Å¸°í °ø°Ý ¾ÈµÊ
            case 20111: // #0000673: [M2EU] »õ·Î¿î Å»°Í Å¸°í °ø°Ý ¾ÈµÊ
            case 20112: // #0000673: [M2EU] »õ·Î¿î Å»°Í Å¸°í °ø°Ý ¾ÈµÊ
            case 20113: // #0000673: [M2EU] »õ·Î¿î Å»°Í Å¸°í °ø°Ý ¾ÈµÊ
            case 20114:
            case 20115:
            case 20116:
            case 20117:
            case 20118:
            case 20120:
            case 20121:
            case 20122:
            case 20123:
            case 20124:
            case 20125:
            case 20225: //	¿ë¸ÍÇÑ Àü°©¼®·æÀÚ
                return 3;
            case 20119: // ¶ó¸¶´Ü ÀÌº¥Æ®¿ë Èæ¸¶´Â ½ºÅ³ºÒ°¡, °ø°Ý°¡´ÉÇÑ ·¹º§2·Î
                        // ¼³Á¤
            case 20219: // ÇÒ·ÎÀ© ÀÌº¥Æ®¿ë Èæ¸¶´Â ½ºÅ³ºÒ°¡, °ø°Ý°¡´ÉÇÑ ·¹º§2·Î
                        // ¼³Á¤ (=¶ó¸¶´Ü Èæ¸¶ Å¬·Ð)
            case 20220:
            case 20221:
            case 20222:
            case 20226:
            case 20227: //	À¯´ÏÄÜ
                return 2;
        }

        // ¸¶¿îÆ® È®Àå ½Ã½ºÅÛ¿ë Æ¯¼ö Ã³¸® (20201 ~ 20212 ´ë¿ªÀ» »ç¿ëÇÏ°í
        // ¼ø¼­´ë·Î 4°³¾¿ ³ª´²¼­ ÃÊ±Þ, Áß±Þ, °í±ÞÀÓ)
        //	-- Å»°Í ·¹º§À» Å¬¶ó¿¡¼­ ÃøÁ¤ÇÏ°í °ø°Ý/½ºÅ³ »ç¿ë°¡´É ¿©ºÎµµ Å¬¶ó¿¡¼­
        //Ã³¸®ÇÏ´Â °Í ÀÚÃ¼¿¡ ¹®Á¦°¡ ÀÖ´Â µí.. [hyo]
        {
            // Áß±Þ Å»°ÍÀº ·¹º§2 (°ø°Ý °¡´É, ½ºÅ³ ºÒ°¡)
            if ((20205 <= mount && 20208 >= mount) || (20214 == mount) ||
                (20217 == mount) // ³­ÆøÇÑ Àü°©¼ø¼ø·Ï, ³­ÆøÇÑ Àü°©¾Ï¼ø·Ï
            )
                return 2;

            // °í±Þ Å»°ÍÀº ·¹º§3 (°ø°Ý °¡´É, ½ºÅ³ °¡´É)
            if ((20209 <= mount && 20212 >= mount) || (20215 == mount) ||
                (20218 == mount) || // ¿ë¸ÍÇÑ Àü°©¼ø¼ø·Ï, ¿ë¸ÍÇÑ Àü°©¾Ï¼ø·Ï
                (20220 == mount))
                return 3;
        }
    }
    return 0;

#else
    return 3;
#endif
}

bool CInstanceBase::SHORSE::IsNewMount() const
{
    if (!m_pkActor)
        return false;
    const auto mount = m_pkActor->GetRace();

    if ((20205 <= mount && 20208 >= mount) || (20214 == mount) ||
        (20217 == mount) // ³­ÆøÇÑ Àü°©¼ø¼ø·Ï, ³­ÆøÇÑ Àü°©¾Ï¼ø·Ï
    )
        return true;

    // °í±Þ Å»°Í
    if ((20209 <= mount && 20212 >= mount) || (20215 == mount) ||
        (20218 == mount) || // ¿ë¸ÍÇÑ Àü°©¼ø¼ø·Ï, ¿ë¸ÍÇÑ Àü°©¾Ï¼ø·Ï
        (20220 == mount))
        return true;

    return false;
}

bool CInstanceBase::SHORSE::CanUseSkill()
{
    return true;
}

bool CInstanceBase::SHORSE::CanAttack() const
{
    if (IsMounting())
        if (GetLevel() <= 1)
            return false;

    return true;
}

bool CInstanceBase::SHORSE::IsMounting() const
{
    return m_isMounting;
}

void CInstanceBase::SHORSE::Deform() const
{
    if (!IsMounting())
        return;

    CActorInstance& rkActor = GetActorRef();
    rkActor.Deform();
}

void CInstanceBase::SHORSE::Render() const
{
    if (!IsMounting())
        return;

    CActorInstance& rkActor = GetActorRef();
    rkActor.Render();
}

void CInstanceBase::__AttachHorseSaddle()
{
    if (!IsMountingHorse())
        return;

    int boneIndex;
    if (!m_kHorse.m_pkActor->FindBoneIndex(PART_MAIN, "saddle", &boneIndex))
        return;

    m_kHorse.m_pkActor->AttachModelInstance(PART_MAIN, m_GraphicThingInstance,
                                            PART_MAIN, boneIndex);
}

void CInstanceBase::__DetachHorseSaddle()
{
    if (!IsMountingHorse())
        return;

    m_kHorse.m_pkActor->DetachModelInstance(PART_MAIN, m_GraphicThingInstance,
                                            PART_MAIN);
}

//////////////////////////////////////////////////////////////////////////////////////

void CInstanceBase::BlockMovement()
{
    m_GraphicThingInstance.BlockMovement();
}

bool CInstanceBase::IsBlockObject(const CGraphicObjectInstance& c_rkBGObj)
{
    return m_GraphicThingInstance.IsBlockObject(c_rkBGObj);
}

bool CInstanceBase::AvoidObject(const CGraphicObjectInstance& c_rkBGObj)
{
    return m_GraphicThingInstance.AvoidObject(c_rkBGObj);
}

///////////////////////////////////////////////////////////////////////////////////

bool __ArmorVnumToShape(int iVnum, uint32_t* pdwShape)
{
    *pdwShape = iVnum;

    /////////////////////////////////////////

    if (0 == iVnum || 1 == iVnum)
        return false;

    CItemData* pItemData;
    if (!CItemManager::Instance().GetItemDataPointer(iVnum, &pItemData))
        return false;

    enum {
        SHAPE_VALUE_SLOT_INDEX = 3,
    };

    *pdwShape = pItemData->GetValue(SHAPE_VALUE_SLOT_INDEX);

    return true;
}

bool CInstanceBase::LessRenderOrder(CInstanceBase* pkInst)
{
    const auto nMainAlpha = (__GetAlphaValue() < 1.0f) ? 1 : 0;
    const auto nTestAlpha = (pkInst->__GetAlphaValue() < 1.0f) ? 1 : 0;
    if (nMainAlpha < nTestAlpha)
        return true;
    if (nMainAlpha > nTestAlpha)
        return false;

    if (GetRace() < pkInst->GetRace())
        return true;
    if (GetRace() > pkInst->GetRace())
        return false;

    if (GetShape() < pkInst->GetShape())
        return true;

    if (GetShape() > pkInst->GetShape())
        return false;

    if (m_awPart[PART_WEAPON].vnum < pkInst->m_awPart[PART_WEAPON].vnum)
        return true;

    return false;
}

bool CInstanceBase::__Background_GetWaterHeight(const TPixelPosition& c_rkPPos,
                                                float* pfHeight)
{
    long lHeight;
    if (!CPythonBackground::Instance().GetWaterHeight(
            int(c_rkPPos.x), int(c_rkPPos.y), &lHeight))
        return false;

    *pfHeight = float(lHeight);

    return true;
}

bool CInstanceBase::__Background_IsWaterPixelPosition(
    const TPixelPosition& c_rkPPos)
{
    return CPythonBackground::Instance().isAttrOn(
        c_rkPPos.x, c_rkPPos.y, CTerrainImpl::ATTRIBUTE_WATER);
}

const float PC_DUST_RANGE = 2000.0f;
const float NPC_DUST_RANGE = 1000.0f;

uint32_t CInstanceBase::ms_dwUpdateCounter = 0;
uint32_t CInstanceBase::ms_dwRenderCounter = 0;
uint32_t CInstanceBase::ms_dwDeformCounter = 0;

bool CInstanceBase::__IsInDustRange()
{
    if (!__IsExistMainInstance())
        return false;

    const auto pkInstMain = __GetMainInstancePtr();

    const auto fDistance = NEW_GetDistanceFromDestInstance(*pkInstMain);

    if (IsPC()) {
        if (fDistance <= PC_DUST_RANGE)
            return true;
    }

    return fDistance <= NPC_DUST_RANGE;
}

uint32_t CInstanceBase::__GetShadowMapColor(float x, float y)
{
    CPythonBackground& rkBG = CPythonBackground::Instance();
    return rkBG.GetShadowMapColor(x, y);
}

float CInstanceBase::__GetBackgroundHeight(float x, float y)
{
    CPythonBackground& rkBG = CPythonBackground::Instance();
    return rkBG.GetHeight(x, y);
}

#ifdef __MOVIE_MODE__

bool CInstanceBase::IsMovieMode()
{
    if (HasAffect(AFFECT_INVISIBILITY))
        return true;

    return false;
}

#endif

bool CInstanceBase::IsInvisibility()
{
    // GM invisibility | Stealth | Recently revived invisibility
    if (HasAffect(AFFECT_INVISIBILITY) || HasAffect(AFFECT_REVIVE_INVISIBLE) ||
        HasAffect(SKILL_EUNHYUNG))
        return true;

    if (IsPet() && Engine::GetSettings().IsHidePets())
        return true;

    if (IsAttackPet() && Engine::GetSettings().IsHidePets())
        return true;

    if (IsMount() && Engine::GetSettings().IsHideMounts())
        return true;

    if (IsShop() && Engine::GetSettings().IsHideShops())
        return true;

    return false;
}

bool CInstanceBase::IsParalysis()
{
    return m_GraphicThingInstance.IsParalysis();
}

void CInstanceBase::SetParalysis(bool val)
{
    m_GraphicThingInstance.SetParalysis(val);
}

bool CInstanceBase::IsGameMaster() const
{
    return m_gmEffect != 0 ||
           CPythonMessenger::instance().IsTeamByName(GetName().c_str());
}

bool CInstanceBase::IsSameEmpire(CInstanceBase& rkInstDst) const
{
    if (0 == rkInstDst.m_dwEmpireID)
        return TRUE;

    if (IsGameMaster())
        return TRUE;

    if (rkInstDst.IsGameMaster())
        return TRUE;

    if (rkInstDst.m_dwEmpireID == m_dwEmpireID)
        return TRUE;

    return FALSE;
}

uint32_t CInstanceBase::GetEmpireID() const
{
    return m_dwEmpireID;
}

uint32_t CInstanceBase::GetGuildID() const
{
    return m_dwGuildID;
}

#if defined(WJ_SHOW_MOB_INFO)
uint32_t CInstanceBase::GetAIFlag() const
{
    return m_dwAIFlag;
}
#endif

int CInstanceBase::GetAlignment() const
{
    return m_sAlignment;
}

UINT CInstanceBase::GetAlignmentGrade() const
{
    if (m_sAlignment == 1000000)
        return 0;

    if (m_sAlignment >= 900000)
        return 1;

    if (m_sAlignment >= 800000)
        return 2;

    if (m_sAlignment >= 700000)
        return 3;

    if (m_sAlignment >= 600000)
        return 4;
    if (m_sAlignment >= 500000)
        return 5;
    if (m_sAlignment >= 400000)
        return 6;
    if (m_sAlignment >= 300000)
        return 7;
    if (m_sAlignment >= 200000)
        return 8;
    if (m_sAlignment >= 100000)
        return 9;
    if (m_sAlignment >= 90000)
        return 10;
    if (m_sAlignment >= 85000)
        return 11;
    if (m_sAlignment >= 75000)
        return 12;
    if (m_sAlignment >= 70000)
        return 13;
    if (m_sAlignment >= 65000)
        return 14;
    if (m_sAlignment >= 60000)
        return 15;
    if (m_sAlignment >= 55000)
        return 16;
    if (m_sAlignment >= 50000)
        return 17;
    if (m_sAlignment >= 45000)
        return 18;
    if (m_sAlignment >= 40000)
        return 19;
    if (m_sAlignment >= 35000)
        return 20;
    if (m_sAlignment >= 30000)
        return 21;
    if (m_sAlignment >= 28000)
        return 22;
    if (m_sAlignment >= 27000)
        return 23;
    if (m_sAlignment >= 26000)
        return 24;
    if (m_sAlignment >= 25000)
        return 25;
    if (m_sAlignment >= 24000)
        return 26;
    if (m_sAlignment >= 23000)
        return 27;
    if (m_sAlignment >= 22000)
        return 28;
    if (m_sAlignment >= 21000)
        return 29;
    if (m_sAlignment >= 20000)
        return 30;
    if (m_sAlignment >= 12000)
        return 31;
    if (m_sAlignment >= 8000)
        return 32;
    if (m_sAlignment >= 4000)
        return 33;
    if (m_sAlignment >= 1000)
        return 34;
    if (m_sAlignment >= 0)
        return 35;
    if (m_sAlignment > -1000)
        return 36;
    if (m_sAlignment > -4000)
        return 37;
    if (m_sAlignment > -8000)
        return 38;
    if (m_sAlignment > -12000)
        return 39;
    if (m_sAlignment > -20000)
        return 40;
    if (m_sAlignment > -25000)
        return 41;
    if (m_sAlignment > -30000)
        return 42;
    if (m_sAlignment > -35000)
        return 43;
    if (m_sAlignment > -40000)
        return 44;
    if (m_sAlignment > -45000)
        return 45;
    if (m_sAlignment > -50000)
        return 46;
    if (m_sAlignment > -55000)
        return 47;
    if (m_sAlignment > -60000)
        return 48;
    if (m_sAlignment > -70000)
        return 49;
    if (m_sAlignment > -80000)
        return 50;
    if (m_sAlignment > -90000)
        return 51;
    if (m_sAlignment > -100000)
        return 52;
    if (m_sAlignment > -200000)
        return 53;
    if (m_sAlignment > -300000)
        return 54;
    if (m_sAlignment > -400000)
        return 55;
    if (m_sAlignment > -500000)
        return 56;
    if (m_sAlignment > -600000)
        return 57;
    if (m_sAlignment > -700000)
        return 58;
    if (m_sAlignment > -800000)
        return 59;
    if (m_sAlignment > -900000)
        return 60;
    if (m_sAlignment >= -1000000)
        return 61;

    return 62;
}

int CInstanceBase::GetAlignmentType() const
{
    const auto grade = GetAlignmentGrade();
    if (grade > 35)
        return ALIGNMENT_TYPE_DARK;

    if (grade < 35)
        return ALIGNMENT_TYPE_WHITE;

    return ALIGNMENT_TYPE_NORMAL;
}

uint8_t CInstanceBase::GetPKMode() const
{
    return m_byPKMode;
}

bool CInstanceBase::IsKiller() const
{
    return m_isKiller;
}

bool CInstanceBase::IsPartyMember() const
{
    return m_isPartyMember;
}

bool CInstanceBase::IsInSafe()
{
    const TPixelPosition& c_rkPPosCur =
        m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
    if (CPythonBackground::Instance().isAttrOn(c_rkPPosCur.x, c_rkPPosCur.y,
                                               CTerrainImpl::ATTRIBUTE_BANPK))
        return TRUE;

    return FALSE;
}

float CInstanceBase::CalculateDistanceSq3d(const TPixelPosition& c_rkPPosDst)
{
    const TPixelPosition& c_rkPPosSrc =
        m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
    return SPixelPosition_CalculateDistanceSq3d(c_rkPPosSrc, c_rkPPosDst);
}

void CInstanceBase::OnSelected()
{
#ifdef __MOVIE_MODE__
    if (!__IsExistMainInstance())
        return;
#endif

    if (IsStoneDoor())
        return;

    if (IsDead())
        return;

    __AttachSelectEffect();
}

void CInstanceBase::OnUnselected()
{
    __DetachSelectEffect();
}

void CInstanceBase::OnTargeted()
{
#ifdef __MOVIE_MODE__
    if (!__IsExistMainInstance())
        return;
#endif

    if (IsStoneDoor())
        return;

    if (IsDead())
        return;

    __AttachTargetEffect();
}

void CInstanceBase::OnUntargeted()
{
    __DetachTargetEffect();
}

void CInstanceBase::SetAlwaysRenderAttachingEffect()
{
    m_GraphicThingInstance.SetAlwaysRenderAttachingEffect();
}

void CInstanceBase::SetTitlePreview(const std::string& title, uint32_t color)
{
    m_titlePreview = title;
    m_titleColorPreview = color;

    RefreshTextTailTitle();
}

void CInstanceBase::SetTitle(const std::string& title, uint32_t color)
{
    m_title = title;
    m_titleColor = color;

    RefreshTextTailTitle();
}

bool CInstanceBase::HasEffect(uint32_t dwEffect)
{
    const auto it = m_attachedEffects.find(dwEffect);
    return m_attachedEffects.end() != it;
}

void CInstanceBase::SetMainInstance()
{
    auto& rkChrMgr = CPythonCharacterManager::Instance();

    const auto dwVID = GetVirtualID();
    rkChrMgr.SetMainInstance(dwVID);

    m_GraphicThingInstance.SetMainInstance();
}

CInstanceBase* CInstanceBase::__GetMainInstancePtr()
{
    return CPythonCharacterManager::Instance().GetMainInstancePtr();
}

void CInstanceBase::__ClearMainInstance()
{
    CPythonCharacterManager::Instance().ClearMainInstance();
}

/* ½ÇÁ¦ ÇÃ·¹ÀÌ¾î Ä³¸¯ÅÍÀÎÁö Á¶»ç.*/
bool CInstanceBase::__IsMainInstance()
{
    return this == __GetMainInstancePtr();
}

bool CInstanceBase::__IsExistMainInstance()
{
    return __GetMainInstancePtr() != nullptr;
}

bool CInstanceBase::__MainCanSeeHiddenThing() const
{
    return __GetMainInstancePtr()->IsGameMaster();
    //	CInstanceBase * pInstance = __GetMainInstancePtr();
    //	return pInstance->IsAffect(AFFECT_GAMJI);
}

float CInstanceBase::__GetBowRange()
{
    float fRange = 2500.0f - 100.0f;

    if (__IsMainInstance()) {
        CPythonPlayer& rPlayer = CPythonPlayer::Instance();
        fRange += float(rPlayer.GetStatus(POINT_BOW_DISTANCE));
        if (m_GraphicThingInstance.m_isUsingQuiver) {
            fRange += 500.0f;
        }
    }

    return fRange;
}

CInstanceBase* CInstanceBase::__FindInstancePtr(uint32_t dwVID)
{
    return CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
}

bool CInstanceBase::__FindRaceType(uint32_t dwRace, uint8_t* pbType)
{
    return NpcManager::Instance().GetInstanceType(dwRace, pbType);
}

bool CInstanceBase::Create(const SCreateData& c_rkCreateData)
{
    SetInstanceType(c_rkCreateData.m_bType);

    if (!SetRace(c_rkCreateData.m_dwRace))
        return false;

    SetVirtualID(c_rkCreateData.m_dwVID);

    const auto proto = NpcManager::Instance().GetTable(c_rkCreateData.m_dwRace);

    if (proto) {
        m_GraphicThingInstance.SetFlag(kActorFlagCannotPush,
                                       (proto->dwAIFlag & AIFLAG_NOMOVE ||
                                        proto->dwAIFlag & AIFLAG_NOPUSH));
    }

    if (c_rkCreateData.m_isMain)
        SetMainInstance();

    if (IsGuildWall()) {
        const float centerZ =
            __GetBackgroundHeight(c_rkCreateData.rotX, c_rkCreateData.rotY);
        NEW_SetPixelPosition(TPixelPosition(float(c_rkCreateData.m_lPosX),
                                            float(c_rkCreateData.m_lPosY),
                                            centerZ));
    } else {
        SCRIPT_SetPixelPosition(float(c_rkCreateData.m_lPosX),
                                float(c_rkCreateData.m_lPosY));
    }

    SetOwnerVid(c_rkCreateData.m_ownerVid);

    SetArmor(c_rkCreateData.m_parts[PART_MAIN]);

    if (IsBuffBot() || IsPC() ||
        (IsNPC() && c_rkCreateData.m_parts[PART_HAIR].vnum))
        SetHair(c_rkCreateData.m_parts[PART_HAIR]);

    if (IsPC() || IsBuffBot()) {
        SetWeapon(c_rkCreateData.m_parts[PART_WEAPON]);
        SetAcce(c_rkCreateData.m_parts[PART_ACCE]);
        SetArrow(c_rkCreateData.m_parts[PART_ARROW]);
        SetPrefixNum(c_rkCreateData.m_parts[PART_RANK]);
        SetWeaponEffect(c_rkCreateData.m_parts[PART_WEAPON_EFFECT]);
        SetArmorEffect(c_rkCreateData.m_parts[PART_BODY_EFFECT]);
        SetWingEffect(c_rkCreateData.m_parts[PART_WING_EFFECT]);
    }

    __Create_SetName(c_rkCreateData);

    m_dwLevel = c_rkCreateData.m_dwLevel;
#if defined(WJ_SHOW_MOB_INFO)
    m_dwAIFlag = c_rkCreateData.m_dwAIFlag;
    m_GraphicThingInstance.SetActorAiFlags(c_rkCreateData.m_dwAIFlag);
#endif
    m_dwGuildID = c_rkCreateData.m_dwGuildID;
    m_langCode = c_rkCreateData.m_langCode;

    m_dwEmpireID = c_rkCreateData.m_dwEmpireID;

    SetTitle(c_rkCreateData.m_pt.title, c_rkCreateData.m_pt.color);

    if (c_rkCreateData.m_scale != 100) {
        const auto fScale = static_cast<float>(c_rkCreateData.m_scale / 100.0f);
        SetScale(fScale);
    } else {
        const auto dwScale = NpcManager::instance().GetScale(m_dwRace);
        if (dwScale != 100) {
            const auto fScale = static_cast<float>(dwScale / 100.0f);
            SetScale(fScale);
        }
    }

    SetRank(NpcManager::instance().GetMobRank(m_dwRace));

    SetVirtualNumber(c_rkCreateData.m_dwRace);
    SetRotation(c_rkCreateData.m_fRot);
#ifdef ENABLE_PLAYTIME_ICON
    SetPlayTime(c_rkCreateData.m_dwPlayTime);
#endif
    SetAlignment(c_rkCreateData.m_sAlignment);
    SetPKMode(c_rkCreateData.m_byPKMode);

    SetMoveSpeed(c_rkCreateData.m_dwMovSpd);
    SetAttackSpeed(c_rkCreateData.m_dwAtkSpd);

    SetGuildLeader(c_rkCreateData.m_isGuildLeader);
    SetGuildGeneral(c_rkCreateData.m_isGuildGeneral);

    // NOTE : Dress ¸¦ ÀÔ°í ÀÖÀ¸¸é Alpha ¸¦ ³ÖÁö ¾Ê´Â´Ù.
    if (!IsWearingDress()) {
        // NOTE : ¹Ýµå½Ã Affect ¼ÂÆÃ À­ÂÊ¿¡ ÀÖ¾î¾ß ÇÔ
        m_GraphicThingInstance.SetAlphaValue(0.0f);
        m_GraphicThingInstance.BlendAlphaValue(1.0f, 0.5f);
    }

    if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_SPAWN) 
    {
        /* AttachEffect(EFFECT_SPAWN_APPEAR, 0, 1, EFFECT_KIND_MISC);
         */
        if (IsPC() || IsBuffBot())
            Refresh(MOTION_WAIT, true);
        else
            Refresh(MOTION_SPAWN, false);
    } else {
        Refresh(MOTION_WAIT, true);
    }

    __AttachEmpireEffect(c_rkCreateData.m_dwEmpireID);

    RegisterBoundingSphere();
    AttachTextTail();
    RefreshTextTail();

    if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_DEAD)
        m_GraphicThingInstance.DieEnd();

    SetStateFlags(c_rkCreateData.m_dwStateFlags);
    SetImmuneFlags(0);

    const auto info = gCharacterEffectRegistry->FindEffect(EFFECT_HIT);
    if (info) {
        m_GraphicThingInstance.SetBattleHitEffect(info->id);
    } else {
        SPDLOG_ERROR("Failed to find effect {0}", EFFECT_HIT);
    }

    if (!IsPC() && !IsBuffBot()) {
        const auto dwBodyColor =
            NpcManager::Instance().GetMonsterColor(c_rkCreateData.m_dwRace);
        if (0 != dwBodyColor) {
            SetAddColor(DirectX::SimpleMath::Color(dwBodyColor));
        }
    }

    if (0 != c_rkCreateData.m_dwMountVnum)
        MountHorse(c_rkCreateData.m_dwMountVnum);

    // ±æµå ½Éº¼À» À§ÇÑ ÀÓ½Ã ÄÚµå, ÀûÁ¤ À§Ä¡¸¦ Ã£´Â Áß
    const int c_iGuildSymbolRace = 14200;
    if (c_iGuildSymbolRace == GetRace()) {
        std::string strFileName = GetGuildSymbolFileName(m_dwGuildID);
        if (IsFile(strFileName.c_str()))
            m_GraphicThingInstance.ChangeMaterial(strFileName.c_str());
    }

    if ((IsStone()) && (GetVirtualNumber() != MELEY_LAIR_DUNGEON_STATUE)) {
        __StoneSmoke_Destroy();
        __StoneSmoke_Create(0);
    }

    return true;
}

void CInstanceBase::__Create_SetName(const SCreateData& c_rkCreateData)
{
    if (IsGoto()) {
        SetNameString("", 0);
        return;
    }
    if (IsWarp()) {
        __Create_SetWarpName(c_rkCreateData);
        return;
    }
    if (IsShop()) {
        const std::string name = c_rkCreateData.m_stName.c_str();
        const auto localeInfo = py::module::import("localeInfo");

        if (name == "Your shop") {
            if (localeInfo) {
                std::string my_shop_name = "Your shop";

                if (py::hasattr(localeInfo, "PRIVATESHOP_YOURSHOP"))
                    my_shop_name = localeInfo.attr("PRIVATESHOP_YOURSHOP")
                                       .cast<std::string>();

                SetNameString(my_shop_name.c_str(), my_shop_name.length());
            }
        } else {
            std::string postfix = " - Shop";

            if (localeInfo) {
                if (py::hasattr(localeInfo, "PRIVATESHOP_POSTFIX")) {
                    postfix = localeInfo.attr("PRIVATESHOP_POSTFIX")
                                  .cast<std::string>();
                }
            }
            const std::string shop_name{name + postfix};

            SetNameString(shop_name.c_str(), shop_name.length());
        }

        return;
    }

    if (IsMount()) {

        std::string postfix = "'s Mount";
        const auto localeInfo = py::module::import("localeInfo");
        if (localeInfo) {
            if (py::hasattr(localeInfo, "MOUNT_POSTIFX")) {
                postfix = localeInfo.attr("MOUNT_POSTIFX").cast<std::string>();
            }
        }

        auto* instance =
            CPythonCharacterManager::instance().GetInstancePtr(m_ownerVid);
        if (instance) {
            m_stName = instance->GetName() + postfix;
        } else {
            m_stName = c_rkCreateData.m_stName;
        }
        return;
    }

    if (IsPet() || IsAttackPet()) {
        std::string postfix = "'s Pet";
        const auto localeInfo = py::module::import("localeInfo");
        if (localeInfo) {
            if (py::hasattr(localeInfo, "PET_POSTIFX")) {
                postfix = localeInfo.attr("PET_POSTIFX").cast<std::string>();
            }
        }

        auto* instance =
            CPythonCharacterManager::instance().GetInstancePtr(m_ownerVid);
        if (instance) {
            m_stName = instance->GetName() + postfix;
        } else {
            m_stName = c_rkCreateData.m_stName;
        }
        return;
    }

    SetNameString(c_rkCreateData.m_stName.c_str(),
                  c_rkCreateData.m_stName.length());
}

void CInstanceBase::__Create_SetWarpName(const SCreateData& c_rkCreateData)
{
    auto c_szName = NpcManager::Instance().GetName(c_rkCreateData.m_dwRace);
    if (c_szName) {
        std::string strName = c_szName.value();
        // Get the string before the last two spaces 1) separating text and
        // number 2) dividing the coordinates.
        strName = strName.substr(
            0, strName.find_last_of(' ', strName.find_last_of(' ') - 1));
        SetNameString(strName.c_str(), strName.length());
    } else {
        SetNameString(c_rkCreateData.m_stName.c_str(),
                      c_rkCreateData.m_stName.length());
    }
}

void CInstanceBase::SetNameString(const char* c_szName, int len)
{
    m_stName.assign(c_szName, len);
}

bool CInstanceBase::SetRace(uint32_t eRace)
{
    m_dwRace = eRace;

    if (!m_GraphicThingInstance.SetRace(eRace))
        return false;

    if (!__FindRaceType(m_dwRace, &m_eRaceType))
        m_eRaceType = CActorInstance::TYPE_PC;

    return true;
}

bool CInstanceBase::__IsChangableWeapon(int iWeaponID) const
{
    if (IsWearingDress()) {
        const int c_iBouquets[] = {
            50201, // Bouquet for Assassin
            50202, // Bouquet for Shaman
            50203, 50204,
            0, // #0000545: [M2CN] 웨딩 드레스와 장비 착용 문제
        };

        for (int i = 0; c_iBouquets[i] != 0; ++i)
            if (iWeaponID == c_iBouquets[i])
                return true;

        return false;
    }

    return true;
}

bool CInstanceBase::IsWearingDress() const
{
    return 201 == m_eShape || 202 == m_eShape;
}

bool CInstanceBase::IsHoldingPickAxe() const
{
    const int c_iPickAxeStart = 29101;
    const int c_iPickAxeEnd =
        29111; // +1 for special mining pickaxe MartPwnS 18.06.2014
    return m_awPart[PART_WEAPON].vnum >= c_iPickAxeStart &&
           m_awPart[PART_WEAPON].vnum <= c_iPickAxeEnd;
}

bool CInstanceBase::IsNewMount() const
{
    return m_kHorse.IsNewMount();
}

bool CInstanceBase::IsMountingHorse() const
{
    return m_kHorse.IsMounting();
}

void CInstanceBase::MountHorse(UINT eRace)
{
    const auto info = gCharacterEffectRegistry->FindEffect(EFFECT_HIT);
    if (!info) {
        SPDLOG_DEBUG("Failed to find effect {0}", EFFECT_HIT);
    }

    // Dismount any previous horse first
    DismountHorse();

    m_kHorse.Create(m_GraphicThingInstance.NEW_GetCurPixelPositionRef(), eRace,
                    info ? info->id : 0);

    SetMotionMode(MOTION_MODE_HORSE);
    SetRotationSpeed(c_fDefaultHorseRotationSpeed);

    m_GraphicThingInstance.MountHorse(m_kHorse.GetActorPtr());
    __AttachHorseSaddle();

    // Necessary because weapon attachments depend on whether we're mounted.
    RefreshWeapon();
    RefreshAcce();

    // TODO: Force motion change?
    RefreshState(MOTION_WAIT, true);
}

void CInstanceBase::DismountHorse()
{
    if (!IsMountingHorse())
        return;

    __DetachHorseSaddle();
    m_GraphicThingInstance.MountHorse(nullptr);
    m_kHorse.Destroy();

    // Necessary because weapon attachments depend on whether we're mounted.
    RefreshWeapon();
    RefreshAcce();

    SetRotationSpeed(c_fDefaultRotationSpeed);

    // TODO(tim): Force motion change?
    RefreshState(MOTION_WAIT, true);
}

void CInstanceBase::ResetPerformanceCounter()
{
    ms_dwUpdateCounter = 0;
    ms_dwRenderCounter = 0;
    ms_dwDeformCounter = 0;
}

bool CInstanceBase::NEW_IsLastPixelPosition()
{
    return m_GraphicThingInstance.IsPushing();
}

const TPixelPosition& CInstanceBase::NEW_GetLastPixelPositionRef()
{
    return m_GraphicThingInstance.NEW_GetLastPixelPositionRef();
}

void CInstanceBase::NEW_SetDstPixelPositionZ(FLOAT z)
{
    m_GraphicThingInstance.NEW_SetDstPixelPositionZ(z);
}

void CInstanceBase::NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst)
{
    m_GraphicThingInstance.NEW_SetDstPixelPosition(c_rkPPosDst);
}

void CInstanceBase::NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosSrc)
{
    m_GraphicThingInstance.NEW_SetSrcPixelPosition(c_rkPPosSrc);
}

const TPixelPosition& CInstanceBase::NEW_GetCurPixelPositionRef()
{
    return m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
}

const TPixelPosition& CInstanceBase::NEW_GetDstPixelPositionRef()
{
    return m_GraphicThingInstance.NEW_GetDstPixelPositionRef();
}

const TPixelPosition& CInstanceBase::NEW_GetSrcPixelPositionRef()
{
    return m_GraphicThingInstance.NEW_GetSrcPixelPositionRef();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CInstanceBase::OnSyncing()
{
    m_GraphicThingInstance.__OnSyncing();
}

void CInstanceBase::OnWaiting()
{
    m_GraphicThingInstance.__OnWaiting();
}

void CInstanceBase::OnMoving()
{
    m_GraphicThingInstance.__OnMoving();
}

void CInstanceBase::ChangeGuild(uint32_t dwGuildID)
{
    m_dwGuildID = dwGuildID;

    DetachTextTail();
    AttachTextTail();
    RefreshTextTail();
}

const Part& CInstanceBase::GetPart(EParts part) const
{
    assert(part >= 0 && part < PART_MAX_NUM);
    return m_awPart[part];
}

uint32_t CInstanceBase::GetShape() const
{
    return m_eShape;
}

bool CInstanceBase::CanAct()
{
    return m_GraphicThingInstance.CanAct();
}

bool CInstanceBase::CanMove()
{
    return m_GraphicThingInstance.CanMove();
}

bool CInstanceBase::CanUseSkill()
{
    if (IsPoly())
        return false;

    if (IsWearingDress())
        return false;

    if (IsHoldingPickAxe())
        return false;

    if (!m_kHorse.CanUseSkill())
        return false;

    if (!m_GraphicThingInstance.CanUseSkill())
        return false;

    return true;
}

bool CInstanceBase::CanAttack()
{
    if (!m_kHorse.CanAttack())
        return false;

    if (IsWearingDress())
        return false;

    if (IsHoldingPickAxe())
        return false;

    return m_GraphicThingInstance.CanAttack();
}

bool CInstanceBase::CanFishing()
{
    return m_GraphicThingInstance.CanFishing();
}

bool CInstanceBase::IsBowMode()
{
    return m_GraphicThingInstance.IsBowMode();
}

bool CInstanceBase::IsHandMode()
{
    return m_GraphicThingInstance.IsHandMode();
}

bool CInstanceBase::IsFishingMode()
{
    if (MOTION_MODE_FISHING == m_GraphicThingInstance.GetMotionMode())
        return true;

    return false;
}

bool CInstanceBase::IsMining()
{
    return m_GraphicThingInstance.IsMining();
}

bool CInstanceBase::IsFishing()
{
    return m_GraphicThingInstance.IsFishing();
}

bool CInstanceBase::IsDead()
{
    return m_GraphicThingInstance.IsDead();
}

bool CInstanceBase::IsStun()
{
    return m_GraphicThingInstance.IsStun();
}

bool CInstanceBase::IsSleep()
{
    return m_GraphicThingInstance.IsSleep();
}

bool CInstanceBase::__IsSyncing()
{
    return m_GraphicThingInstance.__IsSyncing();
}

void CInstanceBase::SetSyncOwner(uint32_t dwVIDOwner)
{
    m_GraphicThingInstance.SetOwner(dwVIDOwner);
}

float CInstanceBase::GetLocalTime()
{
    return m_GraphicThingInstance.GetLocalTime();
}

void CInstanceBase::PushTCPStateExpanded(uint32_t dwCmdTime,
                                         const TPixelPosition& c_rkPPosDst,
                                         float fDstRot, UINT eFunc, UINT uArg,
                                         UINT uTargetVID)
{
    SCommand kCmdNew{};
    kCmdNew.m_kPPosDst = c_rkPPosDst;
    kCmdNew.m_dwChkTime = dwCmdTime + 100;
    kCmdNew.m_dwCmdTime = dwCmdTime;
    kCmdNew.m_fDstRot = fDstRot;
    kCmdNew.m_eFunc = eFunc;
    kCmdNew.m_uArg = uArg;
    kCmdNew.m_uTargetVID = uTargetVID;
    kCmdNew.color = 0;
    m_kQue_kCmdNew.push_back(kCmdNew);
}

void CInstanceBase::PushTCPState(const SNetworkMoveActorData& moveData,
                                 const TPixelPosition& c_rkPPosDst)
{
    if (__IsMainInstance()) {
        // assert(!"CInstanceBase::PushTCPState ÇÃ·¹ÀÌ¾î ÀÚ½Å¿¡°Ô ÀÌµ¿ÆÐÅ¶Àº
        // ¿À¸é ¾ÈµÈ´Ù!");
        SPDLOG_WARN("CInstanceBase::PushTCPState ÇÃ·¹ÀÌ¾î ÀÚ½Å¿¡°Ô ÀÌµ¿ÆÐÅ¶Àº "
                    "¿À¸é ¾ÈµÈ´Ù!");
        return;
    }

    const int nNetworkGap = ELTimer_GetServerFrameMSec() - moveData.m_dwTime;

    m_nAverageNetworkGap = (m_nAverageNetworkGap * 70 + nNetworkGap * 30) / 100;

    SCommand kCmdNew{};
    kCmdNew.m_kPPosDst = c_rkPPosDst;
    kCmdNew.m_dwChkTime = moveData.m_dwTime + m_nAverageNetworkGap;
    kCmdNew.m_dwCmdTime = moveData.m_dwTime;
    kCmdNew.m_fDstRot = moveData.m_fRot;
    kCmdNew.m_eFunc = moveData.m_dwFunc;
    kCmdNew.m_uArg = moveData.m_dwArg;
    kCmdNew.color = moveData.m_skillColor;
    kCmdNew.isMovingSkill = moveData.m_isMovingSkill;
    kCmdNew.loopCount = moveData.m_loopCount;

    m_kQue_kCmdNew.push_back(kCmdNew);
}

void CInstanceBase::PushTCPState(uint32_t dwCmdTime,
                                 const TPixelPosition& c_rkPPosDst,
                                 float fDstRot, UINT eFunc, UINT uArg,
                                 uint32_t color)
{
    if (__IsMainInstance()) {
        // assert(!"CInstanceBase::PushTCPState ÇÃ·¹ÀÌ¾î ÀÚ½Å¿¡°Ô ÀÌµ¿ÆÐÅ¶Àº
        // ¿À¸é ¾ÈµÈ´Ù!");
        SPDLOG_WARN("CInstanceBase::PushTCPState ÇÃ·¹ÀÌ¾î ÀÚ½Å¿¡°Ô ÀÌµ¿ÆÐÅ¶Àº "
                    "¿À¸é ¾ÈµÈ´Ù!");
        return;
    }

    const int nNetworkGap = ELTimer_GetServerFrameMSec() - dwCmdTime;

    m_nAverageNetworkGap = (m_nAverageNetworkGap * 70 + nNetworkGap * 30) / 100;

    /*
    if (m_dwBaseCmdTime == 0)
    {
        m_dwBaseChkTime = ELTimer_GetFrameMSec()-nNetworkGap;
        m_dwBaseCmdTime = dwCmdTime;

        Tracenf("VID[%d] ³×Æ®¿÷°¸ [%d]", GetVirtualID(), nNetworkGap);
    }
    */

    // m_dwBaseChkTime-m_dwBaseCmdTime+ELTimer_GetServerMSec();
    SCommand kCmdNew{};
    kCmdNew.m_kPPosDst = c_rkPPosDst;
    kCmdNew.m_dwChkTime = dwCmdTime + m_nAverageNetworkGap;
    // m_dwBaseChkTime + (dwCmdTime - m_dwBaseCmdTime);// + nNetworkGap;
    kCmdNew.m_dwCmdTime = dwCmdTime;
    kCmdNew.m_fDstRot = fDstRot;
    kCmdNew.m_eFunc = eFunc;
    kCmdNew.m_uArg = uArg;
    kCmdNew.color = color;
    m_kQue_kCmdNew.push_back(kCmdNew);

    // int nApplyGap=kCmdNew.m_dwChkTime-ELTimer_GetServerFrameMSec();

    // if (nApplyGap<-500 || nApplyGap>500)
    //	Tracenf("VID[%d] NAME[%s] ³×Æ®¿÷°¸ [cur:%d ave:%d] ÀÛµ¿½Ã°£ (%d)",
    // GetVirtualID(), GetNameString(), nNetworkGap,
    // m_nAverageNetworkGap, nApplyGap);
}

/*
CInstanceBase::TStateQueue::iterator CInstanceBase::FindSameState(TStateQueue&
rkQuekStt, uint32_t dwCmdTime, UINT eFunc, UINT uArg)
{
    TStateQueue::iterator i=rkQuekStt.begin();
    while (rkQuekStt.end()!=i)
    {
        SState& rkSttEach=*i;
        if (rkSttEach.m_dwCmdTime==dwCmdTime)
            if (rkSttEach.m_eFunc==eFunc)
                if (rkSttEach.m_uArg==uArg)
                    break;
        ++i;
    }

    return i;
}
*/

bool CInstanceBase::__CanProcessNetworkStatePacket()
{
    if (m_GraphicThingInstance.IsDead())
        return FALSE;
    if (m_GraphicThingInstance.IsKnockDown())
        return FALSE;
    if (m_GraphicThingInstance.IsUsingSkill())
        if (!m_GraphicThingInstance.CanCancelSkill())
            return FALSE;

    return TRUE;
}

bool CInstanceBase::__IsEnableTCPProcess(UINT eCurFunc)
{
    if (m_GraphicThingInstance.IsActEmotion()) {
        return FALSE;
    }

    if (!m_bEnableTCPState) {
        if (FUNC_EMOTION != eCurFunc) {
            return FALSE;
        }
    }

    return TRUE;
}

void CInstanceBase::StateProcess()
{
    while (true) {
        if (m_kQue_kCmdNew.empty())
            return;

        uint32_t dwDstChkTime = m_kQue_kCmdNew.front().m_dwChkTime;
        uint32_t dwCurChkTime = ELTimer_GetServerFrameMSec();

        SCommand kCmdTop = m_kQue_kCmdNew.front();
        m_kQue_kCmdNew.pop_front();

        TPixelPosition kPPosDst = kCmdTop.m_kPPosDst;
        // uint32_t dwCmdTime = kCmdTop.m_dwCmdTime;
        FLOAT fRotDst = kCmdTop.m_fDstRot;
        UINT eFunc = kCmdTop.m_eFunc;
        UINT uArg = kCmdTop.m_uArg;
        auto isMovingSkill = kCmdTop.isMovingSkill;
        auto loopCount = kCmdTop.loopCount;

        UINT uVID = GetVirtualID();
        UINT uTargetVID = kCmdTop.m_uTargetVID;
        uint32_t motionColor = kCmdTop.color;

        TPixelPosition kPPosCur;
        NEW_GetPixelPosition(&kPPosCur);

        /*
        if (IsPC())
            Tracenf("%d cmd: vid=%d[%s] func=%d arg=%d  curPos=(%f, %f)
        dstPos=(%f, %f) rot=%f (time %d)", ELTimer_GetMSec(), uVID,
        m_stName.c_str(), eFunc, uArg, kPPosCur.x, kPPosCur.y, kPPosDst.x,
        kPPosDst.y, fRotDst, dwCmdTime-m_dwBaseCmdTime);
        */
        Vector3 cur(NEW_GetCurPixelPositionRef().x,
                    NEW_GetCurPixelPositionRef().y, 0.0f);
        Vector3 dst(kCmdTop.m_kPPosDst.x, kCmdTop.m_kPPosDst.y, 0.0f);

        const auto fDirLen = (dst - cur).Length();

        if (!__CanProcessNetworkStatePacket()) {
            SPDLOG_DEBUG("vid={0} Skip to the immovable state IsDead={1}, "
                         "IsKnockDown={2}",
                         uVID, m_GraphicThingInstance.IsDead(),
                         m_GraphicThingInstance.IsKnockDown());
            return;
        }

        if (!__IsEnableTCPProcess(eFunc)) {
            return;
        }

        switch (eFunc) {
            case FUNC_WAIT: {
                // Tracenf("%s (%f, %f) -> (%f, %f) 남은거리 %f",
                // GetNameString(), kPPosCur.x, kPPosCur.y, kPPosDst.x,
                // kPPosDst.y, fDirLen);
                if (fDirLen > 1.0f) {
                    // NEW_GetSrcPixelPositionRef() = kPPosCur;
                    // NEW_GetDstPixelPositionRef() = kPPosDst;
                    NEW_SetSrcPixelPosition(kPPosCur);
                    NEW_SetDstPixelPosition(kPPosDst);

                    EnableSkipCollision();

                    m_fDstRot = fRotDst;
                    m_isGoing = TRUE;

                    m_kMovAfterFunc.eFunc = FUNC_WAIT;

                    if (!IsWalking())
                        StartWalking();

                    // Tracen("목표정지");
                } else {
                    // Tracen("현재 정지");

                    m_isGoing = FALSE;

                    if (!IsWaiting())
                        EndWalking();

                    SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
                    SetAdvancingRotation(fRotDst);
                    SetRotation(fRotDst);
                }
                break;
            }

            case FUNC_MOVE: {
                // NEW_GetSrcPixelPositionRef() = kPPosCur;
                // NEW_GetDstPixelPositionRef() = kPPosDst;
                NEW_SetSrcPixelPosition(kPPosCur);
                NEW_SetDstPixelPosition(kPPosDst);
                m_fDstRot = fRotDst;
                m_isGoing = TRUE;
                EnableSkipCollision();
                // m_isSyncMov = TRUE;

                m_kMovAfterFunc.eFunc = FUNC_MOVE;

                if (!IsWalking()) {
                    // Tracen("걷고 있지 않아 걷기 시작");
                    StartWalking();
                } else {
                    // Tracen("이미 걷는중 ");
                }
                break;
            }

            case FUNC_COMBO: {
                if (fDirLen >= 50.0f) {
                    NEW_SetSrcPixelPosition(kPPosCur);
                    NEW_SetDstPixelPosition(kPPosDst);
                    m_fDstRot = fRotDst;
                    m_isGoing = TRUE;
                    EnableSkipCollision();

                    m_kMovAfterFunc.eFunc = FUNC_COMBO;
                    m_kMovAfterFunc.uArg = uArg;

                    if (!IsWalking())
                        StartWalking();
                } else {
                    // Tracen("대기 공격 정지");

                    m_isGoing = FALSE;

                    if (IsWalking())
                        EndWalking();

                    SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
                    RunComboAttack(fRotDst, uArg);
                }
                break;
            }

            case FUNC_ATTACK: {
                if (fDirLen >= 50.0f) {
                    // NEW_GetSrcPixelPositionRef() = kPPosCur;
                    // NEW_GetDstPixelPositionRef() = kPPosDst;
                    NEW_SetSrcPixelPosition(kPPosCur);
                    NEW_SetDstPixelPosition(kPPosDst);
                    m_fDstRot = fRotDst;
                    m_isGoing = TRUE;
                    EnableSkipCollision();
                    // m_isSyncMov = TRUE;

                    m_kMovAfterFunc.eFunc = FUNC_ATTACK;

                    if (!IsWalking())
                        StartWalking();

                    // Tracen("너무 멀어서 이동 후 공격");
                } else {
                    // Tracen("노말 공격 정지");

                    m_isGoing = FALSE;

                    if (IsWalking())
                        EndWalking();

                    SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
                    BlendRotation(fRotDst);

                    RunNormalAttack(fRotDst);

                    // Tracen("가깝기 때문에 워프 공격");
                }
                break;
            }

            case FUNC_MOB_SKILL: {
                if (fDirLen >= 50.0f) {
                    NEW_SetSrcPixelPosition(kPPosCur);
                    NEW_SetDstPixelPosition(kPPosDst);
                    m_fDstRot = fRotDst;
                    m_isGoing = TRUE;
                    EnableSkipCollision();

                    m_kMovAfterFunc.eFunc = FUNC_MOB_SKILL;
                    m_kMovAfterFunc.uArg = uArg;

                    if (!IsWalking())
                        StartWalking();
                } else {
                    m_isGoing = FALSE;

                    if (IsWalking())
                        EndWalking();

                    SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
                    BlendRotation(fRotDst);

                    m_GraphicThingInstance.InterceptOnceMotion(
                        MOTION_SPECIAL_1 + uArg);
                }
                break;
            }

            case FUNC_EMOTION: {
                if (fDirLen > 100.0f) {
                    NEW_SetSrcPixelPosition(kPPosCur);
                    NEW_SetDstPixelPosition(kPPosDst);
                    m_fDstRot = fRotDst;
                    m_isGoing = TRUE;

                    if (__IsMainInstance())
                        EnableSkipCollision();

                    m_kMovAfterFunc.eFunc = FUNC_EMOTION;
                    m_kMovAfterFunc.uArg = uArg;
                    m_kMovAfterFunc.uArgExpanded = uTargetVID;
                    m_kMovAfterFunc.kPosDst = kPPosDst;

                    if (!IsWalking())
                        StartWalking();
                } else {
                    __ProcessFunctionEmotion(uArg, uTargetVID, kPPosDst);
                }
                break;
            }
            case FUNC_SKILL: {
                if (fDirLen >= 50.0f) {
                    // NEW_GetSrcPixelPositionRef() = kPPosCur;
                    // NEW_GetDstPixelPositionRef() = kPPosDst;
                    NEW_SetSrcPixelPosition(kPPosCur);
                    NEW_SetDstPixelPosition(kPPosDst);
                    m_fDstRot = fRotDst;
                    m_isGoing = TRUE;
                    // m_isSyncMov = TRUE;
                    EnableSkipCollision();

                    m_kMovAfterFunc.eFunc = eFunc;
                    m_kMovAfterFunc.uArg = uArg;
                    m_kMovAfterFunc.isMovingSkill = isMovingSkill;
                    m_kMovAfterFunc.loopCount = loopCount;
                    if (!IsWalking())
                        StartWalking();

                    // Tracen("너무 멀어서 이동 후 공격");
                } else {
                    // Tracen("스킬 정지");

                    m_isGoing = FALSE;

                    if (IsWalking())
                        EndWalking();

                    SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
                    SetAdvancingRotation(fRotDst);
                    SetRotation(fRotDst);

                    // TraceError("NEW_UseSkill motionIdx %d motLoopCount %d
                    // motLoop %d", uArg>>8, uArg&0x0f,
                    // ((uArg>>4)&1) ? 1 : 0);
                    NEW_UseSkill(0, uArg, loopCount, isMovingSkill,
                                 motionColor);
                    // Tracen("가깝기 때문에 워프 공격");
                }

            } break;
        }
    }
}

void CInstanceBase::MovementProcess()
{
    TPixelPosition kPPosCur;
    NEW_GetPixelPosition(&kPPosCur);

    // 렌더링 좌표계이므로 y를 -화해서 더한다.

    TPixelPosition kPPosNext;
    {
        const Vector3& c_rkV3Mov =
            m_GraphicThingInstance.GetMovementVectorRef();

        kPPosNext.x = kPPosCur.x + (+c_rkV3Mov.x);
        kPPosNext.y = kPPosCur.y + (-c_rkV3Mov.y);
        kPPosNext.z = kPPosCur.z + (+c_rkV3Mov.z);
    }

    TPixelPosition kPPosDeltaSC = kPPosCur - NEW_GetSrcPixelPositionRef();
    TPixelPosition kPPosDeltaSN = kPPosNext - NEW_GetSrcPixelPositionRef();
    TPixelPosition kPPosDeltaSD =
        NEW_GetDstPixelPositionRef() - NEW_GetSrcPixelPositionRef();

    float fCurLen = sqrtf(kPPosDeltaSC.x * kPPosDeltaSC.x +
                          kPPosDeltaSC.y * kPPosDeltaSC.y);
    float fNextLen = sqrtf(kPPosDeltaSN.x * kPPosDeltaSN.x +
                           kPPosDeltaSN.y * kPPosDeltaSN.y);
    float fTotalLen = sqrtf(kPPosDeltaSD.x * kPPosDeltaSD.x +
                            kPPosDeltaSD.y * kPPosDeltaSD.y);
    float fRestLen = fTotalLen - fCurLen;

    if (__IsMainInstance()) {
        if (m_isGoing && IsWalking()) {
            float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(
                NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

            SetAdvancingRotation(fDstRot);

            if (fRestLen <= 0.0) {
                if (IsWalking())
                    EndWalking();

                // Tracen("목표 도달 정지");

                m_isGoing = FALSE;

                BlockMovement();

                if (FUNC_EMOTION == m_kMovAfterFunc.eFunc) {
                    DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
                    DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
                    __ProcessFunctionEmotion(dwMotionNumber, dwTargetVID,
                                             m_kMovAfterFunc.kPosDst);
                    m_kMovAfterFunc.eFunc = FUNC_WAIT;
                    return;
                }
            }
        }
    } else {
        if (m_isGoing && IsWalking()) {
            float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(
                NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

            SetAdvancingRotation(fDstRot);

            // 만약 렌턴시가 늦어 너무 많이 이동했다면..
            if (fRestLen < -100.0f) {
                NEW_SetSrcPixelPosition(kPPosCur);

                float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(
                    kPPosCur, NEW_GetDstPixelPositionRef());
                SetAdvancingRotation(fDstRot);
                // Tracenf("VID %d 오버 방향설정 (%f, %f) %f rest %f",
                // GetVirtualID(), kPPosCur.x, kPPosCur.y, fDstRot, fRestLen);

                // 이동중이라면 다음번에 멈추게 한다
                if (FUNC_MOVE == m_kMovAfterFunc.eFunc) {
                    m_kMovAfterFunc.eFunc = FUNC_WAIT;
                }
            }
            // 도착했다면...
            else if (fCurLen <= fTotalLen && fTotalLen <= fNextLen) {
                if (m_GraphicThingInstance.IsDead() ||
                    m_GraphicThingInstance.IsKnockDown()) {
                    DisableSkipCollision();

                    // Tracen("사망 상태라 동작 스킵");

                    m_isGoing = FALSE;

                    // Tracen("행동 불능 상태라 이후 동작 스킵");
                } else {
                    switch (m_kMovAfterFunc.eFunc) {
                        case FUNC_ATTACK: {
                            if (IsWalking())
                                EndWalking();

                            DisableSkipCollision();
                            m_isGoing = FALSE;

                            BlockMovement();
                            SCRIPT_SetPixelPosition(
                                NEW_GetDstPixelPositionRef().x,
                                NEW_GetDstPixelPositionRef().y);
                            SetAdvancingRotation(m_fDstRot);
                            SetRotation(m_fDstRot);

                            RunNormalAttack(m_fDstRot);
                            break;
                        }

                        case FUNC_COMBO: {
                            if (IsWalking())
                                EndWalking();

                            DisableSkipCollision();
                            m_isGoing = FALSE;

                            BlockMovement();
                            SCRIPT_SetPixelPosition(
                                NEW_GetDstPixelPositionRef().x,
                                NEW_GetDstPixelPositionRef().y);
                            RunComboAttack(m_fDstRot, m_kMovAfterFunc.uArg);
                            break;
                        }

                        case FUNC_EMOTION: {
                            m_isGoing = FALSE;
                            m_kMovAfterFunc.eFunc = FUNC_WAIT;
                            DisableSkipCollision();
                            BlockMovement();

                            DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
                            DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
                            __ProcessFunctionEmotion(dwMotionNumber,
                                                     dwTargetVID,
                                                     m_kMovAfterFunc.kPosDst);
                            break;
                        }

                        case FUNC_MOVE: {
                            break;
                        }

                        case FUNC_MOB_SKILL: {
                            if (IsWalking())
                                EndWalking();

                            DisableSkipCollision();
                            m_isGoing = FALSE;

                            BlockMovement();
                            SCRIPT_SetPixelPosition(
                                NEW_GetDstPixelPositionRef().x,
                                NEW_GetDstPixelPositionRef().y);
                            SetAdvancingRotation(m_fDstRot);
                            SetRotation(m_fDstRot);

                            m_GraphicThingInstance.InterceptOnceMotion(
                                MOTION_SPECIAL_1 + m_kMovAfterFunc.uArg);
                            break;
                        }
                        case FUNC_SKILL: {
                            SetAdvancingRotation(m_fDstRot);
                            BlendRotation(m_fDstRot);
                            NEW_UseSkill(0, m_kMovAfterFunc.uArg,
                                         m_kMovAfterFunc.loopCount,
                                         m_kMovAfterFunc.isMovingSkill,
                                         m_kMovAfterFunc.color);
                        } break;

                        default: {
                            DisableSkipCollision();
                            m_isGoing = FALSE;

                            BlockMovement();
                            SCRIPT_SetPixelPosition(
                                NEW_GetDstPixelPositionRef().x,
                                NEW_GetDstPixelPositionRef().y);
                            SetAdvancingRotation(m_fDstRot);
                            BlendRotation(m_fDstRot);
                            if (!IsWaiting()) {
                                EndWalking();
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill()) {
        float fRotation = m_GraphicThingInstance.GetRotation();
        float fAdvancingRotation =
            m_GraphicThingInstance.GetAdvancingRotation();
        int iDirection = GetRotatingDirection(fRotation, fAdvancingRotation);

        if (DEGREE_DIRECTION_SAME != m_iRotatingDirection) {
            if (DEGREE_DIRECTION_LEFT == iDirection) {
                fRotation = fmodf(
                    fRotation +
                        m_fRotSpd * m_GraphicThingInstance.GetSecondElapsed(),
                    360.0f);
            } else if (DEGREE_DIRECTION_RIGHT == iDirection) {
                fRotation = fmodf(
                    fRotation -
                        m_fRotSpd * m_GraphicThingInstance.GetSecondElapsed() +
                        360.0f,
                    360.0f);
            }

            if (m_iRotatingDirection !=
                GetRotatingDirection(fRotation, fAdvancingRotation)) {
                m_iRotatingDirection = DEGREE_DIRECTION_SAME;
                fRotation = fAdvancingRotation;
            }

            m_GraphicThingInstance.SetRotation(fRotation);
        }

        if (__IsInDustRange()) {
            float fDustDistance =
                NEW_GetDistanceFromDestPixelPosition(m_kPPosDust);
            if (IsMountingHorse()) {
                if (fDustDistance > ms_fHorseDustGap) {
                    NEW_GetPixelPosition(&m_kPPosDust);
                    AttachEffect(EFFECT_HORSE_DUST, 0, 1, EFFECT_KIND_MISC);
                }
            } else {
                if (fDustDistance > ms_fDustGap) {
                    NEW_GetPixelPosition(&m_kPPosDust);
                    AttachEffect(EFFECT_DUST, 0, 1, EFFECT_KIND_MISC);
                }
            }
        }
    }
}

void CInstanceBase::__ProcessFunctionEmotion(uint32_t dwMotionNumber,
                                             uint32_t dwTargetVID,
                                             const TPixelPosition& c_rkPosDst)
{
    if (IsWalking())
        EndWalkingWithoutBlending();

    __EnableChangingTCPState();
    SCRIPT_SetPixelPosition(c_rkPosDst.x, c_rkPosDst.y);

    CInstanceBase* pTargetInstance =
        CPythonCharacterManager::Instance().GetInstancePtr(dwTargetVID);
    if (pTargetInstance) {
        pTargetInstance->__EnableChangingTCPState();

        if (pTargetInstance->IsWalking())
            pTargetInstance->EndWalkingWithoutBlending();

        uint16_t wMotionNumber1 = HIWORD(dwMotionNumber);
        uint16_t wMotionNumber2 = LOWORD(dwMotionNumber);

        int src_job = GetJobByRace(GetRace());
        int dst_job = GetJobByRace(pTargetInstance->GetRace());

        NEW_LookAtDestInstance(*pTargetInstance);
        m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber1 + dst_job);
        m_GraphicThingInstance.SetRotation(
            m_GraphicThingInstance.GetTargetRotation());
        m_GraphicThingInstance.SetAdvancingRotation(
            m_GraphicThingInstance.GetTargetRotation());

        pTargetInstance->NEW_LookAtDestInstance(*this);
        pTargetInstance->m_GraphicThingInstance.InterceptOnceMotion(
            wMotionNumber2 + src_job);
        pTargetInstance->m_GraphicThingInstance.SetRotation(
            pTargetInstance->m_GraphicThingInstance.GetTargetRotation());
        pTargetInstance->m_GraphicThingInstance.SetAdvancingRotation(
            pTargetInstance->m_GraphicThingInstance.GetTargetRotation());

        if (pTargetInstance->__IsMainInstance()) {
            auto& rPlayer = CPythonPlayer::Instance();
            rPlayer.EndEmotionProcess();
            pTargetInstance->DisableSkipCollision();
        }
    }

    if (__IsMainInstance()) {
        auto& rPlayer = CPythonPlayer::Instance();
        rPlayer.EndEmotionProcess();
        DisableSkipCollision();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update & Deform & Render

int g_iAccumulationTime = 0;

void CInstanceBase::UpdateHideStatus(bool isType, bool isHide)
{
    if (isType && isHide) {
        m_GraphicThingInstance.HideAllAttachingEffect();
        if (m_GraphicThingInstance.isShow()) {
            m_GraphicThingInstance.EnableForceHide();
            m_GraphicThingInstance.Hide();
            DetachTextTail();
            Hide();
        }
    } else if (isType && !isHide) {
        m_GraphicThingInstance.ShowAllAttachingEffect();
        if (!m_GraphicThingInstance.isShow()) {
            m_GraphicThingInstance.DisableForceHide();
            m_GraphicThingInstance.Show();
            Show();
            AttachTextTail();
            RefreshTextTail();
        }
    }
}

void CInstanceBase::UpdateOptions()
{
    if (HasAffect(SKILL_EUNHYUNG) || HasAffect(AFFECT_INVISIBILITY)) {
        if (!CPythonPlayer::Instance().IsMainCharacterIndex(GetVirtualID())) {
            GetGraphicThingInstanceRef().HideAllAttachingEffect();
        }
    }

    if (!HasAffect(SKILL_EUNHYUNG) && !HasAffect(AFFECT_INVISIBILITY)) {
        if (m_GraphicThingInstance.IsEmotionMotion() || IsWearingDress()) {
            if (m_hasWeapon)
                __HideWeaponRefineEffect();
        } else {
            if (m_hasWeapon)
                __ShowWeaponRefineEffect();
        }
    }

    UpdateHideStatus(IsAttackPet(), Engine::GetSettings().IsHidePets());
    UpdateHideStatus(IsPet(), Engine::GetSettings().IsHidePets());
    UpdateHideStatus(IsMount(), Engine::GetSettings().IsHideMounts());
    UpdateHideStatus(IsShop(), Engine::GetSettings().IsHideShops());

    if (IsMount() && !m_appendedOwnerName) {

        std::string postfix = "'s Mount";
        const auto localeInfo = py::module::import("localeInfo");
        if (localeInfo) {
            if (py::hasattr(localeInfo, "MOUNT_POSTIFX")) {
                postfix = localeInfo.attr("MOUNT_POSTIFX").cast<std::string>();
            }
        }

        auto* instance =
            CPythonCharacterManager::instance().GetInstancePtr(m_ownerVid);
        if (instance) {
            m_stName = instance->GetName() + postfix;
            CPythonTextTail::instance().UpdateCharacterName(GetVirtualID(),
                                                            m_stName);
            m_appendedOwnerName = TRUE;
        }
    }

    if ((IsPet() || IsAttackPet()) && !m_appendedOwnerName) {
        std::string postfix = "'s Pet";
        const auto localeInfo = py::module::import("localeInfo");
        if (localeInfo) {
            if (py::hasattr(localeInfo, "PET_POSTIFX")) {
                postfix = localeInfo.attr("PET_POSTIFX").cast<std::string>();
            }
        }

        auto* instance =
            CPythonCharacterManager::instance().GetInstancePtr(m_ownerVid);
        if (instance) {
            m_stName = instance->GetName() + postfix;
            CPythonTextTail::instance().UpdateCharacterName(GetVirtualID(),
                                                            m_stName);
            m_appendedOwnerName = TRUE;
        }
    }

    if (IsPet() || IsMount() || IsAttackPet()) {
        if (IsInvisibility()) {
            __EffectContainer_DeactiveteEffect(EFFECT_EMPIRE + EMPIRE_A);
            __EffectContainer_DeactiveteEffect(EFFECT_EMPIRE + EMPIRE_B);
            __EffectContainer_DeactiveteEffect(EFFECT_EMPIRE + EMPIRE_C);
        } else {
            __EffectContainer_ActiveEffect(EFFECT_EMPIRE + EMPIRE_A);
            __EffectContainer_ActiveEffect(EFFECT_EMPIRE + EMPIRE_B);
            __EffectContainer_ActiveEffect(EFFECT_EMPIRE + EMPIRE_C);
        }
    }
    /*
 *
    if (Engine::GetSettings().IsShowShining())
    {
        if(!IsShowingArmorShiningEffect())
            __ShowArmorShiningEffect();
        if (!IsShowingWeaponShiningEffect())
            __ShowWeaponShiningEffect();

        if(IsEnemy() || IsStone())
        {
            if (!m_GraphicThingInstance.IsShowMinorAttachingEffect())
                m_GraphicThingInstance.ShowMinorAttachingEffect();
        }
    }
    else
    {
        if (IsShowingArmorShiningEffect())
            __HideArmorShiningEffect();
        if (IsShowingWeaponShiningEffect())
            __HideWeaponShiningEffect();

        if (IsEnemy() || IsStone())
        {
            if (m_GraphicThingInstance.IsShowMinorAttachingEffect())
                m_GraphicThingInstance.HideMinorAttachingEffect();
        }
    }
*/
}

void CInstanceBase::Update()
{
    ++ms_dwUpdateCounter;

    m_GraphicThingInstance.Update();

    StateProcess();
    m_GraphicThingInstance.PhysicsProcess();
    m_GraphicThingInstance.RotationProcess();
    m_GraphicThingInstance.ComboProcess();
    m_GraphicThingInstance.AccumulationMovement();

    if (m_GraphicThingInstance.IsMovement()) {
        TPixelPosition kPPosCur;
        NEW_GetPixelPosition(&kPPosCur);

        kPPosCur.z = __GetBackgroundHeight(kPPosCur.x, kPPosCur.y);
        NEW_SetPixelPosition(kPPosCur);

        // SetMaterialColor
        {
            DWORD dwMtrlColor = __GetShadowMapColor(kPPosCur.x, kPPosCur.y);
            m_GraphicThingInstance.SetMaterialColor(dwMtrlColor);
        }
    }

    m_GraphicThingInstance.UpdateAdvancingPointInstance();

    AttackProcess();
    MovementProcess();

    m_GraphicThingInstance.MotionProcess(IsPC());

    if (IsMountingHorse()) {
        m_kHorse.m_pkActor->HORSE_MotionProcess(FALSE);
    }

    __ComboProcess();

    ProcessDamage();

    UpdateOptions();
}

void CInstanceBase::Transform()
{
    if (__IsSyncing()) {
        OnSyncing();
    } else {
        if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill()) {
            const Vector3& c_rv3Movment =
                m_GraphicThingInstance.GetMovementVectorRef();

            float len = (c_rv3Movment.x * c_rv3Movment.x) +
                        (c_rv3Movment.y * c_rv3Movment.y);
            if (len > 1.0f)
                OnMoving();
            else
                OnWaiting();
        }
    }

    m_GraphicThingInstance.Transform();
}

void CInstanceBase::Deform()
{
    // 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
    if (!__CanRender())
        return;

    rmt_ScopedCPUSample(InstanceDeform, RMTSF_Aggregate);
    ++ms_dwDeformCounter;

    m_GraphicThingInstance.Deform();

    m_kHorse.Deform();
}

void CInstanceBase::RenderTrace()
{
    if (!__CanRender())
        return;

    m_GraphicThingInstance.RenderTrace();
}

void CInstanceBase::RenderAttachingEffect()
{
    m_GraphicThingInstance.RenderAttachingEffect();
}

void CInstanceBase::Render()
{
    // 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
    if (!__CanRender())
        return;

    rmt_ScopedCPUSample(InstanceRender, RMTSF_Aggregate);

    ++ms_dwRenderCounter;

    m_kHorse.Render();
    m_GraphicThingInstance.Render();

    if (CActorInstance::IsDirLine()) {
        if (NEW_GetDstPixelPositionRef().x != 0.0f) {
            static CScreen s_kScreen;
            uint32_t dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);

            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,
                                              D3DTA_DIFFUSE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,
                                              D3DTOP_SELECTARG1);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,
                                              D3DTOP_DISABLE);
            Engine::GetDevice().SetDepthEnable(false, false);
            STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
            DWORD lighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);

            STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

            TPixelPosition px;
            m_GraphicThingInstance.GetPixelPosition(&px);
            Vector3 kD3DVt3Cur(px.x, px.y, px.z);
            // Vector3 kD3DVt3Cur(NEW_GetSrcPixelPositionRef().x,
            // -NEW_GetSrcPixelPositionRef().y, NEW_GetSrcPixelPositionRef().z);
            Vector3 kD3DVt3Dest(NEW_GetDstPixelPositionRef().x,
                                -NEW_GetDstPixelPositionRef().y,
                                NEW_GetDstPixelPositionRef().z);

            // printf("%s %f\n", GetNameString(), kD3DVt3Cur.y - kD3DVt3Dest.y);
            // float fdx = NEW_GetDstPixelPositionRef().x -
            // NEW_GetSrcPixelPositionRef().x; float fdy =
            // NEW_GetDstPixelPositionRef().y - NEW_GetSrcPixelPositionRef().y;

            s_kScreen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
            s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, px.z,
                                   kD3DVt3Dest.x, kD3DVt3Dest.y, px.z);

            Engine::GetDevice().SetDepthEnable(true, true);

            STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);
            STATEMANAGER.SetRenderState(D3DRS_LIGHTING, lighting);
        }
    }
}

void CInstanceBase::RenderToShadowMap(bool showWeapon)
{
    if (IsDoor())
        return;

    if (IsBuilding())
        return;

    if (!__CanRender())
        return;

    if (!__IsExistMainInstance())
        return;

    CInstanceBase* pkInstMain = __GetMainInstancePtr();

    const float SHADOW_APPLY_DISTANCE = 50000.0f;

    float fDistance = NEW_GetDistanceFromDestInstance(*pkInstMain);
    if (fDistance >= SHADOW_APPLY_DISTANCE)
        return;

    m_GraphicThingInstance.RenderToShadowMap();
}

void CInstanceBase::RenderCollision()
{
    m_GraphicThingInstance.RenderCollisionData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Setting & Getting Data

void CInstanceBase::SetVirtualID(uint32_t dwVirtualID)
{
    m_GraphicThingInstance.SetVirtualID(dwVirtualID);
}

void CInstanceBase::SetVirtualNumber(uint32_t dwVirtualNumber)
{
    m_dwVirtualNumber = dwVirtualNumber;
}

void CInstanceBase::SetInstanceType(int iInstanceType)
{
    m_GraphicThingInstance.SetActorType(iInstanceType);
}

void CInstanceBase::SetAlignment(PlayerAlignment sAlignment)
{
    m_sAlignment = sAlignment;
    RefreshTextTailTitle();
}

void CInstanceBase::SetLevel(uint32_t level)
{
    if (m_dwLevel == level)
        return;

    m_dwLevel = level;

    if (m_dwLevel)
        UpdateTextTailLevel(m_dwLevel);
}

void CInstanceBase::SetPKMode(uint8_t byPKMode)
{
    if (m_byPKMode == byPKMode)
        return;

    m_byPKMode = byPKMode;

    if (__IsMainInstance()) {
        auto& rPlayer = CPythonPlayer::Instance();
        rPlayer.NotifyChangePKMode();
    }

    RefreshWeapon();
    RefreshAcce();
}

void CInstanceBase::SetKiller(bool bFlag)
{
    if (m_isKiller == bFlag)
        return;

    m_isKiller = bFlag;
    RefreshTextTail();
    RefreshWeapon();
    RefreshAcce();
}

void CInstanceBase::SetPartyMemberFlag(bool bFlag)
{
    m_isPartyMember = bFlag;
}

void CInstanceBase::SetStateFlags(uint32_t dwStateFlags)
{
    m_stateFlags = dwStateFlags;

    if (dwStateFlags & ADD_CHARACTER_STATE_KILLER)
        SetKiller(true);
    else
        SetKiller(false);

    if (dwStateFlags & ADD_CHARACTER_STATE_PARTY)
        SetPartyMemberFlag(true);
    else
        SetPartyMemberFlag(false);

    if (m_gmEffect) {
        DetachEffect(m_gmEffect);
        m_gmEffect = 0;
    }

    if (dwStateFlags & ADD_CHARACTER_STATE_GM)
        m_gmEffect = AttachEffect(EFFECT_GM, 0, 1, EFFECT_KIND_SPECIAL_EFFECT);

    if (dwStateFlags & ADD_CHARACTER_STATE_SGM)
        m_gmEffect = AttachEffect(EFFECT_SGM, 0, 1, EFFECT_KIND_SPECIAL_EFFECT);

    if (dwStateFlags & ADD_CHARACTER_STATE_SA)
        m_gmEffect = AttachEffect(EFFECT_SA, 0, 1, EFFECT_KIND_SPECIAL_EFFECT);
}

void CInstanceBase::SetImmuneFlags(uint32_t immuneFlags)
{
    m_immuneFlags = immuneFlags;

    m_GraphicThingInstance.SetResistFallen(HasAffect(SKILL_CHUNKEON));
}

void CInstanceBase::SetComboType(UINT uComboType)
{
    m_GraphicThingInstance.SetComboType(uComboType);
}

const char* CInstanceBase::GetNameString() const
{
    return m_stName.c_str();
}

const std::string& CInstanceBase::GetName() const
{
    return m_stName;
}

#ifdef ENABLE_PLAYTIME_ICON
void CInstanceBase::SetPlayTime(DWORD dwPlayTime)
{
    m_dwPlayTime = dwPlayTime;
    RefreshTextTailTitle();
}
DWORD CInstanceBase::GetPlayTime() const
{
    return m_dwPlayTime;
}
const char* CInstanceBase::GetMyTimeIcon() const
{
    const auto pHour = GetPlayTime() / 24;
    auto rutbe = pHour / 13;

    if (rutbe >= 55) {
        rutbe = 55;
    }

    if (rutbe == 0)
        rutbe = 1;

    auto time_image = fmt::format("d:/ymir work/effect/etc/rutbeler/rutbe_{}.png", rutbe);
    return time_image.c_str();
}
#endif

uint32_t CInstanceBase::GetRace() const
{
    return m_dwRace;
}

bool CInstanceBase::IsConflictAlignmentInstance(
    CInstanceBase& rkInstVictim) const
{
    if (PK_MODE_PROTECT == rkInstVictim.GetPKMode())
        return false;

    switch (GetAlignmentType()) {
        case ALIGNMENT_TYPE_NORMAL:
        case ALIGNMENT_TYPE_WHITE:
            if (ALIGNMENT_TYPE_DARK == rkInstVictim.GetAlignmentType())
                return true;
            break;
        case ALIGNMENT_TYPE_DARK:
            if (GetAlignmentType() != rkInstVictim.GetAlignmentType())
                return true;
            break;
        default:;
    }

    return false;
}

void CInstanceBase::SetDuelMode(uint32_t type)
{
    m_dwDuelMode = type;
}

uint32_t CInstanceBase::GetDuelMode() const
{
    return m_dwDuelMode;
}

bool CInstanceBase::IsAttackableInstance(CInstanceBase& rkInstVictim)
{
    if (__IsMainInstance()) {
        CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
        if (rkPlayer.IsObserverMode()) //&& !rkPlayer.IsGameMaster())
            return false;
    }

    if (GetVirtualID() == rkInstVictim.GetVirtualID())
        return false;

    if ((IsStone()) && (GetVirtualNumber() != MELEY_LAIR_DUNGEON_STATUE)) {
        if (rkInstVictim.IsPC())
            return true;
    } else if (IsPC()) {
        if (rkInstVictim.IsStone())
            return true;

        if (rkInstVictim.IsPC()) {
            if (m_dwLevel < 15 || rkInstVictim.m_dwLevel < 15)
                return false;

            if (__IsMainInstance()) {
                int iPVPTeam = CPythonPlayer::Instance().GetPVPTeam();

                {
                    int iOtherPVPTeam =
                        CPythonCharacterManager::Instance().GetPVPTeam(
                            rkInstVictim.GetVirtualID());
                    if (iOtherPVPTeam != -1)
                        return iPVPTeam != iOtherPVPTeam;
                }
            }

            if (GetDuelMode()) {
                switch (GetDuelMode()) {
                    case DUEL_CANNOTATTACK:
                        return false;
                    case DUEL_START:
                        return __FindDUELKey(GetVirtualID(),
                                             rkInstVictim.GetVirtualID());
                    default:;
                }
            }

            if (PK_MODE_GUILD == GetPKMode())
                if (GetGuildID() == rkInstVictim.GetGuildID())
                    return false;

            if (rkInstVictim.IsKiller())
                if (!CPythonPlayer::Instance().IsSamePartyMember(
                        GetVirtualID(), rkInstVictim.GetVirtualID()))
                    return true;

            if (IsPVPInstance(rkInstVictim))
                return true;

            const auto index = CPythonBackground::Instance().GetMapIndex();
            if (CPythonBackground::instance().IsDuelMap(index))
                return false;

            if (PK_MODE_PROTECT != GetPKMode()) {
                if (PK_MODE_FREE == GetPKMode()) {
                    if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
                        if (!CPythonPlayer::Instance().IsSamePartyMember(
                                GetVirtualID(), rkInstVictim.GetVirtualID()))
                            return true;
                }

                if (PK_MODE_GUILD == GetPKMode()) {
                    if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
                        if (!CPythonPlayer::Instance().IsSamePartyMember(
                                GetVirtualID(), rkInstVictim.GetVirtualID()))
                            if (GetGuildID() != rkInstVictim.GetGuildID())
                                return true;
                }

                if (PK_MODE_REVENGE == GetPKMode())
                    if (!CPythonPlayer::Instance().IsSamePartyMember(
                            GetVirtualID(), rkInstVictim.GetVirtualID()))
                        if (IsConflictAlignmentInstance(rkInstVictim))
                            return true;

                if (!IsSameEmpire(rkInstVictim))
                    return true;
            }
        }

        if (rkInstVictim.IsEnemy())
            return true;

        if (rkInstVictim.IsWoodenDoor())
            return true;
    } else if (IsEnemy()) {
        if (rkInstVictim.IsPC())
            return true;

        if (rkInstVictim.IsBuilding())
            return true;
    } else if (IsPoly()) {
        if (rkInstVictim.IsPC())
            return true;

        if (rkInstVictim.IsEnemy())
            return true;
    }
    return false;
}

bool CInstanceBase::IsTargetableInstance(CInstanceBase& rkInstVictim) const
{
    return rkInstVictim.CanPickInstance(false);
}

// 2004. 07. 07. [levites] - ½ºÅ³ »ç¿ëÁß Å¸°ÙÀÌ ¹Ù²î´Â ¹®Á¦ ÇØ°áÀ» À§ÇÑ ÄÚµå
bool CInstanceBase::CanChangeTarget()
{
    return m_GraphicThingInstance.CanChangeTarget();
}

// 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
bool CInstanceBase::CanPickInstance(bool enableTextTail)
{
    if (!__IsInViewFrustum())
        return false;

    if (IsDoor()) {
        if (IsDead())
            return false;
    }

    if (IsPC()) {
        if (HasAffect(SKILL_EUNHYUNG)) {
            return false;
        }

        if (HasAffect(AFFECT_REVIVE_INVISIBLE))
            return false;
        if (HasAffect(AFFECT_INVISIBILITY))
            return false;
    }

    if (!enableTextTail) {
        if (HasAffect(AFFECT_INVISIBILITY))
            return false;

        if (IsPet())
            return false;

        if (IsAttackPet())
            return false;

        if (IsMount())
            return false;
    }

    if (IsDead())
        return false;

    return true;
}

bool CInstanceBase::CanViewTargetHP(CInstanceBase& rkInstVictim) const
{
    if (rkInstVictim.IsStone())
        return true;

    if (rkInstVictim.IsDoor())
        return true;

    if (rkInstVictim.IsEnemy())
        return true;

    if (rkInstVictim.IsPC())
        return false;

    return false;
}

bool CInstanceBase::IsPoly()
{
    return m_GraphicThingInstance.IsPoly();
}

bool CInstanceBase::IsPC()
{
    return m_GraphicThingInstance.IsPC();
}

bool CInstanceBase::IsNPC()
{
    return m_GraphicThingInstance.IsNPC();
}

bool CInstanceBase::IsBuffBot()
{
    return m_GraphicThingInstance.IsBuffBot();
}

bool CInstanceBase::IsEnemy()
{
    return m_GraphicThingInstance.IsEnemy();
}

bool CInstanceBase::IsStone()
{
    return m_GraphicThingInstance.IsStone();
}

bool CInstanceBase::IsPet()
{
    return m_GraphicThingInstance.IsPet();
}

bool CInstanceBase::IsAttackPet()
{
    return m_GraphicThingInstance.IsAttackPet();
}

bool CInstanceBase::IsMount()
{
    return m_GraphicThingInstance.IsMount();
}

bool CInstanceBase::IsGuildWall() const
// IsBuilding ±æµå°Ç¹°ÀüÃ¼ IsGuildWallÀº ´ãÀåº®¸¸(¹®Àº Á¦¿Ü)
{
    return IsWall(m_dwRace);
}

bool CInstanceBase::IsResource() const
{
    switch (m_dwVirtualNumber) {
        case 20047:
        case 20048:
        case 20049:
        case 20050:
        case 20051:
        case 20052:
        case 20053:
        case 20054:
        case 20055:
        case 20056:
        case 20057:
        case 20058:
        case 20059:
        case 30301:
        case 30302:
        case 30303:
        case 30304:
        case 30305:
            return TRUE;
        default:;
    }

    return FALSE;
}

bool CInstanceBase::IsWarp()
{
    return m_GraphicThingInstance.IsWarp();
}

bool CInstanceBase::IsGoto()
{
    return m_GraphicThingInstance.IsGoto();
}

bool CInstanceBase::IsObject()
{
    return m_GraphicThingInstance.IsObject();
}

bool CInstanceBase::IsShop()
{
    return m_GraphicThingInstance.IsShop();
}

bool CInstanceBase::IsBuilding()
{
    return m_GraphicThingInstance.IsBuilding();
}

bool CInstanceBase::IsDoor()
{
    return m_GraphicThingInstance.IsDoor();
}

bool CInstanceBase::IsWoodenDoor()
{
    if (m_GraphicThingInstance.IsDoor()) {
        int vnum = GetVirtualNumber();
        if (vnum == 13000) // ³ª¹«¹®
            return true;
        if (vnum >= 30111 && vnum <= 30119) // »ç±Í¹®
            return true;
        return false;
    }
    return false;
}

bool CInstanceBase::IsStoneDoor()
{
    return m_GraphicThingInstance.IsDoor() && 13001 == GetVirtualNumber();
}

bool CInstanceBase::IsFlag() const
{
    if (GetRace() == 20035)
        return TRUE;
    if (GetRace() == 20036)
        return TRUE;
    if (GetRace() == 20037)
        return TRUE;

    return FALSE;
}

bool CInstanceBase::IsForceVisible()
{
    // if (IsAffect(AFF_DUNGEON_UNIQUE))
    //	return true;

    if (IsObject() || IsBuilding() || IsDoor())
        return true;

    return false;
}

int CInstanceBase::GetInstanceType() const
{
    return m_GraphicThingInstance.GetActorType();
}

uint32_t CInstanceBase::GetVirtualID()
{
    return m_GraphicThingInstance.GetVirtualID();
}

uint32_t CInstanceBase::GetVirtualNumber() const
{
    return m_dwVirtualNumber;
}

// 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
bool CInstanceBase::__IsInViewFrustum() const
{
    return m_GraphicThingInstance.isShow();
}

bool CInstanceBase::__CanRender()
{
    if (IsAlwaysRender())
        return true;

    if (!__IsInViewFrustum())
        return false;

    if (HasAffect(AFFECT_INVISIBILITY))
        return false;

    if (IsPet() && Engine::GetSettings().IsHidePets())
        return false;

    if (IsAttackPet() && Engine::GetSettings().IsHidePets())
        return false;

    if (IsMount() && Engine::GetSettings().IsHideMounts())
        return false;

    if (IsShop() && Engine::GetSettings().IsHideShops())
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Graphic Control

bool CInstanceBase::IntersectBoundingBox()
{
    float u, v, t;
    return m_GraphicThingInstance.Intersect(&u, &v, &t);
}

bool CInstanceBase::IntersectDefendingSphere()
{
    return m_GraphicThingInstance.IntersectDefendingSphere();
}

float CInstanceBase::GetDistance(CInstanceBase* pkTargetInst)
{
    TPixelPosition TargetPixelPosition;
    pkTargetInst->m_GraphicThingInstance.GetPixelPosition(&TargetPixelPosition);
    return GetDistance(TargetPixelPosition);
}

float CInstanceBase::GetDistance(const TPixelPosition& c_rPixelPosition)
{
    TPixelPosition PixelPosition;
    m_GraphicThingInstance.GetPixelPosition(&PixelPosition);

    float fdx = PixelPosition.x - c_rPixelPosition.x;
    float fdy = PixelPosition.y - c_rPixelPosition.y;

    return sqrtf((fdx * fdx) + (fdy * fdy));
}

CActorInstance& CInstanceBase::GetGraphicThingInstanceRef()
{
    return m_GraphicThingInstance;
}

CActorInstance* CInstanceBase::GetGraphicThingInstancePtr()
{
    return &m_GraphicThingInstance;
}

void CInstanceBase::Refresh(uint32_t dwMotIndex, bool isLoop)
{
    RefreshState(dwMotIndex, isLoop);
}

void CInstanceBase::RestoreRenderMode()
{
    m_GraphicThingInstance.RestoreRenderMode();
}

void CInstanceBase::SetAddRenderMode() {}

void CInstanceBase::SetModulateRenderMode() {}

void CInstanceBase::SetRenderMode(int iRenderMode)
{
    m_GraphicThingInstance.SetRenderMode(iRenderMode);
}

void CInstanceBase::SetAddColor(const DirectX::SimpleMath::Color& c_rColor)
{
    m_GraphicThingInstance.SetAddColor(c_rColor);
}

void CInstanceBase::__SetBlendRenderingMode()
{
    m_GraphicThingInstance.SetBlendRenderMode();
}

void CInstanceBase::__SetAlphaValue(float fAlpha)
{
    m_GraphicThingInstance.SetAlphaValue(fAlpha);
}

float CInstanceBase::__GetAlphaValue()
{
    return m_GraphicThingInstance.GetAlphaValue();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Part

void CInstanceBase::SetHair(const Part& eHair)
{
    if (IsPoly())
        return;

    const auto partData = GetPartData(eHair);
    m_awPart[PART_HAIR] = eHair;
    m_GraphicThingInstance.SetHair(partData.second);
}

void CInstanceBase::ChangeHair(const Part& eHair)
{
    if (!IsPC() && !IsBuffBot())
        return;

    if (IsPoly())
        return;

    if (GetPart(PART_HAIR) == eHair)
        return;

    SetHair(eHair);

    // int type = m_GraphicThingInstance.GetMotionMode();

    RefreshState(MOTION_WAIT, true);
    // RefreshState(type, true);
}

void CInstanceBase::SetArmor(const Part& part)
{
    if (IsPoly()) {
        m_GraphicThingInstance.SetShape(0, 0.0f);
        return;
    }

    bool isCostumeUnequip =
        part.costume != m_awPart[PART_MAIN].costume && part.costume == 0;
    if (isCostumeUnequip) {
        __ClearArmorCostumeShiningEffect();
    }

    m_awPart[PART_MAIN] = part;

    auto partData = GetPartData(part);

    if (!__ArmorVnumToShape(partData.second, &m_eShape)) {
        // Simple case: This is a built-in legacy shape (0 or 1)
        m_GraphicThingInstance.SetShape(part.vnum, 0.0f);
        return;
    }

    if (partData.first) {
        m_GraphicThingInstance.SetShape(m_eShape,
                                        partData.first->GetSpecularPowerf());

        __RefreshRefineEffect(partData.first);
#ifdef ENABLE_SHINING_SYSTEM
        __GetShiningEffect(partData.first);
#endif
    } else {
        __ClearArmorRefineEffect();
#ifdef ENABLE_SHINING_SYSTEM
        __ClearArmorShiningEffect();
#endif
    }
}

void CInstanceBase::SetShape(uint32_t eShape, float fSpecular)
{
    if ((IsNPC() || IsPoly() || IsEnemy()) &&
        Engine::GetSettings().IsShowGeneralEffects()) {
        if (NpcManager::instance().HasSpecularConfig(m_dwRace)) {
            fSpecular = NpcManager::instance().GetSpecularValue(m_dwRace);
        } else {
            fSpecular = 1.0;
        }
    }

    if (IsPoly()) {
        m_GraphicThingInstance.SetShape(0, fSpecular);
    } else {
        m_GraphicThingInstance.SetShape(eShape, fSpecular);
    }

    m_eShape = eShape;
}

void CInstanceBase::RefreshShape()
{
    /*
    float fSpecular = 0.0f;
    if ((IsNPC() || IsPoly() || IsEnemy()) &&
    Engine::GetSettings().IsShowShining())
    {
        if (NpcManager::instance().HasSpecularConfig(m_dwRace)) {
            fSpecular = NpcManager::instance().GetSpecularValue(m_dwRace);
        }
        else {
            fSpecular = 1.0;
        }
    }
    m_GraphicThingInstance.SetShape(m_eShape, fSpecular);
    */
    m_GraphicThingInstance.ClearAttachingEffect();

    __EffectContainer_Destroy();
    m_GraphicThingInstance.SetShape(m_eShape);

    __StoneSmoke_Destroy();
    __StoneSmoke_Create(0);

    Refresh(MOTION_SPAWN, false);
}

uint32_t CInstanceBase::GetWeaponType() const
{
    const auto vnum = EvaluatePartData(GetPart(PART_WEAPON));
    auto itemData = CItemManager::Instance().GetProto(vnum);
    if (!itemData)
        return WEAPON_NUM_TYPES;

    if (itemData->GetType() == ITEM_COSTUME)
        return itemData->GetValue(3);

    return itemData->GetSubType();
}

/*
void CInstanceBase::SetParts(const uint16_t * c_pParts)
{
    if (IsPoly())
        return;

    if (__IsShapeAnimalWear())
        return;

    UINT eWeapon=c_pParts[PART_WEAPON];

    if (__IsChangableWeapon(eWeapon) == false)
            eWeapon = 0;

    if (eWeapon != m_GraphicThingInstance.GetPartItemID(PART_WEAPON))
    {
        m_GraphicThingInstance.AttachPart(PART_MAIN, PART_WEAPON, eWeapon);
        m_awPart[PART_WEAPON] = eWeapon;
    }

    __AttachHorseSaddle();
}
*/

void CInstanceBase::__HideArmorShiningEffect() const
{
    auto& mgr = CEffectManager::Instance();

    if (m_armorShiningEffects) {
        const auto e = mgr.GetEffectInstance(m_armorShiningEffects);
        if (e && e->GetEffectKind() != EFFECT_KIND_SHINING_PERSIST) {
            e->EnableForceHide();
            e->Hide();
        }
    }
}

bool CInstanceBase::IsShowingArmorShiningEffect() const
{
    auto& mgr = CEffectManager::Instance();

    auto shownCount = 0;
    if (m_armorShiningEffects) {
        const auto instance = mgr.GetEffectInstance(m_armorShiningEffects);
        if (instance && instance->isShow())
            ++shownCount;
    }

    return shownCount > 0;
}

void CInstanceBase::__ShowArmorShiningEffect() const
{
    auto& mgr = CEffectManager::Instance();

    if (m_armorShiningEffects) {
        const auto e = mgr.GetEffectInstance(m_armorShiningEffects);
        if (e) {
            e->DisableForceHide();
            e->Show();
        }
    }
}

void CInstanceBase::__HideWeaponShiningEffect() const
{
    auto& mgr = CEffectManager::Instance();

    if (m_weaponShiningEffects[0]) {
        const auto e = mgr.GetEffectInstance(m_weaponShiningEffects[0]);
        if (e && e->GetEffectKind() != EFFECT_KIND_SHINING_PERSIST) {
            e->EnableForceHide();
            e->Hide();
        }
    }

    if (m_weaponShiningEffects[1]) {
        const auto e = mgr.GetEffectInstance(m_weaponShiningEffects[1]);
        if (e && e->GetEffectKind() != EFFECT_KIND_SHINING_PERSIST) {
            e->EnableForceHide();
            e->Hide();
        }
    }
}

bool CInstanceBase::IsShowingWeaponShiningEffect() const
{
    auto& mgr = CEffectManager::Instance();
    auto shownCount = 0;

    if (m_weaponShiningEffects[0]) {
        const auto instance = mgr.GetEffectInstance(m_weaponShiningEffects[0]);
        if (instance && instance->isShow())
            ++shownCount;
    }

    if (m_weaponShiningEffects[1]) {
        const auto instance = mgr.GetEffectInstance(m_weaponShiningEffects[1]);
        if (instance && instance->isShow())
            ++shownCount;
    }

    return shownCount > 0;
}

void CInstanceBase::__ShowWeaponShiningEffect() const
{
    auto& mgr = CEffectManager::Instance();

    if (m_weaponShiningEffects[0]) {
        const auto instance = mgr.GetEffectInstance(m_weaponShiningEffects[0]);
        if (instance) {
            instance->DisableForceHide();
            instance->Show();
        }
    }

    if (m_weaponShiningEffects[1]) {
        const auto instance = mgr.GetEffectInstance(m_weaponShiningEffects[1]);
        if (instance) {
            instance->DisableForceHide();
            instance->Show();
        }
    }
}

void CInstanceBase::__HideWeaponRefineEffect()
{
    auto& mgr = CEffectManager::Instance();

    if (m_swordRefineEffectRight) {
        const auto instance = mgr.GetEffectInstance(m_swordRefineEffectRight);
        if (instance) {
            instance->SetDeactive();
        }
    }
    if (m_swordRefineEffectLeft) {
        const auto instance = mgr.GetEffectInstance(m_swordRefineEffectLeft);
        if (instance) {
            instance->SetDeactive();
        }
    }

    auto effectGeom = mgr.GetEffectInstance(m_affectEffects[SKILL_GEOMKYUNG]);
    auto effectGwig = mgr.GetEffectInstance(m_affectEffects[SKILL_GWIGEOM]);

    if (effectGeom) {
        effectGeom->SetDeactive();
    }
    if (effectGwig) {
        effectGwig->SetDeactive();
    }

    if (m_weaponShiningEffects[0]) {
        auto eff = mgr.GetEffectInstance(m_weaponShiningEffects[0]);
        if (eff)
            eff->SetDeactive();
    }

    if (m_weaponShiningEffects[1]) {
        auto eff = mgr.GetEffectInstance(m_weaponShiningEffects[1]);
        if (eff)
            eff->SetDeactive();
    }

    if (m_weaponCostumeShiningEffects[0]) {
        auto eff = mgr.GetEffectInstance(m_weaponCostumeShiningEffects[0]);
        if (eff)
            eff->SetDeactive();
    }

    if (m_weaponCostumeShiningEffects[1]) {
        auto eff = mgr.GetEffectInstance(m_weaponCostumeShiningEffects[1]);
        if (eff)
            eff->SetDeactive();
    }

    if (m_weaponCostumeEffectShiningEffects[0]) {
        auto eff =
            mgr.GetEffectInstance(m_weaponCostumeEffectShiningEffects[0]);
        if (eff)
            eff->SetDeactive();
    }

    if (m_weaponCostumeEffectShiningEffects[1]) {
        auto eff =
            mgr.GetEffectInstance(m_weaponCostumeEffectShiningEffects[1]);
        if (eff)
            eff->SetDeactive();
    }
}

bool CInstanceBase::IsShowingWeaponRefineEffect() const
{
    auto& mgr = CEffectManager::Instance();

    bool a = false;
    bool b = false;

    if (m_swordRefineEffectRight) {
        const auto instance = mgr.GetEffectInstance(m_swordRefineEffectRight);
        if (instance)
            a = instance->isActive();
        else
            a = false;
    }
    if (m_swordRefineEffectLeft) {
        const auto instance = mgr.GetEffectInstance(m_swordRefineEffectLeft);
        if (instance)
            b = instance->isActive();
        else
            b = false;
    }

    return a || b;
}

void CInstanceBase::__ShowWeaponRefineEffect()
{
    auto& mgr = CEffectManager::Instance();

    if (m_swordRefineEffectRight) {
        const auto instance = mgr.GetEffectInstance(m_swordRefineEffectRight);
        if (instance)
            instance->SetActive();
    }
    if (m_swordRefineEffectLeft) {
        const auto instance = mgr.GetEffectInstance(m_swordRefineEffectLeft);
        if (instance)
            instance->SetActive();
    }

    if (m_hasWeapon) {
        auto effectGeom =
            mgr.GetEffectInstance(m_affectEffects[SKILL_GEOMKYUNG]);
        auto effectGwig = mgr.GetEffectInstance(m_affectEffects[SKILL_GWIGEOM]);

        if (effectGeom) {
            effectGeom->SetActive();
        }
        if (effectGwig) {
            effectGwig->SetActive();
        }

        if (m_weaponShiningEffects[0]) {
            auto eff = mgr.GetEffectInstance(m_weaponShiningEffects[0]);
            if (eff)
                eff->SetActive();
        }

        if (m_weaponShiningEffects[1]) {
            auto eff = mgr.GetEffectInstance(m_weaponShiningEffects[1]);
            if (eff)
                eff->SetActive();
        }

        if (m_weaponCostumeShiningEffects[0]) {
            auto eff = mgr.GetEffectInstance(m_weaponCostumeShiningEffects[0]);
            if (eff)
                eff->SetActive();
        }

        if (m_weaponCostumeShiningEffects[1]) {
            auto eff = mgr.GetEffectInstance(m_weaponCostumeShiningEffects[1]);
            if (eff)
                eff->SetActive();
        }

        if (m_weaponCostumeEffectShiningEffects[0]) {
            auto eff =
                mgr.GetEffectInstance(m_weaponCostumeEffectShiningEffects[0]);
            if (eff)
                eff->SetActive();
        }

        if (m_weaponCostumeEffectShiningEffects[1]) {
            auto eff =
                mgr.GetEffectInstance(m_weaponCostumeEffectShiningEffects[1]);
            if (eff)
                eff->SetActive();
        }
    }
}

void CInstanceBase::__ClearWeaponRefineEffect()
{
    if (m_swordRefineEffectRight) {
        DetachEffect(m_swordRefineEffectRight);
        m_swordRefineEffectRight = 0;
    }
    if (m_swordRefineEffectLeft) {
        DetachEffect(m_swordRefineEffectLeft);
        m_swordRefineEffectLeft = 0;
    }
}

void CInstanceBase::__ClearArmorRefineEffect()
{
    if (m_armorRefineEffect) {
        DetachEffect(m_armorRefineEffect);
        m_armorRefineEffect = 0;
    }
}

void CInstanceBase::__ClearAcceRefineEffect()
{
    if (m_acceRefineEffect) {
        DetachEffect(m_acceRefineEffect);
        m_acceRefineEffect = 0;
    }
}

void CInstanceBase::__RefreshRefineEffect(CItemData* pItem)
{
    static const int METIN_STONE_COUNT = 3;
    uint32_t refine =
        std::max<uint32_t>(pItem->GetRefine() + pItem->GetSocketCount(),
                           METIN_STONE_COUNT) -
        METIN_STONE_COUNT;

    switch (pItem->GetType()) {
        case ITEM_WEAPON: {
            __ClearWeaponRefineEffect();

            if (refine < 7) //현재 제련도 7 이상만 이펙트가 있습니다.
                return;

            switch (pItem->GetSubType()) {
                case WEAPON_DAGGER:
                    m_swordRefineEffectRight =
                        AttachEffect(EFFECT_SMALLSWORD_REFINED7 + refine - 7, 0,
                                     1, EFFECT_KIND_REFINED);

                    m_swordRefineEffectLeft = AttachEffect(
                        EFFECT_SMALLSWORD_REFINED7_LEFT + refine - 7, 0, 1,
                        EFFECT_KIND_REFINED);
                    break;

                case WEAPON_FAN:
                    m_swordRefineEffectRight =
                        AttachEffect(EFFECT_FANBELL_REFINED7 + refine - 7, 0, 1,
                                     EFFECT_KIND_REFINED);
                    if (IsMountingHorse())
                        m_swordRefineEffectLeft = AttachEffect(
                            EFFECT_FANBELL_REFINED7_LEFT + refine - 7, 0, 1,
                            EFFECT_KIND_REFINED);
                    break;

                case WEAPON_ARROW:
                case WEAPON_QUIVER:
                case WEAPON_BELL:
                    m_swordRefineEffectRight =
                        AttachEffect(EFFECT_SMALLSWORD_REFINED7 + refine - 7, 0,
                                     1, EFFECT_KIND_REFINED);
                    break;

                case WEAPON_BOW:
                    m_swordRefineEffectRight =
                        AttachEffect(EFFECT_BOW_REFINED7 + refine - 7, 0, 1,
                                     EFFECT_KIND_REFINED);
                    break;
                case WEAPON_CLAW:
                    m_swordRefineEffectRight =
                        AttachEffect(EFFECT_CLAW_REFINED7 + refine - 7, 0, 1,
                                     EFFECT_KIND_REFINED);
                    m_swordRefineEffectLeft =
                        AttachEffect(EFFECT_CLAW_REFINED7_LEFT + refine - 7, 0,
                                     1, EFFECT_KIND_REFINED);
                    break;
                default:
                    m_swordRefineEffectRight =
                        AttachEffect(EFFECT_SWORD_REFINED7 + refine - 7, 0, 1,
                                     EFFECT_KIND_REFINED);
            }

            break;
        }

        case ITEM_ARMOR: {
            __ClearArmorRefineEffect();

            if (refine < 7) //현재 제련도 7 이상만 이펙트가 있습니다.
                return;

            if (pItem->GetSubType() == ARMOR_BODY) {
                m_armorRefineEffect =
                    AttachEffect(EFFECT_BODYARMOR_REFINED7 + refine - 7, 0, 1,
                                 EFFECT_KIND_REFINED);
            }

            break;
        }
        default:;
    }
}

bool CInstanceBase::SetWeapon(const Part& part)
{
    if (IsPoly()) {
        DetachEffect(m_affectEffects[SKILL_GEOMKYUNG]);
        m_affectEffects[SKILL_GEOMKYUNG] = 0;
        return false;
    }
    if (__IsShapeAnimalWear())
        return false;

    if (!__IsChangableWeapon(part.vnum))
        return false;

    bool isCostumeUnequip =
        part.costume != m_awPart[PART_WEAPON].costume && part.costume == 0;
    bool isCostumeEquip =
        part.costume != m_awPart[PART_WEAPON].costume && part.costume != 0;

    if (isCostumeUnequip) {
        __ClearWeaponCostumeShiningEffect();
    }

    if (isCostumeEquip) {
        __ClearWeaponShiningEffect();
    }

    m_awPart[PART_WEAPON] = part;

    const auto itemData = GetPartData(part);
    auto& effMgr = CEffectManager::Instance();
    auto effectGeom =
        effMgr.GetEffectInstance(m_affectEffects[SKILL_GEOMKYUNG]);
    auto effectGwig = effMgr.GetEffectInstance(m_affectEffects[SKILL_GWIGEOM]);

    m_GraphicThingInstance.AttachWeapon(itemData.first);

    if (itemData.first) {
        m_hasWeapon = true;

        if (effectGeom) {
            auto addColor = effectGeom->GetAddColor();

            int32_t index;
            if (itemData.first->GetType() == ITEM_WEAPON &&
                itemData.first->GetSubType() == WEAPON_TWO_HANDED)
                index = AttachEffect(EFFECT_WEAPON_TWOHANDED, addColor, 1,
                                     EFFECT_KIND_AFFECT);
            else
                index = AttachEffect(EFFECT_WEAPON_ONEHANDED, addColor, 1,
                                     EFFECT_KIND_AFFECT);

            DetachEffect(m_affectEffects[SKILL_GEOMKYUNG]);
            m_affectEffects[SKILL_GEOMKYUNG] = index;
        }
        if (effectGwig) {
            effectGwig->SetActive();
        }
        __RefreshRefineEffect(itemData.first);
        __GetShiningEffect(itemData.first);
    } else {
        m_hasWeapon = false;

        if (effectGeom) {
            effectGeom->SetDeactive();
        }

        if (effectGwig) {
            effectGwig->SetDeactive();
        }

        __ClearWeaponRefineEffect();
        __ClearWeaponShiningEffect();
    }
    return true;
}

void CInstanceBase::SetPrefixNum(const Part& part)
{
    if (part.vnum == m_prefixNum)
        return;

    m_prefixNum = part.vnum;
    RefreshTextTail();
}

bool CInstanceBase::SetArrow(const Part& eArrow)
{
    if (IsPoly())
        return false;

    if (__IsShapeAnimalWear())
        return false;

    if (eArrow.vnum) {
        const auto pItemData = CItemManager::Instance().GetProto(eArrow.vnum);
        if (pItemData) {
            if (pItemData->GetSubType() == WEAPON_ARROW) {
                m_GraphicThingInstance.m_isUsingQuiver = false;
                m_GraphicThingInstance.m_quiverEffectID = 0;
            } else if (pItemData->GetSubType() == WEAPON_QUIVER) {
                m_GraphicThingInstance.m_isUsingQuiver = true;
                m_GraphicThingInstance.m_quiverEffectID = 17;
            }

            return true;
        }
    } else {
        m_GraphicThingInstance.m_isUsingQuiver = false;
        m_GraphicThingInstance.m_quiverEffectID = 0;
    }

    return false;
}

bool CInstanceBase::SetWeaponEffect(const Part& part)
{
    if (IsPoly())
        return false;

    if (__IsShapeAnimalWear())
        return false;

    m_awPart[PART_WEAPON_EFFECT] = part;

    const auto itemData = GetPartData(part);

    if (itemData.first) {
        __GetShiningEffect(itemData.first);
    } else {
        __ClearWeaponCostumeEffectShiningEffect();
    }

    return true;
}

bool CInstanceBase::SetWingEffect(const Part& part)
{
    if (IsPoly())
        return false;

    if (__IsShapeAnimalWear())
        return false;

    m_awPart[PART_WING_EFFECT] = part;

    const auto itemData = GetPartData(part);

    if (itemData.first) {
        __GetShiningEffect(itemData.first);
    } else {
        __ClearWingShining();
    }

    return true;
}

bool CInstanceBase::SetArmorEffect(const Part& part)
{
    if (IsPoly())
        return false;

    if (__IsShapeAnimalWear())
        return false;

    m_awPart[PART_BODY_EFFECT] = part;

    const auto itemData = GetPartData(part);

    if (itemData.first) {
        __GetShiningEffect(itemData.first);
    } else {
        __ClearArmorCostumeEffectShiningEffect();
    }

    return true;
}

bool CInstanceBase::SetAcce(const Part& eAcce)
{
    if (!IsPC() && !IsBuffBot())
        return false;

    if (IsPoly())
        return false;

    if (__IsShapeAnimalWear())
        return false;

    m_awPart[PART_ACCE] = eAcce;

    const auto itemData = GetPartData(eAcce);
    m_GraphicThingInstance.AttachAcce(itemData.first);

    if (eAcce.costume) {
        if (m_acceRefineEffect) {
            __ClearAcceRefineEffect();
            m_acceRefineEffect =
                AttachEffect(EFFECT_ACCE_BACK, 0, 1, EFFECT_KIND_REFINED);
        }
    } else {
        __ClearAcceRefineEffect();
    }

    return true;
}

void CInstanceBase::ChangeWeapon(const Part& eWeapon)
{
    if (eWeapon == GetPart(PART_WEAPON))
        return;

    if (SetWeapon(eWeapon))
        RefreshState(MOTION_WAIT, true);
}

void CInstanceBase::ChangeScale(uint16_t scale)
{
    const auto fScale = static_cast<float>(scale / 100.0f);
    SetScale(fScale);
}

void CInstanceBase::RefreshWeapon()
{
    if (m_awPart[PART_WEAPON].vnum == 0 &&
        m_awPart[PART_WEAPON].appearance == 0 &&
        m_awPart[PART_WEAPON].costume == 0 &&
        m_awPart[PART_WEAPON].costume_appearance == 0)
        return;

    // Make a copy so we don't try to overwrite ourself.
    const auto weapon = GetPart(PART_WEAPON);
    SetWeapon(weapon);
}

void CInstanceBase::RefreshAcce()
{
    if (m_awPart[PART_ACCE].vnum == 0 && m_awPart[PART_ACCE].appearance == 0 &&
        m_awPart[PART_ACCE].costume == 0 &&
        m_awPart[PART_ACCE].costume_appearance == 0)
        return;

    // Make a copy so we don't try to overwrite ourself.
    const auto sash = GetPart(PART_ACCE);
    SetAcce(sash);
}

void CInstanceBase::ChangeArmorEffect(const Part& part)
{
    if (m_awPart[PART_BODY_EFFECT] == part)
        return;

    SetArmorEffect(part);
}

void CInstanceBase::ChangeWingEffect(const Part& part)
{
    if (m_awPart[PART_WING_EFFECT] == part)
        return;

    SetWingEffect(part);
}

void CInstanceBase::ChangeWeaponEffect(const Part& part)
{
    if (m_awPart[PART_WEAPON_EFFECT] == part)
        return;

    SetWeaponEffect(part);
}

void CInstanceBase::ChangeArrow(const Part& eArrow)
{
    SetArrow(eArrow);
}

void CInstanceBase::ChangeAcce(const Part& eAcce)
{
    if (eAcce == GetPart(PART_ACCE))
        return;

    if (SetAcce(eAcce))
        RefreshState(MOTION_WAIT, true);
}

bool CInstanceBase::ChangeArmor(const Part& dwArmor)
{
    auto itemData = GetPartData(dwArmor);
    uint32_t eShape;
    __ArmorVnumToShape(itemData.second, &eShape);

    if (GetShape() == eShape)
        return false;

    uint32_t dwVID = GetVirtualID();
    uint32_t dwRace = GetRace();
    auto eHair = GetPart(PART_HAIR);
    auto eWeapon = GetPart(PART_WEAPON);
    auto eAcce = GetPart(PART_ACCE);
    auto eBodyEffect = GetPart(PART_BODY_EFFECT);
    auto eWeaponEffect = GetPart(PART_WEAPON_EFFECT);
    auto eWingEffect = GetPart(PART_WING_EFFECT);

    float fRot = GetRotation();
    float fAdvRot = GetAdvancingRotation();

    if (IsWalking())
        EndWalking();

    if (!SetRace(dwRace)) {
        SPDLOG_ERROR("CPythonCharacterManager::ChangeArmor - SetRace VID[{0}] "
                     "Race[{1}] ERROR",
                     dwVID, dwRace);
        return false;
    }

    SetArmor(dwArmor);
    SetHair(eHair);
    SetWeapon(eWeapon);
    SetWeaponEffect(eWeaponEffect);
    SetArmorEffect(eBodyEffect);
    SetWingEffect(eWingEffect);

    SetAcce(eAcce);
    SetRotation(fRot);
    SetAdvancingRotation(fAdvRot);

    __AttachHorseSaddle();

    RefreshState(MOTION_WAIT, TRUE);

    CActorInstance::IEventHandler& rkEventHandler = GetEventHandlerRef();
    rkEventHandler.OnChangeShape();

    m_affectEffects.clear();
    for (const auto type : GetAffectTypes(m_affects)) {
        const auto affect = SearchAffect(m_affects, type);
        SetAffectActive(type, true, affect ? affect->color : 0);
    }

    return true;
}

bool CInstanceBase::__IsShapeAnimalWear() const
{
    return 100 == GetShape() || 101 == GetShape() || 102 == GetShape() ||
           103 == GetShape();
}

uint32_t CInstanceBase::__GetRaceType() const
{
    return m_eRaceType;
}

void CInstanceBase::RefreshState(uint32_t motion, bool loop)
{
    auto [itemData, part] = GetPartData(GetPart(PART_WEAPON));

    uint8_t type = 0xff;
    uint8_t subType = 0xff;

    if (itemData) {
        type = itemData->GetType();
        subType = itemData->GetSubType();

        if (type == ITEM_COSTUME)
            subType = itemData->GetValue(3);
    }

    if (IsPoly()) {
        SetMotionMode(MOTION_MODE_GENERAL);
    } else if (IsWearingDress()) {
        SetMotionMode(MOTION_MODE_WEDDING_DRESS);
    } else if (IsHoldingPickAxe()) {
        if (m_kHorse.IsMounting()) {
            SetMotionMode(MOTION_MODE_HORSE);
        } else {
            SetMotionMode(MOTION_MODE_GENERAL);
        }
    } else if (ITEM_ROD == type) {
        if (m_kHorse.IsMounting()) {
            SetMotionMode(MOTION_MODE_HORSE);
        } else {
            SetMotionMode(MOTION_MODE_FISHING);
        }
    } else if (m_kHorse.IsMounting()) {
        switch (subType) {
            case WEAPON_SWORD:
                SetMotionMode(MOTION_MODE_HORSE_ONEHAND_SWORD);
                break;

            case WEAPON_TWO_HANDED:
                SetMotionMode(MOTION_MODE_HORSE_TWOHAND_SWORD); // Only Warrior
                break;

            case WEAPON_DAGGER:
                SetMotionMode(
                    MOTION_MODE_HORSE_DUALHAND_SWORD); // Only Assassin
                break;

            case WEAPON_FAN:
                SetMotionMode(MOTION_MODE_HORSE_FAN); // Only Shaman
                break;

            case WEAPON_BELL:
                SetMotionMode(MOTION_MODE_HORSE_BELL); // Only Shaman
                break;

            case WEAPON_BOW:
                SetMotionMode(MOTION_MODE_HORSE_BOW); // Only Shaman
                break;

            case WEAPON_CLAW:
                SetMotionMode(MOTION_MODE_HORSE_CLAW); // Only Wolfman
                break;

            default:
                SetMotionMode(MOTION_MODE_HORSE);
                break;
        }
    } else {
        switch (subType) {
            case WEAPON_SWORD:
                SetMotionMode(MOTION_MODE_ONEHAND_SWORD);
                break;

            case WEAPON_TWO_HANDED:
                SetMotionMode(MOTION_MODE_TWOHAND_SWORD); // Only Warrior
                break;

            case WEAPON_DAGGER:
                SetMotionMode(MOTION_MODE_DUALHAND_SWORD); // Only Assassin
                break;

            case WEAPON_BOW:
                SetMotionMode(MOTION_MODE_BOW); // Only Assassin
                break;

            case WEAPON_FAN:
                SetMotionMode(MOTION_MODE_FAN); // Only Shaman
                break;

            case WEAPON_BELL:
                SetMotionMode(MOTION_MODE_BELL); // Only Shaman
                break;

            case WEAPON_CLAW:
                SetMotionMode(MOTION_MODE_CLAW); // Only Wolfman
                break;

            case WEAPON_ARROW:
            case WEAPON_QUIVER:

            default:
                SetMotionMode(MOTION_MODE_GENERAL);
                break;
        }
    }

    if (loop)
        m_GraphicThingInstance.InterceptLoopMotion(motion);
    else
        m_GraphicThingInstance.InterceptOnceMotion(motion);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device

void CInstanceBase::RegisterBoundingSphere()
{
    // Stone ÀÏ °æ¿ì DeforomNoSkin À» ÇÏ¸é
    // ³«ÇÏÇÏ´Â ¾Ö´Ï¸ÞÀÌ¼Ç °°Àº °æ¿ì ¾Ö´Ï¸ÞÀÌ¼ÇÀÌ
    // ¹Ù¿îµå ¹Ú½º¿¡ ¿µÇâÀ» ¹ÌÃÄ ÄÃ¸µÀÌ Á¦´ë·Î ÀÌ·ç¾îÁöÁö ¾Ê´Â´Ù.
    if (!IsStone()) {
        m_GraphicThingInstance.DeformNoSkin();
    }

    m_GraphicThingInstance.RegisterBoundingSphere();
}

bool CInstanceBase::CreateDeviceObjects()
{
    return m_GraphicThingInstance.CreateDeviceObjects();
}

void CInstanceBase::DestroyDeviceObjects()
{
    m_GraphicThingInstance.DestroyDeviceObjects();
}

CInstanceBase::CInstanceBase()
    : m_eType(0)
    , m_eRaceType(0)
    , m_eShape(0)
    , m_dwRace(0)
    , m_stateFlags(0)
    , m_immuneFlags(0)
    , m_byPKMode(0)
    , m_isKiller(false)
    , m_isPartyMember(false)
    , m_iRotatingDirection(DEGREE_DIRECTION_SAME)
    , m_dwAdvActorVID(0)
    , m_dwLastDmgActorVID(0)
    , m_dwNextUpdateHeightTime(0)
    , m_isGoing(false)
    , m_kPPosDust(0.0f, 0.0f, 0.0f)
    , m_dwLastComboIndex(0)
    , m_swordRefineEffectRight(0)
    , m_swordRefineEffectLeft(0)
    , m_armorRefineEffect(0)
    , m_fDstRot(0.0f)
    , m_fAtkPosTime(0.0f)
    , m_fRotSpd(0.0f)
    , m_fMaxRotSpd(c_fDefaultRotationSpeed)
    , m_IsAlwaysRender(false)
    , m_dwPlayTime(0)
{
    SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());
}

CInstanceBase::~CInstanceBase()
{
    DetachTextTail();

    std::memset(m_awPart, 0, sizeof(m_awPart));
    SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());

    __DetachHorseSaddle();
    m_GraphicThingInstance.MountHorse(nullptr);
    m_kHorse.Destroy();

    m_GraphicThingInstance.ClearAttachingEffect();
    __EffectContainer_Destroy();
    __StoneSmoke_Destroy();

    if (m_gmEffect) {
        DetachEffect(m_gmEffect);
        m_gmEffect = 0;
    }

    if (__IsMainInstance())
        __ClearMainInstance();
}

void CInstanceBase::GetBoundBox(Vector3* vtMin, Vector3* vtMax)
{
    m_GraphicThingInstance.GetBoundBox(vtMin, vtMax);
}

bool CInstanceBase::IsAlwaysRender() const
{
    return m_IsAlwaysRender;
}

void CInstanceBase::SetAlwaysRender(bool val)
{
    m_IsAlwaysRender = val;
    m_GraphicThingInstance.SetAlwaysRender(val);
}

#ifdef ENABLE_SHINING_SYSTEM

// This method checks if the item has shinings and will attach them.

// Weapons are heaving special checks for two sided weapons like daggers or fans
// while mounting.

// Every itemtype is supported. If no explicit logic is implemented it will be
// attached to "Bip01".

void CInstanceBase::__GetShiningEffect(CItemData* pItem)

{
    // Set this to true if you want to hide the refine effect if there's a
    // shining for this item.

    bool removeRefineEffect = false;

    CItemData::TItemShiningTable shiningTable = pItem->GetItemShiningTable();

    if (pItem->GetType() == ITEM_WEAPON) {
        __ClearWeaponShiningEffect();

        if (shiningTable.Any() && removeRefineEffect) {
            __ClearWeaponRefineEffect();
        }

        auto weaponType = WEAPON_NUM_TYPES;
        const auto equippedWeapon = CItemManager::instance().GetProto(
            GetPartData(m_awPart[PART_WEAPON]).second);
        if (equippedWeapon) {
            weaponType =
                static_cast<EWeaponSubTypes>(equippedWeapon->GetWeaponType());
        }
        if (shiningTable.effects[weaponType].empty())
            weaponType = WEAPON_NUM_TYPES;
        if (!shiningTable.effects[weaponType].empty()) {
            if (weaponType == WEAPON_BOW) {
                __AttachWeaponShiningEffect(
                    shiningTable.persistent,
                    shiningTable.effects[weaponType].c_str(),
                    "PART_WEAPON_LEFT");
            } else {
                // Check for double sided weapons or fan which is attached on
                // both sides when mounted.
#ifdef ENABLE_WOLFMAN

                bool twoSidedWeapon =
                    pItem->GetSubType() == WEAPON_DAGGER ||
                    (IsMountingHorse() && pItem->GetSubType() == WEAPON_FAN) ||
                    pItem->GetSubType() == WEAPON_CLAW;

#else
                const auto twoSidedWeapon =
                    weaponType == WEAPON_DAGGER ||
                    (IsMountingHorse() && weaponType == WEAPON_FAN) ||
                    weaponType == WEAPON_CLAW;
#endif
                if (twoSidedWeapon) {
                    __AttachWeaponShiningEffect(
                        shiningTable.persistent,
                        shiningTable.effects[weaponType].c_str(),
                        "PART_WEAPON_LEFT");
                }

                __AttachWeaponShiningEffect(
                    shiningTable.persistent,
                    shiningTable.effects[weaponType].c_str(), "PART_WEAPON");
            }
        }
    }

    if (pItem->GetType() == ITEM_COSTUME &&
        (pItem->GetSubType() == COSTUME_WEAPON)) {
        __ClearWeaponCostumeShiningEffect();

        if (shiningTable.Any() && removeRefineEffect) {
            __ClearWeaponRefineEffect();
        }

        auto weaponType = WEAPON_NONE;
        const auto equippedWeapon = CItemManager::instance().GetProto(
            GetPartData(m_awPart[PART_WEAPON]).second);
        if (equippedWeapon) {
            weaponType =
                static_cast<EWeaponSubTypes>(equippedWeapon->GetWeaponType());
        }
        if (shiningTable.effects[weaponType].empty())
            weaponType = WEAPON_NONE;
        if (!shiningTable.effects[weaponType].empty()) {
            if (weaponType == WEAPON_BOW) {
                __AttachWeaponCostumeShiningEffect(
                    shiningTable.persistent,
                    shiningTable.effects[weaponType].c_str(),
                    "PART_WEAPON_LEFT");
            } else {
                // Check for double sided weapons or fan which is attached on
                // both sides when mounted.
#ifdef ENABLE_WOLFMAN

                bool twoSidedWeapon =
                    pItem->GetSubType() == WEAPON_DAGGER ||
                    (IsMountingHorse() && pItem->GetSubType() == WEAPON_FAN) ||
                    pItem->GetSubType() == WEAPON_CLAW;

#else
                const auto twoSidedWeapon =
                    weaponType == WEAPON_DAGGER ||
                    (IsMountingHorse() && weaponType == WEAPON_FAN) ||
                    weaponType == WEAPON_CLAW;
#endif
                if (twoSidedWeapon) {
                    __AttachWeaponCostumeShiningEffect(
                        shiningTable.persistent,
                        shiningTable.effects[weaponType].c_str(),
                        "PART_WEAPON_LEFT");
                }

                __AttachWeaponCostumeShiningEffect(
                    shiningTable.persistent,
                    shiningTable.effects[weaponType].c_str(), "PART_WEAPON");
            }
        }
    }

    if (pItem->GetType() == ITEM_COSTUME &&
        (pItem->GetSubType() == COSTUME_WEAPON_EFFECT)) {
        __ClearWeaponCostumeEffectShiningEffect();

        if (shiningTable.Any() && removeRefineEffect) {
            __ClearWeaponRefineEffect();
        }

        auto weaponType = WEAPON_NONE;
        const auto equippedWeapon = CItemManager::instance().GetProto(
            GetPartData(m_awPart[PART_WEAPON]).second);
        if (equippedWeapon) {
            weaponType =
                static_cast<EWeaponSubTypes>(equippedWeapon->GetWeaponType());
        }

        if (shiningTable.effects[weaponType].empty())
            weaponType = WEAPON_NONE;

        if (!shiningTable.effects[weaponType].empty()) {
            if (weaponType == WEAPON_BOW) {
                __AttachWeaponCostumeEffectShiningEffect(
                    shiningTable.persistent,
                    shiningTable.effects[weaponType].c_str(),
                    "PART_WEAPON_LEFT");
            } else {
                // Check for double sided weapons or fan which is attached on
                // both sides when mounted.
#ifdef ENABLE_WOLFMAN

                bool twoSidedWeapon =
                    pItem->GetSubType() == WEAPON_DAGGER ||
                    (IsMountingHorse() && pItem->GetSubType() == WEAPON_FAN) ||
                    pItem->GetSubType() == WEAPON_CLAW;

#else
                const auto twoSidedWeapon =
                    weaponType == WEAPON_DAGGER ||
                    (IsMountingHorse() && weaponType == WEAPON_FAN) ||
                    weaponType == WEAPON_CLAW;
#endif
                if (twoSidedWeapon) {
                    __AttachWeaponCostumeEffectShiningEffect(
                        shiningTable.persistent,
                        shiningTable.effects[weaponType].c_str(),
                        "PART_WEAPON_LEFT");
                }

                __AttachWeaponCostumeEffectShiningEffect(
                    shiningTable.persistent,
                    shiningTable.effects[weaponType].c_str(), "PART_WEAPON");
            }
        }
    }

    if ((pItem->GetType() == ITEM_ARMOR && pItem->GetSubType() == ARMOR_BODY)) {
        __ClearArmorShiningEffect();

        if (shiningTable.Any() && removeRefineEffect) {
            __ClearArmorRefineEffect();
        }

        if (!shiningTable.effects[WEAPON_NONE].empty()) {
            __AttachArmorShiningEffect(
                shiningTable.persistent,
                shiningTable.effects[WEAPON_NONE].c_str(),
                shiningTable.boneName.value_or("Bip01").c_str(),
                shiningTable.scale.value_or(1.0f));
        }
    }

    if (pItem->GetType() == ITEM_COSTUME &&
        (pItem->GetSubType() == COSTUME_BODY)) {
        __ClearArmorCostumeShiningEffect();

        if (shiningTable.Any() && removeRefineEffect) {
            __ClearArmorRefineEffect();
        }

        if (!shiningTable.effects[WEAPON_NONE].empty()) {
            __AttachArmorCostumeShiningEffect(
                shiningTable.persistent,
                shiningTable.effects[WEAPON_NONE].c_str(),
                shiningTable.boneName.value_or("Bip01").c_str(),
                shiningTable.scale.value_or(1.0f));
        }
    }

    if (pItem->GetType() == ITEM_COSTUME &&
        (pItem->GetSubType() == COSTUME_BODY_EFFECT)) {
        __ClearArmorCostumeEffectShiningEffect();

        if (shiningTable.Any() && removeRefineEffect) {
            __ClearArmorRefineEffect();
        }

        if (!shiningTable.effects[WEAPON_NONE].empty()) {
            __AttachArmorCostumeEffectShiningEffect(
                shiningTable.persistent,
                shiningTable.effects[WEAPON_NONE].c_str(),
                shiningTable.boneName.value_or("Bip01").c_str(),
                shiningTable.scale.value_or(1.0f));
        }
    }

    if (pItem->GetType() == ITEM_COSTUME &&
        (pItem->GetSubType() == COSTUME_WING_EFFECT)) {
        __ClearWingShining();

        if (shiningTable.Any() && removeRefineEffect) {
            __ClearArmorRefineEffect();
        }

        if (!shiningTable.effects[WEAPON_NONE].empty()) {
            __AttachWingEffect(shiningTable.persistent,
                               shiningTable.effects[WEAPON_NONE].c_str(),
                               shiningTable.boneName.value_or("Bip01").c_str(),
                               shiningTable.scale.value_or(1.0f));
        }
    }
}

// Attaching the shining effect.

// BoneName can be "PART_WEAPON" or "PART_WEAPON_LEFT" to get the attaching bone
// name dynamically.

// If boneName is not given "Bip01" is used as boneName.

void CInstanceBase::__AttachWeaponShiningEffect(int persistent,
                                                const char* effectFileName,
                                                const char* boneName)

{
    if (HasAffect(AFFECT_INVISIBILITY))
        return;

    uint32_t effId = 0;
    CEffectManager::Instance().RegisterEffect(effectFileName, &effId, false);

    if (!strcmp(boneName, "PART_WEAPON")) {
        const char* c_szRightBoneName;

        m_GraphicThingInstance.GetAttachingBoneName(PART_WEAPON,
                                                    &c_szRightBoneName);

        if (!!strcmp(c_szRightBoneName, "")) {
            m_weaponShiningEffects[0] = m_GraphicThingInstance.AttachEffectByID(
                0, c_szRightBoneName, effId, nullptr, 0, 1,
                persistent ? EFFECT_KIND_SHINING_PERSIST : EFFECT_KIND_SHINING);
        }
    } else if (!strcmp(boneName, "PART_WEAPON_LEFT")) {
        const char* c_szLeftBoneName;

        m_GraphicThingInstance.GetAttachingBoneName(PART_WEAPON_LEFT,
                                                    &c_szLeftBoneName);

        if (!!strcmp(c_szLeftBoneName, "")) {
            m_weaponShiningEffects[1] = m_GraphicThingInstance.AttachEffectByID(
                0, c_szLeftBoneName, effId, nullptr, 0, 1,
                persistent ? EFFECT_KIND_SHINING_PERSIST : EFFECT_KIND_SHINING);
        }
    } else {
        SPDLOG_DEBUG("Invalid partname for getting attaching bone name. {0} - "
                     "{1}",
                     effectFileName, boneName);
    }
}

// Attaching the armor shining effect.

// If boneName is not given "Bip01" is used as boneName.

void CInstanceBase::__AttachArmorShiningEffect(int persistent,
                                               const char* effectFileName,
                                               const char* boneName,
                                               float scale)
{
    if (IsInvisibility())
        return;

    if (!strcmp(boneName, "")) {
        SPDLOG_DEBUG("Empty bone name for attaching armor shining.  "
                     "EffectFileName: {1}",
                     effectFileName);
        return;
    }

    uint32_t effId = 0;
    if (!CEffectManager::Instance().RegisterEffect(effectFileName, &effId,
                                                   false)) {
        SPDLOG_ERROR("Failed to register effect {}", effectFileName);
        return;
    }

    m_armorShiningEffects = m_GraphicThingInstance.AttachEffectByID(
        0, boneName, effId, nullptr, 0, scale,
        persistent ? EFFECT_KIND_SHINING_PERSIST : EFFECT_KIND_SHINING);
}

void CInstanceBase::__AttachWeaponCostumeShiningEffect(
    int persistent, const char* effectFileName, const char* boneName)

{
    if (HasAffect(AFFECT_INVISIBILITY))
        return;

    uint32_t effId = 0;
    CEffectManager::Instance().RegisterEffect(effectFileName, &effId, false);

    if (!strcmp(boneName, "PART_WEAPON")) {
        const char* c_szRightBoneName;

        m_GraphicThingInstance.GetAttachingBoneName(PART_WEAPON,
                                                    &c_szRightBoneName);

        if (!!strcmp(c_szRightBoneName, "")) {
            m_weaponCostumeShiningEffects[0] =
                m_GraphicThingInstance.AttachEffectByID(
                    0, c_szRightBoneName, effId, nullptr, 0, 1,
                    persistent ? EFFECT_KIND_SHINING_PERSIST
                               : EFFECT_KIND_SHINING);
        }
    } else if (!strcmp(boneName, "PART_WEAPON_LEFT")) {
        const char* c_szLeftBoneName;

        m_GraphicThingInstance.GetAttachingBoneName(PART_WEAPON_LEFT,
                                                    &c_szLeftBoneName);

        if (!!strcmp(c_szLeftBoneName, "")) {
            m_weaponCostumeShiningEffects[1] =
                m_GraphicThingInstance.AttachEffectByID(
                    0, c_szLeftBoneName, effId, nullptr, 0, 1,
                    persistent ? EFFECT_KIND_SHINING_PERSIST
                               : EFFECT_KIND_SHINING);
        }
    } else {
        SPDLOG_DEBUG("Invalid partname for getting attaching bone name. {0} - "
                     "{1}",
                     effectFileName, boneName);
    }
}

void CInstanceBase::__AttachArmorCostumeShiningEffect(
    int persistent, const char* effectFileName, const char* boneName,
    float scale)
{
    if (IsInvisibility())
        return;

    if (!strcmp(boneName, "")) {
        SPDLOG_DEBUG("Empty bone name for attaching armor shining.  "
                     "EffectFileName: {1}",
                     effectFileName);
        return;
    }

    uint32_t effId = 0;
    if (!CEffectManager::Instance().RegisterEffect(effectFileName, &effId,
                                                   false)) {
        SPDLOG_ERROR("Failed to register effect {}", effectFileName);
        return;
    }

    m_armorCostumeShiningEffects = m_GraphicThingInstance.AttachEffectByID(
        0, boneName, effId, nullptr, 0, scale,
        persistent ? EFFECT_KIND_SHINING_PERSIST : EFFECT_KIND_SHINING);
}

void CInstanceBase::__AttachWeaponCostumeEffectShiningEffect(
    int persistent, const char* effectFileName, const char* boneName,
    float scale)

{
    if (HasAffect(AFFECT_INVISIBILITY))
        return;

    uint32_t effId = 0;
    CEffectManager::Instance().RegisterEffect(effectFileName, &effId, false);

    if (!strcmp(boneName, "PART_WEAPON")) {
        const char* c_szRightBoneName;

        m_GraphicThingInstance.GetAttachingBoneName(PART_WEAPON,
                                                    &c_szRightBoneName);

        if (!!strcmp(c_szRightBoneName, "")) {

            m_weaponCostumeEffectShiningEffects[0] =
                m_GraphicThingInstance.AttachEffectByID(
                    0, c_szRightBoneName, effId, nullptr, 0, scale,
                    persistent ? EFFECT_KIND_SHINING_PERSIST
                               : EFFECT_KIND_SHINING);
        }
    } else if (!strcmp(boneName, "PART_WEAPON_LEFT")) {
        const char* c_szLeftBoneName;

        m_GraphicThingInstance.GetAttachingBoneName(PART_WEAPON_LEFT,
                                                    &c_szLeftBoneName);

        if (!!strcmp(c_szLeftBoneName, "")) {
            m_weaponCostumeEffectShiningEffects[1] =
                m_GraphicThingInstance.AttachEffectByID(
                    0, c_szLeftBoneName, effId, nullptr, 0, scale,
                    persistent ? EFFECT_KIND_SHINING_PERSIST
                               : EFFECT_KIND_SHINING);
        }
    } else {
        SPDLOG_DEBUG("Invalid partname for getting attaching bone name. {0} - "
                     "{1}",
                     effectFileName, boneName);
    }
}

void CInstanceBase::__AttachArmorCostumeEffectShiningEffect(
    int persistent, const char* effectFileName, const char* boneName,
    float scale)
{
    if (IsInvisibility())
        return;

    if (!strcmp(boneName, "")) {
        SPDLOG_DEBUG("Empty bone name for attaching armor shining.  "
                     "EffectFileName: {1}",
                     effectFileName);
        return;
    }

    uint32_t effId = 0;
    if (!CEffectManager::Instance().RegisterEffect(effectFileName, &effId,
                                                   false)) {
        SPDLOG_ERROR("Failed to register effect {}", effectFileName);
        return;
    }

    m_armorCostumeEffectShiningEffects =
        m_GraphicThingInstance.AttachEffectByID(
            0, boneName, effId, nullptr, 0, scale,
            persistent ? EFFECT_KIND_SHINING_PERSIST : EFFECT_KIND_SHINING);
}

void CInstanceBase::__AttachWingEffect(int persistent,
                                       const char* effectFileName,
                                       const char* boneName, float scale)
{
    if (IsInvisibility())
        return;

    if (!strcmp(boneName, "")) {
        SPDLOG_DEBUG("Empty bone name for attaching armor shining.  "
                     "EffectFileName: {1}",
                     effectFileName);
        return;
    }

    uint32_t effId = 0;
    if (!CEffectManager::Instance().RegisterEffect(effectFileName, &effId,
                                                   false)) {
        SPDLOG_ERROR("Failed to register effect {}", effectFileName);
        return;
    }

    m_wingCostumeShiningEffect = m_GraphicThingInstance.AttachEffectByID(
        0, boneName, effId, nullptr, 0, scale,
        persistent ? EFFECT_KIND_SHINING_PERSIST : EFFECT_KIND_SHINING);
}

// Clears all weapon shining effects. Left & Right if set.

void CInstanceBase::__ClearWeaponShiningEffect(bool detaching)
{
    if (m_weaponShiningEffects[0]) {
        DetachEffect(m_weaponShiningEffects[0]);
    }

    if (m_weaponShiningEffects[1]) {
        DetachEffect(m_weaponShiningEffects[1]);
    }

    memset(&m_weaponShiningEffects, 0, sizeof(m_weaponShiningEffects));
}

void CInstanceBase::__ClearWeaponCostumeShiningEffect()
{
    if (m_weaponCostumeShiningEffects[0]) {
        DetachEffect(m_weaponCostumeShiningEffects[0]);
    }

    if (m_weaponCostumeShiningEffects[1]) {
        DetachEffect(m_weaponCostumeShiningEffects[1]);
    }

    memset(&m_weaponCostumeShiningEffects, 0,
           sizeof(m_weaponCostumeShiningEffects));
}

void CInstanceBase::__ClearWeaponCostumeEffectShiningEffect()
{
    if (m_weaponCostumeEffectShiningEffects[0]) {
        DetachEffect(m_weaponCostumeEffectShiningEffects[0]);
    }

    if (m_weaponCostumeEffectShiningEffects[1]) {
        DetachEffect(m_weaponCostumeEffectShiningEffects[1]);
    }

    memset(&m_weaponCostumeEffectShiningEffects, 0,
           sizeof(m_weaponCostumeEffectShiningEffects));
}

// Clears all armor shining effects.

void CInstanceBase::__ClearArmorShiningEffect()
{
    DetachEffect(m_armorShiningEffects);
    memset(&m_armorShiningEffects, 0, sizeof(m_armorShiningEffects));
}

void CInstanceBase::__ClearArmorCostumeShiningEffect()
{
    DetachEffect(m_armorCostumeShiningEffects);
    memset(&m_armorCostumeShiningEffects, 0,
           sizeof(m_armorCostumeShiningEffects));
}

void CInstanceBase::__ClearArmorCostumeEffectShiningEffect()
{
    DetachEffect(m_armorCostumeEffectShiningEffects);
    memset(&m_armorCostumeEffectShiningEffects, 0,
           sizeof(m_armorCostumeEffectShiningEffects));
}

void CInstanceBase::__ClearWingShining()
{
    DetachEffect(m_wingCostumeShiningEffect);
    memset(&m_wingCostumeShiningEffect, 0, sizeof(m_wingCostumeShiningEffect));
}

#endif
