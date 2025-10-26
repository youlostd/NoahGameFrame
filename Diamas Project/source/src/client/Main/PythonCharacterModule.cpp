#include "StdAfx.h"
#include "PythonCharacterManager.h"
#include "../GameLib/NpcManager.h"
#include <game/Constants.hpp>
#include <game/AffectConstants.hpp>
#include <game/MotionConstants.hpp>

PyObject *chrRaceToJob(PyObject *poSelf, PyObject *poArgs)
{
    int race;
    if (!PyTuple_GetInteger(poArgs, 0, &race))
        return Py_BadArgument();

    return Py_BuildValue("i", GetJobByRace(race));
}

PyObject *chrRaceToSex(PyObject *poSelf, PyObject *poArgs)
{
    int race;
    if (!PyTuple_GetInteger(poArgs, 0, &race))
        return Py_BadArgument();

    return Py_BuildValue("i", GetSexByRace(race));
}

PyObject *chrDestroy(PyObject *poSelf, PyObject *poArgs)
{
    CPythonCharacterManager::Instance().Destroy();
    Py_RETURN_NONE;
}

PyObject *chrUpdate(PyObject *poSelf, PyObject *poArgs)
{
    CPythonCharacterManager::Instance().Update();
    Py_RETURN_NONE;
}

PyObject *chrDeform(PyObject *poSelf, PyObject *poArgs)
{
    CPythonCharacterManager::Instance().Deform();
    Py_RETURN_NONE;
}

PyObject *chrRender(PyObject *poSelf, PyObject *poArgs)
{
    CPythonCharacterManager::Instance().Render();
    Py_RETURN_NONE;
}

PyObject *chrRenderCollision(PyObject *poSelf, PyObject *poArgs)
{
   // CPythonCharacterManager::Instance().RenderCollision();
    Py_RETURN_NONE;
}

// Functions For Python Test Code
PyObject *chrCreateInstance(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();

    CPythonCharacterManager::Instance().RegisterInstance(iVirtualID);
    Py_RETURN_NONE;
}

PyObject *chrDeleteInstance(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();

    CPythonCharacterManager::Instance().DeleteInstance(iVirtualID);
    Py_RETURN_NONE;
}

PyObject *chrDeleteInstanceByFade(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();

    CPythonCharacterManager::Instance().DeleteInstanceByFade(iVirtualID);
    Py_RETURN_NONE;
}

PyObject *chrSelectInstance(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();

    CPythonCharacterManager::Instance().SelectInstance(iVirtualID);
    Py_RETURN_NONE;
}

PyObject *chrHasInstance(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();

    bool bFlag = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID) ? TRUE : FALSE;
    return Py_BuildValue("i", bFlag);
}

PyObject *chrIsEnemy(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (!pInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("i", pInstance->IsEnemy());
}

PyObject *chrIsNPC(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (!pInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("i", pInstance->IsNPC());
}

PyObject *chrIsPartyMember(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (!pInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("i", pInstance->IsPartyMember());
}

PyObject *chrSelect(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pkInst)
        Py_RETURN_NONE;

    switch (NpcManager::Instance().GetEventType(pkInst->GetVirtualNumber()))
    {
    case ON_CLICK_SHOP:
        pkInst->SetAddColor(DirectX::SimpleMath::Color(0.0f, 0.3f, 0.0f, 1.0f));
        break;

    default:
        // NOTE: 빨간색으로 나오게 하면 스샷 찍을 때 보기가 안좋아서 코멘트 하였습니다 [cronan 040226]
        //pkInst->SetAddColor(DirectX::SimpleMath::Color(0.3f, 0.0f, 0.0f, 1.0f));
        break;
    }

    Py_RETURN_NONE;
}

PyObject *chrSetAddRenderMode(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    float fr;
    if (!PyTuple_GetFloat(poArgs, 1, &fr))
        return Py_BuildException();
    float fg;
    if (!PyTuple_GetFloat(poArgs, 2, &fg))
        return Py_BuildException();
    float fb;
    if (!PyTuple_GetFloat(poArgs, 3, &fb))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pkInst)
        Py_RETURN_NONE;

    pkInst->SetAddColor(DirectX::SimpleMath::Color(fr, fg, fb, 1.0f));

    Py_RETURN_NONE;
}

PyObject *chrSetBlendRenderMode(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    float fAlpha;
    if (!PyTuple_GetFloat(poArgs, 1, &fAlpha))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pkInst)
        Py_RETURN_NONE;

    pkInst->SetAlpha(fAlpha);

    Py_RETURN_NONE;
}

PyObject *chrUnselect(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (!pkInst)
        Py_RETURN_NONE;

    pkInst->RestoreRenderMode();
    Py_RETURN_NONE;
}

PyObject *chrPick(PyObject *poSelf, PyObject *poArgs)
{
    uint32_t VirtualID = 0;
    if (CPythonCharacterManager::Instance().OLD_GetPickedInstanceVID(&VirtualID))
        return Py_BuildValue("i", VirtualID);
    else
        return Py_BuildValue("i", -1);
}

PyObject *chrHide(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;

    pkInst->Hide();
    Py_RETURN_NONE;
}

PyObject *chrShow(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;

    pkInst->Show();
    Py_RETURN_NONE;
}

PyObject *chrPickAll(PyObject *poSelf, PyObject *poArgs)
{
    uint32_t VirtualID = CPythonCharacterManager::Instance().PickAll();
    return Py_BuildValue("i", VirtualID);
}

PyObject *chrSetRace(PyObject *poSelf, PyObject *poArgs)
{
    int iRace;
    if (!PyTuple_GetInteger(poArgs, 0, &iRace))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;

    pkInst->SetRace(iRace);

    Py_RETURN_NONE;
}

PyObject *chrSetAcce(PyObject *poSelf, PyObject *poArgs)
{
    Part part{};
    if (!PyTuple_GetUnsignedInteger(poArgs, 0, &part.vnum))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;

    pkInst->SetAcce(part);

    Py_RETURN_NONE;
}

PyObject *chrSetScale(PyObject *poSelf, PyObject *poArgs)
{
    float zScale;
    float yScale;
    float xScale;
    if (!PyTuple_GetFloat(poArgs, 0, &xScale) || PyTuple_GetFloat(poArgs, 1, &yScale) || PyTuple_GetFloat(
            poArgs, 2, &zScale))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;

    CActorInstance& actoInst = pkInst->GetGraphicThingInstanceRef();
    actoInst.SetScale(xScale, yScale, zScale);

    Py_RETURN_NONE;
}

PyObject *chrChangeAcce(PyObject *poSelf, PyObject *poArgs)
{
    Part part{};
    if (!PyTuple_GetUnsignedInteger(poArgs, 0, &part.vnum))
        return Py_BuildException();
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;

    pkInst->ChangeAcce(part);
    Py_RETURN_NONE;
}

PyObject *chrSetHair(PyObject *poSelf, PyObject *poArgs)
{
    Part part{};
    if (!PyTuple_GetUnsignedInteger(poArgs, 0, &part.vnum))
        return Py_BadArgument();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;

    pkInst->SetHair(part);

    Py_RETURN_NONE;
}

PyObject *chrChangeHair(PyObject *poSelf, PyObject *poArgs)
{
    auto cur = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!cur)
        Py_RETURN_NONE;

    switch (PyTuple_Size(poArgs))
    {
    case 1: {
        Part part{};
        if (!PyTuple_GetUnsignedInteger(poArgs, 0, &part.vnum))
            return Py_BadArgument();

        cur->ChangeHair(part);
        break;
    }

    case 2: {
        Part part{};
        if (!PyTuple_GetUnsignedInteger(poArgs, 0, &part.vnum) ||
            !PyTuple_GetUnsignedInteger(poArgs, 1, &part.appearance))
            return Py_BadArgument();

        cur->ChangeHair(part);
        break;
    }
    }
    Py_RETURN_NONE;
}

PyObject *chrSetArmor(PyObject *poSelf, PyObject *poArgs)
{
    auto cur = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!cur)
        Py_RETURN_NONE;

    switch (PyTuple_Size(poArgs))
    {
    case 1: {
        Part part{};
        if (!PyTuple_GetUnsignedInteger(poArgs, 0, &part.vnum))
            return Py_BadArgument();

        cur->SetArmor(part);
        break;
    }

    case 2: {
        Part part{};
        if (!PyTuple_GetUnsignedInteger(poArgs, 0, &part.vnum) ||
            !PyTuple_GetUnsignedInteger(poArgs, 1, &part.appearance))
            return Py_BadArgument();

        cur->SetArmor(part);
        break;
    }
    }

    if (cur)
        cur->RegisterBoundingSphere();

    Py_RETURN_NONE;
}

PyObject *chrChangeShape(PyObject *poSelf, PyObject *poArgs)
{
    auto cur = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!cur)
        Py_RETURN_NONE;

    switch (PyTuple_Size(poArgs))
    {
    case 1: {
        Part part{};
        if (!PyTuple_GetUnsignedInteger(poArgs, 0, &part.vnum))
            return Py_BadArgument();

        cur->ChangeArmor(part);
        break;
    }

    case 2: {
        Part part{};
        if (!PyTuple_GetUnsignedInteger(poArgs, 0, &part.vnum) ||
            !PyTuple_GetUnsignedInteger(poArgs, 1, &part.appearance))
            return Py_BadArgument();

        cur->ChangeArmor(part);
        break;
    }
    }

    Py_RETURN_NONE;
}

PyObject *chrSetWeapon(PyObject *poSelf, PyObject *poArgs)
{
    auto cur = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!cur)
        Py_RETURN_NONE;

    switch (PyTuple_Size(poArgs))
    {
    case 1: {
        Part part{};
        if (!PyTuple_GetUnsignedInteger(poArgs, 0, &part.vnum))
            return Py_BadArgument();

        cur->SetWeapon(part);
        break;
    }

    case 2: {
        Part part{};
        if (!PyTuple_GetUnsignedInteger(poArgs, 0, &part.vnum) ||
            !PyTuple_GetUnsignedInteger(poArgs, 1, &part.appearance))
            return Py_BadArgument();

        cur->SetWeapon(part);
        break;
    }
    }

    Py_RETURN_NONE;
}

PyObject *chrSetVirtualID(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    pkInst->SetVirtualID(iVID);
    Py_RETURN_NONE;
}

PyObject *chrSetNameString(PyObject *poSelf, PyObject *poArgs)
{
    std::string c_szName;
    if (!PyTuple_GetString(poArgs, 0, c_szName))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    pkInst->SetNameString(c_szName.c_str(), c_szName.length());
    Py_RETURN_NONE;
}

PyObject *chrSetInstanceType(PyObject *poSelf, PyObject *poArgs)
{
    int iInstanceType;
    if (!PyTuple_GetInteger(poArgs, 0, &iInstanceType))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    pkInst->SetInstanceType(iInstanceType);
    Py_RETURN_NONE;
}

PyObject *chrAttachEffectByName(PyObject *poSelf, PyObject *poArgs)
{
    int iParentPartIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iParentPartIndex))
        return Py_BuildException();

    std::string pszBoneName;
    if (!PyTuple_GetString(poArgs, 1, pszBoneName))
    {
        pszBoneName = "";
        //return Py_BuildException();
    }

    std::string pszEffectName;
    if (!PyTuple_GetString(poArgs, 2, pszEffectName))
        return Py_BuildException();

    int iLife = CActorInstance::EFFECT_LIFE_INFINITE;

    if (PyTuple_Size(poArgs) == 4)
    {
        if (!PyTuple_GetInteger(poArgs, 3, &iLife))
            return Py_BuildException();
    }

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    if (!pkInst)
        Py_RETURN_NONE;

    //pkInst->AttachEffectByName(iParentPartIndex, pszBoneName, pszEffectName, iLife);
    Py_RETURN_NONE;
}

PyObject *chrRefresh(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    if (!pkInst)
        Py_RETURN_NONE;

    // Select 화면에서는 WAIT 모션이 준비 되지 않은 상태이기 때문에 문제가 생긴다.
    //pkInst->Refresh(MOTION_WAIT, true);
    Py_RETURN_NONE;
}

PyObject *chrMountHorse(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    pkInst->MountHorse(20030);
    Py_RETURN_NONE;
}

PyObject *chrDismountHorse(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    pkInst->DismountHorse();
    Py_RETURN_NONE;
}

PyObject *chrRevive(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    pkInst->Revive();
    Py_RETURN_NONE;
}

PyObject *chrDie(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    pkInst->Die();
    Py_RETURN_NONE;
}

PyObject *chrLookAt(PyObject *poSelf, PyObject *poArgs)
{
    int iCellX;
    if (!PyTuple_GetInteger(poArgs, 0, &iCellX))
        return Py_BuildException();

    int iCellY;
    if (!PyTuple_GetInteger(poArgs, 1, &iCellY))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    // 	pkInst->LookAt(TPixelPosition(iCellX * c_Section_xAttributeCellSize, iCellY * c_Section_xAttributeCellSize));
    Py_RETURN_NONE;
}

PyObject *chrSetMotionMode(PyObject *poSelf, PyObject *poArgs)
{
    int iMotionMode;
    if (!PyTuple_GetInteger(poArgs, 0, &iMotionMode))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;

    pkInst->SetMotionMode(iMotionMode);
    Py_RETURN_NONE;
}

PyObject *chrSetLoopMotion(PyObject *poSelf, PyObject *poArgs)
{
    int iMotionIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iMotionIndex))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    pkInst->SetLoopMotion(uint16_t(iMotionIndex));

    Py_RETURN_NONE;
}

PyObject *chrBlendLoopMotion(PyObject *poSelf, PyObject *poArgs)
{
    int iMotionIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iMotionIndex))
        return Py_BuildException();
    float fBlendTime;
    if (!PyTuple_GetFloat(poArgs, 1, &fBlendTime))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    pkInst->SetLoopMotion(uint16_t(iMotionIndex), fBlendTime);

    Py_RETURN_NONE;
}

PyObject *chrPushOnceMotion(PyObject *poSelf, PyObject *poArgs)
{
    int iMotionIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iMotionIndex))
        return Py_BuildException();

    float fBlendTime;
    if (!PyTuple_GetFloat(poArgs, 1, &fBlendTime))
    {
        fBlendTime = 0.1f;
    }

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    pkInst->PushOnceMotion(uint16_t(iMotionIndex), fBlendTime);

    Py_RETURN_NONE;
}

PyObject *chrPushLoopMotion(PyObject *poSelf, PyObject *poArgs)
{
    int iMotionIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iMotionIndex))
        return Py_BuildException();

    float fBlendTime;
    if (!PyTuple_GetFloat(poArgs, 1, &fBlendTime))
    {
        fBlendTime = 0.1f;
    }

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;
    pkInst->PushLoopMotion(uint16_t(iMotionIndex), fBlendTime);

    Py_RETURN_NONE;
}

PyObject *chrSetPixelPosition(PyObject *poSelf, PyObject *poArgs)
{
    int iX;
    if (!PyTuple_GetInteger(poArgs, 0, &iX))
        return Py_BuildException();
    int iY;
    if (!PyTuple_GetInteger(poArgs, 1, &iY))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;

    int iZ;
    if (PyTuple_GetInteger(poArgs, 2, &iZ))
    {
        pkInst->NEW_SetPixelPosition(TPixelPosition(iX, iY, iZ));
    }
    else
    {
        pkInst->SCRIPT_SetPixelPosition(iX, iY);
    }
    Py_RETURN_NONE;
}

PyObject *chrSetDirection(PyObject *poSelf, PyObject *poArgs)
{
    int iDirection;
    if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    pkInst->SetDirection(iDirection);
    Py_RETURN_NONE;
}

PyObject *chrGetPixelPosition(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pkInst)
        return Py_BuildException();

    TPixelPosition PixelPosition;
    pkInst->NEW_GetPixelPosition(&PixelPosition);

    return Py_BuildValue("fff", PixelPosition.x, PixelPosition.y, PixelPosition.z);
}

PyObject *chrSetRotation(PyObject *poSelf, PyObject *poArgs)
{
    float fRotation;
    if (!PyTuple_GetFloat(poArgs, 0, &fRotation))
        return Py_BuildException();

    CInstanceBase *pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    if (!pCharacterInstance)
        Py_RETURN_NONE;

    fRotation = fmod(fRotation + 180.0f, 360.0f);

    pCharacterInstance->SetRotation(fRotation);

    Py_RETURN_NONE;
}

PyObject *chrSetRotationAll(PyObject *poSelf, PyObject *poArgs)
{
    float fRotX;
    if (!PyTuple_GetFloat(poArgs, 0, &fRotX))
        return Py_BuildException();
    float fRotY;
    if (!PyTuple_GetFloat(poArgs, 1, &fRotY))
        return Py_BuildException();
    float fRotZ;
    if (!PyTuple_GetFloat(poArgs, 2, &fRotZ))
        return Py_BuildException();

    CInstanceBase *pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pCharacterInstance)
        Py_RETURN_NONE;

    pCharacterInstance->GetGraphicThingInstanceRef().SetXYRotation(fRotX, fRotY);
    pCharacterInstance->GetGraphicThingInstanceRef().SetRotation(fRotZ);
    Py_RETURN_NONE;
}

PyObject *chrBlendRotation(PyObject *poSelf, PyObject *poArgs)
{
    float fRotation;
    if (!PyTuple_GetFloat(poArgs, 0, &fRotation))
        return Py_BuildException();
    float fBlendTime;
    if (!PyTuple_GetFloat(poArgs, 1, &fBlendTime))
        return Py_BuildException();

    fRotation = fmod(720.0f - fRotation, 360.0f);

    CInstanceBase *pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pCharacterInstance)
        Py_RETURN_NONE;
    pCharacterInstance->BlendRotation(fRotation, fBlendTime);

    Py_RETURN_NONE;
}

PyObject *chrGetRotation(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    if (!pCharacterInstance)
        return Py_BuildValue("f", 0.0f);

    float fRotation = pCharacterInstance->GetRotation();

    return Py_BuildValue("f", 360.0f - fRotation);
}

PyObject *chrGetRace(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    if (!pCharacterInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("i", pCharacterInstance->GetRace());
}

PyObject *chrGetRaceByVID(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("i", pInstance->GetRace());
}

PyObject *chrGetName(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    if (!pCharacterInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("s", pCharacterInstance->GetNameString());
}

PyObject *chrGetNameByVID(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pInstance)
        return Py_BuildValue("s", "None");

    return Py_BuildValue("s", pInstance->GetNameString());
}

PyObject *chrGetLevel(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    if (!pInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("i", pInstance->GetLevel());
}

PyObject *chrGetLevelByVID(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("i", pInstance->GetLevel());
}

PyObject *chrGetGuildID(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("i", pInstance->GetGuildID());
}

PyObject *chrGetProjectPosition(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();
    int iHeight;
    if (!PyTuple_GetInteger(poArgs, 1, &iHeight))
        return Py_BuildException();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (!pInstance)
        return Py_BuildValue("ii", -100, -100);

    TPixelPosition PixelPosition;
    pInstance->NEW_GetPixelPosition(&PixelPosition);

    CPythonGraphic &rpyGraphic = CPythonGraphic::Instance();

    float fx, fy, fz;
    rpyGraphic.ProjectPosition(PixelPosition.x,
                               -PixelPosition.y,
                               PixelPosition.z + float(iHeight),
                               &fx, &fy, &fz);

    if (1 == int(fz))
        return Py_BuildValue("ii", -100, -100);

    return Py_BuildValue("ii", int(fx), int(fy));
}

PyObject *chrGetVirtualNumber(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (NULL != pkInst)
        return Py_BuildValue("i", pkInst->GetVirtualNumber());

    return Py_BuildValue("i", CActorInstance::TYPE_PC);
}

PyObject *chrGetInstanceType(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

    if (NULL != pkInst)
        return Py_BuildValue("i", pkInst->GetInstanceType());

    return Py_BuildValue("i", CActorInstance::TYPE_PC);
}

PyObject *chrGetBoundBoxOnlyXY(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    if (!pkInst)
        return Py_BuildValue("ffff", 0.0f, 0.0f, 0.0f, 0.0f);

    Vector3 v3Min, v3Max;
    pkInst->GetBoundBox(&v3Min, &v3Max);

    return Py_BuildValue("ffff", v3Min.x, v3Min.y, v3Max.x, v3Max.y);
}

PyObject *chrtestGetPKData(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    if (!pkInst)
        return Py_BuildValue("ii", 0, 4);

    return Py_BuildValue("ii", pkInst->GetAlignment(), pkInst->GetAlignmentGrade());
}

PyObject *chrtestSetComboType(PyObject *poSelf, PyObject *poArgs)
{
    int iComboType;
    if (!PyTuple_GetInteger(poArgs, 0, &iComboType))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (!pkInst)
        Py_RETURN_NONE;

    pkInst->GetGraphicThingInstanceRef().SetComboType(iComboType);

    Py_RETURN_NONE;
}

PyObject *chrtestSetAddRenderMode(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();
    int iColor;
    if (!PyTuple_GetInteger(poArgs, 1, &iColor))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (pkInst)
    {
        pkInst->SetAddColor(DirectX::SimpleMath::Color(0xff000000 | iColor));
    }

    Py_RETURN_NONE;
}

PyObject *chrtestSetModulateRenderMode(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();
    int iColor;
    if (!PyTuple_GetInteger(poArgs, 1, &iColor))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (pkInst)
    {
        pkInst->SetAddColor(DirectX::SimpleMath::Color(0xff000000 | iColor));
    }

    Py_RETURN_NONE;
}

PyObject *chrtestSetAddRenderModeRGB(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();
    float fr;
    if (!PyTuple_GetFloat(poArgs, 1, &fr))
        return Py_BuildException();
    float fg;
    if (!PyTuple_GetFloat(poArgs, 2, &fg))
        return Py_BuildException();
    float fb;
    if (!PyTuple_GetFloat(poArgs, 3, &fb))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (pkInst)
    {
        pkInst->SetAddColor(DirectX::SimpleMath::Color(fr, fg, fb, 1.0f));
    }

    Py_RETURN_NONE;
}

PyObject *chrtestSetModulateRenderModeRGB(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();
    float fr;
    if (!PyTuple_GetFloat(poArgs, 1, &fr))
        return Py_BuildException();
    float fg;
    if (!PyTuple_GetFloat(poArgs, 2, &fg))
        return Py_BuildException();
    float fb;
    if (!PyTuple_GetFloat(poArgs, 3, &fb))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (pkInst)
    {
        pkInst->SetAddColor(DirectX::SimpleMath::Color(fr, fg, fb, 1.0f));
    }

    Py_RETURN_NONE;
}

PyObject *chrtestSetSpecularRenderMode(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();
    int iPart;
    if (!PyTuple_GetInteger(poArgs, 1, &iPart))
        return Py_BuildException();
    float fAlpha;
    if (!PyTuple_GetFloat(poArgs, 2, &fAlpha))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (pkInst)
    {
        pkInst->GetGraphicThingInstanceRef().SetSpecularInfo(TRUE, iPart, fAlpha);
    }

    Py_RETURN_NONE;
}

PyObject *chrtestSetSpecularRenderMode2(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();
    int iPart;
    if (!PyTuple_GetInteger(poArgs, 1, &iPart))
        return Py_BuildException();
    float fAlpha;
    if (!PyTuple_GetFloat(poArgs, 2, &fAlpha))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (pkInst)
    {
        pkInst->GetGraphicThingInstanceRef().SetSpecularInfoForce(TRUE, iPart, fAlpha);
    }

    Py_RETURN_NONE;
}

PyObject *chrtestRestoreRenderMode(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CInstanceBase *pkInst = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (pkInst)
    {
        pkInst->RestoreRenderMode();
    }

    Py_RETURN_NONE;
}

PyObject *chrFaintTest(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
    if (pCharacterInstance)
    {
        auto &charThingRef = pCharacterInstance->GetGraphicThingInstanceRef();
        if (charThingRef.IsFaint())
        {
            charThingRef.SetFaint(false);
        }
        else
        {
            charThingRef.InterceptOnceMotion(MOTION_DAMAGE_FLYING);
            charThingRef.PushOnceMotion(MOTION_STAND_UP);
            charThingRef.PushLoopMotion(MOTION_WAIT);
            charThingRef.SetFaint(true);
        }
    }

    Py_RETURN_NONE;
}

PyObject *chrSetMoveSpeed(PyObject *poSelf, PyObject *poArgs)
{
    //int iSpeed;
    //if (!PyTuple_GetInteger(poArgs, 0, &iSpeed))
    //	return Py_BadArgument();

    //CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    //if (!pCharacterInstance)
    //	return Py_BuildValue("i", 0);

    //pCharacterInstance->SetMoveSpeed(iSpeed);

    Py_RETURN_NONE;
}

PyObject *chrSetAttackSpeed(PyObject *poSelf, PyObject *poArgs)
{
    //int iSpeed;
    //if (!PyTuple_GetInteger(poArgs, 0, &iSpeed))
    //	return Py_BadArgument();

    //CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

    //if (!pCharacterInstance)
    //	return Py_BuildValue("i", 0);

    //pCharacterInstance->SetAttackSpeed(iSpeed);

    Py_RETURN_NONE;
}

PyObject *chrWeaponTraceSetTexture(PyObject *poSelf, PyObject *poArgs)
{
    std::string szPathName;
    if (!PyTuple_GetString(poArgs, 0, szPathName))
        return Py_BadArgument();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
    if (pInstance)
    {
        pInstance->GetGraphicThingInstanceRef().SetWeaponTraceTexture(szPathName.c_str());
    }
    Py_RETURN_NONE;
}

PyObject *chrWeaponTraceUseTexture(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
    if (pInstance)
    {
        pInstance->GetGraphicThingInstanceRef().UseTextureWeaponTrace();
    }
    Py_RETURN_NONE;
}

PyObject *chrWeaponTraceUseAlpha(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
    if (pInstance)
    {
        pInstance->GetGraphicThingInstanceRef().UseAlphaWeaponTrace();
    }
    Py_RETURN_NONE;
}

PyObject *chrMoveToDestPosition(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
        return Py_BadArgument();

    int ix;
    if (!PyTuple_GetInteger(poArgs, 1, &ix))
        return Py_BadArgument();
    int iy;
    if (!PyTuple_GetInteger(poArgs, 2, &iy))
        return Py_BadArgument();

    CInstanceBase *pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVID);
    if (!pCharacterInstance)
        Py_RETURN_NONE;

    pCharacterInstance->NEW_MoveToDestPixelPositionDirection(TPixelPosition(ix, iy, 0.0f));

    Py_RETURN_NONE;
}

PyObject *chrtestSetRideMan(PyObject *poSelf, PyObject *poArgs)
{
    int ix;
    if (!PyTuple_GetInteger(poArgs, 0, &ix))
        return Py_BadArgument();
    int iy;
    if (!PyTuple_GetInteger(poArgs, 1, &iy))
        return Py_BadArgument();
    int imount = 20030;
    PyTuple_GetInteger(poArgs, 2, &imount);

    CInstanceBase *pCharacterInstance = CPythonCharacterManager::Instance().RegisterInstance(1);
    CInstanceBase::SCreateData kCreateData{};
    kCreateData.m_bType = CActorInstance::TYPE_PC;
    kCreateData.m_dwRace = 0;
    kCreateData.m_parts[PART_HAIR] = {100, 0};
    kCreateData.m_dwMovSpd = 100;
    kCreateData.m_dwAtkSpd = 100;
    kCreateData.m_dwMountVnum = imount;
    kCreateData.m_lPosX = ix;
    kCreateData.m_lPosY = iy;
    pCharacterInstance->Create(kCreateData);

    Py_RETURN_NONE;
}

PyObject *chrIsStone(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (!pInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("i", pInstance->IsStone());
}

PyObject *chrIsAggresiveByVid(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();

    CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (!pInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("i", IS_SET(pInstance->GetAIFlag(), CInstanceBase::AIFLAG_AGGRESSIVE));
}

PyObject *chrHasAffectByVid(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BadArgument();

    int iAffect;
    if (!PyTuple_GetInteger(poArgs, 1, &iAffect))
        return Py_BadArgument();

    auto pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);
    if (!pInstance)
        return Py_BuildValue("i", 0);

    return Py_BuildValue("i", pInstance->HasAffect(iAffect));
}

extern "C" void initchr()
{
    static PyMethodDef s_methods[] =
    {
        {"DismountHorse", chrDismountHorse, METH_VARARGS},
        {"MountHorse", chrMountHorse, METH_VARARGS},

        {"Destroy", chrDestroy, METH_VARARGS},
        {"Update", chrUpdate, METH_VARARGS},
        {"Deform", chrDeform, METH_VARARGS},
        {"Render", chrRender, METH_VARARGS},
        {"RenderCollision", chrRenderCollision, METH_VARARGS},

        // Functions For Python Code
        {"CreateInstance", chrCreateInstance, METH_VARARGS},
        {"DeleteInstance", chrDeleteInstance, METH_VARARGS},
        {"DeleteInstanceByFade", chrDeleteInstanceByFade, METH_VARARGS},
        {"SelectInstance", chrSelectInstance, METH_VARARGS},

        {"HasInstance", chrHasInstance, METH_VARARGS},
        {"IsEnemy", chrIsEnemy, METH_VARARGS},
        {"IsNPC", chrIsNPC, METH_VARARGS},
        //{ "IsGameMaster",				chrIsGameMaster,					METH_VARARGS },
        {"IsPartyMember", chrIsPartyMember, METH_VARARGS},

        {"Select", chrSelect, METH_VARARGS},
        {"SetAddRenderMode", chrSetAddRenderMode, METH_VARARGS},
        {"SetBlendRenderMode", chrSetBlendRenderMode, METH_VARARGS},
        {"Unselect", chrUnselect, METH_VARARGS},

        {"Hide", chrHide, METH_VARARGS},
        {"Show", chrShow, METH_VARARGS},
        {"Pick", chrPick, METH_VARARGS},
        {"PickAll", chrPickAll, METH_VARARGS},

        {"SetArmor", chrSetArmor, METH_VARARGS},
        {"SetWeapon", chrSetWeapon, METH_VARARGS},
        {"ChangeShape", chrChangeShape, METH_VARARGS},
        {"SetRace", chrSetRace, METH_VARARGS},
        {"SetHair", chrSetHair, METH_VARARGS},
        {"SetAcce", chrSetAcce, METH_VARARGS},
        {"SetScale", chrSetScale, METH_VARARGS},

        {"ChangeHair", chrChangeHair, METH_VARARGS},
        {"SetVirtualID", chrSetVirtualID, METH_VARARGS},
        {"SetNameString", chrSetNameString, METH_VARARGS},
        {"SetInstanceType", chrSetInstanceType, METH_VARARGS},

        {"SetPixelPosition", chrSetPixelPosition, METH_VARARGS},
        {"SetDirection", chrSetDirection, METH_VARARGS},
        {"Refresh", chrRefresh, METH_VARARGS},
        {"Revive", chrRevive, METH_VARARGS},
        {"Die", chrDie, METH_VARARGS},

        {"AttachEffectByName", chrAttachEffectByName, METH_VARARGS},

        {"LookAt", chrLookAt, METH_VARARGS},
        {"SetMotionMode", chrSetMotionMode, METH_VARARGS},
        {"SetLoopMotion", chrSetLoopMotion, METH_VARARGS},
        {"BlendLoopMotion", chrBlendLoopMotion, METH_VARARGS},
        {"PushOnceMotion", chrPushOnceMotion, METH_VARARGS},
        {"PushLoopMotion", chrPushLoopMotion, METH_VARARGS},
        {"GetPixelPosition", chrGetPixelPosition, METH_VARARGS},
        {"SetRotation", chrSetRotation, METH_VARARGS},
        {"SetRotationAll", chrSetRotationAll, METH_VARARGS},
        {"BlendRotation", chrBlendRotation, METH_VARARGS},
        {"GetRotation", chrGetRotation, METH_VARARGS},
        {"GetRace", chrGetRace, METH_VARARGS},
        {"GetRaceByVID", chrGetRaceByVID, METH_VARARGS},

        {"GetName", chrGetName, METH_VARARGS},
        {"GetLevel", chrGetLevel, METH_VARARGS},
        {"GetNameByVID", chrGetNameByVID, METH_VARARGS},
        {"GetLevelByVID", chrGetLevelByVID, METH_VARARGS},

        {"GetGuildID", chrGetGuildID, METH_VARARGS},
        {"GetProjectPosition", chrGetProjectPosition, METH_VARARGS},

        {"GetVirtualNumber", chrGetVirtualNumber, METH_VARARGS},
        {"GetInstanceType", chrGetInstanceType, METH_VARARGS},

        {"GetBoundBoxOnlyXY", chrGetBoundBoxOnlyXY, METH_VARARGS},

        {"RaceToJob", chrRaceToJob, METH_VARARGS},
        {"RaceToSex", chrRaceToSex, METH_VARARGS},

        {"IsStone", chrIsStone, METH_VARARGS},
        {"IsAggressiveByVid", chrIsAggresiveByVid, METH_VARARGS},
        {"HasAffectByVid", chrHasAffectByVid, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("chr", s_methods);

    // Length
    PyModule_AddIntConstant(poModule, "PLAYER_NAME_MAX_LEN", PLAYER_NAME_MAX_LEN);

    // General
    PyModule_AddIntMacro(poModule, MOTION_NONE);
    PyModule_AddIntMacro(poModule, MOTION_SPAWN);
    PyModule_AddIntMacro(poModule, MOTION_WAIT);
    PyModule_AddIntMacro(poModule, MOTION_WALK);
    PyModule_AddIntMacro(poModule, MOTION_RUN);
    PyModule_AddIntMacro(poModule, MOTION_CHANGE_WEAPON);
    PyModule_AddIntMacro(poModule, MOTION_DAMAGE);
    PyModule_AddIntMacro(poModule, MOTION_DAMAGE_FLYING);
    PyModule_AddIntMacro(poModule, MOTION_STAND_UP);
    PyModule_AddIntMacro(poModule, MOTION_DAMAGE_BACK);
    PyModule_AddIntMacro(poModule, MOTION_DAMAGE_FLYING_BACK);
    PyModule_AddIntMacro(poModule, MOTION_STAND_UP_BACK);
    PyModule_AddIntMacro(poModule, MOTION_DEAD);
    PyModule_AddIntMacro(poModule, MOTION_DEAD_BACK);
    PyModule_AddIntMacro(poModule, MOTION_NORMAL_ATTACK);
    PyModule_AddIntMacro(poModule, MOTION_COMBO_ATTACK_1);
    PyModule_AddIntMacro(poModule, MOTION_COMBO_ATTACK_2);
    PyModule_AddIntMacro(poModule, MOTION_COMBO_ATTACK_3);
    PyModule_AddIntMacro(poModule, MOTION_COMBO_ATTACK_4);
    PyModule_AddIntMacro(poModule, MOTION_COMBO_ATTACK_5);
    PyModule_AddIntMacro(poModule, MOTION_COMBO_ATTACK_6);
    PyModule_AddIntMacro(poModule, MOTION_COMBO_ATTACK_7);
    PyModule_AddIntMacro(poModule, MOTION_COMBO_ATTACK_8);
    PyModule_AddIntMacro(poModule, MOTION_INTRO_WAIT);
    PyModule_AddIntMacro(poModule, MOTION_INTRO_SELECTED);
    PyModule_AddIntMacro(poModule, MOTION_INTRO_NOT_SELECTED);
    PyModule_AddIntMacro(poModule, MOTION_FISHING_THROW);
    PyModule_AddIntMacro(poModule, MOTION_FISHING_WAIT);
    PyModule_AddIntMacro(poModule, MOTION_FISHING_STOP);
    PyModule_AddIntMacro(poModule, MOTION_FISHING_REACT);
    PyModule_AddIntMacro(poModule, MOTION_FISHING_CATCH);
    PyModule_AddIntMacro(poModule, MOTION_FISHING_FAIL);
    PyModule_AddIntMacro(poModule, MOTION_STOP);
    PyModule_AddIntMacro(poModule, MOTION_SKILL);
    PyModule_AddIntMacro(poModule, MOTION_CLAP);
    PyModule_AddIntMacro(poModule, MOTION_DANCE_1);
    PyModule_AddIntMacro(poModule, MOTION_DANCE_2);
    PyModule_AddIntMacro(poModule, MOTION_DANCE_3);
    PyModule_AddIntMacro(poModule, MOTION_DANCE_4);
    PyModule_AddIntMacro(poModule, MOTION_DANCE_5);
    PyModule_AddIntMacro(poModule, MOTION_DANCE_6);
    PyModule_AddIntMacro(poModule, MOTION_DANCE_7);
    PyModule_AddIntMacro(poModule, MOTION_CONGRATULATION);
    PyModule_AddIntMacro(poModule, MOTION_FORGIVE);
    PyModule_AddIntMacro(poModule, MOTION_ANGRY);
    PyModule_AddIntMacro(poModule, MOTION_ATTRACTIVE);
    PyModule_AddIntMacro(poModule, MOTION_SAD);
    PyModule_AddIntMacro(poModule, MOTION_SHY);
    PyModule_AddIntMacro(poModule, MOTION_CHEERUP);
    PyModule_AddIntMacro(poModule, MOTION_BANTER);
    PyModule_AddIntMacro(poModule, MOTION_JOY);
    PyModule_AddIntMacro(poModule, MOTION_THROW_MONEY);

    PyModule_AddIntMacro(poModule, MOTION_EMOTION_PUSH_UP);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_DANCE_7);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_DOZE);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_SELFIE);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_NOSAY);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_WEATHER_1);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_WEATHER_2);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_WEATHER_3);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_HUNGRY);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_SIREN);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_LETTER);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_CALL);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_CELEBRATION);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_ALCOHOL);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_BUSY);
    PyModule_AddIntMacro(poModule, MOTION_EMOTION_EXERCISE);

    PyModule_AddIntMacro(poModule, MOTION_CHEERS_1);
    PyModule_AddIntMacro(poModule, MOTION_CHEERS_2);
    PyModule_AddIntMacro(poModule, MOTION_KISS_WITH_WARRIOR);
    PyModule_AddIntMacro(poModule, MOTION_KISS_WITH_ASSASSIN);
    PyModule_AddIntMacro(poModule, MOTION_KISS_WITH_SURA);
    PyModule_AddIntMacro(poModule, MOTION_KISS_WITH_SHAMAN);
    PyModule_AddIntMacro(poModule, MOTION_KISS_WITH_WOLFMAN);
    PyModule_AddIntMacro(poModule, MOTION_FRENCH_KISS_WITH_WARRIOR);
    PyModule_AddIntMacro(poModule, MOTION_FRENCH_KISS_WITH_ASSASSIN);
    PyModule_AddIntMacro(poModule, MOTION_FRENCH_KISS_WITH_SURA);
    PyModule_AddIntMacro(poModule, MOTION_FRENCH_KISS_WITH_SHAMAN);
    PyModule_AddIntMacro(poModule, MOTION_FRENCH_KISS_WITH_WOLFMAN);
    PyModule_AddIntMacro(poModule, MOTION_SLAP_HIT_WITH_WARRIOR);
    PyModule_AddIntMacro(poModule, MOTION_SLAP_HIT_WITH_ASSASSIN);
    PyModule_AddIntMacro(poModule, MOTION_SLAP_HIT_WITH_SURA);
    PyModule_AddIntMacro(poModule, MOTION_SLAP_HIT_WITH_SHAMAN);
    PyModule_AddIntMacro(poModule, MOTION_SLAP_HIT_WITH_WOLFMAN);
    PyModule_AddIntMacro(poModule, MOTION_SLAP_HURT_WITH_WARRIOR);
    PyModule_AddIntMacro(poModule, MOTION_SLAP_HURT_WITH_ASSASSIN);
    PyModule_AddIntMacro(poModule, MOTION_SLAP_HURT_WITH_SURA);
    PyModule_AddIntMacro(poModule, MOTION_SLAP_HURT_WITH_SHAMAN);
    PyModule_AddIntMacro(poModule, MOTION_SLAP_HURT_WITH_WOLFMAN);
    PyModule_AddIntMacro(poModule, MOTION_DIG);
    PyModule_AddIntMacro(poModule, MOTION_MODE_RESERVED);
    PyModule_AddIntMacro(poModule, MOTION_MODE_GENERAL);
    PyModule_AddIntMacro(poModule, MOTION_MODE_ONEHAND_SWORD);
    PyModule_AddIntMacro(poModule, MOTION_MODE_TWOHAND_SWORD);
    PyModule_AddIntMacro(poModule, MOTION_MODE_DUALHAND_SWORD);
    PyModule_AddIntMacro(poModule, MOTION_MODE_BOW);
    PyModule_AddIntMacro(poModule, MOTION_MODE_FAN);
    PyModule_AddIntMacro(poModule, MOTION_MODE_BELL);
    PyModule_AddIntMacro(poModule, MOTION_MODE_CLAW);
    PyModule_AddIntMacro(poModule, MOTION_MODE_FISHING);
    PyModule_AddIntMacro(poModule, MOTION_MODE_HORSE);
    PyModule_AddIntMacro(poModule, MOTION_MODE_HORSE_ONEHAND_SWORD);
    PyModule_AddIntMacro(poModule, MOTION_MODE_HORSE_TWOHAND_SWORD);
    PyModule_AddIntMacro(poModule, MOTION_MODE_HORSE_DUALHAND_SWORD);
    PyModule_AddIntMacro(poModule, MOTION_MODE_HORSE_BOW);
    PyModule_AddIntMacro(poModule, MOTION_MODE_HORSE_FAN);
    PyModule_AddIntMacro(poModule, MOTION_MODE_HORSE_BELL);
    PyModule_AddIntMacro(poModule, MOTION_MODE_HORSE_CLAW);
    PyModule_AddIntMacro(poModule, MOTION_MODE_WEDDING_DRESS);
    PyModule_AddIntMacro(poModule, MOTION_MODE_HORSE_CLAW);
    PyModule_AddIntMacro(poModule, MOTION_MODE_CLAW);

    PyModule_AddIntConstant(poModule, "DIR_NORTH", CInstanceBase::DIR_NORTH);
    PyModule_AddIntConstant(poModule, "DIR_NORTHEAST", CInstanceBase::DIR_NORTHEAST);
    PyModule_AddIntConstant(poModule, "DIR_EAST", CInstanceBase::DIR_EAST);
    PyModule_AddIntConstant(poModule, "DIR_SOUTHEAST", CInstanceBase::DIR_SOUTHEAST);
    PyModule_AddIntConstant(poModule, "DIR_SOUTH", CInstanceBase::DIR_SOUTH);
    PyModule_AddIntConstant(poModule, "DIR_SOUTHWEST", CInstanceBase::DIR_SOUTHWEST);
    PyModule_AddIntConstant(poModule, "DIR_WEST", CInstanceBase::DIR_WEST);
    PyModule_AddIntConstant(poModule, "DIR_NORTHWEST", CInstanceBase::DIR_NORTHWEST);

    PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_PLAYER", CActorInstance::TYPE_PC);
    PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_NPC", CActorInstance::TYPE_NPC);
    PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_ENEMY", CActorInstance::TYPE_ENEMY);
    PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_BUILDING", CActorInstance::TYPE_BUILDING);
    PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_OBJECT", CActorInstance::TYPE_OBJECT);
    PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_SHOP", CActorInstance::TYPE_SHOP);
    PyModule_AddIntConstant(poModule, "INSTANCE_TYPE_BUFFBOT", CActorInstance::TYPE_BUFFBOT);

    PyModule_AddIntConstant(poModule, "PART_WEAPON", PART_WEAPON);
    PyModule_AddIntConstant(poModule, "PART_HEAD", PART_HEAD);
    PyModule_AddIntConstant(poModule, "PART_WEAPON_LEFT", PART_WEAPON_LEFT);
    PyModule_AddIntConstant(poModule, "PART_ACCE", PART_ACCE);

    /////
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
    PyModule_AddIntMacro(poModule, AFFECT_QUEST_START_IDX);
}
