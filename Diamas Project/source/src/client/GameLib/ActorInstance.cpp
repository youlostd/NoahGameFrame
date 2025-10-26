#include "StdAfx.h"
#include "ActorInstance.h"
#include "AreaTerrain.h"
#include "RaceData.h"
#include "../SpeedTreeLib/SpeedTreeForest.h"
#include "../SpeedTreeLib/SpeedTreeWrapper.h"
#include <game/MotionConstants.hpp>
#include <game/MotionTypes.hpp>
#include "RaceManager.h"
#include "base/Remotery.h"

void CActorInstance::Deform()
{
    rmt_ScopedCPUSample(ActorInstanceDeform, RMTSF_Aggregate);

    CGraphicObjectInstance::Deform();
    TraceProcess();
}

void CActorInstance::Transform()
{
    if (m_pkHorse)
    {
        m_pkHorse->Transform();

        m_x = m_pkHorse->NEW_GetCurPixelPositionRef().x;
        m_y = -m_pkHorse->NEW_GetCurPixelPositionRef().y;
        m_z = m_pkHorse->NEW_GetCurPixelPositionRef().z;
        m_bNeedUpdateCollision = TRUE;
    }

    UpdateLODLevel();

    TransformMovement();

    CGraphicObjectInstance::Transform();

    UpdatePointInstance();
    ShakeProcess();

    // This should be done in Update()?
    UpdateBoundingSphere();

    UpdateAttribute();
}

void CActorInstance::OnUpdate()
{
    if (m_pkHorse)
        m_pkHorse->Update();

    if (!IsParalysis())
        CGraphicThingInstance::OnUpdate();

    UpdateAttachingInstances();

    __BlendAlpha_Update();
}

// 2004.07.05.myevan. ±Ã½ÅÅº¿µ ¸Ê¿¡ ³¢ÀÌ´Â ¹®Á¦ÇØ°á

void CActorInstance::SetMainInstance()
{
    m_isMain = true;
}

void CActorInstance::SetParalysis(bool isParalysis)
{
    m_isParalysis = isParalysis;
}

void CActorInstance::SetFaint(bool isFaint)
{
    m_isFaint = isFaint;
}

void CActorInstance::SetSleep(bool isSleep)
{
    m_isSleep = isSleep;

    Stop();
}

void CActorInstance::SetResistFallen(bool isResistFallen)
{
    m_isResistFallen = isResistFallen;
}

void CActorInstance::SetReachScale(float fScale)
{
    m_fReachScale = fScale;
}

float CActorInstance::__GetReachScale()
{
    return m_fReachScale;
}

float CActorInstance::__GetAttackSpeed()
{
    return m_fAtkSpd;
}

uint16_t CActorInstance::__GetCurrentComboType()
{
    if (IsBowMode())
        return 0;
    if (IsHandMode())
        return 0;
    if (__IsMountingHorse())
        return 0;

    return m_wcurComboType;
}

void CActorInstance::SetComboType(uint16_t wComboType)
{
    m_wcurComboType = wComboType;
}

void CActorInstance::SetAttackSpeed(float fAtkSpd)
{
    m_fAtkSpd = fAtkSpd;
}

void CActorInstance::SetMoveSpeed(float fMovSpd)
{
    if (m_fMovSpd == fMovSpd)
        return;

    m_fMovSpd = fMovSpd;

    if (__IsMoveMotion())
    {
        Stop();
        Move();
    }
}

void CActorInstance::SetFishingPosition(Vector3 &rv3Position)
{
    m_v3FishingPosition = rv3Position;
}

// ActorInstanceMotion.cpp ¿¡ ³Öµµ·Ï ÇÏÀÚ
void CActorInstance::Move()
{
    if (m_isWalking)
    {
        SetLoopMotion(MOTION_WALK, 0.15f, m_fMovSpd);
    }
    else
    {
        SetLoopMotion(MOTION_RUN, 0.15f, m_fMovSpd);
    }
}

void CActorInstance::Stop(float fBlendingTime)
{
    __ClearMotion();
    SetLoopMotion(MOTION_WAIT, fBlendingTime);
}

void CActorInstance::SetOwner(uint32_t dwOwnerVID)
{
    m_fOwnerBaseTime = GetLocalTime();
    m_dwOwnerVID = dwOwnerVID;
}

void CActorInstance::SetActorType(UINT eType)
{
    m_eActorType = eType;
}

void CActorInstance::SetActorAiFlags(uint32_t flags)
{
    m_eAiFlag = flags;
}

UINT CActorInstance::GetActorType() const
{
    return m_eActorType;
}

bool CActorInstance::IsHandMode()
{
    if (MOTION_MODE_GENERAL == GetMotionMode())
        return true;

    if (MOTION_MODE_HORSE == GetMotionMode())
        return true;

    return false;
}

bool CActorInstance::IsTwoHandMode()
{
    if (MOTION_MODE_TWOHAND_SWORD == GetMotionMode())
        return true;

    return false;
}

bool CActorInstance::IsBowMode()
{
    if (MOTION_MODE_BOW == GetMotionMode())
        return true;

    if (MOTION_MODE_HORSE_BOW == GetMotionMode())
        return true;

    return false;
}

bool CActorInstance::IsPoly()
{
    return (m_eActorType == TYPE_POLY || (m_eActorType == TYPE_PC && m_eRace >= MAIN_RACE_MAX_NUM));
}

bool CActorInstance::IsPC()
{
    return (m_eActorType == TYPE_PC);
}

bool CActorInstance::IsNPC()
{
    return (m_eActorType == TYPE_NPC) || (m_eActorType == TYPE_PET) || (m_eActorType == TYPE_ATTACK_PET) || (
               m_eActorType == TYPE_MOUNT);
}

bool CActorInstance::IsBuffBot()
{
    return (m_eActorType == TYPE_BUFFBOT);
}

bool CActorInstance::IsEnemy()
{
    return (m_eActorType == TYPE_ENEMY);
}

bool CActorInstance::IsStone()
{
    return (m_eActorType == TYPE_STONE);
}

bool CActorInstance::IsWarp()
{
    return (m_eActorType == TYPE_WARP);
}

bool CActorInstance::IsGoto()
{
    return (m_eActorType == TYPE_GOTO);
}

bool CActorInstance::IsBuilding()
{
    return (m_eActorType == TYPE_BUILDING);
}

bool CActorInstance::IsDoor()
{
    return (m_eActorType == TYPE_DOOR);
}

bool CActorInstance::IsObject()
{
    return (m_eActorType == TYPE_OBJECT);
}

bool CActorInstance::IsShop()
{
    return (m_eRace == 30008);
}

bool CActorInstance::IsPet()
{
    return (m_eActorType == TYPE_PET);
}

bool CActorInstance::IsAttackPet()
{
    return (m_eActorType == TYPE_ATTACK_PET);
}

bool CActorInstance::IsMount()
{
    return (m_eActorType == TYPE_MOUNT);
}

bool CActorInstance::IsGrowthPet()
{
    return (m_eActorType == TYPE_GROWTH_PET);
}

uint32_t CActorInstance::GetRank()
{
    return m_eRank;
}

void CActorInstance::SetRank(uint8_t rank)
{
    m_eRank = rank;
}

void CActorInstance::DestroySystem()
{
}

void CActorInstance::DieEnd()
{
    Die();

    CGraphicThingInstance::SetMotionAtEnd();
}

void CActorInstance::Die()
{
    if (m_isRealDead)
        return;

    if (__IsMoveMotion())
        Stop();

    SetAdvancingRotation(GetRotation());

    if (IsStone())
    {
        InterceptOnceMotion(MOTION_DEAD);
    }
    else
    {
        if (!__IsDieMotion())
        {
            InterceptOnceMotion(MOTION_DEAD);
        }
    }

    m_isRealDead = TRUE;
}

bool CActorInstance::IsSleep()
{
    return m_isSleep;
}

bool CActorInstance::IsParalysis()
{
    return m_isParalysis;
}

bool CActorInstance::IsFaint()
{
    return m_isFaint;
}

bool CActorInstance::IsResistFallen()
{
    return m_isResistFallen;
}

bool CActorInstance::IsMoving()
{
    return __IsMoveMotion();
}

bool CActorInstance::IsWaiting()
{
    return __IsWaitMotion();
}

bool CActorInstance::IsDead()
{
    return m_isRealDead;
}

bool CActorInstance::IsKnockDown()
{
    return __IsKnockDownMotion();
}

bool CActorInstance::IsEmotionMotion()
{
    return __IsEmotionMotion();
}

bool CActorInstance::IsDamage()
{
    return __IsDamageMotion();
}

bool CActorInstance::IsAttacked()
{
    if (IsPushing())
        return TRUE;

    if (__IsDamageMotion())
        return TRUE;

    if (__IsKnockDownMotion())
        return TRUE;

    if (__IsDieMotion())
        return TRUE;

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Process
void CActorInstance::PhysicsProcess()
{
    m_PhysicsObject.Update(m_fSecondElapsed);
    AddMovement(m_PhysicsObject.GetXMovement(), m_PhysicsObject.GetYMovement(), 0.0f);
}

void CActorInstance::__AccumulationMovement(float fRot)
{
    // NOTE - ÀÏ´ÜÀº WAIT·Î ¹Ì²ô·¯Áü ¹æÁö
    //        ÃßÈÄ¿¡´Â RaceMotionData°¡ ÀÌµ¿µÇ´Â ¸ð¼ÇÀÎÁö¿¡ ´ëÇÑ Flag¸¦ °®°í ÀÖ°Ô²û ÇÑ´Ù. - [levites]
    if (MOTION_WAIT == __GetCurrentMotionIndex())
        return;

    Matrix s_matRotationZ = Matrix::CreateRotationZ(DirectX::XMConvertToRadians(fRot));
    UpdateTransform(&s_matRotationZ, GetAverageSecondElapsed());

#ifdef ENABLE_FLY_MOUNT
    AddMovement(s_matRotationZ._41, s_matRotationZ._42,
                s_matRotationZ._43 + ucanAdam_z);
#else
    AddMovement(s_matRotationZ._41, s_matRotationZ._42, s_matRotationZ._43);
#endif
}

void CActorInstance::AccumulationMovement()
{
    if (m_pkTree)
        return;

    if (m_pkHorse)
    {
        m_pkHorse->__AccumulationMovement(m_fcurRotation);
        //      		m_pkHorse->__AccumulationMovement(m_fAdvancingRotation);
        return;
    }

    __AccumulationMovement(m_fAdvancingRotation);
}

void CActorInstance::TransformMovement()
{
    if (!IsParalysis())
    {
        m_x += m_v3Movement.x;
        m_y += m_v3Movement.y;
        m_z += m_v3Movement.z;
    }

    __InitializeMovement();

    SetPosition(m_x, m_y, m_z);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Process

void CActorInstance::OnUpdateCollisionData(const CStaticCollisionDataVector *pscdVector)
{
    assert(pscdVector);
    CStaticCollisionDataVector::const_iterator it;
    for (it = pscdVector->begin(); it != pscdVector->end(); ++it)
    {
        const CStaticCollisionData &c_rColliData = *it;
        const Matrix &c_rMatrix = GetTransform();
        AddCollision(&c_rColliData, &c_rMatrix);
    }
}

void CActorInstance::OnUpdateHeighInstance(CAttributeInstance *pAttributeInstance)
{
    assert(pAttributeInstance);
    SetHeightInstance(pAttributeInstance);
}

bool CActorInstance::OnGetObjectHeight(float fX, float fY, float *pfHeight)
{
    if (!m_pHeightAttributeInstance)
        return false;

    if (TYPE_BUILDING != GetType())
        return false;

    return m_pHeightAttributeInstance->GetHeight(fX, fY, pfHeight) == 1 ? true : false;
}

//////////////////////////////////////////////////////////////////
// Battle
void CActorInstance::Revive()
{
    m_isSleep = FALSE;
    m_isParalysis = FALSE;
    m_isFaint = FALSE;
    m_isRealDead = FALSE;
    m_isStun = FALSE;
    m_isWalking = FALSE;
    m_isMain = FALSE;
    m_isResistFallen = FALSE;

    __InitializeCollisionData();
}

bool CActorInstance::IsStun()
{
    return m_isStun;
}

void CActorInstance::Stun()
{
    m_isStun = TRUE;
}

void CActorInstance::SetWalkMode()
{
    m_isWalking = true;
    if (MOTION_RUN == MakeMotionId(m_kCurMotNode.dwMotionKey).index)
        SetLoopMotion(MOTION_WALK, 0.15f, m_fMovSpd);
}

void CActorInstance::SetRunMode()
{
    m_isWalking = false;
    if (MOTION_WALK == MakeMotionId(m_kCurMotNode.dwMotionKey).index)
        SetLoopMotion(MOTION_RUN, 0.15f, m_fMovSpd);
}

uint32_t CActorInstance::GetNormalAttackIndex()
{
    uint16_t wMotionIndex;
    m_pkCurRaceData->GetNormalAttackIndex(GetMotionMode(), &wMotionIndex);

    return MakeMotionKey(GetMotionMode(), wMotionIndex);
}

//////////////////////////////////////////////////////////////////
// Movement
void CActorInstance::__InitializeMovement()
{
    m_v3Movement.x = 0.0f;
    m_v3Movement.y = 0.0f;
    m_v3Movement.z = 0.0f;
}

void CActorInstance::AddMovement(float fx, float fy, float fz)
{
    m_v3Movement.x += fx;
    m_v3Movement.y += fy;
    m_v3Movement.z += fz;
}

#ifdef ENABLE_FLY_MOUNT
void CActorInstance::UcanAdam(float z)
{
    if (m_pkHorse) {
        if ((m_pkHorse->ucanAdam_z + z) < 0.0f)
            return;

        if ((m_pkHorse->ucanAdam_z + z) == 0.0f)
            DisableSkipCollision();
        else if ((m_pkHorse->ucanAdam_z + z) > 0.0f)
            EnableSkipCollision();

        switch (m_pkHorse->GetRace()) {
            case 48524: // Ucan binek kodu binek kodunu mob olarak
                        // giriceksiniz item olarak cağırcaksınız.
                m_pkHorse->ucanAdam_z += z;
                break;
            default:
                break;
        }

        if (MOTION_WAIT == __GetCurrentMotionIndex())
            m_pkHorse->AddMovement(0.0f, 0.0f, m_pkHorse->ucanAdam_z);
    }
}
#endif

const float gc_fActorSlideMoveSpeed = 5.0f;

void CActorInstance::AdjustDynamicCollisionMovement(const CActorInstance *c_pActorInstance)
{
    if (m_pkHorse)
    {
        m_pkHorse->AdjustDynamicCollisionMovement(c_pActorInstance);
        return;
    }

    #ifdef ENABLE_FLY_MOUNT
    if (ucanAdam_z > 0.0f)
        return;
#endif

    // NOTE : 기존의 Sphere Overlap됬을경우 처리가 비비기를 하면은 Penetration될 위험이 많아서 ( 실제로도 나왔고 --)
    // Sphere간 Collision이 생겼을 경우 이전위치로 RollBack하는 방식으로 바꿨다.
    // 단 BGObject에 대해서만.

    if (isAttacking())
        return;

    UINT uActorType = c_pActorInstance->GetActorType();
    if ((uActorType == TYPE_BUILDING) || (uActorType == TYPE_OBJECT) || (uActorType == TYPE_DOOR) || (
            uActorType == TYPE_STONE))
    {
        BlockMovement();

        //Movement초기화
        /*	m_v3Movement = Vector3(0.f,0.f,0.f);
    
            TCollisionPointInstanceListIterator itMain = m_BodyPointInstanceList.begin();
            for (; itMain != m_BodyPointInstanceList.end(); ++itMain)
            {
                CDynamicSphereInstanceVector & c_rMainSphereVector = (*itMain).SphereInstanceVector;
                for (uint32_t i = 0; i < c_rMainSphereVector.size(); ++i)
                {
                    CDynamicSphereInstance & c_rMainSphere = c_rMainSphereVector[i];
                    c_rMainSphere.v3Position =c_rMainSphere.v3LastPosition;
                }
            }*/
    }
    else
    {
        float move_length = m_v3Movement.Length();
        if (move_length > gc_fActorSlideMoveSpeed)
            m_v3Movement *= gc_fActorSlideMoveSpeed / move_length;

        TCollisionPointInstanceListIterator itMain = m_BodyPointInstanceList.begin();
        for (; itMain != m_BodyPointInstanceList.end(); ++itMain)
        {
            CDynamicSphereInstanceVector &c_rMainSphereVector = (*itMain).SphereInstanceVector;
            for (uint32_t i = 0; i < c_rMainSphereVector.size(); ++i)
            {
                CDynamicSphereInstance &c_rMainSphere = c_rMainSphereVector[i];

                TCollisionPointInstanceList::const_iterator itOpp = c_pActorInstance->m_BodyPointInstanceList.begin();
                for (; itOpp != c_pActorInstance->m_BodyPointInstanceList.end(); ++itOpp)
                {
                    CSphereCollisionInstance s;
                    s.GetAttribute().fRadius = itOpp->SphereInstanceVector[0].fRadius;
                    s.GetAttribute().v3Position = itOpp->SphereInstanceVector[0].v3Position;
                    Vector3 v3Delta = s.GetCollisionMovementAdjust(c_rMainSphere);
                    m_v3Movement += v3Delta;
                    c_rMainSphere.v3Position += v3Delta;

                    if (v3Delta.x != 0.0f || v3Delta.y != 0.0f || v3Delta.z != 0.0f)
                    {
                        move_length = m_v3Movement.Length();
                        if (move_length > gc_fActorSlideMoveSpeed)
                        {
                            m_v3Movement *= gc_fActorSlideMoveSpeed / move_length;
                            c_rMainSphere.v3Position = c_rMainSphere.v3LastPosition;
                            c_rMainSphere.v3Position += m_v3Movement;
                        }
                    }
                }
            }
        }
    }
}

void CActorInstance::__AdjustCollisionMovement(const CGraphicObjectInstance *c_pGraphicObjectInstance)
{
    if (m_pkHorse)
    {
        m_pkHorse->__AdjustCollisionMovement(c_pGraphicObjectInstance);
        return;
    }

    #ifdef ENABLE_FLY_MOUNT
    if (ucanAdam_z > 0.0f)
        return;
#endif

    // Body는 하나임을 가정합니다.

    if (m_v3Movement.x == 0.0f && m_v3Movement.y == 0.0f && m_v3Movement.z == 0.0f)
        return;

    float move_length = m_v3Movement.Length();
    if (move_length > gc_fActorSlideMoveSpeed)
        m_v3Movement *= gc_fActorSlideMoveSpeed / move_length;

    TCollisionPointInstanceListIterator itMain = m_BodyPointInstanceList.begin();
    for (; itMain != m_BodyPointInstanceList.end(); ++itMain)
    {
        CDynamicSphereInstanceVector &c_rMainSphereVector = (*itMain).SphereInstanceVector;
        for (uint32_t i = 0; i < c_rMainSphereVector.size(); ++i)
        {
            CDynamicSphereInstance &c_rMainSphere = c_rMainSphereVector[i];

            Vector3 v3Delta = c_pGraphicObjectInstance->GetCollisionMovementAdjust(c_rMainSphere);
            m_v3Movement += v3Delta;
            c_rMainSphere.v3Position += v3Delta;

            if (v3Delta.x != 0.0f || v3Delta.y != 0.0f || v3Delta.z != 0.0f)
            {
                move_length = m_v3Movement.Length();
                if (move_length > gc_fActorSlideMoveSpeed)
                {
                    m_v3Movement *= gc_fActorSlideMoveSpeed / move_length;
                    c_rMainSphere.v3Position = c_rMainSphere.v3LastPosition;
                    c_rMainSphere.v3Position += m_v3Movement;
                }
            }

            /*if (c_pObjectInstance->CollisionDynamicSphere(c_rMainSphere))
            {
                const Vector3 & c_rv3Position = c_pObjectInstance->GetPosition();
                //if (GetVector3Distance(c_rMainSphere.v3Position, c_rv3Position) <
                //	GetVector3Distance(c_rMainSphere.v3LastPosition, c_rv3Position))
                {
                    return TRUE;
                }

                return FALSE;
            }*/
        }
    }
}

bool CActorInstance::IsMovement()
{
    if (m_pkHorse)
        if (m_pkHorse->IsMovement())
            return TRUE;

    if (0.0f != m_v3Movement.x)
        return TRUE;
    if (0.0f != m_v3Movement.y)
        return TRUE;
    if (0.0f != m_v3Movement.z)
        return TRUE;

    return FALSE;
}

float CActorInstance::GetHeight()
{
    auto raceHeight = CRaceManager::instance().GetRaceHeight(m_eRace);
    if (raceHeight)
        return raceHeight.value();

    const auto height = CGraphicThingInstance::GetHeight();
    CRaceManager::instance().SetRaceHeight(m_eRace, height);
    return height;
}

bool CActorInstance::IntersectDefendingSphere()
{
    for (TCollisionPointInstanceList::iterator it = m_DefendingPointInstanceList.begin();
         it != m_DefendingPointInstanceList.end(); ++it)
    {
        CDynamicSphereInstanceVector &rSphereInstanceVector = (*it).SphereInstanceVector;

        CDynamicSphereInstanceVector::iterator it2 = rSphereInstanceVector.begin();
        for (; it2 != rSphereInstanceVector.end(); ++it2)
        {
            CDynamicSphereInstance &rInstance = *it2;
            Vector3 v3SpherePosition = rInstance.v3Position;
            float fRadius = rInstance.fRadius;

            Vector3 v3Orig;
            Vector3 v3Dir;
            float fRange;
            ms_Ray.GetStartPoint(&v3Orig);
            ms_Ray.GetDirection(&v3Dir, &fRange);

            Vector3 v3Distance = v3Orig - v3SpherePosition;
            float b = v3Dir.Dot(v3Distance);
            float c = v3Distance.Dot(v3Distance) - fRadius * fRadius;

            if (b * b - c >= 0)
                return true;
        }
    }
    return false;
}

bool CActorInstance::__IsMountingHorse() const
{
    return NULL != m_pkHorse;
}

CActorInstance *CActorInstance::GetHorseActor()
{
    return m_pkHorse;
}

void CActorInstance::MountHorse(CActorInstance *pkHorse)
{
    m_pkHorse = pkHorse;

    if (m_pkHorse)
    {
        m_pkHorse->SetCurPixelPosition(NEW_GetCurPixelPositionRef());
        m_pkHorse->SetRotation(GetRotation());
        m_pkHorse->SetAdvancingRotation(GetRotation());
    }
}

void CActorInstance::__CreateTree(const char *c_szFileName)
{
  /*
   *
    __DestroyTree();
    
    auto &rkForest = CSpeedTreeForest::Instance();
    m_pkTree = rkForest.CreateInstance(m_x, m_y, m_z, ComputeCrc32(0, c_szFileName, strlen(c_szFileName)),
                                       c_szFileName);
    m_pkTree->SetPosition(m_x, m_y, m_z);
    m_pkTree->UpdateBoundingSphere();
    m_pkTree->UpdateCollisionData();
       */

}

void CActorInstance::__DestroyTree()
{
    if (!m_pkTree)
        return;

    //CSpeedTreeForest::Instance().DeleteInstance(m_pkTree);
}

void CActorInstance::__SetTreePosition(float fx, float fy, float fz)
{
     /*

    if (!m_pkTree)
        return;
    if (m_x == fx && m_y == fy && m_z == fz)
        return;

    m_pkTree->SetPosition(fx, fy, fz);
    m_pkTree->UpdateBoundingSphere();
    m_pkTree->UpdateCollisionData();
     */

}

bool CActorInstance::HasFlag(uint32_t flag) const
{
    return 0 != (m_flags & flag);
}

void CActorInstance::SetFlag(uint32_t flag, bool value)
{
    if (value)
        m_flags |= flag;
    else
        m_flags &= ~flag;
}

void CActorInstance::ClearAttachingEffect()
{
    __ClearAttachingEffect();
}

void CActorInstance::Destroy()
{
    ClearFlyTargeter();

    m_HitDataMap.clear();
    m_MotionDeque.clear();

    if (m_pAttributeInstance)
    {
        delete m_pAttributeInstance;
        m_pAttributeInstance = NULL;
    }

    __ClearAttachingEffect();

    CGraphicThingInstance::Clear();

    __DestroyWeaponTrace();
    __DestroyTree();
    //m_PhysicsObject.SetActorInstance(NULL);

    __Initialize();
}

void CActorInstance::__InitializeRotationData()
{
    m_fAtkDirRot = 0.0f;
    m_fcurRotation = 0.0f;
    m_rotBegin = 0.0f;
    m_rotEnd = 0.0f;
    m_rotEndTime = 0.0f;
    m_rotBeginTime = 0.0f;
    m_rotBlendTime = 0.0f;
    m_fAdvancingRotation = 0.0f;
    m_rotX = 0.0f;
    m_rotY = 0.0f;
}

void CActorInstance::__InitializeStateData()
{
    m_bEffectInitialized = false;

    m_isPreInput = FALSE;
    m_isNextPreInput = FALSE;

    m_isSleep = FALSE;
    m_isParalysis = FALSE;
    m_isFaint = FALSE;
    m_isRealDead = FALSE;
    m_isWalking = FALSE;
    m_isMain = FALSE;
    m_isStun = FALSE;
    m_isHiding = FALSE;
    m_isResistFallen = FALSE;

    m_iRenderMode = RENDER_MODE_NORMAL;
    m_fAlphaValue = 0.0f;
    m_AddColor = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f);

    m_dwMtrlColor = 0xffffffff;
    m_dwMtrlAlpha = 0xff000000;

    m_dwBattleHitEffectID = 0;
    m_dwBattleAttachEffectID = 0;
}

void CActorInstance::__InitializeMotionData()
{
    m_wcurMotionMode = MOTION_MODE_GENERAL;
    m_wcurComboType = 0;

    m_fReachScale = 1.0f;
    m_fMovSpd = 1.0f;
    m_fAtkSpd = 1.0f;

    m_fInvisibleTime = 0.0f;

    m_kSplashArea.isEnableHitProcess = TRUE;
    m_kSplashArea.uSkill = 0;
    m_kSplashArea.MotionKey = 0;
    m_kSplashArea.fDisappearingTime = 0.0f;
    m_kSplashArea.SphereInstanceVector.clear();
    m_kSplashArea.HittedInstanceMap.clear();

    m_isUsingQuiver = false;
    m_quiverEffectID = 0;

    memset(&m_kCurMotNode, 0, sizeof(m_kCurMotNode));

    __ClearCombo();
}

void CActorInstance::__Initialize()
{
#ifdef ENABLE_FLY_MOUNT
    ucanAdam_z = 0.0f;
#endif

    m_pkCurRaceMotionData = NULL;
    m_pkCurRaceData = NULL;
    m_pkHorse = NULL;
    m_pkTree = NULL;

    m_fOwnerBaseTime = 0.0f;

    m_eActorType = TYPE_PC;
    m_eRace = 0;
    m_eRank = 0;

    m_eShape = 0;
    m_eHair = 0;

    m_dwSelfVID = 0;
    m_dwOwnerVID = 0;

    m_pkEventHandler = NULL;

    m_PhysicsObject.Initialize();

    m_pAttributeInstance = NULL;

    m_pFlyEventHandler = 0;

    m_v3FishingPosition = Vector3(0.0f, 0.0f, 0.0f);
    m_iFishingEffectID = -1;

    m_pkHorse = NULL;

    m_flags = 0;

    m_moveFactor = 1.0f;

    __InitializePositionData();
    __InitializeRotationData();
    __InitializeMotionData();
    __InitializeStateData();
    __InitializeCollisionData();

    __BlendAlpha_Initialize();

    ClearFlyTargeter();
}

CActorInstance::CActorInstance()
{
    __Initialize();
    m_PhysicsObject.SetActorInstance(this);
}

CActorInstance::~CActorInstance()
{
    Destroy();
}
