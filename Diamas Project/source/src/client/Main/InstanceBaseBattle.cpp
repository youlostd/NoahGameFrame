#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "../PRTerrainLib/Terrain.h"
#include <game/GamePacket.hpp>
#include <game/MotionConstants.hpp>
#include <game/MotionTypes.hpp>

float NEW_UnsignedDegreeToSignedDegree(float fUD)
{
    float fSD;
    if (fUD > 180.0f)
        fSD = -(360.0f - fUD);
    else if (fUD < -180.0f)
        fSD = +(360.0f + fUD);
    else
        fSD = fUD;

    return fSD;
}

float NEW_GetSignedDegreeFromDirPixelPosition(const TPixelPosition &kPPosDir)
{
    Vector3 vtDir(kPPosDir.x, -kPPosDir.y, kPPosDir.z);
    Vector3 vtDirNormal;
    vtDir.Normalize(vtDirNormal);

    Vector3 vtDirNormalStan(0, -1, 0);
    float fDirRot = XMConvertToDegrees(acosf(vtDirNormal.Dot(vtDirNormalStan)));

    if (vtDirNormal.x < 0.0f)
        fDirRot = -fDirRot;

    return fDirRot;
}

bool CInstanceBase::IsFlyTargetObject()
{
    return m_GraphicThingInstance.IsFlyTargetObject();
}

float CInstanceBase::GetFlyTargetDistance()
{
    return m_GraphicThingInstance.GetFlyTargetDistance();
}

void CInstanceBase::ClearFlyTargetInstance()
{
    m_GraphicThingInstance.ClearFlyTarget();
}

void CInstanceBase::SetFlyTargetInstance(CInstanceBase &rkInstDst)
{
    m_GraphicThingInstance.SetFlyTarget(rkInstDst.GetGraphicThingInstancePtr());
}

void CInstanceBase::AddFlyTargetPosition(const TPixelPosition &c_rkPPosDst)
{
    m_GraphicThingInstance.AddFlyTarget(c_rkPPosDst);
}

void CInstanceBase::AddFlyTargetInstance(CInstanceBase &rkInstDst)
{
    m_GraphicThingInstance.AddFlyTarget(rkInstDst.GetGraphicThingInstancePtr());
}

float CInstanceBase::NEW_GetDistanceFromDestInstance(CInstanceBase &rkInstDst)
{
    TPixelPosition kPPosDst;
    rkInstDst.NEW_GetPixelPosition(&kPPosDst);

    return NEW_GetDistanceFromDestPixelPosition(kPPosDst);
}

float CInstanceBase::NEW_GetDistanceFromDestPixelPosition(const TPixelPosition &c_rkPPosDst)
{
    TPixelPosition kPPosCur;
    NEW_GetPixelPosition(&kPPosCur);

    TPixelPosition kPPosDir;
    kPPosDir = c_rkPPosDst - kPPosCur;

    return NEW_GetDistanceFromDirPixelPosition(kPPosDir);
}

float CInstanceBase::NEW_GetDistanceFromDirPixelPosition(const TPixelPosition &c_rkPPosDir)
{
    return sqrtf(c_rkPPosDir.x * c_rkPPosDir.x + c_rkPPosDir.y * c_rkPPosDir.y);
}

float CInstanceBase::NEW_GetRotation()
{
    float fCurRot = GetRotation();
    return NEW_UnsignedDegreeToSignedDegree(fCurRot);
}

float CInstanceBase::NEW_GetRotationFromDirPixelPosition(const TPixelPosition &c_rkPPosDir)
{
    return NEW_GetSignedDegreeFromDirPixelPosition(c_rkPPosDir);
}

float CInstanceBase::NEW_GetRotationFromDestPixelPosition(const TPixelPosition &c_rkPPosDst)
{
    TPixelPosition kPPosCur;
    NEW_GetPixelPosition(&kPPosCur);

    TPixelPosition kPPosDir;
    kPPosDir = c_rkPPosDst - kPPosCur;

    return NEW_GetRotationFromDirPixelPosition(kPPosDir);
}

float CInstanceBase::NEW_GetRotationFromDestInstance(CInstanceBase &rkInstDst)
{
    TPixelPosition kPPosDst;
    rkInstDst.NEW_GetPixelPosition(&kPPosDst);

    return NEW_GetRotationFromDestPixelPosition(kPPosDst);
}

void CInstanceBase::NEW_GetRandomPositionInFanRange(CInstanceBase &rkInstTarget, TPixelPosition *pkPPosDst)
{
    float fDstDirRot = NEW_GetRotationFromDestInstance(rkInstTarget);

    float fRot = GetRandom(fDstDirRot - 10.0f, fDstDirRot + 10.0f);

    Matrix kMatRot = Matrix::CreateRotationZ(DirectX::XMConvertToRadians(-fRot));

    Vector3 v3Src(0.0f, 8000.0f, 0.0f);
    Vector3 v3Pos = Vector3::Transform(v3Src, kMatRot);

    const TPixelPosition &c_rkPPosCur = NEW_GetCurPixelPositionRef();
    //const TPixelPosition& c_rkPPosFront=rkInstTarget.NEW_GetCurPixelPositionRef();

    pkPPosDst->x = c_rkPPosCur.x + v3Pos.x;
    pkPPosDst->y = c_rkPPosCur.y + v3Pos.y;
    pkPPosDst->z = __GetBackgroundHeight(c_rkPPosCur.x, c_rkPPosCur.y);
}

bool CInstanceBase::NEW_GetFrontInstance(CInstanceBase **ppoutTargetInstance, float fDistance)
{
    const float HALF_FAN_ROT_MIN = 10.0f;
    const float HALF_FAN_ROT_MAX = 50.0f;
    const float HALF_FAN_ROT_MIN_DISTANCE = 1000.0f;
    const float RPM = (HALF_FAN_ROT_MAX - HALF_FAN_ROT_MIN) / HALF_FAN_ROT_MIN_DISTANCE;

    float fDstRot = NEW_GetRotation();

    std::multimap<float, CInstanceBase *> kMap_pkInstNear;
    {
        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CPythonCharacterManager::CharacterIterator i;
        for (i = rkChrMgr.CharacterInstanceBegin(); i != rkChrMgr.CharacterInstanceEnd(); ++i)
        {
            CInstanceBase *pkInstEach = *i;
            if (pkInstEach == this)
                continue;

            if (!IsAttackableInstance(*pkInstEach))
                continue;

            if (NEW_GetDistanceFromDestInstance(*pkInstEach) > fDistance)
                continue;

            float fEachInstDistance = std::min(NEW_GetDistanceFromDestInstance(*pkInstEach), HALF_FAN_ROT_MIN_DISTANCE);
            float fEachInstDirRot = NEW_GetRotationFromDestInstance(*pkInstEach);

            float fHalfFanRot = (HALF_FAN_ROT_MAX - HALF_FAN_ROT_MIN) - RPM * fEachInstDistance + HALF_FAN_ROT_MIN;

            float fMinDstDirRot = fDstRot - fHalfFanRot;
            float fMaxDstDirRot = fDstRot + fHalfFanRot;

            if (fEachInstDirRot >= fMinDstDirRot && fEachInstDirRot <= fMaxDstDirRot)
                kMap_pkInstNear.
                    insert(std::multimap<float, CInstanceBase *>::value_type(fEachInstDistance, pkInstEach));
        }
    }

    if (kMap_pkInstNear.empty())
        return false;

    *ppoutTargetInstance = kMap_pkInstNear.begin()->second;

    return true;
}

// 2004.07.21.levites - ºñÆÄºÎ ´ÙÁß Å¸°Ù Áö¿ø
bool CInstanceBase::NEW_GetInstanceVectorInFanRange(float fSkillDistance, CInstanceBase &rkInstTarget,
                                                    std::vector<CInstanceBase *> *pkVct_pkInst)
{
    const float HALF_FAN_ROT_MIN = 20.0f;
    const float HALF_FAN_ROT_MAX = 40.0f;
    const float HALF_FAN_ROT_MIN_DISTANCE = 1000.0f;
    const float RPM = (HALF_FAN_ROT_MAX - HALF_FAN_ROT_MIN) / HALF_FAN_ROT_MIN_DISTANCE;

    float fDstDirRot = NEW_GetRotationFromDestInstance(rkInstTarget);

    // 2004.07.24.myevan - 비파부 가까이 있는 적부터 공격
    std::multimap<float, CInstanceBase *> kMap_pkInstNear;
    {
        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CPythonCharacterManager::CharacterIterator i;
        for (i = rkChrMgr.CharacterInstanceBegin(); i != rkChrMgr.CharacterInstanceEnd(); ++i)
        {
            CInstanceBase *pkInstEach = *i;
            if (pkInstEach == this)
                continue;

            // 2004.07.25.myevan - 적인 경우만 추가한다
            if (!IsAttackableInstance(*pkInstEach))
                continue;

            // 2004.07.21.levites - 비파부 다중 타겟 지원
            if (m_GraphicThingInstance.IsClickableDistanceDestInstance(pkInstEach->m_GraphicThingInstance,
                                                                       fSkillDistance))
            {
                float fEachInstDistance = std::min(NEW_GetDistanceFromDestInstance(*pkInstEach),
                                                   HALF_FAN_ROT_MIN_DISTANCE);
                float fEachInstDirRot = NEW_GetRotationFromDestInstance(*pkInstEach);

                float fHalfFanRot = (HALF_FAN_ROT_MAX - HALF_FAN_ROT_MIN) - RPM * fEachInstDistance + HALF_FAN_ROT_MIN;

                float fMinDstDirRot = fDstDirRot - fHalfFanRot;
                float fMaxDstDirRot = fDstDirRot + fHalfFanRot;

                if (fEachInstDirRot >= fMinDstDirRot && fEachInstDirRot <= fMaxDstDirRot)
                    kMap_pkInstNear.insert(
                        std::multimap<float, CInstanceBase *>::value_type(fEachInstDistance, pkInstEach));
            }
        }
    }

    {
        std::multimap<float, CInstanceBase *>::iterator i = kMap_pkInstNear.begin();
        for (i = kMap_pkInstNear.begin(); i != kMap_pkInstNear.end(); ++i)
            pkVct_pkInst->push_back(i->second);
    }

    if (pkVct_pkInst->empty())
        return false;

    return true;
}

bool CInstanceBase::NEW_GetInstanceVectorInCircleRange(float fSkillDistance, std::vector<CInstanceBase *> *pkVct_pkInst)
{
    std::multimap<float, CInstanceBase *> kMap_pkInstNear;

    {
        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CPythonCharacterManager::CharacterIterator i;
        for (i = rkChrMgr.CharacterInstanceBegin(); i != rkChrMgr.CharacterInstanceEnd(); ++i)
        {
            CInstanceBase *pkInstEach = *i;

            // 자신인 경우 추가하지 않는다
            if (pkInstEach == this)
                continue;

            // 적인 경우만 추가한다
            if (!IsAttackableInstance(*pkInstEach))
                continue;

            if (m_GraphicThingInstance.IsClickableDistanceDestInstance(pkInstEach->m_GraphicThingInstance,
                                                                       fSkillDistance))
            {
                float fEachInstDistance = NEW_GetDistanceFromDestInstance(*pkInstEach);
                kMap_pkInstNear.emplace(fEachInstDistance, pkInstEach);
            }
        }
    }

    {
        std::multimap<float, CInstanceBase *>::iterator i = kMap_pkInstNear.begin();
        for (i = kMap_pkInstNear.begin(); i != kMap_pkInstNear.end(); ++i)
            pkVct_pkInst->push_back(i->second);
    }

    if (pkVct_pkInst->empty())
        return false;

    return true;
}

bool CInstanceBase::NEW_IsClickableDistanceDestPixelPosition(const TPixelPosition &c_rkPPosDst)
{
    float fDistance = NEW_GetDistanceFromDestPixelPosition(c_rkPPosDst);

    if (fDistance > 150.0f)
        return false;

    return true;
}

bool CInstanceBase::NEW_IsClickableDistanceDestInstance(CInstanceBase &rkInstDst)
{
    float fDistance = 150.0f;

    if (IsBowMode())
        fDistance = __GetBowRange();

    if (rkInstDst.IsNPC())
        fDistance = 500.0f;

    if (rkInstDst.IsBuffBot())
        fDistance = 500.0f;

    if (rkInstDst.IsResource())
        fDistance = 100.0f;

    if (IsPoly())
        fDistance = 150.0f;

    return m_GraphicThingInstance.IsClickableDistanceDestInstance(rkInstDst.m_GraphicThingInstance, fDistance);
}

bool CInstanceBase::NEW_UseSkill(UINT uSkill, UINT uMot, UINT uMotLoopCount, bool isMovingSkill, uint32_t color)
{
    if (IsDead())
        return false;

    if (IsStun())
        return false;

    if (IsKnockDown())
        return false;

    if (isMovingSkill)
    {
        if (!IsWalking())
            StartWalking();

        m_isGoing = true;
    }
    else
    {
        if (IsWalking())
            EndWalking();

        m_isGoing = false;
    }

    float fCurRot = m_GraphicThingInstance.GetTargetRotation();
    SetAdvancingRotation(fCurRot);

    m_GraphicThingInstance.InterceptOnceMotion(MOTION_SKILL + uMot, 0.1f, uSkill, 1.0f, color);

    m_GraphicThingInstance.__OnUseSkill(uMot, uMotLoopCount, isMovingSkill, color);

    if (uMotLoopCount > 0)
        m_GraphicThingInstance.SetMotionLoopCount(uMotLoopCount);

    return true;
}

void CInstanceBase::NEW_Attack()
{
    float fDirRot = GetRotation();
    NEW_Attack(fDirRot);
}

void CInstanceBase::NEW_Attack(float fDirRot)
{
    if (IsDead())
        return;

    if (IsStun())
        return;

    if (IsKnockDown())
        return;

    if (IsUsingSkill())
        return;

    if (IsWalking())
        EndWalking();

    m_isGoing = false;

    if (IsPoly())
    {
        InputNormalAttack(fDirRot);
    }
    else
    {
        if (m_kHorse.IsMounting())
        {
            InputComboAttack(fDirRot);
        }
        else
        {
            InputComboAttack(fDirRot);
        }
    }
}

void CInstanceBase::NEW_AttackToDestPixelPositionDirection(const TPixelPosition &c_rkPPosDst)
{
    float fDirRot = NEW_GetRotationFromDestPixelPosition(c_rkPPosDst);

    NEW_Attack(fDirRot);
}

bool CInstanceBase::NEW_AttackToDestInstanceDirection(CInstanceBase &rkInstDst, IFlyEventHandler *pkFlyHandler)
{
    return NEW_AttackToDestInstanceDirection(rkInstDst);
}

bool CInstanceBase::NEW_AttackToDestInstanceDirection(CInstanceBase &rkInstDst)
{
    TPixelPosition kPPosDst;
    rkInstDst.NEW_GetPixelPosition(&kPPosDst);
    NEW_AttackToDestPixelPositionDirection(kPPosDst);

    return true;
}

void CInstanceBase::AttackProcess()
{
    if (!m_GraphicThingInstance.CanCheckAttacking())
        return;

    CInstanceBase *pkInstLast = NULL;
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin();
    while (rkChrMgr.CharacterInstanceEnd() != i)
    {
        CInstanceBase *pkInstEach = *i;
        ++i;

        // 서로간의 InstanceType 비교
        if (!IsAttackableInstance(*pkInstEach))
            continue;

        if (pkInstEach != this)
        {
            if (CheckAttacking(*pkInstEach))
            {
                pkInstLast = pkInstEach;
            }
        }
    }

    if (pkInstLast)
    {
        m_dwLastDmgActorVID = pkInstLast->GetVirtualID();
    }
}

void CInstanceBase::InputNormalAttack(float fAtkDirRot)
{
    m_GraphicThingInstance.InputNormalAttackCommand(fAtkDirRot);
}

void CInstanceBase::InputComboAttack(float fAtkDirRot)
{
    m_GraphicThingInstance.InputComboAttackCommand(fAtkDirRot);
    __ComboProcess();
}

void CInstanceBase::RunNormalAttack(float fAtkDirRot)
{
    EndGoing();
    m_GraphicThingInstance.NormalAttack(fAtkDirRot);
}

void CInstanceBase::RunComboAttack(float fAtkDirRot, uint32_t wMotionIndex)
{
    EndGoing();
    m_GraphicThingInstance.ComboAttack(wMotionIndex, fAtkDirRot);
}

// ¸®ÅÏ°ª TRUE°¡ ¹«¾ùÀÎ°¡°¡ ÀÖ´Ù
bool CInstanceBase::CheckAdvancing()
{
    if (!__IsMainInstance() && !IsAttacking())
    {
        if (IsPC() && IsWalking())
        {
            CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
            for (CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin();
                 i != rkChrMgr.CharacterInstanceEnd(); ++i)
            {
                CInstanceBase *pkInstEach = *i;
                if (pkInstEach == this)
                    continue;
                if (!pkInstEach->IsDoor())
                    continue;

                if (m_GraphicThingInstance.TestActorCollision(pkInstEach->GetGraphicThingInstanceRef()))
                {
                    BlockMovement();
                    return true;
                }
            }
        }
        return FALSE;
    }

    if (m_GraphicThingInstance.CanSkipCollision())
    {
        //Tracenf("%x VID %d 충돌 스킵", ELTimer_GetMSec(), GetVirtualID());
        return FALSE;
    }


    bool bUsingSkill = m_GraphicThingInstance.IsUsingSkill();

    m_dwAdvActorVID = 0;
    UINT uCollisionCount = 0;
    CPythonBackground &rkBG = CPythonBackground::Instance();
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    for (CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin();
         i != rkChrMgr.CharacterInstanceEnd(); ++i)
    {
        CInstanceBase *pkInstEach = *i;
        if (pkInstEach == this)
            continue;

        CActorInstance &rkActorSelf = m_GraphicThingInstance;
        CActorInstance &rkActorEach = pkInstEach->GetGraphicThingInstanceRef();

        if (pkInstEach->IsInSafe())
            continue;

        if (pkInstEach->IsGameMaster() || IsGameMaster())
            continue;

    	
        //NOTE : Skil을 쓰더라도 Door Type과는 Collision체크 한다.
        if (bUsingSkill && !rkActorEach.IsDoor())
            continue;

        if (rkActorEach.IsMount() || rkActorEach.IsPet() || rkActorEach.IsShop() || rkActorEach.IsAttackPet())
            continue;

        if (rkActorEach.IsPC() && rkBG.IsDuelMap(rkBG.GetMapIndex()) && !__FindPVPKey(
                rkActorEach.GetVirtualID(), rkActorSelf.GetVirtualID()))
            continue;

#ifdef __OFFLINE_SHOP__
        if (pkInstEach->GetRace() == 30008)
        {
            continue;
        }
#endif

        if(!rkActorEach.IsPC() && rkBG.IsDuelMap(rkBG.GetMapIndex()))
            continue;

        if (rkActorEach.IsPC() && strcmp(rkBG.GetWarpMapName(), "metin2_map_oxevent") == 0)
        {
            TPixelPosition pPixelPosition = rkActorSelf.NEW_GetCurPixelPositionRef();

            int x = pPixelPosition.x / 100;
            int y = pPixelPosition.y / 100;

            Vector2 playerPos(x, y);
            Vector2 cornerA(221, 228);
            Vector2 cornerB(300, 228);
            Vector2 cornerC(300, 265);
            Vector2 cornerD(221, 265);

            int width = cornerB.x - cornerA.x;
            int height = cornerD.y * cornerC.y;

            if (playerPos.x > cornerA.x && playerPos.x < cornerA.x + width &&
                playerPos.y > cornerA.y && playerPos.y < cornerA.x + height) //Inside the rectangle
                continue;
        }

        // 앞으로 전진할수 있는가?
        if (rkActorSelf.TestActorCollision(rkActorEach))
        {
            uCollisionCount++;
            if (uCollisionCount == 2)
            {
                rkActorSelf.BlockMovement();
                return TRUE;
            }
            rkActorSelf.AdjustDynamicCollisionMovement(&rkActorEach);

            if (rkActorSelf.TestActorCollision(rkActorEach))
            {
                rkActorSelf.BlockMovement();
                return TRUE;
            }
            else
            {
                NEW_MoveToDestPixelPositionDirection(NEW_GetDstPixelPositionRef());
            }
        }
    }

    // 맵속성 체크

    const Vector3 &rv3Position = m_GraphicThingInstance.GetPosition();
    const Vector3 &rv3MoveDirection = m_GraphicThingInstance.GetMovementVectorRef();

    // NOTE : 만약 이동 거리가 크다면 쪼개서 구간 별로 속성을 체크해 본다
    //        현재 설정해 놓은 10.0f는 임의의 거리 - [levites]
    int iStep = int(rv3MoveDirection.Length() / 10.0f);
    Vector3 v3CheckStep = rv3MoveDirection / float(iStep);
    Vector3 v3CheckPosition = rv3Position;
    for (int j = 0; j < iStep; ++j)
    {
        v3CheckPosition += v3CheckStep;

        // Check
        if (rkBG.isAttrOn(v3CheckPosition.x, -v3CheckPosition.y, CTerrainImpl::ATTRIBUTE_BLOCK))
        {
            BlockMovement();
            //return TRUE;
        }
    }

    // Check
    Vector3 v3NextPosition = rv3Position + rv3MoveDirection;
    if (rkBG.isAttrOn(v3NextPosition.x, -v3NextPosition.y, CTerrainImpl::ATTRIBUTE_BLOCK))
    {
        BlockMovement();
        return TRUE;
    }

    return FALSE;
}

bool CInstanceBase::CheckAttacking(CInstanceBase &rkInstVictim)
{
    if (IsInSafe())
        return false;

    if (rkInstVictim.IsInSafe())
        return false;

    if (!m_GraphicThingInstance.AttackingProcess(rkInstVictim.m_GraphicThingInstance))
        return false;

    return true;
}

bool CInstanceBase::isNormalAttacking()
{
    return m_GraphicThingInstance.isNormalAttacking();
}

bool CInstanceBase::isComboAttacking()
{
    return m_GraphicThingInstance.isComboAttacking();
}

bool CInstanceBase::IsUsingSkill()
{
    return m_GraphicThingInstance.IsUsingSkill();
}

bool CInstanceBase::IsUsingMovingSkill()
{
    return m_GraphicThingInstance.IsUsingMovingSkill();
}

bool CInstanceBase::CanCancelSkill()
{
    return m_GraphicThingInstance.CanCancelSkill();
}

bool CInstanceBase::CanAttackHorseLevel()
{
    if (!IsMountingHorse())
        return FALSE;

    return m_kHorse.CanAttack();
}

uint32_t CInstanceBase::GetNormalAttackIndex()
{
    return m_GraphicThingInstance.GetNormalAttackIndex();
}

uint32_t CInstanceBase::GetComboIndex()
{
    return m_GraphicThingInstance.GetComboIndex();
}

float CInstanceBase::GetAttackingElapsedTime()
{
    return m_GraphicThingInstance.GetAttackingElapsedTime();
}

void CInstanceBase::ProcessHitting(uint32_t dwMotionKey, CInstanceBase *pVictimInstance)
{
    assert(!"-_-" && "CInstanceBase::ProcessHitting");
    //m_GraphicThingInstance.ProcessSucceedingAttacking(dwMotionKey, pVictimInstance->m_GraphicThingInstance);
}

void CInstanceBase::ProcessHitting(uint32_t dwMotionKey, uint8_t byEventIndex, CInstanceBase *pVictimInstance)
{
    assert(!"-_-" && "CInstanceBase::ProcessHitting");
    //m_GraphicThingInstance.ProcessSucceedingAttacking(dwMotionKey, byEventIndex, pVictimInstance->m_GraphicThingInstance);
}

void CInstanceBase::GetBlendingPosition(TPixelPosition *pPixelPosition)
{
    m_GraphicThingInstance.GetBlendingPosition(pPixelPosition);
}

void CInstanceBase::SetBlendingPosition(const TPixelPosition &c_rPixelPosition)
{
    m_GraphicThingInstance.SetBlendingPosition(c_rPixelPosition);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInstanceBase::Revive()
{
    m_isGoing = false;
    m_GraphicThingInstance.Revive();

    __AttachHorseSaddle();
}

void CInstanceBase::Stun()
{
    NEW_Stop();
    m_GraphicThingInstance.Stun();

    AttachEffect(EFFECT_STUN, 0, 1, EFFECT_KIND_BATTLE);
}

void CInstanceBase::Die()
{
    __DetachHorseSaddle();

 /*if (m_stateFlags & ADD_CHARACTER_STATE_SPAWN)
        AttachEffect(EFFECT_SPAWN_DISAPPEAR, 0, 1, EFFECT_KIND_MISC);*/ 
    ClearAffects();

    OnUnselected();
    OnUntargeted();

    m_GraphicThingInstance.Die();
}

void CInstanceBase::Hide()
{
    m_GraphicThingInstance.SetAlphaValue(0.0f);
    m_GraphicThingInstance.BlendAlphaValue(0.0f, 0.1f);
}

void CInstanceBase::Show()
{
    m_GraphicThingInstance.SetAlphaValue(1.0f);
    m_GraphicThingInstance.BlendAlphaValue(1.0f, 0.1f);
}
