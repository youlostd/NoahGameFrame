#include "StdAfx.h"
#include "RaceMotionDataEvent.h"

void NMotionEvent::SMotionEventDataRelativeMoveOn::Save(FILE *File, int iTabs)
{
    PrintfTabs(File, iTabs, "\n");
    PrintfTabs(File, iTabs, "BaseVelocity      %f\n", fVelocity);
}

bool NMotionEvent::SMotionEventDataRelativeMoveOn::Load(CTextFileLoader &rTextFileLoader)
{
    if (!rTextFileLoader.GetTokenFloat("basevelocity", &fVelocity))
        return false;

    return true;
}

void NMotionEvent::SMotionEventDataRelativeMoveOff::Save(FILE *File, int iTabs)
{
}

void NMotionEvent::SMotionEventDataRelativeMoveOff::Load()
{
}
