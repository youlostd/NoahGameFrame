#include "StdAfx.h"
#include "PythonPlayer.h"

#include "InstanceBase.h"

void CPythonPlayer::SetAttackKeyState(bool isPress)
{
    if (isPress)
    {
        CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
        if (pkInstMain && pkInstMain->IsFishingMode())
        {
            NEW_Fishing();
            return;
        }
    }

    m_isAtkKey = isPress;
}

void CPythonPlayer::NEW_SetSingleDIKKeyState(int eDIKKey, bool isPress)
{
    if (!isPress)
    {
        CInstanceBase *pkInstMain = NEW_GetMainActorPtr();
        if (pkInstMain && pkInstMain->IsMining())
        {
            return;
        }
    }

    if (isPress)
    {
        if (NEW_CancelFishing())
            return;
    }

    switch (eDIKKey)
    {
    case DIR_UP:
        NEW_SetSingleDirKeyState(DIR_UP, isPress);
        break;
    case DIR_DOWN:
        NEW_SetSingleDirKeyState(DIR_DOWN, isPress);
        break;
    case DIR_LEFT:
        NEW_SetSingleDirKeyState(DIR_LEFT, isPress);
        break;
    case DIR_RIGHT:
        NEW_SetSingleDirKeyState(DIR_RIGHT, isPress);
        break;
    }
}

void CPythonPlayer::NEW_SetSingleDirKeyState(int eDirKey, bool isPress)
{
    switch (eDirKey)
    {
    case DIR_UP:
        m_isUp = isPress;
        break;
    case DIR_DOWN:
        m_isDown = isPress;
        break;
    case DIR_LEFT:
        m_isLeft = isPress;
        break;
    case DIR_RIGHT:
        m_isRight = isPress;
        break;
    }

    m_isDirKey = (m_isUp || m_isDown || m_isLeft || m_isRight);

    SetMultiDirKeyState(m_isLeft, m_isRight, m_isUp, m_isDown);
}

void CPythonPlayer::SetMultiDirKeyState(bool isLeft, bool isRight, bool isUp, bool isDown)
{
    if (!__CanMove())
        return;

    if (isLeft || isRight || isUp || isDown)
    {
        const auto width = UI::CWindowManager::Instance().GetScreenWidth();
        const auto height = UI::CWindowManager::Instance().GetScreenHeight();

        const auto dir = GetKeyMovementDirection(isLeft, isRight, isUp, isDown);
        const auto dirRot = GetKeyMovementRotation(dir.first, dir.second,
                                                   width, height);

        if (!NEW_MoveToDirection(dirRot))
            SPDLOG_ERROR("Failed to move {0}", dirRot);
    }
    else
    {
        NEW_Stop();
    }
}
