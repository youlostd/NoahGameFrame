#include "StdAfx.h"
#include "ActorInstance.h"
#include "WeaponTrace.h"
#include "base/Remotery.h"

void CActorInstance::TraceProcess()
{
    rmt_ScopedCPUSample(ActorWeaponTrace, RMTSF_Aggregate);

    if (!m_WeaponTraceVector.empty())
    {
        for (const auto &wt : m_WeaponTraceVector)
        {
            wt->SetPosition(m_x, m_y, m_z);
            wt->SetRotation(m_fcurRotation);
            wt->Update(__GetReachScale());
        }
    }
}

void CActorInstance::RenderTrace()
{
    for (const auto &wt : m_WeaponTraceVector)
        wt->Render();
}

void CActorInstance::__DestroyWeaponTrace()
{
    for (const auto &wt : m_WeaponTraceVector)
        CWeaponTrace::Delete(wt);

    m_WeaponTraceVector.clear();
}

void CActorInstance::__ShowWeaponTrace()
{
    for (const auto &wt : m_WeaponTraceVector)
        wt->TurnOn();
}

void CActorInstance::__HideWeaponTrace()
{
    for (const auto &wt : m_WeaponTraceVector)
        wt->TurnOff();
}
