#include "StdAfx.h"
#include "ActorInstance.h"
#include "GameUtil.h"
#include "RaceManager.h"
#include "../EterLib/GrpMath.h"
#include "../EterGrnLib/ModelInstance.h"


void CActorInstance::__InitializeCollisionData()
{
    m_canSkipCollision = 0xf35f5;
}

void CActorInstance::EnableSkipCollision()
{
    m_canSkipCollision = 0xa35f5;
}

void CActorInstance::DisableSkipCollision()
{
    m_canSkipCollision = 0xf35f5;
}

bool CActorInstance::CanSkipCollision() const
{

#ifdef ENABLE_FLY_MOUNT
    if (ucanAdam_z > 0.0f)
        return true;
#endif

    return m_canSkipCollision == 0xa35f5;
}

void CActorInstance::UpdatePointInstance()
{
    TCollisionPointInstanceListIterator itor;
    for (itor = m_DefendingPointInstanceList.begin(); itor != m_DefendingPointInstanceList.end(); ++itor)
        UpdatePointInstance(&(*itor));
}

void CActorInstance::UpdatePointInstance(TCollisionPointInstance *pPointInstance)
{
    if (!pPointInstance)
    {
        assert(!"CActorInstance::UpdatePointInstance - pPointInstance is NULL"); // 레퍼런스로 교체하시오
        return;
    }

    Matrix matBone;

    if (pPointInstance->isAttached)
    {
        if (pPointInstance->dwModelIndex >= m_modelInstances.size())
        {
            //Tracenf("CActorInstance::UpdatePointInstance - rInstance.dwModelIndex=%d >= m_modelInstances.size()=%d",
            //		pPointInstance->dwModelIndex>m_modelInstances.size());
            return;
        }

        auto pModelInstance = m_modelInstances[pPointInstance->dwModelIndex];
        if (!pModelInstance)
        {
            //Tracenf("CActorInstance::UpdatePointInstance - m_modelInstances[pPointInstance->dwModelIndex=%d] is NULL", pPointInstance->dwModelIndex);
            return;
        }

        Matrix *pmatBone = (Matrix *)pModelInstance->GetBoneMatrixPointer(pPointInstance->dwBoneIndex);
        matBone = *(Matrix *)pModelInstance->GetCompositeBoneMatrixPointer(pPointInstance->dwBoneIndex);
        matBone._41 = pmatBone->_41;
        matBone._42 = pmatBone->_42;
        matBone._43 = pmatBone->_43;
        matBone *= m_worldMatrix;
    }
    else
    {
        matBone = m_worldMatrix;
    }

    // Update Collsion Sphere
    CSphereCollisionInstanceVector::const_iterator sit = pPointInstance->c_pCollisionData->SphereDataVector.begin();
    CDynamicSphereInstanceVector::iterator dit = pPointInstance->SphereInstanceVector.begin();
    for (; sit != pPointInstance->c_pCollisionData->SphereDataVector.end(); ++sit, ++dit)
    {
        const TSphereData &c = sit->GetAttribute(); //c_pCollisionData->SphereDataVector[j].GetAttribute();

        Matrix matPoint = Matrix::CreateTranslation(c.v3Position);
        matPoint = matPoint * matBone;

        dit->v3LastPosition = dit->v3Position;
        dit->v3Position.x = matPoint._41;
        dit->v3Position.y = matPoint._42;
        dit->v3Position.z = matPoint._43;
    }
}

void CActorInstance::UpdateAdvancingPointInstance()
{
    // 말을 탔을 경우 사람은 이동값을 가지고 있지 않기 때문에 말로 부터 얻어와야 한다 - [levites]
    Vector3 v3Movement = m_v3Movement;
    if (m_pkHorse)
        v3Movement = m_pkHorse->m_v3Movement;

    // 말은 업데이트 하지 않아도 된다 - [levites]
    if (m_pkHorse)
        m_pkHorse->UpdateAdvancingPointInstance();

    Matrix matPoint;
    Matrix matCenter;

    TCollisionPointInstanceListIterator itor = m_BodyPointInstanceList.begin();
    for (; itor != m_BodyPointInstanceList.end(); ++itor)
    {
        TCollisionPointInstance &rInstance = *itor;

        if (rInstance.isAttached)
        {
            if (rInstance.dwModelIndex >= m_modelInstances.size())
            {
                SPDLOG_ERROR(
                    "CActorInstance::UpdateAdvancingPointInstance - rInstance.dwModelIndex={0} >= m_modelInstances.size()={1}",
                    rInstance.dwModelIndex, m_modelInstances.size());
                continue;
            }

            auto pModelInstance = m_modelInstances[rInstance.dwModelIndex];
            if (!pModelInstance)
            {
                //Tracenf("CActorInstance::UpdateAdvancingPointInstance - pGrnLODController->GetModelInstance() is NULL");
                continue;
            }

            auto pBoneMat = (Matrix *)pModelInstance->GetBoneMatrixPointer(rInstance.dwBoneIndex);
            matCenter = *(Matrix *)pModelInstance->GetCompositeBoneMatrixPointer(rInstance.dwBoneIndex);
            matCenter._41 = pBoneMat->_41;
            matCenter._42 = pBoneMat->_42;
            matCenter._43 = pBoneMat->_43;
            matCenter *= m_worldMatrix;
        }
        else
        {
            matCenter = m_worldMatrix;
        }

        // Update Collision Sphere
        const NRaceData::TCollisionData *c_pCollisionData = rInstance.c_pCollisionData;
        if (c_pCollisionData)
        {
            for (uint32_t j = 0; j < c_pCollisionData->SphereDataVector.size(); ++j)
            {
                const TSphereData &c = c_pCollisionData->SphereDataVector[j].GetAttribute();
                CDynamicSphereInstance &rSphereInstance = rInstance.SphereInstanceVector[j];

                matPoint = Matrix::CreateTranslation(c.v3Position);
                matPoint = matPoint * matCenter;

                rSphereInstance.v3LastPosition.x = matPoint._41;
                rSphereInstance.v3LastPosition.y = matPoint._42;
                rSphereInstance.v3LastPosition.z = matPoint._43;
                rSphereInstance.v3Position = rSphereInstance.v3LastPosition;
                rSphereInstance.v3Position += v3Movement;
            }
        }
    }
}

bool CActorInstance::CheckCollisionDetection(const CDynamicSphereInstanceVector *c_pAttackingSphereVector,
                                             Vector3 *pv3Position)
{
    if (!c_pAttackingSphereVector)
    {
        assert(!"CActorInstance::CheckCollisionDetection - c_pAttackingSphereVector is NULL"); // ·¹ÆÛ·±½º·Î ±³Ã¼ÇÏ½Ã¿À
        return false;
    }

    for (auto& itor : m_DefendingPointInstanceList) {
        const CDynamicSphereInstanceVector *c_pDefendingSphereVector = &itor.SphereInstanceVector;
        for (const auto& c_rAttackingSphere : *c_pAttackingSphereVector)
            for (const auto& c_rDefendingSphere : *c_pDefendingSphereVector) {
                auto b1 = BoundingSphere(c_rAttackingSphere.v3Position, c_rAttackingSphere.fRadius);
                auto b2 = BoundingSphere(c_rDefendingSphere.v3Position, c_rDefendingSphere.fRadius);

                if (b1.Intersects(b2))
                {
                    // FIXME : You have to change it to find the intersection of the two circles. 
                    *pv3Position = Vector3(c_rAttackingSphere.v3Position + c_rDefendingSphere.v3Position) / 2.0f;
                    return true;
                }
            }
    }

    return false;
}

bool CActorInstance::CreateCollisionInstancePiece(uint32_t dwAttachingModelIndex,
                                                  const NRaceData::TAttachingData *c_pAttachingData,
                                                  TCollisionPointInstance *pPointInstance)
{
    if (!c_pAttachingData)
    {
        assert(!"CActorInstance::CreateCollisionInstancePiece - c_pAttachingData is NULL"); // ·¹ÆÛ·±½º·Î ±³Ã¼ÇÏ½Ã¿À
        return false;
    }

    if (!c_pAttachingData->pCollisionData)
    {
        assert(!"CActorInstance::CreateCollisionInstancePiece - c_pAttachingData->pCollisionData is NULL");
        // ·¹ÆÛ·±½º·Î ±³Ã¼ÇÏ½Ã¿À
        return false;
    }

    if (!pPointInstance)
    {
        assert(!"CActorInstance::CreateCollisionInstancePiece - pPointInstance is NULL"); // ·¹ÆÛ·±½º·Î ±³Ã¼ÇÏ½Ã¿À
        return false;
    }

    pPointInstance->dwModelIndex = dwAttachingModelIndex;
    pPointInstance->isAttached = FALSE;
    pPointInstance->dwBoneIndex = 0;
    pPointInstance->c_pCollisionData = c_pAttachingData->pCollisionData;

    if (c_pAttachingData->isAttaching)
    {
        int iAttachingBoneIndex;

        const auto pModelInstance = m_modelInstances[dwAttachingModelIndex];
        if (pModelInstance &&
            pModelInstance->GetBoneIndexByName(c_pAttachingData->strAttachingBoneName.c_str(),
                                               &iAttachingBoneIndex))
        {
            pPointInstance->isAttached = true;
            pPointInstance->dwBoneIndex = iAttachingBoneIndex;
        }
        else
        {
            //SPDLOG_ERROR("Failed to find bone {0} in model {1} for collision attachment",
            //	c_pAttachingData->strAttachingBoneName.c_str(), dwAttachingModelIndex);
            pPointInstance->isAttached = true;
            pPointInstance->dwBoneIndex = 0;
        }
    }

    const CSphereCollisionInstanceVector &c_rSphereDataVector = c_pAttachingData->pCollisionData->SphereDataVector;

    pPointInstance->SphereInstanceVector.clear();
    pPointInstance->SphereInstanceVector.reserve(c_rSphereDataVector.size());

    CSphereCollisionInstanceVector::const_iterator it;
    CDynamicSphereInstance dsi;

    dsi.v3LastPosition = Vector3(0.0f, 0.0f, 0.0f);
    dsi.v3Position = Vector3(0.0f, 0.0f, 0.0f);
    for (it = c_rSphereDataVector.begin(); it != c_rSphereDataVector.end(); ++it)
    {
        const TSphereData &c_rSphereData = it->GetAttribute();
        dsi.fRadius = c_rSphereData.fRadius;
        pPointInstance->SphereInstanceVector.push_back(dsi);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool CActorInstance::__SplashAttackProcess(CActorInstance &rVictim)
{
    Vector3 v3Distance(rVictim.m_x - m_x, rVictim.m_z - m_z, rVictim.m_z - m_z);
    float fDistance = v3Distance.LengthSquared();
    if (fDistance >= 1000.0f * 1000.0f)
        return FALSE;

    // Check Distance
    if (!__IsInSplashTime())
        return FALSE;

    const CRaceMotionData::TMotionAttackingEventData *c_pAttackingEvent = m_kSplashArea.c_pAttackingEvent;
    const NRaceData::TAttackData &c_rAttackData = c_pAttackingEvent->AttackData;
    THittedInstanceMap &rHittedInstanceMap = m_kSplashArea.HittedInstanceMap;

    // NOTE : ÀÌ¹Ì ¶§·È´Ù¸é ¶§¸± ¼ö ¾øÀ½
    if (rHittedInstanceMap.end() != rHittedInstanceMap.find(&rVictim))
    {
        return FALSE;
    }

    // NOTE : Snipe ¸ðµåÀÌ°í..
	if (NRaceData::ATTACK_TYPE_SNIPE == c_rAttackData.iAttackType)
	{
		// If the target is a PC... 
		if (__IsFlyTargetPC())
			// Other objects cannot be hit 
			if (!__IsSameFlyTarget(&rVictim))
				return false;
	}

    Vector3 v3HitPosition;
    if (rVictim.CheckCollisionDetection(&m_kSplashArea.SphereInstanceVector, &v3HitPosition))
    {
        rHittedInstanceMap.insert(std::make_pair(&rVictim, GetLocalTime() + c_rAttackData.fInvisibleTime));

        int iCurrentHitCount = rHittedInstanceMap.size();
        int iMaxHitCount = (0 == c_rAttackData.iHitLimitCount ? 16 : c_rAttackData.iHitLimitCount);
        //Tracef(" ------------------- Splash Hit : %d\n", iCurrentHitCount);

        if (iCurrentHitCount > iMaxHitCount)
        {
            //Tracef(" ------------------- OVER FLOW :: Splash Hit Count : %d\n", iCurrentHitCount);
            return FALSE;
        }

        NEW_SetAtkPixelPosition(NEW_GetCurPixelPositionRef());
        __ProcessDataAttackSuccess(m_kCurMotNode.dwMotionKey, c_rAttackData, rVictim, v3HitPosition,
                                   m_kSplashArea.uSkill, m_kSplashArea.isEnableHitProcess);
        return TRUE;
    }

    return FALSE;
}

bool CActorInstance::__NormalAttackProcess(CActorInstance &rVictim)
{
    // Check Distance
    // NOTE - 일단 근접 체크만 하고 있음
    Vector3 v3Distance(rVictim.m_x - m_x, rVictim.m_z - m_z, rVictim.m_z - m_z);
    float fDistance = v3Distance.LengthSquared();

    if (CRaceManager::instance().IsHugeRace(rVictim.GetRace()))
    {
        if (fDistance >= 500.0f * 500.0f)
            return FALSE;
    }
    else
    {
        if (fDistance >= 300.0f * 300.0f)
            return FALSE;
    }

    if (!isValidAttacking())
        return FALSE;

    const float c_fAttackRadius = 25.0f;
    const NRaceData::TMotionAttackData *pad = m_pkCurRaceMotionData->GetMotionAttackDataPointer();

    const float motiontime = GetAttackingElapsedTime();

    for (const NRaceData::THitData &c_rHitData : pad->HitDataContainer)
    {
        // NOTE : 이미 맞았는지 체크
        const auto itHitData = m_HitDataMap.find(&c_rHitData);
        if (itHitData != m_HitDataMap.end())
        {
            THittedInstanceMap &rHittedInstanceMap = itHitData->second;

            const auto itInstance = rHittedInstanceMap.find(&rVictim);
            if (itInstance != rHittedInstanceMap.end())
            {
                if (pad->iMotionType == NRaceData::MOTION_TYPE_COMBO ||
                    itInstance->second > GetLocalTime())
                    continue;
            }
        }

        // NOTE : 공격시 공격 속도가 빨라지면 일부 MotionData가 skip되는 현상때문에 임시로 이렇게 고쳤다. (Mantis #75176)
        // 이 코드도 공격 판정 데이터와 실제 결과가 달라질 수 있는 문제가 있어 좋은 코드는 못되지만 공격이 아예 안들어가는것보단(..) 나으므로
        // 일단 이렇게 고쳐둔다.. 완벽하게 고치려면 모션중 판정 데이터와 관련된 모든 코드를 갈아엎는게 빠르다.

        // 주요 문제점은 이전 Frame에서의 time과 현재 Frame에서의 time의 오차가 크면 중간에 처리가 skip되는 hitdata가 생긴다.
        // motiontime-Timer::Instance().GetElapsedSecond() 이걸로 어떻게 해보려했던것 같지만 로그찍어본 결과 그렇게 안되더라.
        // ~ ityz ~

        /*-------------------------원본 코드------------------------------
        range_start = c_rHitData.mapHitPosition.lower_bound(motiontime-Timer::Instance().GetElapsedSecond());
        range_end = c_rHitData.mapHitPosition.upper_bound(motiontime);

        float c = cosf(DirectX::XMConvertToRadians(GetRotation()));
        float s = sinf(DirectX::XMConvertToRadians(GetRotation()));

        for(;range_start!=range_end;++range_start)
        {
        ----------------------------------------------------------------*/

        if (c_rHitData.fAttackStartTime > motiontime)
            continue;
        if (c_rHitData.fAttackEndTime < motiontime)
            continue;

        float c = std::cos(DirectX::XMConvertToRadians(GetRotation()));
        float s = std::sin(DirectX::XMConvertToRadians(GetRotation()));

        for (const auto &hitPos : c_rHitData.mapHitPosition)
        {
            const CDynamicSphereInstance &dsiSrc = hitPos.second;
            CDynamicSphereInstance dsi = dsiSrc;
            dsi.fRadius = c_fAttackRadius;

            {
                Vector3 v3SrcDir = dsiSrc.v3Position - dsiSrc.v3LastPosition;
                v3SrcDir *= __GetReachScale();

                const Vector3 &v3Src = dsiSrc.v3LastPosition + v3SrcDir;
                Vector3 &v3Dst = dsi.v3Position;
                v3Dst.x = v3Src.x * c - v3Src.y * s;
                v3Dst.y = v3Src.x * s + v3Src.y * c;
                v3Dst += GetPosition();
            }
            {
                const Vector3 &v3Src = dsiSrc.v3LastPosition;
                Vector3 &v3Dst = dsi.v3LastPosition;
                v3Dst.x = v3Src.x * c - v3Src.y * s;
                v3Dst.y = v3Src.x * s + v3Src.y * c;
                v3Dst += GetPosition();
            }

            for (const auto &defendingPoint : rVictim.m_DefendingPointInstanceList)
            {
                const auto &defendingSpheres = defendingPoint.SphereInstanceVector;
                for (const CDynamicSphereInstance &sub : defendingSpheres)
                {
                    if (DetectCollisionDynamicZCylinderVSDynamicZCylinder(dsi, sub))
                    {
                        auto &hitInstances = m_HitDataMap[&c_rHitData];
                        hitInstances.insert(std::make_pair(&rVictim, GetLocalTime() + pad->fInvisibleTime));

                        int iCurrentHitCount = hitInstances.size();
                        // NOTE : 보통 공격은 16명이 한계
                        if (NRaceData::MOTION_TYPE_COMBO != pad->iMotionType && NRaceData::MOTION_TYPE_NORMAL != pad->
                            iMotionType)
                        {
                            if (iCurrentHitCount > pad->iHitLimitCount)
                            {
                                //Tracef(" Type SKILL :: Overflow - Can't process, skip\n");
                                return false;
                            }
                        }

                        Vector3 v3HitPosition = (GetPosition() + rVictim.GetPosition()) * 0.5f;

                        // #0000780: [M2KR] 수룡 타격구 문제
                        if (CRaceManager::instance().IsHugeRace(rVictim.GetRace()))
                            v3HitPosition = (GetPosition() + sub.v3Position) * 0.5f;

                        __ProcessDataAttackSuccess(m_kCurMotNode.dwMotionKey,
                                                   *pad,
                                                   rVictim,
                                                   v3HitPosition,
                                                   m_kCurMotNode.uSkill);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool CActorInstance::AttackingProcess(CActorInstance &rVictim)
{
    if (rVictim.__isInvisible())
        return FALSE;

    if (__SplashAttackProcess(rVictim))
        return TRUE;

    if (__NormalAttackProcess(rVictim))
        return TRUE;

    return FALSE;
}

bool CActorInstance::TestPhysicsBlendingCollision(CActorInstance &rVictim)
{
    if (rVictim.IsDead())
        return FALSE;

    #ifdef ENABLE_FLY_MOUNT
    if (ucanAdam_z > 0.0f)
        return FALSE;
#endif

    TPixelPosition kPPosLast;
    GetBlendingPosition(&kPPosLast);

    Vector3 v3Distance = Vector3(rVictim.m_x - kPPosLast.x, rVictim.m_y - kPPosLast.y,
                                         rVictim.m_z - kPPosLast.z);
    float fDistance = v3Distance.LengthSquared();
    if (fDistance > 800.0f * 800.0f)
        return FALSE;

    // NOTE : 공격 중일때는 Defending Sphere로 Collision Check를 합니다.
    // NOTE : Wait로 블렌딩 되는 도중에 뚫고 들어가는 문제가 있어서.. - [levites]
    TCollisionPointInstanceList *pMainList;
    TCollisionPointInstanceList *pVictimList;
    if (isAttacking() || IsWaiting())
    {
        pMainList = &m_DefendingPointInstanceList;
        pVictimList = &rVictim.m_DefendingPointInstanceList;
    }
    else
    {
        pMainList = &m_BodyPointInstanceList;
        pVictimList = &rVictim.m_BodyPointInstanceList;
    }

    TPixelPosition kPDelta;
    m_PhysicsObject.GetLastPosition(&kPDelta);

    Vector3 prevLastPosition, prevPosition;
    const int nSubCheckCount = 50;

    TCollisionPointInstanceListIterator itorMain = pMainList->begin();
    TCollisionPointInstanceListIterator itorVictim = pVictimList->begin();
    for (; itorMain != pMainList->end(); ++itorMain)
    {
        for (; itorVictim != pVictimList->end(); ++itorVictim)
        {
            CDynamicSphereInstanceVector &c_rMainSphereVector = (*itorMain).SphereInstanceVector;
            CDynamicSphereInstanceVector &c_rVictimSphereVector = (*itorVictim).SphereInstanceVector;

            for (uint32_t i = 0; i < c_rMainSphereVector.size(); ++i)
            {
                CDynamicSphereInstance &c_rMainSphere = c_rMainSphereVector[i];
                //adjust main sphere center
                prevLastPosition = c_rMainSphere.v3LastPosition;
                prevPosition = c_rMainSphere.v3Position;

                c_rMainSphere.v3LastPosition = prevPosition;

                for (int i = 1; i <= nSubCheckCount; ++i)
                {
                    c_rMainSphere.v3Position = prevPosition + kPDelta * (float)(i / (float)nSubCheckCount);

                    for (uint32_t j = 0; j < c_rVictimSphereVector.size(); ++j)
                    {
                        CDynamicSphereInstance &c_rVictimSphere = c_rVictimSphereVector[j];

                        if (DetectCollisionDynamicSphereVSDynamicSphere(c_rMainSphere, c_rVictimSphere))
                        {
                            bool bResult = GetVector3Distance(c_rMainSphere.v3Position, c_rVictimSphere.v3Position) <=
                                           GetVector3Distance(c_rMainSphere.v3LastPosition, c_rVictimSphere.v3Position);

                            c_rMainSphere.v3LastPosition = prevLastPosition;
                            c_rMainSphere.v3Position = prevPosition;

                            return bResult;
                        }
                    }
                }

                //restore
                c_rMainSphere.v3LastPosition = prevLastPosition;
                c_rMainSphere.v3Position = prevPosition;
            }
        }
    }

    return FALSE;
}

bool CActorInstance::TestActorCollision(CActorInstance &rVictim)
{
    /*
        if (m_pkHorse)
        {
            if (m_pkHorse->TestActorCollision(rVictim))
                return TRUE;

            return FALSE;
        }
    */

    if (rVictim.IsDead())
        return FALSE;

    // Check Distance
    // NOTE : 적당히 멀면 체크 안함
    //        프레임 스킵시나 대상 오브젝트의 크기가 클경우 문제가 생길 여지가 있음
    //        캐릭터가 자신의 Body Sphere Radius 보다 더 크게 이동했는지를 체크하고,
    //        만약 그렇지 않다면 거리로 체크해서 걸러준다.
    Vector3 v3Distance = Vector3(rVictim.m_x - m_x, rVictim.m_y - m_y, rVictim.m_z - m_z);
    float fDistance = v3Distance.LengthSquared();
    if (fDistance > 800.0f * 800.0f)
        return FALSE;

    // NOTE : 공격 중일때는 Defending Sphere로 Collision Check를 합니다.
    // NOTE : Wait로 블렌딩 되는 도중에 뚫고 들어가는 문제가 있어서.. - [levites]
    TCollisionPointInstanceList *pMainList;
    TCollisionPointInstanceList *pVictimList;
    if (isAttacking() || IsWaiting())
    {
        pMainList = &m_DefendingPointInstanceList;
        pVictimList = &rVictim.m_DefendingPointInstanceList;
    }
    else
    {
        pMainList = &m_BodyPointInstanceList;
        pVictimList = &rVictim.m_BodyPointInstanceList;
    }

    TCollisionPointInstanceListIterator itorMain = pMainList->begin();
    TCollisionPointInstanceListIterator itorVictim = pVictimList->begin();
    for (; itorMain != pMainList->end(); ++itorMain)
        for (; itorVictim != pVictimList->end(); ++itorVictim)
        {
            const CDynamicSphereInstanceVector &c_rMainSphereVector = (*itorMain).SphereInstanceVector;
            const CDynamicSphereInstanceVector &c_rVictimSphereVector = (*itorVictim).SphereInstanceVector;

            for (uint32_t i = 0; i < c_rMainSphereVector.size(); ++i)
                for (uint32_t j = 0; j < c_rVictimSphereVector.size(); ++j)
                {
                    const CDynamicSphereInstance &c_rMainSphere = c_rMainSphereVector[i];
                    const CDynamicSphereInstance &c_rVictimSphere = c_rVictimSphereVector[j];

                    if (DetectCollisionDynamicSphereVSDynamicSphere(c_rMainSphere, c_rVictimSphere))
                    {
                        if (GetVector3Distance(c_rMainSphere.v3Position, c_rVictimSphere.v3Position) <=
                            GetVector3Distance(c_rMainSphere.v3LastPosition, c_rVictimSphere.v3Position))
                        {
                            return TRUE;
                        }
                        return FALSE;
                    }
                }
        }

    return FALSE;
}

bool CActorInstance::AvoidObject(const CGraphicObjectInstance &c_rkBGObj)
{
    if (this == &c_rkBGObj)
        return false;

    if (!__TestObjectCollision(&c_rkBGObj))
        return false;

    __AdjustCollisionMovement(&c_rkBGObj);
    return true;
}

bool CActorInstance::IsBlockObject(const CGraphicObjectInstance &c_rkBGObj)
{
    if (this == &c_rkBGObj)
        return false;

    if (!__TestObjectCollision(&c_rkBGObj))
        return false;

    return true;
}

void CActorInstance::BlockMovement()
{
    if (m_pkHorse)
    {
        m_pkHorse->__InitializeMovement();
        return;
    }

    __InitializeMovement();
}

bool CActorInstance::__TestObjectCollision(const CGraphicObjectInstance *c_pObjectInstance)
{
    if (m_pkHorse)
    {
        if (m_pkHorse->__TestObjectCollision(c_pObjectInstance))
            return TRUE;

        return FALSE;
    }

    #ifdef ENABLE_FLY_MOUNT
    if (ucanAdam_z > 0.0f)
        return FALSE;
#endif

    if (CanSkipCollision())
        return FALSE;

    if (m_v3Movement.x == 0.0f && m_v3Movement.y == 0.0f && m_v3Movement.z == 0.0f)
        return FALSE;

    TCollisionPointInstanceListIterator itorMain = m_BodyPointInstanceList.begin();
    for (; itorMain != m_BodyPointInstanceList.end(); ++itorMain)
    {
        const CDynamicSphereInstanceVector &c_rMainSphereVector = (*itorMain).SphereInstanceVector;
        for (uint32_t i = 0; i < c_rMainSphereVector.size(); ++i)
        {
            const CDynamicSphereInstance &c_rMainSphere = c_rMainSphereVector[i];

            if (c_pObjectInstance->MovementCollisionDynamicSphere(c_rMainSphere))
            {
                //const Vector3 & c_rv3Position = c_pObjectInstance->GetPosition();
                //if (GetVector3Distance(c_rMainSphere.v3Position, c_rv3Position) <
                //	GetVector3Distance(c_rMainSphere.v3LastPosition, c_rv3Position))
                {
                    return TRUE;
                }

                //return FALSE;
            }
        }
    }

    return FALSE;
}

bool CActorInstance::TestCollisionWithDynamicSphere(const CDynamicSphereInstance &dsi)
{
    TCollisionPointInstanceListIterator itorMain = m_BodyPointInstanceList.begin();
    for (; itorMain != m_BodyPointInstanceList.end(); ++itorMain)
    {
        const CDynamicSphereInstanceVector &c_rMainSphereVector = (*itorMain).SphereInstanceVector;
        for (uint32_t i = 0; i < c_rMainSphereVector.size(); ++i)
        {
            const CDynamicSphereInstance &c_rMainSphere = c_rMainSphereVector[i];

            if (DetectCollisionDynamicSphereVSDynamicSphere(c_rMainSphere, dsi))
            {
                return true;
            }
        }
    }

    return false;
}
