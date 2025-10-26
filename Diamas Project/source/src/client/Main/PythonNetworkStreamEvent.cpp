#include "StdAfx.h"
#include "PythonNetworkStream.h"

void CPythonNetworkStream::OnRemoteDisconnect()
{
    ClosePhase();
}

void CPythonNetworkStream::OnDisconnect()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main Game
void CPythonNetworkStream::OnScriptEventStart(int iSkin, int iIndex)

{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenQuestWindow", iSkin, iIndex);
}

void CPythonNetworkStream::OnQuestScriptEventStart(int iSkin, int iIndex)
{
#ifndef USE_SLIDE_QUEST_WINDOW
    OnScriptEventStart(iSkin, iIndex);
#else
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenQuestSlideWindow", iSkin, iIndex);
#endif
}
