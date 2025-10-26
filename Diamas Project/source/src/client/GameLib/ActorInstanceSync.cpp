#include "ActorInstance.h"
#include "RaceData.h"
#include "StdAfx.h"
#include <game/MotionConstants.hpp>


void CActorInstance::__Push(int x, int y)
{
    if (IsResistFallen())
        return;

    const Vector3& c_rv3Src = GetPosition();
    const Vector3 c_v3Dst = Vector3(x, -y, c_rv3Src.z);
    const Vector3 c_v3Delta = c_v3Dst - c_rv3Src;

    const int LoopValue = 100;
    const Vector3 inc = c_v3Delta / LoopValue;

    Vector3 v3Movement(0.0f, 0.0f, 0.0f);

    IPhysicsWorld* pWorld = IPhysicsWorld::GetPhysicsWorld();

    if (!pWorld) {
        return;
    }

    for (int i = 0; i < LoopValue; ++i) {
        if (pWorld->isPhysicalCollision(c_rv3Src + v3Movement)) {
            ResetBlendingPosition();
            return;
        }
        v3Movement += inc;
    }

    SetBlendingPosition(c_v3Dst);

    if (!IsUsingSkill()) {
        float len = sqrt(c_v3Delta.x * c_v3Delta.x + c_v3Delta.y * c_v3Delta.y);

        if (len > 150.0f) {
            InterceptOnceMotion(MOTION_DAMAGE_FLYING);
            PushOnceMotion(MOTION_STAND_UP);
        }
    }
}

void CActorInstance::TEMP_Push(int x, int y)
{
    __Push(x, y);
}

bool CActorInstance::__IsSyncing()
{
    if (IsDead())
        return true;

    if (IsStun())
        return true;
    //Düello esnasýnda kilitlenme olayýný devre dýþý býraktýk, denendi þuan için sorun görünmedi.
    if (IsPushing())
       return true;

    return false;
}

bool CActorInstance::IsPushing()
{
    return m_PhysicsObject.isBlending();
}
