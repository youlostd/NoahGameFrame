#include "StdAfx.h"
#include "PythonCharacterManager.h"
#include "PythonBackground.h"
#include "InstanceBase.h"
#include "../gamelib/RaceManager.h"
#include "../EterLib/CharacterEffectRegistry.hpp"
#include <game/SkillConstants.hpp>
#include <game/AffectConstants.hpp>

//////////////////////////////////////////////////////////////////////////
// RaceData 관련 시작
//////////////////////////////////////////////////////////////////////////

PyObject *chrmgrSetEmpireNameMode(PyObject *poSelf, PyObject *poArgs)
{
    int iEnable;
    if (!PyTuple_GetInteger(poArgs, 0, &iEnable))
        return Py_BadArgument();

    CInstanceBase::SetEmpireNameMode(iEnable ? true : false);
    CPythonCharacterManager::Instance().RefreshAllPCTextTail();

    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterTitleName(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BadArgument();
    std::string szTitleName;
    if (!PyTuple_GetString(poArgs, 1, szTitleName))
        return Py_BadArgument();

    std::string szTitleNameFemale;
    if (!PyTuple_GetString(poArgs, 2, szTitleNameFemale))
        return Py_BadArgument();

    RegisterTitleName(iIndex, szTitleName.c_str(), szTitleNameFemale.c_str());
    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterNamePrefix(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BadArgument();
    std::string prefix;
    if (!PyTuple_GetString(poArgs, 1, prefix))
        return Py_BadArgument();

    RegisterNamePrefix(iIndex, prefix.c_str());
    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterNameColor(PyObject *poSelf, PyObject *poArgs)
{
    int index;
    if (!PyTuple_GetInteger(poArgs, 0, &index))
        return Py_BadArgument();

    int ir;
    if (!PyTuple_GetInteger(poArgs, 1, &ir))
        return Py_BadArgument();
    int ig;
    if (!PyTuple_GetInteger(poArgs, 2, &ig))
        return Py_BadArgument();
    int ib;
    if (!PyTuple_GetInteger(poArgs, 3, &ib))
        return Py_BadArgument();

 //   CInstanceBase::RegisterNameColor(index, ir, ig, ib);
    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterTitleColor(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BadArgument();
    int ir;
    if (!PyTuple_GetInteger(poArgs, 1, &ir))
        return Py_BadArgument();
    int ig;
    if (!PyTuple_GetInteger(poArgs, 2, &ig))
        return Py_BadArgument();
    int ib;
    if (!PyTuple_GetInteger(poArgs, 3, &ib))
        return Py_BadArgument();

  //  CInstanceBase::RegisterTitleColor(iIndex, ir, ig, ib);
    Py_RETURN_NONE;
}

PyObject *chrmgrGetPickedVID(PyObject *poSelf, PyObject *poArgs)
{
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();

    uint32_t dwPickedActorID;
    if (rkChrMgr.OLD_GetPickedInstanceVID(&dwPickedActorID))
        return Py_BuildValue("i", dwPickedActorID);
    else
        return Py_BuildValue("i", -1);
}

PyObject *chrmgrGetVIDInfo(PyObject *poSelf, PyObject *poArgs)
{
    int nVID;
    if (!PyTuple_GetInteger(poArgs, 0, &nVID))
        return Py_BadArgument();

    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();

    char szDetail[256] = "";
    CInstanceBase *pkInstBase = rkChrMgr.GetInstancePtr(nVID);
    if (pkInstBase)
    {
        TPixelPosition kPPosInst;
        pkInstBase->NEW_GetPixelPosition(&kPPosInst);

        sprintf(szDetail, "pos=(%f, %f)", kPPosInst.x, kPPosInst.y);
    }

    char szInfo[1024];
    sprintf(szInfo, "VID %d (isRegistered=%d, isAlive=%d, isDead=%d) %s",
            nVID,
            rkChrMgr.IsRegisteredVID(nVID),
            rkChrMgr.IsAliveVID(nVID),
            rkChrMgr.IsDeadVID(nVID),
            szDetail
        );

    return Py_BuildValue("s", szInfo);
}

PyObject *chrmgrSetPathName(PyObject *poSelf, PyObject *poArgs)
{
    std::string szPathName;
    if (!PyTuple_GetString(poArgs, 0, szPathName))
        return Py_BadArgument();

    CRaceManager::Instance().SetPathName(szPathName);
    Py_RETURN_NONE;
}

PyObject *chrmgrCreateRace(PyObject *poSelf, PyObject *poArgs)
{
    int iRace;
    if (!PyTuple_GetInteger(poArgs, 0, &iRace))
        return Py_BadArgument();

    CRaceManager::Instance().CreateRace(iRace);
    Py_RETURN_NONE;
}

PyObject *chrmgrSelectRace(PyObject *poSelf, PyObject *poArgs)
{
    int iRace;
    if (!PyTuple_GetInteger(poArgs, 0, &iRace))
        return Py_BadArgument();

    CRaceManager::Instance().SelectRace(iRace);
    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterAttachingBoneName(PyObject *poSelf, PyObject *poArgs)
{
    int iPartIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iPartIndex))
        return Py_BadArgument();
    std::string szBoneName;
    if (!PyTuple_GetString(poArgs, 1, szBoneName))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    pRaceData->RegisterAttachingBoneName(iPartIndex, szBoneName.c_str());
    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterMotionMode(PyObject *poSelf, PyObject *poArgs)
{
    int iMotionIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iMotionIndex))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    pRaceData->RegisterMotionMode(iMotionIndex);
    Py_RETURN_NONE;
}

PyObject *chrmgrSetMotionRandomWeight(PyObject *poSelf, PyObject *poArgs)
{
    int iMode;
    if (!PyTuple_GetInteger(poArgs, 0, &iMode))
        return Py_BadArgument();

    int iMotion;
    if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
        return Py_BadArgument();

    int iSubMotion;
    if (!PyTuple_GetInteger(poArgs, 2, &iSubMotion))
        return Py_BadArgument();

    int iPercentage;
    if (!PyTuple_GetInteger(poArgs, 3, &iPercentage))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    if (!pRaceData->SetMotionRandomWeight(iMode, iMotion, iSubMotion, iPercentage))
        Py_BuildException("Failed to SetMotionRandomWeight");

    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterNormalAttack(PyObject *poSelf, PyObject *poArgs)
{
    int iMode;
    if (!PyTuple_GetInteger(poArgs, 0, &iMode))
        return Py_BadArgument();

    int iMotion;
    if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    pRaceData->RegisterNormalAttack(iMode, iMotion);
    Py_RETURN_NONE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 없어질 함수들
PyObject *chrmgrReserveComboAttack(PyObject *poSelf, PyObject *poArgs)
{
    int iMode;
    if (!PyTuple_GetInteger(poArgs, 0, &iMode))
        return Py_BadArgument();

    int iCount;
    if (!PyTuple_GetInteger(poArgs, 1, &iCount))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    pRaceData->ReserveComboAttack(iMode, 0, iCount);
    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterComboAttack(PyObject *poSelf, PyObject *poArgs)
{
    int iMode;
    if (!PyTuple_GetInteger(poArgs, 0, &iMode))
        return Py_BadArgument();

    int iComboIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iComboIndex))
        return Py_BadArgument();

    int iMotionIndex;
    if (!PyTuple_GetInteger(poArgs, 2, &iMotionIndex))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    pRaceData->RegisterComboAttack(iMode, 0, iComboIndex, iMotionIndex);
    Py_RETURN_NONE;
}

// 없어질 함수들
///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject *chrmgrReserveComboAttackNew(PyObject *poSelf, PyObject *poArgs)
{
    int iMotionMode;
    if (!PyTuple_GetInteger(poArgs, 0, &iMotionMode))
        return Py_BadArgument();

    int iComboType;
    if (!PyTuple_GetInteger(poArgs, 1, &iComboType))
        return Py_BadArgument();

    int iCount;
    if (!PyTuple_GetInteger(poArgs, 2, &iCount))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    pRaceData->ReserveComboAttack(iMotionMode, iComboType, iCount);
    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterComboAttackNew(PyObject *poSelf, PyObject *poArgs)
{
    int iMotionMode;
    if (!PyTuple_GetInteger(poArgs, 0, &iMotionMode))
        return Py_BadArgument();

    int iComboType;
    if (!PyTuple_GetInteger(poArgs, 1, &iComboType))
        return Py_BadArgument();

    int iComboIndex;
    if (!PyTuple_GetInteger(poArgs, 2, &iComboIndex))
        return Py_BadArgument();

    int iMotionIndex;
    if (!PyTuple_GetInteger(poArgs, 3, &iMotionIndex))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    pRaceData->RegisterComboAttack(iMotionMode, iComboType, iComboIndex, iMotionIndex);
    Py_RETURN_NONE;
}

PyObject *chrmgrLoadRaceData(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    const char *c_szFullFileName = CRaceManager::Instance().GetFullPathFileName(szFileName.c_str());
    if (!pRaceData->LoadRaceData(c_szFullFileName))
    {
        SPDLOG_ERROR("Failed to load race data : {0}\n", c_szFullFileName);
    }

    Py_RETURN_NONE;
}

PyObject *chrmgrLoadLocalRaceData(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    if (!pRaceData->LoadRaceData(szFileName.c_str()))
    {
        SPDLOG_ERROR("Failed to load race data : {0}\n", szFileName);
    }

    Py_RETURN_NONE;
}

/*
PyObject * chrmgrRegisterMotion(PyObject* poSelf, PyObject* poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BadArgument();

	int iMotion;
	if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
		return Py_BadArgument();

	char* szFileName;
	if (!PyTuple_GetString(poArgs, 2, &szFileName))
		return Py_BadArgument();

	int iWeight = 0;
	PyTuple_GetInteger(poArgs, 3, &iWeight);
	iWeight = MIN(100, iWeight);

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	pRaceData->NEW_RegisterMotion(iMode, iMotion, CRaceManager::Instance().GetFullPathFileName(szFileName), iWeight);
	Py_RETURN_NONE;
}
*/

PyObject *chrmgrRegisterCacheMotionData(PyObject *poSelf, PyObject *poArgs)
{
    int iMode;
    if (!PyTuple_GetInteger(poArgs, 0, &iMode))
        return Py_BadArgument();

    int iMotion;
    if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
        return Py_BadArgument();

    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 2, szFileName))
        return Py_BadArgument();

    int iWeight = 0;
    PyTuple_GetInteger(poArgs, 3, &iWeight);
    iWeight = MIN(100, iWeight);

    auto pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    const char *c_szFullFileName = CRaceManager::Instance().GetFullPathFileName(szFileName.c_str());
    auto r = pRaceData->RegisterMotionData(iMode, iMotion, c_szFullFileName, iWeight);

    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterMotionData(PyObject *poSelf, PyObject *poArgs)
{
    int iMode;
    if (!PyTuple_GetInteger(poArgs, 0, &iMode))
        return Py_BadArgument();

    int iMotion;
    if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
        return Py_BadArgument();

    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 2, szFileName))
        return Py_BadArgument();

    int iWeight = 0;
    PyTuple_GetInteger(poArgs, 3, &iWeight);
    iWeight = MIN(100, iWeight);

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    const char *c_szFullFileName = CRaceManager::Instance().GetFullPathFileName(szFileName.c_str());
    pRaceData->RegisterMotionData(iMode, iMotion, c_szFullFileName, iWeight);

    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterRace(PyObject *poSelf, PyObject *poArgs)
{
    int iRaceIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iRaceIndex))
        return Py_BadArgument();

    std::string szName;
    if (!PyTuple_GetString(poArgs, 1, szName))
        return Py_BadArgument();

    CRaceManager::Instance().RegisterRace(iRaceIndex, szName);
    Py_RETURN_NONE;
}

PyObject *chrmgrSetShapeModel(PyObject *poSelf, PyObject *poArgs)
{
    int eShape;
    if (!PyTuple_GetInteger(poArgs, 0, &eShape))
        return Py_BadArgument();

    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    pRaceData->SetShapeModel(eShape, szFileName.c_str());
    Py_RETURN_NONE;
}

PyObject *chrmgrAppendShapeSkin(PyObject *poSelf, PyObject *poArgs)
{
    int eShape;
    if (!PyTuple_GetInteger(poArgs, 0, &eShape))
        return Py_BadArgument();

    int ePart;
    if (!PyTuple_GetInteger(poArgs, 1, &ePart))
        return Py_BadArgument();

    std::string szSrcFileName;
    if (!PyTuple_GetString(poArgs, 2, szSrcFileName))
        return Py_BadArgument();

    std::string szDstFileName;
    if (!PyTuple_GetString(poArgs, 3, szDstFileName))
        return Py_BadArgument();

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
        return Py_BuildException("RaceData has not selected!");

    pRaceData->AppendShapeSkin(eShape, ePart, szSrcFileName.c_str(), szDstFileName.c_str());
    Py_RETURN_NONE;
}

//////////////////////////////////////////////////////////////////////////
// RaceData 관련 끝
//////////////////////////////////////////////////////////////////////////

PyObject *chrmgrSetMovingSpeed(PyObject *poSelf, PyObject *poArgs)
{
#ifndef _DISTRIBUTE
    int nMovSpd;
    if (!PyTuple_GetInteger(poArgs, 0, &nMovSpd))
        return Py_BadArgument();

    if (nMovSpd < 0)
        return Py_BuildException("MovingSpeed < 0");

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    if (!pkInst)
        return Py_BuildException("MainCharacter has not selected!");

    pkInst->SetMoveSpeed(nMovSpd);
#endif
    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterEffect(PyObject *poSelf, PyObject *poArgs)
{
    int eEftType;
    if (!PyTuple_GetInteger(poArgs, 0, &eEftType))
        return Py_BadArgument();

    std::string szBoneName;
    if (!PyTuple_GetString(poArgs, 1, szBoneName))
        return Py_BadArgument();

    std::string szPathName;
    if (!PyTuple_GetString(poArgs, 2, szPathName))
        return Py_BadArgument();

    gCharacterEffectRegistry->RegisterEffect(eEftType, szBoneName, szPathName.c_str());
    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterAffect(PyObject *poSelf, PyObject *poArgs)
{
    int eEftType;
    if (!PyTuple_GetInteger(poArgs, 0, &eEftType))
        return Py_BadArgument();

    std::string szBoneName;
    if (!PyTuple_GetString(poArgs, 1, szBoneName))
        return Py_BadArgument();

    std::string szPathName;
    if (!PyTuple_GetString(poArgs, 2, szPathName))
        return Py_BadArgument();

    gCharacterEffectRegistry->RegisterAffect(eEftType, szBoneName, szPathName.c_str());
    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterCacheEffect(PyObject *poSelf, PyObject *poArgs)
{
    int eEftType;
    if (!PyTuple_GetInteger(poArgs, 0, &eEftType))
        return Py_BadArgument();

    std::string szBoneName;
    if (!PyTuple_GetString(poArgs, 1, szBoneName))
        return Py_BadArgument();

    std::string szPathName;
    if (!PyTuple_GetString(poArgs, 2, szPathName))
        return Py_BadArgument();

    gCharacterEffectRegistry->RegisterEffect(eEftType, szBoneName, szPathName.c_str());

    Py_RETURN_NONE;
}

PyObject *chrmgrSetDustGap(PyObject *poSelf, PyObject *poArgs)
{
    int nGap;
    if (!PyTuple_GetInteger(poArgs, 0, &nGap))
        return Py_BadArgument();

    CInstanceBase::SetDustGap(nGap);
    CInstanceBase::SetHorseDustGap(nGap);
    Py_RETURN_NONE;
}

PyObject *chrmgrSetHorseDustGap(PyObject *poSelf, PyObject *poArgs)
{
    int nGap;
    if (!PyTuple_GetInteger(poArgs, 0, &nGap))
        return Py_BadArgument();

    CInstanceBase::SetHorseDustGap(nGap);
    Py_RETURN_NONE;
}

PyObject *chrmgrToggleDirectionLine(PyObject *poSelf, PyObject *poArgs)
{
    static bool s_isVisible = true;
    CActorInstance::ShowDirectionLine(s_isVisible);

    s_isVisible = !s_isVisible;
    Py_RETURN_NONE;
}

PyObject *chrmgrRegisterPointEffect(PyObject *poSelf, PyObject *poArgs)
{
    int iEft;
    if (!PyTuple_GetInteger(poArgs, 0, &iEft))
        return Py_BadArgument();

    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
        return Py_BadArgument();

    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    rkChrMgr.RegisterPointEffect(iEft, szFileName.c_str());
    Py_RETURN_NONE;
}

PyObject *chrmgrShowPointEffect(PyObject *poSelf, PyObject *poArgs)
{
    int nVID;
    if (!PyTuple_GetInteger(poArgs, 0, &nVID))
        return Py_BadArgument();

    int nEft;
    if (!PyTuple_GetInteger(poArgs, 1, &nEft))
        return Py_BadArgument();

    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    rkChrMgr.ShowPointEffect(nEft, nVID >= 0 ? nVID : 0xffffffff);
    Py_RETURN_NONE;
}

PyObject *chrmgrSetAffect(PyObject *poSelf, PyObject *poArgs)
{
    int nVID;
    if (!PyTuple_GetInteger(poArgs, 0, &nVID))
        return Py_BadArgument();

    int nEft;
    if (!PyTuple_GetInteger(poArgs, 1, &nEft))
        return Py_BadArgument();

    int nVisible;
    if (!PyTuple_GetInteger(poArgs, 2, &nVisible))
        return Py_BadArgument();

    // TODO
    Py_RETURN_NONE;
}

void CPythonCharacterManager::SetEmoticon(uint32_t dwVID, uint32_t eState)
{
    CInstanceBase *pkInstSel = (dwVID == 0xffffffff) ? GetSelectedInstancePtr() : GetInstancePtr(dwVID);
    if (!pkInstSel)
        return;

    pkInstSel->SetEmoticon(eState);
}

bool CPythonCharacterManager::IsPossibleEmoticon(uint32_t dwVID)
{
    CInstanceBase *pkInstSel = (dwVID == 0xffffffff) ? GetSelectedInstancePtr() : GetInstancePtr(dwVID);
    if (!pkInstSel)
        return false;

    return pkInstSel->IsPossibleEmoticon();
}

PyObject *chrmgrSetEmoticon(PyObject *poSelf, PyObject *poArgs)
{
    int nVID;
    if (!PyTuple_GetInteger(poArgs, 0, &nVID))
        return Py_BadArgument();

    int nEft;
    if (!PyTuple_GetInteger(poArgs, 1, &nEft))
        return Py_BadArgument();

    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    rkChrMgr.SetEmoticon(nVID >= 0 ? nVID : 0xffffffff, nEft);
    Py_RETURN_NONE;
}

PyObject *chrmgrIsPossibleEmoticon(PyObject *poSelf, PyObject *poArgs)
{
    int nVID;
    if (!PyTuple_GetInteger(poArgs, 0, &nVID))
        return Py_BadArgument();

    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    int result = rkChrMgr.IsPossibleEmoticon(nVID >= 0 ? nVID : 0xffffffff);

    return Py_BuildValue("i", result);
}

#ifdef ENABLE_FLY_MOUNT
PyObject* chrmgrUcanAdam(PyObject* poSelf, PyObject* poArgs)
{
    float yukariAssa;
    if (!PyTuple_GetFloat(poArgs, 0, &yukariAssa))
        return Py_BadArgument();

    CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
    CInstanceBase* pkInsMain = rkChrMgr.GetMainInstancePtr();
    if (pkInsMain) {
        pkInsMain->GetGraphicThingInstancePtr()->UcanAdam(yukariAssa);
    }
    return Py_BuildNone();
}
#endif

PyObject *chrmgrLoadRaceMotions(PyObject *poSelf, PyObject *poArgs)
{
    int eRace;
    if (!PyTuple_GetInteger(poArgs, 0, &eRace))
        return Py_BadArgument();

    auto pRaceData = CRaceManager::instance().GetRaceDataPointer(eRace);
    if (pRaceData)
        pRaceData.value()->LoadMotions();

    return Py_BuildNone();
}

PyObject *chrmgrSetRaceHeight(PyObject *poSelf, PyObject *poArgs)
{
    int iRaceIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iRaceIndex))
        return Py_BadArgument();
    float fRaceHeight = 0.0f;
    if (!PyTuple_GetFloat(poArgs, 1, &fRaceHeight))
        return Py_BadArgument();

    CRaceManager::Instance().SetRaceHeight(iRaceIndex, fRaceHeight);
    return Py_BuildNone();
}

PyObject *chrmgrHasEffect(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    DWORD dwEffect;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID) ||
        !PyTuple_GetUnsignedLong(poArgs, 1, &dwEffect))
    {
        return Py_BuildException();
    }

    CInstanceBase *pkInst = CPythonCharacterManager::instance().GetInstancePtr(iVID);
    if (!pkInst)
    {
        return Py_BuildValue("i", FALSE);
    }

    return Py_BuildValue("i", pkInst->HasEffect(dwEffect) ? 1 : 0);
}

extern "C" void initchrmgr()
{
    static PyMethodDef s_methods[] =
    {
        // RaceData 관련		
        {"SetEmpireNameMode", chrmgrSetEmpireNameMode, METH_VARARGS},
        {"GetVIDInfo", chrmgrGetVIDInfo, METH_VARARGS},
        {"GetPickedVID", chrmgrGetPickedVID, METH_VARARGS},
        {"SetShapeModel", chrmgrSetShapeModel, METH_VARARGS},
        {"AppendShapeSkin", chrmgrAppendShapeSkin, METH_VARARGS},
        {"SetPathName", chrmgrSetPathName, METH_VARARGS},
        {"LoadRaceData", chrmgrLoadRaceData, METH_VARARGS},
        {"LoadLocalRaceData", chrmgrLoadLocalRaceData, METH_VARARGS},
        {"CreateRace", chrmgrCreateRace, METH_VARARGS},
        {"SelectRace", chrmgrSelectRace, METH_VARARGS},
        {"RegisterAttachingBoneName", chrmgrRegisterAttachingBoneName, METH_VARARGS},
        {"RegisterMotionMode", chrmgrRegisterMotionMode, METH_VARARGS},
        //{ "RegisterMotion",				chrmgrRegisterMotion,					METH_VARARGS },
        {"SetMotionRandomWeight", chrmgrSetMotionRandomWeight, METH_VARARGS},
        {"RegisterNormalAttack", chrmgrRegisterNormalAttack, METH_VARARGS},
        {"ReserveComboAttack", chrmgrReserveComboAttack, METH_VARARGS},
        {"RegisterComboAttack", chrmgrRegisterComboAttack, METH_VARARGS},
        {"ReserveComboAttackNew", chrmgrReserveComboAttackNew, METH_VARARGS},
        {"RegisterComboAttackNew", chrmgrRegisterComboAttackNew, METH_VARARGS},
#ifdef ENABLE_FLY_MOUNT
        {"UcanAdam", chrmgrUcanAdam, METH_VARARGS},
#endif
        {"RegisterMotionData", chrmgrRegisterMotionData, METH_VARARGS},
        {"RegisterRace", chrmgrRegisterRace, METH_VARARGS},
        {"RegisterCacheMotionData", chrmgrRegisterCacheMotionData, METH_VARARGS},

        // ETC
        //{ "SetAffect",					chrmgrSetAffect,						METH_VARARGS },
        {"LoadRaceMotions", chrmgrLoadRaceMotions, METH_VARARGS},
        {"SetEmoticon", chrmgrSetEmoticon, METH_VARARGS},
        {"IsPossibleEmoticon", chrmgrIsPossibleEmoticon, METH_VARARGS},
        {"RegisterEffect", chrmgrRegisterEffect, METH_VARARGS},
        {"RegisterAffect", chrmgrRegisterAffect, METH_VARARGS},
        {"RegisterCacheEffect", chrmgrRegisterCacheEffect, METH_VARARGS},
        {"RegisterPointEffect", chrmgrRegisterPointEffect, METH_VARARGS},
        {"ShowPointEffect", chrmgrShowPointEffect, METH_VARARGS},
        {"ToggleDirectionLine", chrmgrToggleDirectionLine, METH_VARARGS},
        {"SetMovingSpeed", chrmgrSetMovingSpeed, METH_VARARGS},
        {"SetDustGap", chrmgrSetDustGap, METH_VARARGS},
        {"SetHorseDustGap", chrmgrSetHorseDustGap, METH_VARARGS},

        {"RegisterTitleName", chrmgrRegisterTitleName, METH_VARARGS},
        {"RegisterNamePrefix", chrmgrRegisterNamePrefix, METH_VARARGS},

        {"RegisterNameColor", chrmgrRegisterNameColor, METH_VARARGS},
        {"RegisterTitleColor", chrmgrRegisterTitleColor, METH_VARARGS},
        {"HasEffect", chrmgrHasEffect, METH_VARARGS},
        {"SetRaceHeight", chrmgrSetRaceHeight, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("chrmgr", s_methods);

    PyModule_AddIntConstant(poModule, "NAMECOLOR_MOB", CInstanceBase::NAMECOLOR_NORMAL_MOB);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_NPC", CInstanceBase::NAMECOLOR_NORMAL_NPC);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_PC", CInstanceBase::NAMECOLOR_NORMAL_PC);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_STONE", CInstanceBase::NAMECOLOR_STONE);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_MOB", CInstanceBase::NAMECOLOR_EMPIRE_MOB);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_NPC", CInstanceBase::NAMECOLOR_EMPIRE_NPC);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_PC", CInstanceBase::NAMECOLOR_EMPIRE_PC);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_FUNC", CInstanceBase::NAMECOLOR_FUNC);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_PK", CInstanceBase::NAMECOLOR_PK);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_PVP", CInstanceBase::NAMECOLOR_PVP);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_PARTY", CInstanceBase::NAMECOLOR_PARTY);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_WARP", CInstanceBase::NAMECOLOR_WARP);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_WAYPOINT", CInstanceBase::NAMECOLOR_WAYPOINT);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_EXTRA", CInstanceBase::NAMECOLOR_EXTRA);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_BUFFBOT", CInstanceBase::NAMECOLOR_BUFFBOT);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_TARGET_VICTIM", CInstanceBase::NAMECOLOR_TARGET_VICTIM);
    PyModule_AddIntConstant(poModule, "NAMECOLOR_DEAD", CInstanceBase::NAMECOLOR_DEAD);
#ifdef __OFFLINE_SHOP__
    PyModule_AddIntConstant(poModule, "NAMECOLOR_OFFLINE_SHOP", CInstanceBase::NAMECOLOR_OFFLINE_SHOP);
#endif

    PyModule_AddIntConstant(poModule, "EFFECT_SPAWN_DISAPPEAR", EFFECT_SPAWN_DISAPPEAR);
    PyModule_AddIntConstant(poModule, "EFFECT_SPAWN_APPEAR", EFFECT_SPAWN_APPEAR);
    PyModule_AddIntConstant(poModule, "EFFECT_DUST", EFFECT_DUST);
    PyModule_AddIntConstant(poModule, "EFFECT_HORSE_DUST", EFFECT_HORSE_DUST);
    PyModule_AddIntConstant(poModule, "EFFECT_STUN", EFFECT_STUN);
    PyModule_AddIntConstant(poModule, "EFFECT_HIT", EFFECT_HIT);
    PyModule_AddIntConstant(poModule, "EFFECT_FLAME_ATTACK", EFFECT_FLAME_ATTACK);
    PyModule_AddIntConstant(poModule, "EFFECT_FLAME_HIT", EFFECT_FLAME_HIT);
    PyModule_AddIntConstant(poModule, "EFFECT_FLAME_ATTACH", EFFECT_FLAME_ATTACH);
    PyModule_AddIntConstant(poModule, "EFFECT_ELECTRIC_ATTACK", EFFECT_ELECTRIC_ATTACK);
    PyModule_AddIntConstant(poModule, "EFFECT_ELECTRIC_HIT", EFFECT_ELECTRIC_HIT);
    PyModule_AddIntConstant(poModule, "EFFECT_ELECTRIC_ATTACH", EFFECT_ELECTRIC_ATTACH);
    PyModule_AddIntConstant(poModule, "EFFECT_SELECT", EFFECT_SELECT);
    PyModule_AddIntConstant(poModule, "EFFECT_TARGET", EFFECT_TARGET);
    PyModule_AddIntConstant(poModule, "EFFECT_CRITICAL", EFFECT_CRITICAL);

    PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_TARGET", EFFECT_DAMAGE_TARGET);
    PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_NOT_TARGET", EFFECT_DAMAGE_NOT_TARGET);
    PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_SELFDAMAGE", EFFECT_DAMAGE_SELFDAMAGE);
    PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_SELFDAMAGE2", EFFECT_DAMAGE_SELFDAMAGE2);
    PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_POISON", EFFECT_DAMAGE_POISON);
    PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_MISS", EFFECT_DAMAGE_MISS);
    PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_TARGETMISS", EFFECT_DAMAGE_TARGETMISS);
    PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_CRITICAL", EFFECT_DAMAGE_CRITICAL);

    PyModule_AddIntConstant(poModule, "EFFECT_LEVELUP", EFFECT_LEVELUP);
    PyModule_AddIntConstant(poModule, "EFFECT_SKILLUP", EFFECT_SKILLUP);
    PyModule_AddIntConstant(poModule, "EFFECT_HPUP_RED", EFFECT_HPUP_RED);
    PyModule_AddIntConstant(poModule, "EFFECT_SPUP_BLUE", EFFECT_SPUP_BLUE);
    PyModule_AddIntConstant(poModule, "EFFECT_SPEEDUP_GREEN", EFFECT_SPEEDUP_GREEN);
    PyModule_AddIntConstant(poModule, "EFFECT_DXUP_PURPLE", EFFECT_DXUP_PURPLE);
    PyModule_AddIntConstant(poModule, "EFFECT_PENETRATE", EFFECT_PENETRATE);
    PyModule_AddIntConstant(poModule, "EFFECT_BLOCK", EFFECT_BLOCK);
    PyModule_AddIntConstant(poModule, "EFFECT_DODGE", EFFECT_DODGE);
    PyModule_AddIntConstant(poModule, "EFFECT_FIRECRACKER", EFFECT_FIRECRACKER);
    PyModule_AddIntConstant(poModule, "EFFECT_SPIN_TOP", EFFECT_SPIN_TOP);

    PyModule_AddIntConstant(poModule, "EFFECT_WEAPON", EFFECT_WEAPON);
    PyModule_AddIntConstant(poModule, "EFFECT_WEAPON_ONEHANDED", EFFECT_WEAPON_ONEHANDED);
    PyModule_AddIntConstant(poModule, "EFFECT_WEAPON_TWOHANDED", EFFECT_WEAPON_TWOHANDED);
    PyModule_AddIntConstant(poModule, "EFFECT_WEAPON_DUALHANDED", EFFECT_WEAPON_DUALHANDED);
    PyModule_AddIntConstant(poModule, "EFFECT_GM", EFFECT_GM);

    PyModule_AddIntMacro(poModule, AFFECT_NONE);
    PyModule_AddIntMacro(poModule, AFFECT_MOV_SPEED);
    PyModule_AddIntMacro(poModule, AFFECT_ATT_SPEED);
    PyModule_AddIntMacro(poModule, AFFECT_ATT_GRADE);
    PyModule_AddIntMacro(poModule, AFFECT_INVISIBILITY);
    PyModule_AddIntMacro(poModule, AFFECT_STR);
    PyModule_AddIntMacro(poModule, AFFECT_DEX);
    PyModule_AddIntMacro(poModule, AFFECT_CON);
    PyModule_AddIntMacro(poModule, AFFECT_INT);
    PyModule_AddIntMacro(poModule, AFFECT_FISH_MIND_PILL);
    PyModule_AddIntMacro(poModule, AFFECT_POISON);
    PyModule_AddIntMacro(poModule, AFFECT_STUN);
    PyModule_AddIntMacro(poModule, AFFECT_SLOW);
    PyModule_AddIntMacro(poModule, AFFECT_DUNGEON_READY);
    PyModule_AddIntMacro(poModule, AFFECT_DUNGEON_UNIQUE);
    PyModule_AddIntMacro(poModule, AFFECT_BUILDING);
    PyModule_AddIntMacro(poModule, AFFECT_REVIVE_INVISIBLE);
    PyModule_AddIntMacro(poModule, AFFECT_FIRE);
    PyModule_AddIntMacro(poModule, AFFECT_CAST_SPEED);
    PyModule_AddIntMacro(poModule, AFFECT_HP_RECOVER_CONTINUE);
    PyModule_AddIntMacro(poModule, AFFECT_SP_RECOVER_CONTINUE);
    PyModule_AddIntMacro(poModule, AFFECT_POLYMORPH);
    PyModule_AddIntMacro(poModule, AFFECT_MOUNT);
    PyModule_AddIntMacro(poModule, AFFECT_BLOCK_CHAT);
    PyModule_AddIntMacro(poModule, AFFECT_CHINA_FIREWORK);
    PyModule_AddIntMacro(poModule, AFFECT_BOW_DISTANCE);
    PyModule_AddIntMacro(poModule, AFFECT_DEF_GRADE);
    PyModule_AddIntMacro(poModule, AFFECT_BLEEDING);
    PyModule_AddIntMacro(poModule, AFFECT_RAMADAN_ABILITY);
    PyModule_AddIntMacro(poModule, AFFECT_RAMADAN_RING);
    PyModule_AddIntMacro(poModule, AFFECT_NOG_ABILITY);
    PyModule_AddIntMacro(poModule, AFFECT_HOLLY_STONE_POWER);
    PyModule_AddIntMacro(poModule, AFFECT_PREMIUM_START);
    PyModule_AddIntMacro(poModule, AFFECT_EXP_BONUS);
    PyModule_AddIntMacro(poModule, AFFECT_ITEM_BONUS);
    PyModule_AddIntMacro(poModule, AFFECT_SAFEBOX);
    PyModule_AddIntMacro(poModule, AFFECT_AUTOLOOT);
    PyModule_AddIntMacro(poModule, AFFECT_FISH_MIND);
    PyModule_AddIntMacro(poModule, AFFECT_MARRIAGE_FAST);
    PyModule_AddIntMacro(poModule, AFFECT_GOLD_BONUS);
    PyModule_AddIntMacro(poModule, AFFECT_SHOP_DOUBLE_UP);
    PyModule_AddIntMacro(poModule, AFFECT_VOTE_BONUS);
    PyModule_AddIntMacro(poModule, AFFECT_PREMIUM_END);
    PyModule_AddIntMacro(poModule, AFFECT_MALL);
    PyModule_AddIntMacro(poModule, AFFECT_NO_DEATH_PENALTY);
    PyModule_AddIntMacro(poModule, AFFECT_SKILL_BOOK_BONUS);
    PyModule_AddIntMacro(poModule, AFFECT_SKILL_NO_BOOK_DELAY);
    PyModule_AddIntMacro(poModule, AFFECT_HAIR);
    PyModule_AddIntMacro(poModule, AFFECT_COLLECT);
    PyModule_AddIntMacro(poModule, AFFECT_EXP_BONUS_EURO_FREE);
    PyModule_AddIntMacro(poModule, AFFECT_EXP_BONUS_EURO_FREE_UNDER_15);
    PyModule_AddIntMacro(poModule, AFFECT_UNIQUE_ABILITY);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_1);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_2);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_3);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_4);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_5);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_6);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_7);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_8);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_9);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_10);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_11);
    PyModule_AddIntMacro(poModule, AFFECT_CUBE_12);
    PyModule_AddIntMacro(poModule, AFFECT_BLEND);
    PyModule_AddIntMacro(poModule, AFFECT_HORSE_NAME);
    PyModule_AddIntMacro(poModule, AFFECT_MOUNT_BONUS);
    PyModule_AddIntMacro(poModule, AFFECT_AUTO_HP_RECOVERY);
    PyModule_AddIntMacro(poModule, AFFECT_AUTO_SP_RECOVERY);
    PyModule_AddIntMacro(poModule, AFFECT_PREVENT_GOOD);
    PyModule_AddIntMacro(poModule, AFFECT_WAR_FLAG_1);
    PyModule_AddIntMacro(poModule, AFFECT_WAR_FLAG_2);
    PyModule_AddIntMacro(poModule, AFFECT_WAR_FLAG_3);
    PyModule_AddIntMacro(poModule, AFFECT_DRAGON_SOUL_QUALIFIED);
    PyModule_AddIntMacro(poModule, AFFECT_DRAGON_SOUL_DECK_0);
    PyModule_AddIntMacro(poModule, AFFECT_DRAGON_SOUL_DECK_1);
    PyModule_AddIntMacro(poModule, AFFECT_VOTE_BONUS2);
    PyModule_AddIntMacro(poModule, AFFECT_VOTE_BONUS3);
    PyModule_AddIntMacro(poModule, AFFECT_VOTE_BONUS4);
    PyModule_AddIntMacro(poModule, AFFECT_STATUE_1);
    PyModule_AddIntMacro(poModule, AFFECT_STATUE_2);
    PyModule_AddIntMacro(poModule, AFFECT_STATUE_3);
    PyModule_AddIntMacro(poModule, AFFECT_STATUE_4);
    PyModule_AddIntMacro(poModule, AFFECT_DS_SET);
#ifdef ENABLE_BATTLE_PASS
    PyModule_AddIntMacro(poModule, AFFECT_BATTLE_PASS);
#endif

    PyModule_AddIntMacro(poModule, AFFECT_PREMIUM_SWITCHBOT);

    
    PyModule_AddIntMacro(poModule, AFFECT_QUEST_START_IDX);

    PyModule_AddIntMacro(poModule, SKILL_RESERVED);
    PyModule_AddIntMacro(poModule, SKILL_SAMYEON);
    PyModule_AddIntMacro(poModule, SKILL_PALBANG);
    PyModule_AddIntMacro(poModule, SKILL_JEONGWI);
    PyModule_AddIntMacro(poModule, SKILL_GEOMKYUNG);
    PyModule_AddIntMacro(poModule, SKILL_TANHWAN);
    PyModule_AddIntMacro(poModule, SKILL_GIGONGCHAM);
    PyModule_AddIntMacro(poModule, SKILL_GYOKSAN);
    PyModule_AddIntMacro(poModule, SKILL_DAEJINGAK);
    PyModule_AddIntMacro(poModule, SKILL_CHUNKEON);
    PyModule_AddIntMacro(poModule, SKILL_GEOMPUNG);
    PyModule_AddIntMacro(poModule, SKILL_AMSEOP);
    PyModule_AddIntMacro(poModule, SKILL_GUNGSIN);
    PyModule_AddIntMacro(poModule, SKILL_CHARYUN);
    PyModule_AddIntMacro(poModule, SKILL_EUNHYUNG);
    PyModule_AddIntMacro(poModule, SKILL_SANGONG);
    PyModule_AddIntMacro(poModule, SKILL_YEONSA);
    PyModule_AddIntMacro(poModule, SKILL_KWANKYEOK);
    PyModule_AddIntMacro(poModule, SKILL_HWAJO);
    PyModule_AddIntMacro(poModule, SKILL_GYEONGGONG);
    PyModule_AddIntMacro(poModule, SKILL_GIGUNG);
    PyModule_AddIntMacro(poModule, SKILL_SWAERYUNG);
    PyModule_AddIntMacro(poModule, SKILL_YONGKWON);
    PyModule_AddIntMacro(poModule, SKILL_GWIGEOM);
    PyModule_AddIntMacro(poModule, SKILL_TERROR);
    PyModule_AddIntMacro(poModule, SKILL_JUMAGAP);
    PyModule_AddIntMacro(poModule, SKILL_PABEOB);
    PyModule_AddIntMacro(poModule, SKILL_MARYUNG);
    PyModule_AddIntMacro(poModule, SKILL_HWAYEOMPOK);
    PyModule_AddIntMacro(poModule, SKILL_MUYEONG);
    PyModule_AddIntMacro(poModule, SKILL_MANASHIELD);
    PyModule_AddIntMacro(poModule, SKILL_TUSOK);
    PyModule_AddIntMacro(poModule, SKILL_MAHWAN);
    PyModule_AddIntMacro(poModule, SKILL_BIPABU);
    PyModule_AddIntMacro(poModule, SKILL_YONGBI);
    PyModule_AddIntMacro(poModule, SKILL_PAERYONG);
    PyModule_AddIntMacro(poModule, SKILL_HOSIN);
    PyModule_AddIntMacro(poModule, SKILL_REFLECT);
    PyModule_AddIntMacro(poModule, SKILL_GICHEON);
    PyModule_AddIntMacro(poModule, SKILL_NOEJEON);
    PyModule_AddIntMacro(poModule, SKILL_BYEURAK);
    PyModule_AddIntMacro(poModule, SKILL_CHAIN);
    PyModule_AddIntMacro(poModule, SKILL_JEONGEOP);
    PyModule_AddIntMacro(poModule, SKILL_KWAESOK);
    PyModule_AddIntMacro(poModule, SKILL_JEUNGRYEOK);
    PyModule_AddIntMacro(poModule, SKILL_7_A_ANTI_TANHWAN);
    PyModule_AddIntMacro(poModule, SKILL_7_B_ANTI_AMSEOP);
    PyModule_AddIntMacro(poModule, SKILL_7_C_ANTI_SWAERYUNG);
    PyModule_AddIntMacro(poModule, SKILL_7_D_ANTI_YONGBI);
    PyModule_AddIntMacro(poModule, SKILL_8_B_ANTI_YEONSA);
    PyModule_AddIntMacro(poModule, SKILL_8_C_ANTI_MAHWAN);
    PyModule_AddIntMacro(poModule, SKILL_8_D_ANTI_BYEURAK);
    PyModule_AddIntMacro(poModule, SKILL_LEADERSHIP);
    PyModule_AddIntMacro(poModule, SKILL_COMBO);
    PyModule_AddIntMacro(poModule, SKILL_CREATE);
    PyModule_AddIntMacro(poModule, SKILL_MINING);
    PyModule_AddIntMacro(poModule, SKILL_LANGUAGE1);
    PyModule_AddIntMacro(poModule, SKILL_LANGUAGE2);
    PyModule_AddIntMacro(poModule, SKILL_LANGUAGE3);
    PyModule_AddIntMacro(poModule, SKILL_POLYMORPH);
    PyModule_AddIntMacro(poModule, SKILL_HORSE);
    PyModule_AddIntMacro(poModule, SKILL_HORSE_SUMMON);
    PyModule_AddIntMacro(poModule, SKILL_HORSE_WILDATTACK);
    PyModule_AddIntMacro(poModule, SKILL_HORSE_CHARGE);
    PyModule_AddIntMacro(poModule, SKILL_HORSE_ESCAPE);
    PyModule_AddIntMacro(poModule, SKILL_HORSE_WILDATTACK_RANGE);
    PyModule_AddIntMacro(poModule, SKILL_ADD_HP);
    PyModule_AddIntMacro(poModule, SKILL_RESIST_PENETRATE);

    PyModule_AddIntMacro(poModule, SKILL_CHAYEOL);
    PyModule_AddIntMacro(poModule, SKILL_SALPOONG);
    PyModule_AddIntMacro(poModule, SKILL_GONGDAB);
    PyModule_AddIntMacro(poModule, SKILL_PASWAE);
    PyModule_AddIntMacro(poModule, SKILL_JEOKRANG);
    PyModule_AddIntMacro(poModule, SKILL_CHEONGRANG);

    PyModule_AddIntConstant(poModule, "EFFECT_EMOTICON", EFFECT_EMOTICON);
    PyModule_AddIntConstant(poModule, "EFFECT_EMPIRE", EFFECT_EMPIRE);

    // Refine effects

    PyModule_AddIntConstant(poModule, "EFFECT_SWORD_REFINED7", EFFECT_SWORD_REFINED7);
    PyModule_AddIntConstant(poModule, "EFFECT_SWORD_REFINED8", EFFECT_SWORD_REFINED8);
    PyModule_AddIntConstant(poModule, "EFFECT_SWORD_REFINED9", EFFECT_SWORD_REFINED9);
    PyModule_AddIntConstant(poModule, "EFFECT_BOW_REFINED7", EFFECT_BOW_REFINED7);
    PyModule_AddIntConstant(poModule, "EFFECT_BOW_REFINED8", EFFECT_BOW_REFINED8);
    PyModule_AddIntConstant(poModule, "EFFECT_BOW_REFINED9", EFFECT_BOW_REFINED9);
    PyModule_AddIntConstant(poModule, "EFFECT_FANBELL_REFINED7", EFFECT_FANBELL_REFINED7);
    PyModule_AddIntConstant(poModule, "EFFECT_FANBELL_REFINED8", EFFECT_FANBELL_REFINED8);
    PyModule_AddIntConstant(poModule, "EFFECT_FANBELL_REFINED9", EFFECT_FANBELL_REFINED9);
    PyModule_AddIntConstant(poModule, "EFFECT_FANBELL_REFINED7_LEFT", EFFECT_FANBELL_REFINED7_LEFT);
    PyModule_AddIntConstant(poModule, "EFFECT_FANBELL_REFINED8_LEFT", EFFECT_FANBELL_REFINED8_LEFT);
    PyModule_AddIntConstant(poModule, "EFFECT_FANBELL_REFINED9_LEFT", EFFECT_FANBELL_REFINED9_LEFT);
    PyModule_AddIntConstant(poModule, "EFFECT_SMALLSWORD_REFINED7", EFFECT_SMALLSWORD_REFINED7);
    PyModule_AddIntConstant(poModule, "EFFECT_SMALLSWORD_REFINED8", EFFECT_SMALLSWORD_REFINED8);
    PyModule_AddIntConstant(poModule, "EFFECT_SMALLSWORD_REFINED9", EFFECT_SMALLSWORD_REFINED9);
    PyModule_AddIntConstant(poModule, "EFFECT_SMALLSWORD_REFINED7_LEFT", EFFECT_SMALLSWORD_REFINED7_LEFT);
    PyModule_AddIntConstant(poModule, "EFFECT_SMALLSWORD_REFINED8_LEFT", EFFECT_SMALLSWORD_REFINED8_LEFT);
    PyModule_AddIntConstant(poModule, "EFFECT_SMALLSWORD_REFINED9_LEFT", EFFECT_SMALLSWORD_REFINED9_LEFT);
    PyModule_AddIntConstant(poModule, "EFFECT_CLAW_REFINED7", EFFECT_CLAW_REFINED7);
    PyModule_AddIntConstant(poModule, "EFFECT_CLAW_REFINED8", EFFECT_CLAW_REFINED8);
    PyModule_AddIntConstant(poModule, "EFFECT_CLAW_REFINED9", EFFECT_CLAW_REFINED9);
    PyModule_AddIntConstant(poModule, "EFFECT_CLAW_REFINED7_LEFT", EFFECT_CLAW_REFINED7_LEFT);
    PyModule_AddIntConstant(poModule, "EFFECT_CLAW_REFINED8_LEFT", EFFECT_CLAW_REFINED8_LEFT);
    PyModule_AddIntConstant(poModule, "EFFECT_CLAW_REFINED9_LEFT", EFFECT_CLAW_REFINED9_LEFT);
    PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_REFINED7", EFFECT_BODYARMOR_REFINED7);
    PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_REFINED8", EFFECT_BODYARMOR_REFINED8);
    PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_REFINED9", EFFECT_BODYARMOR_REFINED9);
    PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_SPECIAL", EFFECT_BODYARMOR_SPECIAL);
    PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_SPECIAL2", EFFECT_BODYARMOR_SPECIAL2);
    PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_SPECIAL3", EFFECT_BODYARMOR_SPECIAL3);

#ifdef EVILLON_REFINE_EFFECTS
	PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_LOVE", EFFECT_BODYARMOR_LOVE);
	PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_BLACK", EFFECT_BODYARMOR_BLACK);
	PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_WHITE", EFFECT_BODYARMOR_WHITE);
	PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_RED", EFFECT_BODYARMOR_RED);
	PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_VIP", EFFECT_BODYARMOR_VIP);
	PyModule_AddIntConstant(poModule, "EFFECT_BODYARMOR_EPIC", EFFECT_BODYARMOR_EPIC);
#endif

    PyModule_AddIntConstant(poModule, "EFFECT_SUCCESS", EFFECT_SUCCESS);
    PyModule_AddIntConstant(poModule, "EFFECT_FAIL", EFFECT_FAIL);
    PyModule_AddIntConstant(poModule, "EFFECT_FR_SUCCESS", EFFECT_FR_SUCCESS);
    PyModule_AddIntConstant(poModule, "EFFECT_LEVELUP_ON_14_FOR_GERMANY", EFFECT_LEVELUP_ON_14_FOR_GERMANY);
    //레벨업 14일때 ( 독일전용 )
    PyModule_AddIntConstant(poModule, "EFFECT_LEVELUP_UNDER_15_FOR_GERMANY", EFFECT_LEVELUP_UNDER_15_FOR_GERMANY);
    //레벨업 15일때 ( 독일전용 )
    PyModule_AddIntConstant(poModule, "EFFECT_PERCENT_DAMAGE1", EFFECT_PERCENT_DAMAGE1);
    PyModule_AddIntConstant(poModule, "EFFECT_PERCENT_DAMAGE2", EFFECT_PERCENT_DAMAGE2);
    PyModule_AddIntConstant(poModule, "EFFECT_PERCENT_DAMAGE3", EFFECT_PERCENT_DAMAGE3);

    // 자동물약 HP, SP
    PyModule_AddIntConstant(poModule, "EFFECT_AUTO_HPUP", EFFECT_AUTO_HPUP);
    PyModule_AddIntConstant(poModule, "EFFECT_AUTO_SPUP", EFFECT_AUTO_SPUP);

    PyModule_AddIntConstant(poModule, "EFFECT_RAMADAN_RING_EQUIP", EFFECT_RAMADAN_RING_EQUIP);
    PyModule_AddIntConstant(poModule, "EFFECT_HALLOWEEN_CANDY_EQUIP", EFFECT_HALLOWEEN_CANDY_EQUIP);
    PyModule_AddIntConstant(poModule, "EFFECT_HAPPINESS_RING_EQUIP", EFFECT_HAPPINESS_RING_EQUIP);
    PyModule_AddIntConstant(poModule, "EFFECT_LOVE_PENDANT_EQUIP", EFFECT_LOVE_PENDANT_EQUIP);

    PyModule_AddIntConstant(poModule, "EFFECT_ACCE_SUCESS_ABSORB", EFFECT_ACCE_SUCESS_ABSORB);
    PyModule_AddIntConstant(poModule, "EFFECT_ACCE_EQUIP", EFFECT_ACCE_EQUIP);
    PyModule_AddIntConstant(poModule, "EFFECT_ACCE_BACK", EFFECT_ACCE_BACK);
    PyModule_AddIntConstant(poModule, "EFFECT_EASTER_CANDY_EQIP", EFFECT_EASTER_CANDY_EQIP);
    PyModule_AddIntConstant(poModule, "EFFECT_CAPE_OF_COURAGE", EFFECT_CAPE_OF_COURAGE);
}
