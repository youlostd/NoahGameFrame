#include "StdAfx.h"
#include "PythonPlayer.h"
#include "PythonApplication.h"
#include "PythonItemRenderTargetManager.h"
#include <pybind11/stl.h>

#include "../EterPythonLib/PythonSlotWindow.h"
extern const uint32_t c_iSkillIndex_Tongsol = 121;
extern const uint32_t c_iSkillIndex_Combo = 122;
extern const uint32_t c_iSkillIndex_Fishing = 123;
extern const uint32_t c_iSkillIndex_Mining = 124;
extern const uint32_t c_iSkillIndex_Making = 125;
extern const uint32_t c_iSkillIndex_Language1 = 126;
extern const uint32_t c_iSkillIndex_Language2 = 127;
extern const uint32_t c_iSkillIndex_Language3 = 128;
extern const uint32_t c_iSkillIndex_Polymorph = 129;
extern const uint32_t c_iSkillIndex_Riding = 130;
extern const uint32_t c_iSkillIndex_Summon = 131;

enum
{
    EMOTION_CLAP = 1,
    EMOTION_CONGRATULATION = 2,
    EMOTION_FORGIVE = 3,
    EMOTION_ANGRY = 4,
    EMOTION_ATTRACTIVE = 5,
    EMOTION_SAD = 6,
    EMOTION_SHY = 7,
    EMOTION_CHEERUP = 8,
    EMOTION_BANTER = 9,
    EMOTION_JOY = 10,
    EMOTION_CHEERS_1 = 11,
    EMOTION_CHEERS_2 = 12,
    EMOTION_DANCE_1 = 13,
    EMOTION_DANCE_2 = 14,
    EMOTION_DANCE_3 = 15,
    EMOTION_DANCE_4 = 16,
    EMOTION_DANCE_5 = 17,
    EMOTION_DANCE_6 = 18,
    EMOTION_DANCE_7 = 19,
    EMOTION_THROW_MONEY = 20,
    EMOTION_PUSH_UP,
    EMOTION_EXERCISE,
    EMOTION_DOZE,
    EMOTION_SELFIE,
    EMOTION_CHARGING,
    EMOTION_NOSAY,
    EMOTION_WEATHER_1,
    EMOTION_WEATHER_2,
    EMOTION_WEATHER_3,
    EMOTION_HUNGRY,
    EMOTION_SIREN,
    EMOTION_LETTER,
    EMOTION_CALL,
    EMOTION_CELEBRATION,
    EMOTION_ALCOHOL,
    EMOTION_BUSY,
    EMOTION_WHIRL,

    EMOTION_KISS = 51,
    EMOTION_FRENCH_KISS = 52,
    EMOTION_SLAP = 53
};

std::map<int, CGraphicImage::Ptr> m_kMap_iEmotionIndex_pkIconImage;

extern int TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE;

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
class CBeltInventoryHelper
{
public:
    typedef uint8_t TGradeUnit;

    static TGradeUnit GetBeltGradeByRefineLevel(int refineLevel)
    {
        static TGradeUnit beltGradeByLevelTable[] =
        {
            0, // 벨트+0
            1, // +1
            1, // +2
            2, // +3
            2, // +4,
            3, // +5
            4, // +6,
            5, // +7,
            6, // +8,
            7, // +9
        };

        return beltGradeByLevelTable[refineLevel];
    }

    // 현재 벨트 레벨을 기준으로, 어떤 셀들을 이용할 수 있는지 리턴
    static const TGradeUnit *GetAvailableRuleTableByGrade()
    {
        /**
            벨트는 총 +0 ~ +9 레벨을 가질 수 있으며, 레벨에 따라 7단계 등급으로 구분되어 인벤토리가 활성 화 된다.
            벨트 레벨에 따른 사용 가능한 셀은 아래 그림과 같음. 현재 등급 >= 활성가능 등급이면 사용 가능.
            (단, 현재 레벨이 0이면 무조건 사용 불가, 괄호 안의 숫자는 등급)

                2(1)  4(2)  6(4)  8(6)
                5(3)  5(3)  6(4)  8(6)
                7(5)  7(5)  7(5)  8(6)
                9(7)  9(7)  9(7)  9(7)

            벨트 인벤토리의 크기는 4x4 (16칸)
        */

        static TGradeUnit availableRuleByGrade[BELT_INVENTORY_SLOT_COUNT] =
        {
            1, 2, 4, 6,
            3, 3, 4, 6,
            5, 5, 5, 6,
            7, 7, 7, 7
        };

        return availableRuleByGrade;
    }

    static bool IsAvailableCell(uint16_t cell, int beltGrade /*int beltLevel*/)
    {
        // 기획 또 바뀜.. 아놔...
        //const TGradeUnit beltGrade = GetBeltGradeByRefineLevel(beltLevel);
        const TGradeUnit *ruleTable = GetAvailableRuleTableByGrade();

        return ruleTable[cell] <= beltGrade;
    }
};
#endif

PyObject *playerSetGameWindow(PyObject *poSelf, PyObject *poArgs)
{
    PyObject *pyHandle;
    if (!PyTuple_GetObject(poArgs, 0, &pyHandle))
    {
        return Py_BadArgument();
    }

    //CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
    //rkPlayer.SetGameWindow (pyHandle);
    Py_RETURN_NONE;
}

PyObject *playerSetQuickCameraMode(PyObject *poSelf, PyObject *poArgs)
{
    int nIsEnable;
    if (!PyTuple_GetInteger(poArgs, 0, &nIsEnable))
    {
        return Py_BadArgument();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.SetQuickCameraMode(nIsEnable ? true : false);

    Py_RETURN_NONE;
}

// Test Code
PyObject *playerSetMainCharacterIndex(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().SetMainCharacterIndex(iVID);
    CPythonCharacterManager::Instance().SetMainInstance(iVID);

    Py_RETURN_NONE;
}

// Test Code

PyObject *playerGetMainCharacterIndex(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().GetMainCharacterIndex());
}

PyObject *playerGetMainCharacterName(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("s", CPythonPlayer::Instance().GetName());
}

PyObject *playerGetMainCharacterPosition(PyObject *poSelf, PyObject *poArgs)
{
    TPixelPosition kPPosMainActor;
    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
    return Py_BuildValue("fff", kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
}

PyObject *playerCanAttackInstance(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    CInstanceBase *pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    CInstanceBase *pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVID);
    if (!pMainInstance)
    {
        return Py_BuildValue("i", 0);
    }
    if (!pTargetInstance)
    {
        return Py_BuildValue("i", 0);
    }

    return Py_BuildValue("i", pMainInstance->IsAttackableInstance(*pTargetInstance));
}

PyObject *playerIsActingEmotion(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    if (!pMainInstance)
    {
        return Py_BuildValue("i", 0);
    }

    return Py_BuildValue("i", pMainInstance->IsActingEmotion());
}

PyObject *playerIsPVPInstance(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    CInstanceBase *pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    CInstanceBase *pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVID);
    if (!pMainInstance)
    {
        return Py_BuildValue("i", 0);
    }
    if (!pTargetInstance)
    {
        return Py_BuildValue("i", 0);
    }

    return Py_BuildValue("i", pMainInstance->IsPVPInstance(*pTargetInstance));
}

PyObject *playerIsSameEmpire(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    CInstanceBase *pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    CInstanceBase *pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVID);
    if (!pMainInstance)
    {
        return Py_BuildValue("i", FALSE);
    }
    if (!pTargetInstance)
    {
        return Py_BuildValue("i", FALSE);
    }

    return Py_BuildValue("i", pMainInstance->IsSameEmpire(*pTargetInstance));
}

PyObject *playerIsChallengeInstance(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", CPythonPlayer::Instance().IsChallengeInstance(iVID));
}

PyObject *playerIsRevengeInstance(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", CPythonPlayer::Instance().IsRevengeInstance(iVID));
}

PyObject *playerIsCantFightInstance(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", CPythonPlayer::Instance().IsCantFightInstance(iVID));
}

PyObject *playerGetCharacterDistance(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    CInstanceBase *pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    CInstanceBase *pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVID);
    if (!pMainInstance)
    {
        return Py_BuildValue("f", -1.0f);
    }
    if (!pTargetInstance)
    {
        return Py_BuildValue("f", -1.0f);
    }

    return Py_BuildValue("f", pMainInstance->GetDistance(pTargetInstance));
}

PyObject *playerIsInSafeArea(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    if (!pMainInstance)
    {
        return Py_BuildValue("i", FALSE);
    }

    return Py_BuildValue("i", pMainInstance->IsInSafe());
}

PyObject *playerIsMountingHorse(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    if (!pMainInstance)
    {
        return Py_BuildValue("i", FALSE);
    }

    return Py_BuildValue("i", pMainInstance->IsMountingHorse());
}

PyObject *playerIsGameMaster(PyObject *poSelf, PyObject *poArgs)
{
    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    return Py_BuildValue("i", rkPlayer.IsGameMaster());
}

PyObject *playerIsObserverMode(PyObject *poSelf, PyObject *poArgs)
{
    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    return Py_BuildValue("i", rkPlayer.IsObserverMode());
}

PyObject *playerActEmotion(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.ActEmotion(iVID);
    Py_RETURN_NONE;
}

PyObject *playerSetParalysis(PyObject *poSelf, PyObject *poArgs)
{
    int val;
    if (!PyTuple_GetInteger(poArgs, 0, &val))
    {
        return Py_BadArgument();
    }

    CInstanceBase *pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    if (pMainInstance)
    {
        pMainInstance->SetParalysis(val);
    }
    Py_RETURN_NONE;
}

PyObject *playerShowPlayer(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    if (pMainInstance)
    {
        pMainInstance->GetGraphicThingInstanceRef().Show();
    }
    Py_RETURN_NONE;
}

PyObject *playerHidePlayer(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    if (pMainInstance)
    {
        pMainInstance->GetGraphicThingInstanceRef().Hide();
    }
    Py_RETURN_NONE;
}

PyObject *playerComboAttack(PyObject *poSelf, PyObject *poArgs)
{
    CPythonPlayer::Instance().NEW_Attack();
    Py_RETURN_NONE;
}

PyObject *playerRegisterEffect(PyObject *poSelf, PyObject *poArgs)
{
    int iEft;
    if (!PyTuple_GetInteger(poArgs, 0, &iEft))
    {
        return Py_BadArgument();
    }

    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
    {
        return Py_BadArgument();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    if (!rkPlayer.RegisterEffect(iEft, szFileName.c_str(), false))
    {
        return Py_BuildException("CPythonPlayer::RegisterEffect(eEft=%d, szFileName=%s", iEft, szFileName);
    }

    Py_RETURN_NONE;
}

PyObject *playerRegisterCacheEffect(PyObject *poSelf, PyObject *poArgs)
{
    int iEft;
    if (!PyTuple_GetInteger(poArgs, 0, &iEft))
    {
        return Py_BadArgument();
    }

    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
    {
        return Py_BadArgument();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    if (!rkPlayer.RegisterEffect(iEft, szFileName.c_str(), true))
    {
        return Py_BuildException("CPythonPlayer::RegisterEffect(eEft=%d, szFileName=%s", iEft, szFileName);
    }

    Py_RETURN_NONE;
}

PyObject *playerSetAttackKeyState(PyObject *poSelf, PyObject *poArgs)
{
    int isPressed;
    if (!PyTuple_GetInteger(poArgs, 0, &isPressed))
    {
        return Py_BuildException("playerSetAttackKeyState(isPressed) - There is no first argument");
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.SetAttackKeyState(isPressed ? true : false);
    Py_RETURN_NONE;
}

PyObject *playerSetSingleDIKKeyState(PyObject *poSelf, PyObject *poArgs)
{
    int eDIK;
    if (!PyTuple_GetInteger(poArgs, 0, &eDIK))
    {
        return Py_BuildException("playerSetSingleDIKKeyState(eDIK, isPressed) - There is no first argument");
    }

    int isPressed;
    if (!PyTuple_GetInteger(poArgs, 1, &isPressed))
    {
        return Py_BuildException("playerSetSingleDIKKeyState(eDIK, isPressed) - There is no second argument");
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.NEW_SetSingleDIKKeyState(eDIK, isPressed ? true : false);
    Py_RETURN_NONE;
}

PyObject *playerEndKeyWalkingImmediately(PyObject *poSelf, PyObject *poArgs)
{
    CPythonPlayer::Instance().NEW_Stop();
    Py_RETURN_NONE;
}

PyObject *playerStartMouseWalking(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

PyObject *playerEndMouseWalking(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

PyObject *playerResetCameraRotation(PyObject *poSelf, PyObject *poArgs)
{
    CPythonPlayer::Instance().NEW_ResetCameraRotation();
    Py_RETURN_NONE;
}

PyObject *playerSetAutoCameraRotationSpeed(PyObject *poSelf, PyObject *poArgs)
{
    float fCmrRotSpd;
    if (!PyTuple_GetFloat(poArgs, 0, &fCmrRotSpd))
    {
        return Py_BuildException();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.NEW_SetAutoCameraRotationSpeed(fCmrRotSpd);
    Py_RETURN_NONE;
}

PyObject *playerSetMouseState(PyObject *poSelf, PyObject *poArgs)
{
    int eMBT;
    if (!PyTuple_GetInteger(poArgs, 0, &eMBT))
    {
        return Py_BuildException();
    }

    int eMBS;
    if (!PyTuple_GetInteger(poArgs, 1, &eMBS))
    {
        return Py_BuildException();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.NEW_SetMouseState(eMBT, eMBS);

    Py_RETURN_NONE;
}

PyObject *playerSetMouseFunc(PyObject *poSelf, PyObject *poArgs)
{
    int eMBT;
    if (!PyTuple_GetInteger(poArgs, 0, &eMBT))
    {
        return Py_BuildException();
    }

    int eMBS;
    if (!PyTuple_GetInteger(poArgs, 1, &eMBS))
    {
        return Py_BuildException();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.NEW_SetMouseFunc(eMBT, eMBS);

    Py_RETURN_NONE;
}

PyObject *playerGetMouseFunc(PyObject *poSelf, PyObject *poArgs)
{
    int eMBT;
    if (!PyTuple_GetInteger(poArgs, 0, &eMBT))
    {
        return Py_BuildException();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    return Py_BuildValue("i", rkPlayer.NEW_GetMouseFunc(eMBT));
}

PyObject *playerSetMouseMiddleButtonState(PyObject *poSelf, PyObject *poArgs)
{
    int eMBS;
    if (!PyTuple_GetInteger(poArgs, 0, &eMBS))
    {
        return Py_BuildException();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.NEW_SetMouseMiddleButtonState(eMBS);

    Py_RETURN_NONE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject *playerGetName(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("s", CPythonPlayer::Instance().GetName());
}

PyObject *playerGetRace(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().GetRace());
}

PyObject *playerGetJob(PyObject *poSelf, PyObject *poArgs)
{
    int race = CPythonPlayer::Instance().GetRace();
    return Py_BuildValue("i", GetJobByRace(race));
}

PyObject *playerGetPlayTime(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().GetPlayTime());
}

PyObject *playerSetPlayTime(PyObject *poSelf, PyObject *poArgs)
{
    int iTime;
    if (!PyTuple_GetInteger(poArgs, 0, &iTime))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().SetPlayTime(iTime);
    Py_RETURN_NONE;
}

PyObject *playerIsSkillCoolTime(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", CPythonPlayer::Instance().IsSkillCoolTime(iSlotIndex));
}

PyObject *playerGetSkillCoolTime(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    const auto fCoolTime = CPythonPlayer::Instance().GetSkillCoolTime(iSlotIndex);
    const auto fElapsedCoolTime = CPythonPlayer::Instance().GetSkillElapsedCoolTime(iSlotIndex);
    return Py_BuildValue("dd", fCoolTime, fElapsedCoolTime);
}

PyObject *playerIsSkillActive(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", CPythonPlayer::Instance().IsSkillActive(iSlotIndex));
}

PyObject *playerUseGuildSkill(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillSlotIndex))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().UseGuildSkill(iSkillSlotIndex);
    Py_RETURN_NONE;
}

PyObject *playerGetEXP(PyObject *poSelf, PyObject *poArgs)
{
    PointValue dwEXP = CPythonPlayer::Instance().GetStatus(POINT_EXP);
    return Py_BuildValue("L", dwEXP);
}

PyObject *playerSetStatus(PyObject *poSelf, PyObject *poArgs)
{
    int iType;
    if (!PyTuple_GetInteger(poArgs, 0, &iType))
    {
        return Py_BuildException();
    }

    int iValue;
    if (!PyTuple_GetInteger(poArgs, 1, &iValue))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().SetStatus(iType, iValue);
    Py_RETURN_NONE;
}

PyObject *playerGetElk(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("K", CPythonPlayer::Instance().GetGold());
}

#ifdef ENABLE_GEM_SYSTEM
PyObject* playerGetGem(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetGem());
}
#endif

PyObject *playerGetGuildID(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    if (!pInstance)
    {
        return Py_BuildValue("i", 0);
    }
    return Py_BuildValue("i", pInstance->GetGuildID());
}

PyObject *playerGetGuildName(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    if (pInstance)
    {
        uint32_t dwID = pInstance->GetGuildID();
        return Py_BuildValue("s", CPythonGuild::Instance().GetGuildName(dwID).value_or("").c_str());

    }

  return Py_BuildValue("s", "");
  
}

PyObject *playerGetAlignmentData(PyObject *poSelf, PyObject *poArgs)
{
    CInstanceBase *pInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
    long iAlignmentPoint = 0;
    int iAlignmentGrade = 19;
    if (pInstance)
    {
        iAlignmentPoint = pInstance->GetAlignment();
        iAlignmentGrade = pInstance->GetAlignmentGrade();
    }
    return Py_BuildValue("ii", iAlignmentPoint, iAlignmentGrade);
}

PyObject *playerSetSkill(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSkillIndex))
    {
        return Py_BuildException();
    }

    //CPythonPlayer::Instance().SetSkill(iSlotIndex, iSkillIndex);
    Py_RETURN_NONE;
}

PyObject * playerGetSkillDisplayLevel(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	const auto level = CPythonPlayer::Instance().GetSkillLevel(iSlotIndex);
	const auto grade = CPythonPlayer::Instance().GetSkillGrade(iSlotIndex);

	return Py_BuildValue("i", GetRelativeSkillLevel(grade, level));
}

PyObject *playerSetSkillInfo(PyObject *poSelf, PyObject *poArgs)
{
    int index;
    if (!PyTuple_GetInteger(poArgs, 0, &index))
    {
        return Py_BuildException();
    }

    int grade;
    if (!PyTuple_GetInteger(poArgs, 1, &grade))
    {
        return Py_BuildException();
    }

    int level;
    if (!PyTuple_GetInteger(poArgs, 2, &level))
    {
        return Py_BuildException();
    }

    int color;
    if (!PyTuple_GetInteger(poArgs, 3, &color))
    {
        return Py_BuildException();
    }


    CPythonPlayer::Instance().SetSkillInfo(index, grade, level, color);
    Py_RETURN_NONE;
}

PyObject *playerGetSkillIndex(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", CPythonPlayer::Instance().GetSkillIndex(iSlotIndex));
}

PyObject *playerGetSkillSlotIndex(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    {
        return Py_BuildException();
    }

    uint32_t dwSlotIndex;
    if (!CPythonPlayer::Instance().GetSkillSlotIndex(iSkillIndex, &dwSlotIndex))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", dwSlotIndex);
}

PyObject *playerGetSkillGrade(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", CPythonPlayer::Instance().GetSkillGrade(iSlotIndex));
}

PyObject *playerGetSkillLevel(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", CPythonPlayer::Instance().GetSkillLevel(iSlotIndex));
}

PyObject *playerGetSkillColor(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        return Py_BuildException();

    return Py_BuildValue("I", CPythonPlayer::Instance().GetSkillColor(iSlotIndex));
}

PyObject *playerGetSkillCurrentEfficientPercentage(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("f", CPythonPlayer::Instance().GetSkillCurrentEfficientPercentage(iSlotIndex));
}

PyObject *playerGetSkillNextEfficientPercentage(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("f", CPythonPlayer::Instance().GetSkillNextEfficientPercentage(iSlotIndex));
}

PyObject *playerClickSkillSlot(PyObject *poSelf, PyObject *poArgs)
{
    int iSkillSlot;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillSlot))
    {
        return Py_BadArgument();
    }

    CPythonPlayer::Instance().ClickSkillSlot(iSkillSlot);

    Py_RETURN_NONE;
}

PyObject *playerChangeCurrentSkillNumberOnly(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BadArgument();
    }

    CPythonPlayer::Instance().ChangeCurrentSkillNumberOnly(iSlotIndex);

    Py_RETURN_NONE;
}

PyObject *playerMoveItem(PyObject *poSelf, PyObject *poArgs)
{
    TItemPos srcCell;
    TItemPos dstCell;
    switch (PyTuple_Size(poArgs))
    {
    case 2:
        //int iSourceSlotIndex = 0;
        if (!PyTuple_GetInteger(poArgs, 0, &srcCell.cell))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &dstCell.cell))
        {
            return Py_BuildException();
        }
        break;
    case 4:
        if (!PyTuple_GetByte(poArgs, 0, &srcCell.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &srcCell.cell))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetByte(poArgs, 2, &dstCell.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 3, &dstCell.cell))
        {
            return Py_BuildException();
        }
    default:
        return Py_BuildException();
    }

    CPythonPlayer::Instance().MoveItemData(srcCell, dstCell);
    Py_RETURN_NONE;
}

PyObject *playerSendClickItemPacket(PyObject *poSelf, PyObject *poArgs)
{
    int ivid;
    if (!PyTuple_GetInteger(poArgs, 0, &ivid))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().SendClickItemPacket(ivid);
    Py_RETURN_NONE;
}

PyObject *playerIsFlagBySlot(PyObject *poSelf, PyObject *poArgs)
{
    switch (PyTuple_Size(poArgs))
    {
    case 2: {
        int iSlotIndex;
        if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        {
            return Py_BuildException();
        }
        int iFlagIndex;
        if (!PyTuple_GetInteger(poArgs, 1, &iFlagIndex))
        {
            return Py_BuildException();
        }
        auto dwFlags = CPythonPlayer::Instance().GetItemFlags(TItemPos(INVENTORY, iSlotIndex));
        return Py_BuildValue("i", iFlagIndex & dwFlags);
    }
    case 3: {
        TItemPos Cell;
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
        {
            return Py_BuildException();
        }
        int iFlagIndex;
        if (!PyTuple_GetInteger(poArgs, 2, &iFlagIndex))
        {
            return Py_BuildException();
        }
        auto dwFlags = CPythonPlayer::Instance().GetItemFlags(Cell);
        return Py_BuildValue("i", iFlagIndex & dwFlags);
    }
    default:
        return Py_BuildException();
    }
}

PyObject *playerIsAntiFlagBySlot(PyObject *poSelf, PyObject *poArgs)
{
    switch (PyTuple_Size(poArgs))
    {
    case 2: {
        int iSlotIndex;
        if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        {
            return Py_BuildException();
        }
        int iFlagIndex;
        if (!PyTuple_GetInteger(poArgs, 1, &iFlagIndex))
        {
            return Py_BuildException();
        }
        auto dwAntiFlags = CPythonPlayer::Instance().GetItemAntiFlags(TItemPos(INVENTORY, iSlotIndex));
        return Py_BuildValue("i", iFlagIndex & dwAntiFlags);
    }
    case 3: {
        TItemPos Cell;
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
        {
            return Py_BuildException();
        }
        int iFlagIndex;
        if (!PyTuple_GetInteger(poArgs, 2, &iFlagIndex))
        {
            return Py_BuildException();
        }
        auto dwAntiFlags = CPythonPlayer::Instance().GetItemAntiFlags(Cell);
        return Py_BuildValue("i", iFlagIndex & dwAntiFlags);
    }
    default:
        return Py_BuildException();
    }
}

PyObject *playerIsSealedItemBySlot(PyObject *poSelf, PyObject *poArgs)
{
    switch (PyTuple_Size(poArgs))
    {
    case 2: {
        int iSlotIndex;
        if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        {
            return Py_BuildException();
        }
        int iFlagIndex;
        if (!PyTuple_GetInteger(poArgs, 1, &iFlagIndex))
        {
            return Py_BuildException();
        }
        auto pItemData = CPythonPlayer::Instance().GetItemData(TItemPos(INVENTORY, iSlotIndex));
        if (!pItemData)
            return Py_BuildException("Could not access item data");
        return Py_BuildValue("i", pItemData->nSealDate != -1);
    }
    case 3: {
        TItemPos Cell;
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
        {
            return Py_BuildException();
        }
        int iFlagIndex;
        if (!PyTuple_GetInteger(poArgs, 2, &iFlagIndex))
        {
            return Py_BuildException();
        }
        auto pItemData = CPythonPlayer::Instance().GetItemData(Cell);
        if (!pItemData)
            return Py_BuildException("Could not access item data");

        return Py_BuildValue("i", pItemData->nSealDate != -1);
    }
    default:
        return Py_BuildException();
    }
}

PyObject *playerGetItemTypeBySlot(PyObject *poSelf, PyObject *poArgs)
{
    switch (PyTuple_Size(poArgs))
    {
    case 1: {
        int iSlotIndex;
        if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        {
            return Py_BuildException();
        }

        return Py_BuildValue("i", CPythonPlayer::Instance().GetItemTypeBySlot(TItemPos(INVENTORY, iSlotIndex)));
    }
    case 2: {
        TItemPos Cell;
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
        {
            return Py_BuildException();
        }
        return Py_BuildValue("i", CPythonPlayer::Instance().GetItemTypeBySlot(Cell));
    }
    default:
        return Py_BuildException();
    }
}

PyObject *playerGetItemSubTypeBySlot(PyObject *poSelf, PyObject *poArgs)
{
    switch (PyTuple_Size(poArgs))
    {
    case 1: {
        int iSlotIndex;
        if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        {
            return Py_BuildException();
        }

        return Py_BuildValue("i", CPythonPlayer::Instance().GetItemSubTypeBySlot(TItemPos(INVENTORY, iSlotIndex)));
    }
    case 2: {
        TItemPos Cell;
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
        {
            return Py_BuildException();
        }

        return Py_BuildValue("i", CPythonPlayer::Instance().GetItemSubTypeBySlot(Cell));
    }
    default:
        return Py_BuildException();
    }
}

PyObject *playerGetItemIndex(PyObject *poSelf, PyObject *poArgs)
{
    switch (PyTuple_Size(poArgs))
    {
    case 1: {
        int iSlotIndex;
        if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
            return Py_BadArgument();

        int ItemIndex = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, iSlotIndex));
        return Py_BuildValue("i", ItemIndex);
    }
    case 2: {
        TItemPos Cell;
        if (!PyTuple_GetInteger(poArgs, 0, &Cell.window_type))
            return Py_BadArgument();
        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
            return Py_BadArgument();

        int ItemIndex = CPythonPlayer::Instance().GetItemIndex(Cell);
        return Py_BuildValue("i", ItemIndex);
    }
    default:
        return Py_BadArgument();
    }
}

PyObject *playerGetChangeLookVnum(PyObject *poSelf, PyObject *poArgs)
{
    switch (PyTuple_Size(poArgs))
    {
    case 1: {
        int iSlotIndex;
        if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        {
            return Py_BuildException();
        }

        auto data = CPythonPlayer::Instance().GetItemData(TItemPos(INVENTORY, iSlotIndex));
        return Py_BuildValue("i", data->transVnum);
    }
    case 2: {
        TItemPos Cell;
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
        {
            return Py_BuildException();
        }

        const ClientItemData *data = nullptr;

        switch (Cell.window_type)
        {
        case SAFEBOX:
            data = CPythonSafeBox::instance().GetItemData(Cell.cell);
            break;
        case MALL:
            data = CPythonSafeBox::instance().GetMallItemData(Cell.cell);
            break;
        case NPC_SHOP:
            data = CPythonShop::instance().GetItemData(Cell.cell);
            break;
        default:
            data = CPythonPlayer::Instance().GetItemData(Cell);
            break;
        }

        if (!data)
            return Py_BuildValue("i", 0);

        return Py_BuildValue("i", data->transVnum);
    }
    default:
        return Py_BuildException();
    }
}

PyObject *playerGetItemFlags(PyObject *poSelf, PyObject *poArgs)
{
    switch (PyTuple_Size(poArgs))
    {
    case 1: {
        int iSlotIndex;
        if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        {
            return Py_BuildException();
        }

        uint32_t flags = CPythonPlayer::Instance().GetItemFlags(TItemPos(INVENTORY, iSlotIndex));
        return Py_BuildValue("i", flags);
    }
    case 2: {
        TItemPos Cell;
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
        {
            return Py_BuildException();
        }

        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
        {
            return Py_BuildException();
        }

        uint32_t flags = CPythonPlayer::Instance().GetItemFlags(Cell);
        return Py_BuildValue("i", flags);
    }
    default:
        return Py_BuildException();
    }
}

PyObject *playerGetItemCount(PyObject *poSelf, PyObject *poArgs)
{
    switch (PyTuple_Size(poArgs))
    {
    case 1: {
        int iSlotIndex;
        if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        {
            return Py_BuildException();
        }

        int ItemNum = CPythonPlayer::Instance().GetItemCount(TItemPos(INVENTORY, iSlotIndex));
        return Py_BuildValue("i", ItemNum);
    }
    case 2: {
        TItemPos Cell;
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
        {
            return Py_BuildException();
        }

        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
        {
            return Py_BuildException();
        }

        int ItemNum = CPythonPlayer::Instance().GetItemCount(Cell);

        return Py_BuildValue("i", ItemNum);
    }
    default:
        return Py_BuildException();
    }
}

PyObject *playerGetItemCountByVnum(PyObject *poSelf, PyObject *poArgs)
{
    int ivnum;
    if (!PyTuple_GetInteger(poArgs, 0, &ivnum))
    {
        return Py_BuildException();
    }

    auto ItemNum = CPythonPlayer::Instance().GetItemCountByVnum(ivnum);
    return Py_BuildValue("L", ItemNum);
}

PyObject *playerGetItemMetinSocket(PyObject *poSelf, PyObject *poArgs)
{
    TItemPos Cell;
    int iMetinSocketIndex;

    switch (PyTuple_Size(poArgs))
    {
    case 2:
        if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
        {
            return Py_BuildException();
        }

        if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketIndex))
        {
            return Py_BuildException();
        }

        break;
    case 3:
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 2, &iMetinSocketIndex))
        {
            return Py_BuildException();
        }

        break;

    default:
        return Py_BuildException();
    }
    SocketValue nMetinSocketValue = CPythonPlayer::Instance().GetItemMetinSocket(Cell, iMetinSocketIndex);
    return Py_BuildValue("L", nMetinSocketValue);
}

PyObject *playerGetItemAttribute(PyObject *poSelf, PyObject *poArgs)
{
    TItemPos Cell;
    //int iSlotPos = 0;
    int iAttributeSlotIndex;
    switch (PyTuple_Size(poArgs))
    {
    case 2:
        if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
        {
            return Py_BuildException();
        }

        if (!PyTuple_GetInteger(poArgs, 1, &iAttributeSlotIndex))
        {
            return Py_BuildException();
        }

        break;
    case 3:
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
        {
            return Py_BuildException();
        }

        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
        {
            return Py_BuildException();
        }

        if (!PyTuple_GetInteger(poArgs, 2, &iAttributeSlotIndex))
        {
            return Py_BuildException();
        }
        break;
    default:
        return Py_BuildException();
    }
    ApplyType byType;
    ApplyValue sValue;
    CPythonPlayer::Instance().GetItemAttribute(Cell, iAttributeSlotIndex, &byType, &sValue);

    return Py_BuildValue("id", byType, sValue);
}

PyObject *playerGetISellItemPrice(PyObject *poSelf, PyObject *poArgs)
{
    TItemPos Cell;

    switch (PyTuple_Size(poArgs))
    {
    case 1:
        if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
        {
            return Py_BuildException();
        }
        break;
    case 2:
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
        {
            return Py_BuildException();
        }
        break;
    default:
        return Py_BuildException();
    }
    CItemData *pItemData;

    if (!CItemManager::Instance().GetItemDataPointer(CPythonPlayer::Instance().GetItemIndex(Cell), &pItemData))
    {
        return Py_BuildValue("i", 0);
    }

    int iPrice;

    if (pItemData->IsFlag(ITEM_FLAG_COUNT_PER_1GOLD))
    {
        iPrice = CPythonPlayer::Instance().GetItemCount(Cell) / pItemData->GetISellItemPrice();
    }
    else
    {
        iPrice = pItemData->GetISellItemPrice() * CPythonPlayer::Instance().GetItemCount(Cell);
    }
    iPrice /= 5;

    return Py_BuildValue("i", iPrice);
}

PyObject *playerGetQuickPage(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().GetQuickPage());
}

PyObject *playerSetQuickPage(PyObject *poSelf, PyObject *poArgs)
{
    int iPageIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iPageIndex))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().SetQuickPage(iPageIndex);
    Py_RETURN_NONE;
}

PyObject *playerLocalQuickSlotIndexToGlobalQuickSlotIndex(PyObject *poSelf, PyObject *poArgs)
{
    int iLocalSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iLocalSlotIndex))
    {
        return Py_BuildException();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    return Py_BuildValue("i", rkPlayer.LocalQuickSlotIndexToGlobalQuickSlotIndex(iLocalSlotIndex));
}

PyObject *playerGetLocalQuickSlot(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    uint32_t dwWndType;
    uint32_t dwWndItemPos;

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.GetLocalQuickSlotData(iSlotIndex, &dwWndType, &dwWndItemPos);

    return Py_BuildValue("ii", dwWndType, dwWndItemPos);
}

PyObject *playerGetGlobalQuickSlot(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    uint32_t dwWndType;
    uint32_t dwWndItemPos;

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.GetGlobalQuickSlotData(iSlotIndex, &dwWndType, &dwWndItemPos);

    return Py_BuildValue("ii", dwWndType, dwWndItemPos);
}

PyObject *playerRequestAddLocalQuickSlot(PyObject *poSelf, PyObject *poArgs)
{
    int nSlotIndex;
    int nWndType;
    int nWndItemPos;

    if (!PyTuple_GetInteger(poArgs, 0, &nSlotIndex))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetInteger(poArgs, 1, &nWndType))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetInteger(poArgs, 2, &nWndItemPos))
    {
        return Py_BuildException();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.RequestAddLocalQuickSlot(nSlotIndex, nWndType, nWndItemPos);

    Py_RETURN_NONE;
}

PyObject *playerRequestAddToEmptyLocalQuickSlot(PyObject *poSelf, PyObject *poArgs)
{
    int nWndType;
    if (!PyTuple_GetInteger(poArgs, 0, &nWndType))
    {
        return Py_BuildException();
    }

    int nWndItemPos;
    if (!PyTuple_GetInteger(poArgs, 1, &nWndItemPos))
    {
        return Py_BuildException();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.RequestAddToEmptyLocalQuickSlot(nWndType, nWndItemPos);

    Py_RETURN_NONE;
}

PyObject *playerRequestDeleteGlobalQuickSlot(PyObject *poSelf, PyObject *poArgs)
{
    int nGlobalSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &nGlobalSlotIndex))
    {
        return Py_BuildException();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.RequestDeleteGlobalQuickSlot(nGlobalSlotIndex);
    Py_RETURN_NONE;
}

PyObject *playerRequestMoveGlobalQuickSlotToLocalQuickSlot(PyObject *poSelf, PyObject *poArgs)
{
    int nGlobalSrcSlotIndex;
    int nLocalDstSlotIndex;

    if (!PyTuple_GetInteger(poArgs, 0, &nGlobalSrcSlotIndex))
    {
        return Py_BuildException();
    }

    if (!PyTuple_GetInteger(poArgs, 1, &nLocalDstSlotIndex))
    {
        return Py_BuildException();
    }

    CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
    rkPlayer.RequestMoveGlobalQuickSlotToLocalQuickSlot(nGlobalSrcSlotIndex, nLocalDstSlotIndex);
    Py_RETURN_NONE;
}

PyObject *playerRequestUseLocalQuickSlot(PyObject *poSelf, PyObject *poArgs)
{
    int iLocalPosition;
    if (!PyTuple_GetInteger(poArgs, 0, &iLocalPosition))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().RequestUseLocalQuickSlot(iLocalPosition);

    Py_RETURN_NONE;
}

PyObject *playerRemoveQuickSlotByValue(PyObject *poSelf, PyObject *poArgs)
{
    int iType;
    if (!PyTuple_GetInteger(poArgs, 0, &iType))
    {
        return Py_BuildException();
    }

    int iPosition;
    if (!PyTuple_GetInteger(poArgs, 1, &iPosition))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().RemoveQuickSlotByValue(iType, iPosition);

    Py_RETURN_NONE;
}

PyObject *playerisItem(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    char Flag = CPythonPlayer::Instance().IsItem(TItemPos(INVENTORY, iSlotIndex));

    return Py_BuildValue("i", Flag);
}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
PyObject *playerIsBeltInventorySlot(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    char Flag = CPythonPlayer::Instance().IsBeltInventorySlot(TItemPos(INVENTORY, iSlotIndex));

    return Py_BuildValue("i", Flag);
}
#endif

PyObject *playerIsEquipmentSlot(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        return Py_BadArgument();

    if (iSlotIndex >= 0 &&
        iSlotIndex <= WEAR_MAX_NUM)
        return Py_BuildValue("i", 1);

    return Py_BuildValue("i", 0);
}

PyObject *playerIsDSEquipmentSlot(PyObject *poSelf, PyObject *poArgs)
{
    uint8_t bWindowType;
    if (!PyTuple_GetInteger(poArgs, 0, &bWindowType))
    {
        return Py_BuildException();
    }
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
    {
        return Py_BuildException();
    }

    if (EQUIPMENT == bWindowType)
    {
        if (iSlotIndex >= DRAGON_SOUL_EQUIP_SLOT_START && iSlotIndex <= DRAGON_SOUL_EQUIP_RESERVED_SLOT_END)
        {
            return Py_BuildValue("i", 1);
        }
    }

    return Py_BuildValue("i", 0);
}

PyObject *playerIsCostumeSlot(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }

    static const std::array<uint16_t, 9> costumeWears = {
        WEAR_COSTUME_BODY,   // 19
        WEAR_COSTUME_HAIR,   // 20
        WEAR_COSTUME_MOUNT,  // 21
        WEAR_COSTUME_ACCE,   // 22
        WEAR_COSTUME_WEAPON, // 23
        WEAR_COSTUME_BODY_EFFECT,
        WEAR_COSTUME_WEAPON_EFFECT,
        WEAR_COSTUME_WING_EFFECT,
        WEAR_COSTUME_RANK,
    };

    return Py_BuildValue("i", std::find(costumeWears.begin(), costumeWears.end(), iSlotIndex) != costumeWears.end());
}

PyObject *playerIsOpenPrivateShop(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().IsOpenPrivateShop());
}

PyObject *playerIsValuableItem(PyObject *poSelf, PyObject *poArgs)
{
    TItemPos SlotIndex;

    switch (PyTuple_Size(poArgs))
    {
    case 1:
        if (!PyTuple_GetInteger(poArgs, 0, &SlotIndex.cell))
        {
            return Py_BuildException();
        }
        break;
    case 2:
        if (!PyTuple_GetInteger(poArgs, 0, &SlotIndex.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 0, &SlotIndex.cell))
        {
            return Py_BuildException();
        }
        break;
    default:
        return Py_BuildException();
    }

    uint32_t dwItemIndex = CPythonPlayer::Instance().GetItemIndex(SlotIndex);
    CItemManager::Instance().SelectItemData(dwItemIndex);
    CItemData *pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
    {
        return Py_BuildException("Can't find item data");
    }

    bool hasMetinSocket = FALSE;
    bool isHighPrice = FALSE;

    for (int i = 0; i < METIN_SOCKET_COUNT; ++i)
        if (CPythonPlayer::METIN_SOCKET_TYPE_NONE != CPythonPlayer::Instance().GetItemMetinSocket(SlotIndex, i))
        {
            hasMetinSocket = TRUE;
        }

    uint32_t dwValue = pItemData->GetISellItemPrice();
    if (dwValue > 5000)
    {
        isHighPrice = TRUE;
    }

    return Py_BuildValue("i", hasMetinSocket || isHighPrice);
}

int GetItemGrade(const char *c_szItemName)
{
    std::string strName = c_szItemName;
    if (strName.empty())
    {
        return 0;
    }

    char chGrade = strName[strName.length() - 1];
    if (chGrade < '0' || chGrade > '9')
    {
        chGrade = '0';
    }

    int iGrade = chGrade - '0';
    return iGrade;
}

PyObject *playerGetItemGrade(PyObject *poSelf, PyObject *poArgs)
{
    TItemPos SlotIndex;
    switch (PyTuple_Size(poArgs))
    {
    case 1:
        if (!PyTuple_GetInteger(poArgs, 0, &SlotIndex.cell))
        {
            return Py_BuildException();
        }
        break;
    case 2:
        if (!PyTuple_GetInteger(poArgs, 0, &SlotIndex.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &SlotIndex.cell))
        {
            return Py_BuildException();
        }
        break;
    default:
        return Py_BuildException();
    }

    int iItemIndex = CPythonPlayer::Instance().GetItemIndex(SlotIndex);
    CItemManager::Instance().SelectItemData(iItemIndex);
    CItemData *pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pItemData)
    {
        return Py_BuildException("Can't find item data");
    }

    return Py_BuildValue("i", GetItemGrade(pItemData->GetName()));
}

#if defined(GAIDEN)
PyObject* playerGetItemUnbindTime(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", (int)CPythonPlayer::instance().GetItemUnbindTime(iSlotIndex));
}
#endif

PyObject *playerIsRefineGradeScroll(PyObject *poSelf, PyObject *poArgs)
{
    TItemPos ScrollSlotIndex;
    switch (PyTuple_Size(poArgs))
    {
    case 1:
        if (!PyTuple_GetInteger(poArgs, 0, &ScrollSlotIndex.cell))
        {
            return Py_BuildException();
        }
    case 2:
        if (!PyTuple_GetInteger(poArgs, 0, &ScrollSlotIndex.window_type))
        {
            return Py_BuildException();
        }
        if (!PyTuple_GetInteger(poArgs, 1, &ScrollSlotIndex.cell))
        {
            return Py_BuildException();
        }
    }

    int iScrollItemIndex = CPythonPlayer::Instance().GetItemIndex(ScrollSlotIndex);
    CItemManager::Instance().SelectItemData(iScrollItemIndex);
    CItemData *pScrollItemData = CItemManager::Instance().GetSelectedItemDataPointer();
    if (!pScrollItemData)
    {
        return Py_BuildException("Can't find item data");
    }

    return Py_BuildValue("i", REFINE_SCROLL_TYPE_UP_GRADE == pScrollItemData->GetValue(0));
}

PyObject *playerUpdate(PyObject *poSelf, PyObject *poArgs)
{
    CPythonPlayer::Instance().Update();
    Py_RETURN_NONE;
}

PyObject *playerRender(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

PyObject *playerClear(PyObject *poSelf, PyObject *poArgs)
{
    CPythonPlayer::Instance().Clear();
    Py_RETURN_NONE;
}

PyObject *playerClearTarget(PyObject *poSelf, PyObject *poArgs)
{
    CPythonPlayer::Instance().SetTarget(0);
    Py_RETURN_NONE;
}

PyObject *playerSetTarget(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().SetTarget(iVID);
    Py_RETURN_NONE;
}

PyObject *playerOpenCharacterMenu(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().OpenCharacterMenu(iVID);
    Py_RETURN_NONE;
}

PyObject *playerIsPartyMember(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", CPythonPlayer::Instance().IsPartyMemberByVID(iVID));
}

PyObject *playerIsPartyLeader(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    uint32_t dwPID;
    if (!CPythonPlayer::Instance().PartyMemberVIDToPID(iVID, &dwPID))
    {
        return Py_BuildValue("i", FALSE);
    }

    CPythonPlayer::TPartyMemberInfo *pPartyMemberInfo;
    if (!CPythonPlayer::Instance().GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
    {
        return Py_BuildValue("i", FALSE);
    }

    return Py_BuildValue("i", pPartyMemberInfo->byIsLeader);
}

PyObject *playerIsPartyLeaderByPID(PyObject *poSelf, PyObject *poArgs)
{
    int iPID;
    if (!PyTuple_GetInteger(poArgs, 0, &iPID))
    {
        return Py_BuildException();
    }

    CPythonPlayer::TPartyMemberInfo *pPartyMemberInfo;
    if (!CPythonPlayer::Instance().GetPartyMemberPtr(iPID, &pPartyMemberInfo))
    {
        return Py_BuildValue("i", FALSE);
    }

    return Py_BuildValue("i", pPartyMemberInfo->byIsLeader);
}

PyObject * playerGetPartyMemberRaceByVID(PyObject *poSelf, PyObject *poArgs)
{
    int iVID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVID))
    {
        return Py_BuildException();
    }

    uint32_t dwPID;
    if (!CPythonPlayer::Instance().PartyMemberVIDToPID(iVID, &dwPID))
    {
        return Py_BuildValue("i", FALSE);
    }

    CPythonPlayer::TPartyMemberInfo *pPartyMemberInfo;
    if (!CPythonPlayer::Instance().GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
    {
        return Py_BuildValue("i", FALSE);
    }

    return Py_BuildValue("i", pPartyMemberInfo->byRace);
}

PyObject * playerGetPartyMemberRaceByPID(PyObject *poSelf, PyObject *poArgs)
{
    int iPID;
    if (!PyTuple_GetInteger(poArgs, 0, &iPID))
    {
        return Py_BuildException();
    }

    CPythonPlayer::TPartyMemberInfo *pPartyMemberInfo;
    if (!CPythonPlayer::Instance().GetPartyMemberPtr(iPID, &pPartyMemberInfo))
    {
        return Py_BuildValue("i", 0);
    }

    return Py_BuildValue("i", pPartyMemberInfo->byRace);
}

PyObject *playerGetPartyMemberHPPercentage(PyObject *poSelf, PyObject *poArgs)
{
    int iPID;
    if (!PyTuple_GetInteger(poArgs, 0, &iPID))
    {
        return Py_BuildException();
    }

    CPythonPlayer::TPartyMemberInfo *pPartyMemberInfo;
    if (!CPythonPlayer::Instance().GetPartyMemberPtr(iPID, &pPartyMemberInfo))
    {
        return Py_BuildValue("i", FALSE);
    }

    return Py_BuildValue("i", pPartyMemberInfo->byHPPercentage);
}

PyObject *playerGetPartyMemberState(PyObject *poSelf, PyObject *poArgs)
{
    int iPID;
    if (!PyTuple_GetInteger(poArgs, 0, &iPID))
    {
        return Py_BuildException();
    }

    CPythonPlayer::TPartyMemberInfo *pPartyMemberInfo;
    if (!CPythonPlayer::Instance().GetPartyMemberPtr(iPID, &pPartyMemberInfo))
    {
        return Py_BuildValue("i", FALSE);
    }

    return Py_BuildValue("i", pPartyMemberInfo->byState);
}

PyObject *playerGetPartyMemberAffects(PyObject *poSelf, PyObject *poArgs)
{
    int iPID;
    if (!PyTuple_GetInteger(poArgs, 0, &iPID))
    {
        return Py_BuildException();
    }

    CPythonPlayer::TPartyMemberInfo *pPartyMemberInfo;
    if (!CPythonPlayer::Instance().GetPartyMemberPtr(iPID, &pPartyMemberInfo))
    {
        return Py_BuildValue("i", FALSE);
    }

    return Py_BuildValue("iiiiiii", pPartyMemberInfo->sAffects[0],
                         pPartyMemberInfo->sAffects[1],
                         pPartyMemberInfo->sAffects[2],
                         pPartyMemberInfo->sAffects[3],
                         pPartyMemberInfo->sAffects[4],
                         pPartyMemberInfo->sAffects[5],
                         pPartyMemberInfo->sAffects[6]);
}

PyObject *playerRemovePartyMember(PyObject *poSelf, PyObject *poArgs)
{
    int iPID;
    if (!PyTuple_GetInteger(poArgs, 0, &iPID))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().RemovePartyMember(iPID);
    Py_RETURN_NONE;
}

PyObject *playerExitParty(PyObject *poSelf, PyObject *poArgs)
{
    CPythonPlayer::Instance().ExitParty();
    Py_RETURN_NONE;
}

PyObject *playerGetPKMode(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().GetPKMode());
}

PyObject *playerHasMobilePhoneNumber(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", 0);
}

PyObject *playerSetWeaponAttackBonusFlag(PyObject *poSelf, PyObject *poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
    {
        return Py_BuildException();
    }

    Py_RETURN_NONE;
}

PyObject *playerToggleCoolTime(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().__ToggleCoolTime());
}

PyObject *playerToggleLevelLimit(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().__ToggleLevelLimit());
}

PyObject *playerRegisterEmotionIcon(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    {
        return Py_BuildException();
    }
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
    {
        return Py_BuildException();
    }

    auto pImage = CResourceManager::Instance().LoadResource<CGraphicImage>(szFileName);
    m_kMap_iEmotionIndex_pkIconImage.emplace(iIndex, pImage);

    Py_RETURN_NONE;
}

PyObject *playerGetEmotionIconImage(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    {
        return Py_BuildException();
    }

    if (m_kMap_iEmotionIndex_pkIconImage.end() == m_kMap_iEmotionIndex_pkIconImage.find(iIndex))
    {
        return Py_BuildValue("s", "");
    }
    auto val = m_kMap_iEmotionIndex_pkIconImage[iIndex];

    return Py_BuildValue("s", val->GetFileName());
}

PyObject *playerSetAutoPotionInfo(PyObject *poSelf, PyObject *poArgs)
{
    int potionType = 0;
    if (!PyTuple_GetInteger(poArgs, 0, &potionType))
    {
        return Py_BadArgument();
    }

    CPythonPlayer *player = CPythonPlayer::InstancePtr();

    CPythonPlayer::SAutoPotionInfo &potionInfo = player->GetAutoPotionInfo(potionType);

    if (!PyTuple_GetBoolean(poArgs, 1, &potionInfo.bActivated))
    {
        return Py_BadArgument();
    }

    if (!PyTuple_GetLong(poArgs, 2, &potionInfo.currentAmount))
    {
        return Py_BadArgument();
    }

    if (!PyTuple_GetLong(poArgs, 3, &potionInfo.totalAmount))
    {
        return Py_BadArgument();
    }

    if (!PyTuple_GetLong(poArgs, 4, &potionInfo.inventorySlotIndex))
    {
        return Py_BadArgument();
    }

    Py_RETURN_NONE;
}

PyObject *playerGetAutoPotionInfo(PyObject *poSelf, PyObject *poArgs)
{
    CPythonPlayer *player = CPythonPlayer::InstancePtr();

    int potionType = 0;
    if (!PyTuple_GetInteger(poArgs, 0, &potionType))
    {
        return Py_BadArgument();
    }

    CPythonPlayer::SAutoPotionInfo &potionInfo = player->GetAutoPotionInfo(potionType);

    return Py_BuildValue("biii", potionInfo.bActivated, int(potionInfo.currentAmount), int(potionInfo.totalAmount),
                         int(potionInfo.inventorySlotIndex));
}

PyObject *playerSlotTypeToInvenType(PyObject *poSelf, PyObject *poArgs)
{
    int slotType = 0;
    if (!PyTuple_GetInteger(poArgs, 0, &slotType))
    {
        return Py_BadArgument();
    }

    return Py_BuildValue("i", SlotTypeToInvenType((uint8_t)slotType));
}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
// 플레이어가 벨트를 착용 중인지?
PyObject *playerIsEquippingBelt(PyObject *poSelf, PyObject *poArgs)
{
    const CPythonPlayer *player = CPythonPlayer::InstancePtr();
    bool bEquipping = false;

    const ClientItemData *data = player->GetItemData(TItemPos(EQUIPMENT, WEAR_BELT));

    if (NULL != data)
    {
        bEquipping = 0 < data->count;
    }

    return Py_BuildValue("b", bEquipping);
}

// 검사하려는 벨트 인벤토리 Cell이 사용 가능한 칸인지? (사용가능 여부는 착용 중인 벨트의 강화 정도에 따라 달라짐)
PyObject *playerIsAvailableBeltInventoryCell(PyObject *poSelf, PyObject *poArgs)
{
    const CPythonPlayer *player = CPythonPlayer::InstancePtr();
    const auto pData = player->GetItemData(TItemPos(EQUIPMENT, WEAR_BELT));

    if (NULL == pData || 0 == pData->count)
        return Py_BuildValue("b", false);

    CItemManager::Instance().SelectItemData(pData->vnum);
    CItemData *pItem = CItemManager::Instance().GetSelectedItemDataPointer();

    long beltGrade = pItem->GetValue(0);

    int pos = 0;
    if (!PyTuple_GetInteger(poArgs, 0, &pos))
        return Py_BadArgument();

    //return Py_BuildValue("b", CBeltInventoryHelper::IsAvailableCell(pos - c_Belt_Inventory_Slot_Start, GetItemGrade(pItem->GetName())));
    return Py_BuildValue("b", CBeltInventoryHelper::IsAvailableCell(pos, beltGrade));
}
#endif

// 용혼석 강화
PyObject *playerSendDragonSoulRefine(PyObject *poSelf, PyObject *poArgs)
{
    uint8_t bSubHeader;
    PyObject *pDic;
    TItemPos RefineItemPoses[DS_REFINE_WINDOW_MAX_NUM];
    if (!PyTuple_GetByte(poArgs, 0, &bSubHeader))
    {
        return Py_BuildException();
    }
    switch (bSubHeader)
    {
    case DS_SUB_HEADER_CLOSE:
        break;
    case DS_SUB_HEADER_DO_REFINE_GRADE:
    case DS_SUB_HEADER_DO_REFINE_STEP:
    case DS_SUB_HEADER_DO_REFINE_STRENGTH: {
        if (!PyTuple_GetObject(poArgs, 1, &pDic))
        {
            return Py_BuildException();
        }
        Py_ssize_t pos = 0;
        PyObject *key, *value;
        auto size = PyDict_Size(pDic);

        while (PyDict_Next(pDic, &pos, &key, &value))
        {
            int i = PyInt_AsLong(key);
            if (i > DS_REFINE_WINDOW_MAX_NUM - 1)
            {
                return Py_BuildException();
            }

            if (!PyTuple_GetByte(value, 0, &RefineItemPoses[i].window_type)
                || !PyTuple_GetInteger(value, 1, &RefineItemPoses[i].cell))
            {
                return Py_BuildException();
            }
        }
    }
    break;
    }

    gPythonNetworkStream->SendDragonSoulRefinePacket(bSubHeader, RefineItemPoses);

    Py_RETURN_NONE;
}

PyObject *playerGetItemSealDate(PyObject *poSelf, PyObject *poArgs)
{
    TItemPos Cell;

    if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
    {
        return Py_BuildException();
    }
    if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
    {
        return Py_BuildException();
    }

    const ClientItemData *pPlayerItem;
    if (Cell.window_type == SAFEBOX || Cell.window_type == MALL)
    {
        pPlayerItem = CPythonSafeBox::Instance().GetItemData(Cell.cell);
    }
    else
    {
        pPlayerItem = CPythonPlayer::Instance().GetItemData(Cell);
    }

    if (!pPlayerItem)
    {
        return Py_BuildValue("i", 0);
    }

    long sealTime = pPlayerItem->nSealDate;

    return Py_BuildValue("i", sealTime);
}

PyObject *playerGetItemUnSealLeftTime(PyObject *poSelf, PyObject *poArgs)
{
    TItemPos Cell;

    if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
    {
        return Py_BuildException();
    }
    if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
    {
        return Py_BuildException();
    }

    ClientItemData *pPlayerItem = nullptr;
    if (Cell.window_type == SAFEBOX)
    {
        CPythonSafeBox::Instance().GetItemDataPtr(Cell.cell, &pPlayerItem);
    }
    else
    {
        pPlayerItem = const_cast<ClientItemData *>(CPythonPlayer::Instance().GetItemData(Cell));
    }

    if (!pPlayerItem)
    {
        return Py_BuildValue("ii", 0, 0);
    }

    long sealTime = pPlayerItem->nSealDate;
    if (sealTime != -1 && sealTime != 0 && sealTime > CPythonApplication::AppInst().GetServerTimeStamp())
    {
        return Py_BuildValue("ii", ((sealTime - CPythonApplication::AppInst().GetServerTimeStamp()) / 3600),
                             (sealTime - CPythonApplication::AppInst().GetServerTimeStamp()) % 3600 / 60);
    }
    else
    {
        return Py_BuildValue("ii", 0, 0);
    }
}

PyObject *playerGetAcceItemID(PyObject *poSelf, PyObject *poArgs)
{
    int ipos;
    if (!PyTuple_GetInteger(poArgs, 0, &ipos))
    {
        return Py_BadArgument();
    }

    ClientItemData *pInstance;
    if (!CPythonPlayer::Instance().GetAcceItemDataPtr(ipos, &pInstance))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", pInstance->vnum);
}

PyObject *playerGetAcceItemTransID(PyObject *poSelf, PyObject *poArgs)
{
    int ipos;
    if (!PyTuple_GetInteger(poArgs, 0, &ipos))
    {
        return Py_BadArgument();
    }

    ClientItemData *pInstance;
    if (!CPythonPlayer::Instance().GetAcceItemDataPtr(ipos, &pInstance))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", pInstance->transVnum);
}

PyObject *playerGetAcceItemSize(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().GetCurrentAcceSize());
}

PyObject *playerGetAcceItemFlags(PyObject *poSelf, PyObject *poArgs)
{
    int ipos;
    if (!PyTuple_GetInteger(poArgs, 0, &ipos))
    {
        return Py_BadArgument();
    }

    ClientItemData *pInstance;
    if (!CPythonPlayer::Instance().GetAcceItemDataPtr(ipos, &pInstance))
    {
        return Py_BuildException();
    }
    auto proto = CItemManager::instance().GetProto(pInstance->vnum);
    if (proto)
    {
        return Py_BuildValue("i", proto->GetFlags());
    }

    return Py_BuildValue("i", 0);
}

PyObject *playerGetAcceItemMetinSocket(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BadArgument();
    }
    int iSocketIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
    {
        return Py_BadArgument();
    }

    if (iSocketIndex >= ITEM_SOCKET_MAX_NUM)
    {
        return Py_BuildException();
    }

    ClientItemData *pItemData;
    if (!CPythonPlayer::Instance().GetAcceItemDataPtr(iSlotIndex, &pItemData))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", pItemData->sockets[iSocketIndex]);
}

PyObject *playerGetAcceItemAttribute(PyObject *poSelf, PyObject *poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    {
        return Py_BuildException();
    }
    int iAttrSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
    {
        return Py_BuildException();
    }

    if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_MAX_NUM)
    {
        ClientItemData *pItemData;
        if (CPythonPlayer::Instance().GetAcceItemDataPtr(iSlotIndex, &pItemData))
        {
            return Py_BuildValue("id", pItemData->attrs[iAttrSlotIndex].bType, pItemData->attrs[iAttrSlotIndex].sValue);
        }
    }

    return Py_BuildValue("ii", 0, 0);
}

PyObject *playerIsAcceWindowEmpty(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().IsEmtpyAcceWindow());
}

PyObject *playerGetCurrentItemCount(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().GetCurrentAcceItemCount());
}

PyObject *playerSetAcceRefineWindowOpen(PyObject *poSelf, PyObject *poArgs)
{
    int windowType;
    if (!PyTuple_GetInteger(poArgs, 0, &windowType))
    {
        return Py_BuildException();
    }

    CPythonPlayer::instance().SetAcceRefineWindowOpen(windowType);

    Py_RETURN_NONE;
}

PyObject *playerGetAcceRefineWindowOpen(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().m_acceRefineWindowIsOpen);
}

PyObject *playerGetAcceRefineWindowType(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().m_acceRefineWindowType);
}

PyObject *playerFineMoveAcceItemSlot(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().FineMoveAcceItemSlot());
}

PyObject *playerSetAcceActivedItemSlot(PyObject *poSelf, PyObject *poArgs)
{
    int acceSlot;
    if (!PyTuple_GetInteger(poArgs, 0, &acceSlot))
    {
        return Py_BuildException();
    }

    int itemPos;
    if (!PyTuple_GetInteger(poArgs, 1, &itemPos))
    {
        return Py_BuildException();
    }

    CPythonPlayer::Instance().SetActivedAcceSlot(acceSlot, itemPos);
    Py_RETURN_NONE;
}

PyObject *playerFindActivedAcceSlot(PyObject *poSelf, PyObject *poArgs)
{
    int acceSlot;
    if (!PyTuple_GetInteger(poArgs, 0, &acceSlot))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", CPythonPlayer::Instance().FindActivedSlot(acceSlot));
}

PyObject *playerFindUsingAcceSlot(PyObject *poSelf, PyObject *poArgs)
{
    int acceSlot;
    if (!PyTuple_GetInteger(poArgs, 0, &acceSlot))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("i", CPythonPlayer::Instance().FindUsingSlot(acceSlot));
}

PyObject *playerCanAcceClearItem(PyObject *poSelf, PyObject *poArgs)
{
    gPythonNetworkStream->NotifyHack("CHECK_FAILURE");
    return Py_BuildValue("i", GetRandom(100, 200000));
}

PyObject *playerCanAcceClearItemReal(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", 1);
}

PyObject *playerGetStatusFake(PyObject *poSelf, PyObject *poArgs)
{
    gPythonNetworkStream->NotifyHack("CHECK_STATUS");

    int iType;
    if (!PyTuple_GetInteger(poArgs, 0, &iType))
    {
        return Py_BuildException();
    }

    PointValue iValue = CPythonPlayer::Instance().GetStatus(iType);


    return Py_BuildValue("L", iValue);
}

#ifdef ENABLE_EXTEND_INVEN_SYSTEM

PyObject* playerGetExtendInvenStage(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetExtendInvenStage());
}

PyObject* playerGetExtendInvenMax(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetExtendInvenMax());
}

PyObject* playerSendExtendInvenUpgrade(PyObject* poSelf, PyObject* poArgs)
{
	gPythonNetworkStream->SendExtendInvenUpgrade();
	Py_RETURN_NONE;
}

PyObject* playerSendExtendInvenButtonClick(PyObject* poSelf, PyObject* poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
	{
		return Py_BuildException();
	}

	gPythonNetworkStream->SendExtendInvenButtonClick(index);
	Py_RETURN_NONE;
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM

PyObject *playerGetItemLook(PyObject *poSelf, PyObject *poArgs)
{
    switch (PyTuple_Size(poArgs))
    {
    case 1: {
        int iSlotIndex;
        if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
            return Py_BuildException();

        uint32_t dwVnum = CPythonPlayer::Instance().GetItemChangeLookVnum(TItemPos(INVENTORY, iSlotIndex));
        return Py_BuildValue("i", dwVnum);
    }
    case 2: {
        TItemPos Cell;
        if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
            return Py_BuildException();

        if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
            return Py_BuildException();

        uint32_t dwVnum = CPythonPlayer::Instance().GetItemChangeLookVnum(Cell);
        return Py_BuildValue("i", dwVnum);
    }
    default:
        return Py_BuildException();
    }
}
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
PyObject *playerGetInvenSlotAttachedToConbWindowSlot(PyObject *poSelf, PyObject *poArgs)
{
    int index;
    if (!PyTuple_GetInteger(poArgs, 0, &index))
        return Py_BadArgument();

    return Py_BuildValue("i", CPythonPlayer::instance().GetInvenSlotAttachedToConbWindowSlot(index));
}

PyObject *playerGetConbWindowSlotByAttachedInvenSlot(PyObject *poSelf, PyObject *poArgs)
{
    int index;
    if (!PyTuple_GetInteger(poArgs, 0, &index))
        return Py_BadArgument();

    return Py_BuildValue("i", CPythonPlayer::instance().GetConbWindowSlotByAttachedInvenSlot(index));
}

PyObject *playerSetItemCombinationWindowActivedItemSlot(PyObject *poSelf, PyObject *poArgs)
{
    int index;
    if (!PyTuple_GetInteger(poArgs, 0, &index))
        return Py_BadArgument();

    int value;
    if (!PyTuple_GetInteger(poArgs, 1, &value))
        return Py_BadArgument();

    CPythonPlayer::instance().SetItemCombinationWindowActivedItemSlot(index, value);

    Py_RETURN_NONE;
}

PyObject *playerCanAttachToCombMediumSlot(PyObject *poSelf, PyObject *poArgs)
{
    if (PyTuple_Size(poArgs) != 3)
        return Py_BadArgument();

    int iCombSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iCombSlotIndex))
        return Py_BadArgument();

    TItemPos Cell;
    if (!PyTuple_GetByte(poArgs, 1, &Cell.window_type))
        return Py_BadArgument();

    if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
        return Py_BadArgument();

    CItemManager::Instance().SelectItemData(CPythonPlayer::Instance().GetItemIndex(Cell));
    auto pItemData = CItemManager::Instance().GetSelectedItemDataPointer();

    if (!pItemData)
        return Py_BuildException("Can't find item data");

    if (iCombSlotIndex == COMB_WND_SLOT_BASE)
        return Py_BuildException("SlotIndex Error");

    if (pItemData->GetType() != ITEM_MEDIUM)
    {
        return Py_BuildValue("i", 0);
    }
    if (pItemData->GetSubType() != MEDIUM_MOVE_COSTUME_ATTR)
    {
        return Py_BuildValue("i", 0);
    }

    return Py_BuildValue("i", 1);
}

PyObject *playerCanAttachToCombItemSlot(PyObject *poSelf, PyObject *poArgs)
{
    if (PyTuple_Size(poArgs) != 3)
        return Py_BadArgument();

    int combSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &combSlotIndex))
        return Py_BadArgument();

    TItemPos Cell;
    if (!PyTuple_GetByte(poArgs, 1, &Cell.window_type))
        return Py_BadArgument();

    if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
        return Py_BadArgument();

    CItemManager::Instance().SelectItemData(CPythonPlayer::Instance().GetItemIndex(Cell));
    auto itemData = CItemManager::Instance().GetSelectedItemDataPointer();

    if (!itemData)
        return Py_BuildException("Can't find item data");

    if (combSlotIndex < COMB_WND_SLOT_BASE || combSlotIndex > COMB_WND_SLOT_MATERIAL)
        return Py_BuildException("SlotIndex Error");

    TItemPos pos(INVENTORY, CPythonPlayer::instance().GetInvenSlotAttachedToConbWindowSlot(COMB_WND_SLOT_MEDIUM));
    if (!pos.IsValidItemPosition())
        return Py_BuildValue("i", 0);

    auto pMedium = CItemManager::instance().GetProto(CPythonPlayer::instance().GetItemIndex(pos));
    if (!pMedium)
        return Py_BuildValue("i", 0);

    if (pMedium->GetSubType() != MEDIUM_MOVE_COSTUME_ATTR)
        return Py_BuildValue("i", 0);

    if (itemData->GetSubType() != COSTUME_BODY && itemData->GetSubType() != COSTUME_WEAPON)
        return Py_BuildValue("i", 0);


    if (!CPythonPlayer::instance().CanAttachItemToCombSlotIndex(combSlotIndex, itemData))
    {
        return Py_BuildValue("i", 0);
    }

    return Py_BuildValue("i", 1);
}

#endif

PyObject *playerWindowTypeToSlotType(PyObject *poSelf, PyObject *poArgs)
{
    int iWindowType = 0;
    if (!PyTuple_GetInteger(poArgs, 0, &iWindowType))
        return Py_BadArgument();

    return Py_BuildValue("i", WindowTypeToSlotType(iWindowType));
}

PyObject *playerMyShopDecoShow(PyObject *poSelf, PyObject *poArgs)
{
    bool isShow = false;
    if (!PyTuple_GetBoolean(poArgs, 0, &isShow))
        return Py_BadArgument();

    CPythonMyShopDecoManager::instance().m_isShow = isShow;

    Py_RETURN_NONE;
}

PyObject *playerSelectShopModel(PyObject *poSelf, PyObject *poArgs)
{
    DWORD modelIndex = 0u;
    if (!PyTuple_GetUnsignedLong(poArgs, 0, &modelIndex))
        return Py_BadArgument();

    CPythonMyShopDecoManager::instance().SelectModel(modelIndex);

    Py_RETURN_NONE;
}

PyObject *playerItemRenderTargetSetData(PyObject *poSelf, PyObject *poArgs)
{
    PyObject *poDict = PyTuple_GetItem(poArgs, 0);
    if (!PyDict_Check(poDict))
        return Py_BuildException();

    CInstanceBase::SCreateData kCreateData = {};

    PyObject *poType = PyDict_GetItemString(poDict, "charType");
    if (poType)
        kCreateData.m_bType = PyLong_AsLong(poType);

    PyObject *poRace = PyDict_GetItemString(poDict, "race");
    if (poRace)
        kCreateData.m_dwRace = PyLong_AsLong(poRace);

    PyObject *poMountVnum = PyDict_GetItemString(poDict, "mountVnum");
    if (poMountVnum)
        kCreateData.m_dwMountVnum = PyLong_AsLong(poMountVnum);

    if (kCreateData.m_bType != CActorInstance::TYPE_NPC)
    {
        PyObject *poWeapon = PyDict_GetItemString(poDict, "weapon");
        if (poWeapon)
        {
            kCreateData.m_parts[PART_WEAPON].costume = PyLong_AsLong(poWeapon);
        }
        else
        {
            kCreateData.m_parts[PART_WEAPON].vnum = CPythonPlayer::Instance().GetItemIndex(
                TItemPos(EQUIPMENT, WEAR_WEAPON));
            kCreateData.m_parts[PART_WEAPON].costume = CPythonPlayer::Instance().GetItemIndex(
                TItemPos(EQUIPMENT, WEAR_COSTUME_WEAPON));
        }

        PyObject *poHair = PyDict_GetItemString(poDict, "hair");
        if (poHair)
            kCreateData.m_parts[PART_HAIR].vnum = PyLong_AsLong(poHair);
        else
        {
            kCreateData.m_parts[PART_HAIR].vnum = CPythonPlayer::Instance().GetItemValue(
                TItemPos(EQUIPMENT, WEAR_COSTUME_HAIR), 3);
            kCreateData.m_parts[PART_HAIR].costume = CPythonPlayer::Instance().GetItemValue(
                TItemPos(EQUIPMENT, WEAR_COSTUME_HAIR), 3);
        }

        PyObject *poAcce = PyDict_GetItemString(poDict, "acce");
        if (poAcce)
            kCreateData.m_parts[PART_ACCE].vnum = PyLong_AsLong(poAcce);
        else
        {
            kCreateData.m_parts[PART_ACCE].vnum = CPythonPlayer::Instance().GetItemIndex(
                TItemPos(EQUIPMENT, WEAR_COSTUME_ACCE));
        }

        PyObject *poBody = PyDict_GetItemString(poDict, "armor");
        if (poBody)
        {
            kCreateData.m_parts[PART_MAIN].costume = PyLong_AsLong(poBody);
        }
        else
        {
            kCreateData.m_parts[PART_MAIN].vnum = CPythonPlayer::Instance().
                GetItemIndex(TItemPos(EQUIPMENT, WEAR_BODY));
            kCreateData.m_parts[PART_MAIN].costume = CPythonPlayer::Instance().GetItemIndex(
                TItemPos(EQUIPMENT, WEAR_COSTUME_BODY));
        }

        PyObject *poBodyEffect = PyDict_GetItemString(poDict, "body_effect");
        if (poBodyEffect)
        {
            kCreateData.m_parts[PART_BODY_EFFECT].vnum = PyLong_AsLong(poBodyEffect);
        }
        else
        {
            kCreateData.m_parts[PART_BODY_EFFECT].vnum = CPythonPlayer::Instance().GetItemIndex(
                TItemPos(EQUIPMENT, WEAR_COSTUME_BODY_EFFECT));
            kCreateData.m_parts[PART_BODY_EFFECT].costume = CPythonPlayer::Instance().GetItemIndex(
                TItemPos(EQUIPMENT, WEAR_COSTUME_BODY_EFFECT));
        }

        PyObject *poWingEffect = PyDict_GetItemString(poDict, "wing_effect");
        if (poWingEffect)
        {
            kCreateData.m_parts[PART_WING_EFFECT].vnum = PyLong_AsLong(poWingEffect);
        }
        else
        {
            kCreateData.m_parts[PART_WING_EFFECT].vnum = CPythonPlayer::Instance().GetItemIndex(
                TItemPos(EQUIPMENT, WEAR_COSTUME_WING_EFFECT));
            kCreateData.m_parts[PART_WING_EFFECT].costume = CPythonPlayer::Instance().GetItemIndex(
                TItemPos(EQUIPMENT, WEAR_COSTUME_WING_EFFECT));
        }

        PyObject *poWeaponEffect = PyDict_GetItemString(poDict, "weapon_effect");
        if (poWeaponEffect)
        {
            kCreateData.m_parts[PART_WEAPON_EFFECT].vnum = PyLong_AsLong(poWeaponEffect);
        }
        else
        {
            kCreateData.m_parts[PART_WEAPON_EFFECT].vnum = CPythonPlayer::Instance().GetItemIndex(
                TItemPos(EQUIPMENT, WEAR_COSTUME_WEAPON_EFFECT));
            kCreateData.m_parts[PART_WEAPON_EFFECT].costume = CPythonPlayer::Instance().GetItemIndex(
                TItemPos(EQUIPMENT, WEAR_COSTUME_WEAPON_EFFECT));
        }
    }

    PyObject *poScale = PyDict_GetItemString(poDict, "scale");
    if (poScale)
    {
        kCreateData.m_scale = PyLong_AsLong(poScale);
    }
    else
    {
        kCreateData.m_scale = 100;
    }

    CPythonItemRenderTargetManager::instance().Select(kCreateData);

    Py_RETURN_NONE;
}

PyObject *playerItemRenderTargetSetFov(PyObject *poSelf, PyObject *poArgs)
{
    float fov = 0.0f;
    if (!PyTuple_GetFloat(poArgs, 0, &fov))
        return Py_BadArgument();

    CPythonItemRenderTargetManager::instance().SetFov(fov);

    Py_RETURN_NONE;
}

PyObject *playerItemRenderTargetSetModelZoom(PyObject *poSelf, PyObject *poArgs)
{
    int isNear = 1;
    if (!PyTuple_GetInteger(poArgs, 0, &isNear))
        return Py_BadArgument();

    CPythonItemRenderTargetManager::instance().SetModelZoom(isNear);

    Py_RETURN_NONE;
}

PyObject *playerItemRenderTargetDrag(PyObject *poSelf, PyObject *poArgs)
{
    int32_t x = 0;
    if (!PyTuple_GetInteger(poArgs, 0, &x))
        return Py_BadArgument();

    int32_t y = 0.0f;
    if (!PyTuple_GetInteger(poArgs, 0, &y))
        return Py_BadArgument();

    CPythonItemRenderTargetManager::instance().Drag(x, y);

    Py_RETURN_NONE;
}

PyObject *playerItemRenderTargetShow(PyObject *poSelf, PyObject *poArgs)
{
    CPythonItemRenderTargetManager::instance().Show();
    Py_RETURN_NONE;
}

PyObject *playerItemRenderTargetHide(PyObject *poSelf, PyObject *poArgs)
{
    CPythonItemRenderTargetManager::instance().Hide();
    Py_RETURN_NONE;
}

PyObject *playerItemRenderTargetSetTargetZ(PyObject *poSelf, PyObject *poArgs)
{
    float z = 0.0f;
    if (!PyTuple_GetFloat(poArgs, 0, &z))
        return Py_BadArgument();

    CPythonItemRenderTargetManager::instance().SetTargetZ(z);

    Py_RETURN_NONE;
}

PyObject *playerItemRenderTargetEnableRotation(PyObject *poSelf, PyObject *poArgs)
{
    bool z = false;
    if (!PyTuple_GetBoolean(poArgs, 0, &z))
        return Py_BadArgument();

    CPythonItemRenderTargetManager::instance().EnableRotation(z);

    Py_RETURN_NONE;
}

PyObject *playerItemRenderTargetSetRotSpeed(PyObject *poSelf, PyObject *poArgs)
{
    float z = 0.0f;
    if (!PyTuple_GetFloat(poArgs, 0, &z))
        return Py_BadArgument();

    CPythonItemRenderTargetManager::instance().SetRotSpeed(z);

    Py_RETURN_NONE;
}

PyObject *playerItemRenderTargetSetBaseRot(PyObject *poSelf, PyObject *poArgs)
{
    float z = 0.0f;
    if (!PyTuple_GetFloat(poArgs, 0, &z))
        return Py_BadArgument();

    CPythonItemRenderTargetManager::instance().SetBaseRot(z);

    Py_RETURN_NONE;
}

#ifdef ENABLE_GEM_SYSTEM
PyObject* playerSetGemShopWindowOpen(PyObject* poSelf, PyObject* poArgs)
{
	int windowType;
	if (!PyTuple_GetInteger(poArgs, 0, &windowType))
		return Py_BuildException();

	CPythonPlayer::instance().SetGemShopWindowOpen(windowType);

	Py_RETURN_NONE;
}

PyObject* playerIsGemShopWindowOpen(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().IsGemShopWindowOpen());
}

#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject *playerSetChangeLookWindowOpen(PyObject *poSelf, PyObject *poArgs)
{
    int isOpen;
    if (!PyTuple_GetInteger(poArgs, 0, &isOpen))
        return Py_BuildException();

    CPythonPlayer::instance().SetChangeLookWindowOpen(isOpen);

    Py_RETURN_NONE;
}

PyObject *playerGetChangeLookWindowOpen(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonPlayer::Instance().GetChangeLookWindowOpen());
}

#endif

extern "C" void initplayer()
{
    static PyMethodDef s_methods[] =
    {
        {"SetMouseState", playerSetMouseState, METH_VARARGS},
        {"SetMouseFunc", playerSetMouseFunc, METH_VARARGS},
        {"GetMouseFunc", playerGetMouseFunc, METH_VARARGS},
        {"SetMouseMiddleButtonState", playerSetMouseMiddleButtonState, METH_VARARGS},
        {"GetMainCharacterName", playerGetMainCharacterName, METH_VARARGS},
        {"GetMainCharacterPosition", playerGetMainCharacterPosition, METH_VARARGS},
        {"CanAttackInstance", playerCanAttackInstance, METH_VARARGS},
        {"IsActingEmotion", playerIsActingEmotion, METH_VARARGS},
        {"IsPVPInstance", playerIsPVPInstance, METH_VARARGS},
        {"IsSameEmpire", playerIsSameEmpire, METH_VARARGS},
        {"IsChallengeInstance", playerIsChallengeInstance, METH_VARARGS},
        {"IsRevengeInstance", playerIsRevengeInstance, METH_VARARGS},
        {"IsCantFightInstance", playerIsCantFightInstance, METH_VARARGS},
        {"GetCharacterDistance", playerGetCharacterDistance, METH_VARARGS},
        {"IsInSafeArea", playerIsInSafeArea, METH_VARARGS},
        {"IsMountingHorse", playerIsMountingHorse, METH_VARARGS},
        {"IsObserverMode", playerIsObserverMode, METH_VARARGS},
        {"ActEmotion", playerActEmotion, METH_VARARGS},
        {"IsGameMaster", playerIsGameMaster, METH_VARARGS}, //{ "ShowPlayer",					playerShowPlayer,					METH_VARARGS },
        //{ "HidePlayer",					playerHidePlayer,					METH_VARARGS },

        //{ "ComboAttack",				playerComboAttack,					METH_VARARGS },

        {"SetAutoCameraRotationSpeed", playerSetAutoCameraRotationSpeed, METH_VARARGS},
        {"SetAttackKeyState", playerSetAttackKeyState, METH_VARARGS},
        {"SetSingleDIKKeyState", playerSetSingleDIKKeyState, METH_VARARGS},
        {"EndKeyWalkingImmediately", playerEndKeyWalkingImmediately, METH_VARARGS},
        {"StartMouseWalking", playerStartMouseWalking, METH_VARARGS},
        {"EndMouseWalking", playerEndMouseWalking, METH_VARARGS},
        {"ResetCameraRotation", playerResetCameraRotation, METH_VARARGS},
        {"SetQuickCameraMode", playerSetQuickCameraMode, METH_VARARGS},

        ///////////////////////////////////////////////////////////////////////////////////////////

        {"SetSkill", playerSetSkill, METH_VARARGS},
        {"GetSkillIndex", playerGetSkillIndex, METH_VARARGS},
        {"GetSkillSlotIndex", playerGetSkillSlotIndex, METH_VARARGS},
        {"GetSkillGrade", playerGetSkillGrade, METH_VARARGS},
        {"GetSkillLevel", playerGetSkillLevel, METH_VARARGS},
        {"GetSkillColor", playerGetSkillColor, METH_VARARGS},

        {"SetSkillInfo", playerSetSkillInfo, METH_VARARGS},
        {"GetSkillDisplayLevel", playerGetSkillDisplayLevel, METH_VARARGS},

        
        {"GetSkillCurrentEfficientPercentage", playerGetSkillCurrentEfficientPercentage, METH_VARARGS},
        {"GetSkillNextEfficientPercentage", playerGetSkillNextEfficientPercentage, METH_VARARGS},
        {"ClickSkillSlot", playerClickSkillSlot, METH_VARARGS},
        {"ChangeCurrentSkillNumberOnly", playerChangeCurrentSkillNumberOnly, METH_VARARGS},

        {"GetItemIndex", playerGetItemIndex, METH_VARARGS},
        {"GetItemFlags", playerGetItemFlags, METH_VARARGS},
        {"GetItemCount", playerGetItemCount, METH_VARARGS},
        {"GetItemCountByVnum", playerGetItemCountByVnum, METH_VARARGS},
        {"GetItemMetinSocket", playerGetItemMetinSocket, METH_VARARGS},
        {"GetItemAttribute", playerGetItemAttribute, METH_VARARGS},

        {"IsFlagBySlot", playerIsFlagBySlot, METH_VARARGS},
        {"IsAntiFlagBySlot", playerIsAntiFlagBySlot, METH_VARARGS},
        {"IsSealedItemBySlot", playerIsSealedItemBySlot, METH_VARARGS},
        {"GetItemTypeBySlot", playerGetItemTypeBySlot, METH_VARARGS},
        {"GetItemSubTypeBySlot", playerGetItemSubTypeBySlot, METH_VARARGS},

        {"GetISellItemPrice", playerGetISellItemPrice, METH_VARARGS},
        {"MoveItem", playerMoveItem, METH_VARARGS},
        {"SendClickItemPacket", playerSendClickItemPacket, METH_VARARGS},

        ///////////////////////////////////////////////////////////////////////////////////////////

        {"GetName", playerGetName, METH_VARARGS},
        {"GetJob", playerGetJob, METH_VARARGS},
        {"GetRace", playerGetRace, METH_VARARGS},
        {"GetPlayTime", playerGetPlayTime, METH_VARARGS},
        {"SetPlayTime", playerSetPlayTime, METH_VARARGS},

        {"IsSkillCoolTime", playerIsSkillCoolTime, METH_VARARGS},
        {"GetSkillCoolTime", playerGetSkillCoolTime, METH_VARARGS},
        {"IsSkillActive", playerIsSkillActive, METH_VARARGS},
        {"UseGuildSkill", playerUseGuildSkill, METH_VARARGS},
        {"GetEXP", playerGetEXP, METH_VARARGS},
        {"SetStatus", playerSetStatus, METH_VARARGS},
        {"GetElk", playerGetElk, METH_VARARGS},
        {"GetMoney", playerGetElk, METH_VARARGS},
#ifdef ENABLE_GEM_SYSTEM
		{ "GetGem", playerGetGem, METH_VARARGS },
#endif
        {"GetGuildID", playerGetGuildID, METH_VARARGS},
        {"GetGuildName", playerGetGuildName, METH_VARARGS},
        {"GetAlignmentData", playerGetAlignmentData, METH_VARARGS},
        {"RequestAddLocalQuickSlot", playerRequestAddLocalQuickSlot, METH_VARARGS},
        {"RequestAddToEmptyLocalQuickSlot", playerRequestAddToEmptyLocalQuickSlot, METH_VARARGS},
        {"RequestDeleteGlobalQuickSlot", playerRequestDeleteGlobalQuickSlot, METH_VARARGS},
        {"RequestMoveGlobalQuickSlotToLocalQuickSlot", playerRequestMoveGlobalQuickSlotToLocalQuickSlot, METH_VARARGS},
        {"RequestUseLocalQuickSlot", playerRequestUseLocalQuickSlot, METH_VARARGS},
        {"LocalQuickSlotIndexToGlobalQuickSlotIndex", playerLocalQuickSlotIndexToGlobalQuickSlotIndex, METH_VARARGS},

        {"GetQuickPage", playerGetQuickPage, METH_VARARGS},
        {"SetQuickPage", playerSetQuickPage, METH_VARARGS},
        {"GetLocalQuickSlot", playerGetLocalQuickSlot, METH_VARARGS},
        {"GetGlobalQuickSlot", playerGetGlobalQuickSlot, METH_VARARGS},
        {"RemoveQuickSlotByValue", playerRemoveQuickSlotByValue, METH_VARARGS},

        {"isItem", playerisItem, METH_VARARGS},
        {"IsEquipmentSlot", playerIsEquipmentSlot, METH_VARARGS},
        {"IsDSEquipmentSlot", playerIsDSEquipmentSlot, METH_VARARGS},
        {"IsCostumeSlot", playerIsCostumeSlot, METH_VARARGS},
        {"IsValuableItem", playerIsValuableItem, METH_VARARGS},
        {"IsOpenPrivateShop", playerIsOpenPrivateShop, METH_VARARGS},

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
        {"IsBeltInventorySlot", playerIsBeltInventorySlot, METH_VARARGS},
        {"IsEquippingBelt", playerIsEquippingBelt, METH_VARARGS},
        {"IsAvailableBeltInventoryCell", playerIsAvailableBeltInventoryCell, METH_VARARGS},
#endif

        // Refine
        {"GetItemGrade", playerGetItemGrade, METH_VARARGS},
        {"IsRefineGradeScroll", playerIsRefineGradeScroll, METH_VARARGS},

        {"ClearTarget", playerClearTarget, METH_VARARGS},
        {"SetTarget", playerSetTarget, METH_VARARGS},
        {"OpenCharacterMenu", playerOpenCharacterMenu, METH_VARARGS},

        {"Update", playerUpdate, METH_VARARGS},
        {"Render", playerRender, METH_VARARGS},
        {"Clear", playerClear, METH_VARARGS},

        // Party
        {"IsPartyMember", playerIsPartyMember, METH_VARARGS},
        {"IsPartyLeader", playerIsPartyLeader, METH_VARARGS},
        {"IsPartyLeaderByPID", playerIsPartyLeaderByPID, METH_VARARGS},
        {"GetPartyMemberHPPercentage", playerGetPartyMemberHPPercentage, METH_VARARGS},
        {"GetPartyMemberState", playerGetPartyMemberState, METH_VARARGS},
        {"GetPartyMemberAffects", playerGetPartyMemberAffects, METH_VARARGS},
        {"RemovePartyMember", playerRemovePartyMember, METH_VARARGS},
        {"ExitParty", playerExitParty, METH_VARARGS},

        // PK Mode
        {"GetPKMode", playerGetPKMode, METH_VARARGS},

        // Mobile
        {"HasMobilePhoneNumber", playerHasMobilePhoneNumber, METH_VARARGS},

        // Emotion
        {"RegisterEmotionIcon", playerRegisterEmotionIcon, METH_VARARGS},
        {"GetEmotionIconImage", playerGetEmotionIconImage, METH_VARARGS},

        // For System
        {"SetWeaponAttackBonusFlag", playerSetWeaponAttackBonusFlag, METH_VARARGS},
        {"ToggleCoolTime", playerToggleCoolTime, METH_VARARGS},
        {"ToggleLevelLimit", playerToggleLevelLimit, METH_VARARGS},

        {"SlotTypeToInvenType", playerSlotTypeToInvenType, METH_VARARGS},
        {"SendDragonSoulRefine", playerSendDragonSoulRefine, METH_VARARGS},

        // Item Sealing
        {"GetItemSealDate", playerGetItemSealDate, METH_VARARGS},
        {"GetItemUnSealLeftTime", playerGetItemUnSealLeftTime, METH_VARARGS},

        // ACCE

        {"GetAcceItemID", playerGetAcceItemID, METH_VARARGS},
        {"GetAcceItemTransmutationID", playerGetAcceItemTransID, METH_VARARGS},

        {"GetAcceItemSize", playerGetAcceItemSize, METH_VARARGS},
        {"GetAcceItemFlags", playerGetAcceItemFlags, METH_VARARGS},
        {"GetAcceItemMetinSocket", playerGetAcceItemMetinSocket, METH_VARARGS},
        {"GetAcceItemAttribute", playerGetAcceItemAttribute, METH_VARARGS},
        {"IsAcceWindowEmpty", playerIsAcceWindowEmpty, METH_VARARGS},
        {"GetCurrentItemCount", playerGetCurrentItemCount, METH_VARARGS},
        {"SetAcceRefineWindowOpen", playerSetAcceRefineWindowOpen, METH_VARARGS},
        {"GetAcceRefineWindowOpen", playerGetAcceRefineWindowOpen, METH_VARARGS},
        {"GetAcceRefineWindowType", playerGetAcceRefineWindowType, METH_VARARGS},
        {"FineMoveAcceItemSlot", playerFineMoveAcceItemSlot, METH_VARARGS},
        {"SetAcceActivedItemSlot", playerSetAcceActivedItemSlot, METH_VARARGS},
        {"FindActivedAcceSlot", playerFindActivedAcceSlot, METH_VARARGS},
        {"FindUsingAcceSlot", playerFindUsingAcceSlot, METH_VARARGS},
        {"CanAcceClearItem", playerCanAcceClearItemReal, METH_VARARGS},
        {"WindowTypeToSlotType", playerWindowTypeToSlotType, METH_VARARGS},

        {"MyShopDecoShow", playerMyShopDecoShow, METH_VARARGS},
        {"SelectShopModel", playerSelectShopModel, METH_VARARGS},

#ifdef ENABLE_GEM_SYSTEM
		{ "SetGemShopWindowOpen", playerSetGemShopWindowOpen, METH_VARARGS },
		{ "IsGemShopWindowOpen", playerIsGemShopWindowOpen, METH_VARARGS },
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
        {"GetChangeLookWindowOpen", playerGetChangeLookWindowOpen, METH_VARARGS},
        {"SetChangeLookWindowOpen", playerSetChangeLookWindowOpen, METH_VARARGS},
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
        {"GetInvenSlotAttachedToConbWindowSlot", playerGetInvenSlotAttachedToConbWindowSlot, METH_VARARGS},
        {"GetConbWindowSlotByAttachedInvenSlot", playerGetConbWindowSlotByAttachedInvenSlot, METH_VARARGS},
        {"SetItemCombinationWindowActivedItemSlot", playerSetItemCombinationWindowActivedItemSlot, METH_VARARGS},
        {"CanAttachToCombMediumSlot", playerCanAttachToCombMediumSlot, METH_VARARGS},
        {"CanAttachToCombItemSlot", playerCanAttachToCombItemSlot, METH_VARARGS},
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
		{ "GetExtendInvenStage", playerGetExtendInvenStage, METH_VARARGS },
		{ "GetExtendInvenMax", playerGetExtendInvenMax, METH_VARARGS },
		{ "SendExtendInvenUpgrade", playerSendExtendInvenUpgrade, METH_VARARGS },
		{ "SendExtendInvenButtonClick", playerSendExtendInvenButtonClick, METH_VARARGS },
#endif

        {"GetChangeLookVnum", playerGetChangeLookVnum, METH_VARARGS},
        {"SetItemRenderTargetData", playerItemRenderTargetSetData, METH_VARARGS},
        {"SetItemRenderTargetFov", playerItemRenderTargetSetFov, METH_VARARGS},
        {"SetItemRenderTargetZ", playerItemRenderTargetSetTargetZ, METH_VARARGS},
        {"ShowItemRenderTarget", playerItemRenderTargetShow, METH_VARARGS},
        {"HideItemRenderTarget", playerItemRenderTargetHide, METH_VARARGS},
        {"SetItemRenderEnableRotation", playerItemRenderTargetEnableRotation, METH_VARARGS},
        {"SetItemRenderSetRotSpeed", playerItemRenderTargetSetRotSpeed, METH_VARARGS},
        {"SetItemRenderSetBaseRot", playerItemRenderTargetSetBaseRot, METH_VARARGS},
        {"ItemRenderTargetDrag", playerItemRenderTargetDrag, METH_VARARGS},
        {"SetItemRenderTargetModelZoom", playerItemRenderTargetSetModelZoom, METH_VARARGS,},

        {"SetParalysis", playerSetParalysis, METH_VARARGS},

        {"GetPartyMemberRaceByVID", playerGetPartyMemberRaceByVID, METH_VARARGS},
        {"GetPartyMemberRaceByPID", playerGetPartyMemberRaceByPID, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("player", s_methods);
    PyModule_AddIntConstant(poModule, "LEVEL", POINT_LEVEL);
    PyModule_AddIntConstant(poModule, "VOICE", POINT_VOICE);
    PyModule_AddIntConstant(poModule, "EXP", POINT_EXP);
    PyModule_AddIntConstant(poModule, "NEXT_EXP", POINT_NEXT_EXP);
    PyModule_AddIntConstant(poModule, "HP", POINT_HP);
    PyModule_AddIntConstant(poModule, "MAX_HP", POINT_MAX_HP);
    PyModule_AddIntConstant(poModule, "SP", POINT_SP);
    PyModule_AddIntConstant(poModule, "MAX_SP", POINT_MAX_SP);
    PyModule_AddIntConstant(poModule, "STAMINA", POINT_STAMINA);
    PyModule_AddIntConstant(poModule, "MAX_STAMINA", POINT_MAX_STAMINA);
    PyModule_AddIntConstant(poModule, "ELK", POINT_GOLD);
#ifdef ENABLE_GEM_SYSTEM
	PyModule_AddIntConstant(poModule, "GEM", POINT_GEM);
#endif
    PyModule_AddIntConstant(poModule, "ST", POINT_ST);
    PyModule_AddIntConstant(poModule, "HT", POINT_HT);
    PyModule_AddIntConstant(poModule, "DX", POINT_DX);
    PyModule_AddIntConstant(poModule, "IQ", POINT_IQ);
    PyModule_AddIntConstant(poModule, "ATT_MIN", POINT_WEAPON_MIN);
    PyModule_AddIntConstant(poModule, "ATT_MAX", POINT_WEAPON_MAX);
    PyModule_AddIntConstant(poModule, "MIN_MAGIC_WEP", POINT_MIN_MAGIC_WEP);
    PyModule_AddIntConstant(poModule, "MAX_MAGIC_WEP", POINT_MAX_MAGIC_WEP);
    PyModule_AddIntConstant(poModule, "ATT_SPEED", POINT_ATT_SPEED);
    PyModule_AddIntConstant(poModule, "ATT_BONUS", POINT_ATT_GRADE_BONUS);
    //  PyModule_AddIntConstant(poModule, "EVADE_RATE",				POINT_EVADE_RATE);
    PyModule_AddIntConstant(poModule, "MOVING_SPEED", POINT_MOV_SPEED);
    PyModule_AddIntConstant(poModule, "DEF_GRADE", POINT_DEF_GRADE);
    PyModule_AddIntConstant(poModule, "DEF_BONUS", POINT_DEF_GRADE_BONUS);
    PyModule_AddIntConstant(poModule, "CASTING_SPEED", POINT_CASTING_SPEED);
    PyModule_AddIntConstant(poModule, "MAG_ATT", POINT_MAGIC_ATT_GRADE);
    PyModule_AddIntConstant(poModule, "MAG_DEF", POINT_MAGIC_DEF_GRADE);
    PyModule_AddIntConstant(poModule, "EMPIRE_POINT", POINT_EMPIRE_POINT);
    PyModule_AddIntConstant(poModule, "STAT", POINT_STAT);
    PyModule_AddIntConstant(poModule, "SKILL_PASSIVE", POINT_SUB_SKILL);
    PyModule_AddIntConstant(poModule, "SKILL_SUPPORT", POINT_SUB_SKILL);
    PyModule_AddIntConstant(poModule, "SKILL_ACTIVE", POINT_SKILL);
    PyModule_AddIntConstant(poModule, "SKILL_HORSE", POINT_HORSE_SKILL);
    PyModule_AddIntConstant(poModule, "SKILL_TREE", POINT_SKILLTREE_POINTS);

    PyModule_AddIntConstant(poModule, "PLAYTIME", POINT_PLAYTIME);
    PyModule_AddIntConstant(poModule, "BOW_DISTANCE", POINT_BOW_DISTANCE);
    PyModule_AddIntConstant(poModule, "HP_RECOVERY", POINT_HP_RECOVERY);
    PyModule_AddIntConstant(poModule, "SP_RECOVERY", POINT_SP_RECOVERY);
    PyModule_AddIntConstant(poModule, "ATTACKER_BONUS", POINT_PARTY_ATTACKER_BONUS);
    PyModule_AddIntConstant(poModule, "TANKER_BONUS", POINT_PARTY_TANKER_BONUS);

    PyModule_AddIntConstant(poModule, "MAX_NUM", POINT_MAX_NUM);
    ////
    PyModule_AddIntConstant(poModule, "POINT_CRITICAL_PCT", POINT_CRITICAL_PCT);
    PyModule_AddIntConstant(poModule, "POINT_PENETRATE_PCT", POINT_PENETRATE_PCT);
    PyModule_AddIntConstant(poModule, "POINT_MALL_ATTBONUS", POINT_MALL_ATTBONUS);
    PyModule_AddIntConstant(poModule, "POINT_MALL_DEFBONUS", POINT_MALL_DEFBONUS);
    PyModule_AddIntConstant(poModule, "POINT_MALL_EXPBONUS", POINT_MALL_EXPBONUS);
    PyModule_AddIntConstant(poModule, "POINT_MALL_ITEMBONUS", POINT_MALL_ITEMBONUS);
    PyModule_AddIntConstant(poModule, "POINT_MALL_GOLDBONUS", POINT_MALL_GOLDBONUS);
    PyModule_AddIntConstant(poModule, "POINT_MAX_HP_PCT", POINT_MAX_HP_PCT);
    PyModule_AddIntConstant(poModule, "POINT_MAX_SP_PCT", POINT_MAX_SP_PCT);

    PyModule_AddIntConstant(poModule, "POINT_SKILL_DAMAGE_BONUS", POINT_SKILL_DAMAGE_BONUS);
    PyModule_AddIntConstant(poModule, "POINT_NORMAL_HIT_DAMAGE_BONUS", POINT_NORMAL_HIT_DAMAGE_BONUS);
    PyModule_AddIntConstant(poModule, "POINT_SKILL_DEFEND_BONUS", POINT_SKILL_DEFEND_BONUS);
    PyModule_AddIntConstant(poModule, "POINT_NORMAL_HIT_DEFEND_BONUS", POINT_NORMAL_HIT_DEFEND_BONUS);
    PyModule_AddIntConstant(poModule, "POINT_PC_BANG_EXP_BONUS", POINT_PC_BANG_EXP_BONUS);
    PyModule_AddIntConstant(poModule, "POINT_PC_BANG_DROP_BONUS", POINT_PC_BANG_DROP_BONUS);

    /*Added missing POINTS MartPwnS*/

    PyModule_AddIntConstant(poModule, "POINT_COSTUME_ATTR_BONUS", POINT_COSTUME_ATTR_BONUS);
    PyModule_AddIntConstant(poModule, "POINT_MAGIC_ATT_BONUS_PER", POINT_MAGIC_ATT_BONUS_PER);
    PyModule_AddIntConstant(poModule, "POINT_MELEE_MAGIC_ATT_BONUS_PER", POINT_MELEE_MAGIC_ATT_BONUS_PER);
    PyModule_AddIntConstant(poModule, "POINT_RESIST_ICE", POINT_RESIST_ICE);
    PyModule_AddIntConstant(poModule, "POINT_RESIST_EARTH", POINT_RESIST_EARTH);
    PyModule_AddIntConstant(poModule, "POINT_RESIST_DARK", POINT_RESIST_DARK);
    PyModule_AddIntConstant(poModule, "POINT_RESIST_CRITICAL", POINT_RESIST_CRITICAL);
    PyModule_AddIntConstant(poModule, "POINT_RESIST_PENETRATE", POINT_RESIST_PENETRATE);
    PyModule_AddIntConstant(poModule, "POINT_BOOST_CRITICAL", POINT_BOOST_CRITICAL);
    PyModule_AddIntConstant(poModule, "POINT_BOOST_PENETRATE", POINT_BOOST_PENETRATE);

    PyModule_AddIntConstant(poModule, "POINT_ATTBONUS_METIN", POINT_ATTBONUS_METIN);
    PyModule_AddIntConstant(poModule, "POINT_ATTBONUS_TRENT", POINT_ATTBONUS_TRENT);
    PyModule_AddIntConstant(poModule, "POINT_ATTBONUS_BOSS", POINT_ATTBONUS_BOSS);
    PyModule_AddIntConstant(poModule, "POINT_CASH", POINT_CASH);

    PyModule_AddIntConstant(poModule, "ENERGY", POINT_ENERGY);
    PyModule_AddIntConstant(poModule, "ENERGY_END_TIME", POINT_ENERGY_END_TIME);

    PyModule_AddIntConstant(poModule, "SKILL_GRADE_NORMAL", CPythonPlayer::SKILL_NORMAL);
    PyModule_AddIntConstant(poModule, "SKILL_GRADE_MASTER", CPythonPlayer::SKILL_MASTER);
    PyModule_AddIntConstant(poModule, "SKILL_GRADE_GRAND_MASTER", CPythonPlayer::SKILL_GRAND_MASTER);
    PyModule_AddIntConstant(poModule, "SKILL_GRADE_PERFECT_MASTER", CPythonPlayer::SKILL_PERFECT_MASTER);
    PyModule_AddIntConstant(poModule, "SKILL_GRADE_LEGENDARY_MASTER", CPythonPlayer::SKILL_LEGENDARY_MASTER);

    PyModule_AddIntConstant(poModule, "CATEGORY_ACTIVE", CPythonPlayer::CATEGORY_ACTIVE);
    PyModule_AddIntConstant(poModule, "CATEGORY_PASSIVE", CPythonPlayer::CATEGORY_PASSIVE);

    PyModule_AddIntConstant(poModule, "INVENTORY_PAGE_SIZE", INVENTORY_PAGE_SIZE);
    PyModule_AddIntConstant(poModule, "INVENTORY_PAGE_COUNT", INVENTORY_PAGE_COUNT);
    PyModule_AddIntConstant(poModule, "INVENTORY_SLOT_COUNT", INVENTORY_MAX_NUM);
    PyModule_AddIntConstant(poModule, "ITEM_SLOT_COUNT", INVENTORY_MAX_NUM);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_SLOT_START", 0);
    PyModule_AddIntConstant(poModule, "EQUIPMENT_PAGE_COUNT", WEAR_MAX_NUM);

    PyModule_AddIntMacro(poModule, DRAGON_SOUL_EQUIP_SLOT_START);

    PyModule_AddIntConstant(poModule, "MBF_SKILL", CPythonPlayer::MBF_SKILL);
    PyModule_AddIntConstant(poModule, "MBF_ATTACK", CPythonPlayer::MBF_ATTACK);
    PyModule_AddIntConstant(poModule, "MBF_CAMERA", CPythonPlayer::MBF_CAMERA);
    PyModule_AddIntConstant(poModule, "MBF_SMART", CPythonPlayer::MBF_SMART);
    PyModule_AddIntConstant(poModule, "MBF_MOVE", CPythonPlayer::MBF_MOVE);
    PyModule_AddIntConstant(poModule, "MBF_AUTO", CPythonPlayer::MBF_AUTO);
    PyModule_AddIntConstant(poModule, "MBS_PRESS", CPythonPlayer::MBS_PRESS);
    PyModule_AddIntConstant(poModule, "MBS_CLICK", CPythonPlayer::MBS_CLICK);
    PyModule_AddIntConstant(poModule, "MBT_RIGHT", CPythonPlayer::MBT_RIGHT);
    PyModule_AddIntConstant(poModule, "MBT_LEFT", CPythonPlayer::MBT_LEFT);

    // Public code with server
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_NONE", SLOT_TYPE_NONE);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_INVENTORY", SLOT_TYPE_INVENTORY);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_SKILL", SLOT_TYPE_SKILL);
    // Special indexes for client
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_SHOP", SLOT_TYPE_SHOP);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_EXCHANGE_OWNER", SLOT_TYPE_EXCHANGE_OWNER);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_EXCHANGE_TARGET", SLOT_TYPE_EXCHANGE_TARGET);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_QUICK_SLOT", SLOT_TYPE_QUICK_SLOT);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_SAFEBOX", SLOT_TYPE_SAFEBOX);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_PRIVATE_SHOP", SLOT_TYPE_PRIVATE_SHOP);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_MALL", SLOT_TYPE_MALL);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_EMOTION", SLOT_TYPE_EMOTION);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_DRAGON_SOUL_INVENTORY", SLOT_TYPE_DRAGON_SOUL_INVENTORY);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_MYSHOP", SLOT_TYPE_MYSHOP);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_CHANGE_LOOK", SLOT_TYPE_CHANGE_LOOK);
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_ACCE", SLOT_TYPE_ACCE);
#ifdef ENABLE_GUILD_STORAGE
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_GUILDSTORAGE", SLOT_TYPE_GUILDSTORAGE);
#endif
#ifdef __OFFLINE_SHOP__
    PyModule_AddIntConstant(poModule, "SLOT_TYPE_OFFLINE_SHOP", SLOT_TYPE_OFFLINE_SHOP);
#endif

    PyModule_AddIntConstant(poModule, "ACCE_SLOT_TYPE_COMBINE", 0);
    PyModule_AddIntConstant(poModule, "ACCE_SLOT_TYPE_ABSORB", 1);
    PyModule_AddIntConstant(poModule, "ACCE_SLOT_TYPE_MAX", 2);
    PyModule_AddIntConstant(poModule, "ACCE_SLOT_LEFT", 0);
    PyModule_AddIntConstant(poModule, "ACCE_SLOT_RIGHT", 1);
    PyModule_AddIntConstant(poModule, "ACCE_SLOT_MAX", 3);
    PyModule_AddIntConstant(poModule, "ACCEREFINE", ACCEREFINE);

    PyModule_AddIntConstant(poModule, "RESERVED_WINDOW", RESERVED_WINDOW);
    PyModule_AddIntConstant(poModule, "INVENTORY", INVENTORY);
    PyModule_AddIntConstant(poModule, "EQUIPMENT", EQUIPMENT);
    PyModule_AddIntConstant(poModule, "SAFEBOX", SAFEBOX);
    PyModule_AddIntConstant(poModule, "MALL", MALL);
    PyModule_AddIntConstant(poModule, "DRAGON_SOUL_INVENTORY", DRAGON_SOUL_INVENTORY);
    PyModule_AddIntConstant(poModule, "GROUND", GROUND);
    PyModule_AddIntConstant(poModule, "NPC_SHOP", NPC_SHOP);
    PyModule_AddIntConstant(poModule, "PC_SHOP", PC_SHOP);
    PyModule_AddIntConstant(poModule, "SWITCHBOT", SWITCHBOT);

    PyModule_AddIntConstant(poModule, "ITEM_MONEY", -1);

    PyModule_AddIntConstant(poModule, "SKILL_SLOT_COUNT", SKILL_MAX_NUM);

    PyModule_AddIntConstant(poModule, "EFFECT_PICK", CPythonPlayer::EFFECT_PICK);

    PyModule_AddIntConstant(poModule, "METIN_SOCKET_TYPE_NONE", CPythonPlayer::METIN_SOCKET_TYPE_NONE);
    PyModule_AddIntConstant(poModule, "METIN_SOCKET_TYPE_SILVER", CPythonPlayer::METIN_SOCKET_TYPE_SILVER);
    PyModule_AddIntConstant(poModule, "METIN_SOCKET_TYPE_GOLD", CPythonPlayer::METIN_SOCKET_TYPE_GOLD);
    PyModule_AddIntConstant(poModule, "METIN_SOCKET_MAX_NUM", ITEM_SOCKET_MAX_NUM);
    PyModule_AddIntConstant(poModule, "ATTRIBUTE_SLOT_MAX_NUM", ITEM_ATTRIBUTE_MAX_NUM);

    PyModule_AddIntConstant(poModule, "REFINE_CANT", REFINE_CANT);
    PyModule_AddIntConstant(poModule, "REFINE_OK", REFINE_OK);
    PyModule_AddIntConstant(poModule, "REFINE_ALREADY_MAX_SOCKET_COUNT", REFINE_ALREADY_MAX_SOCKET_COUNT);
    PyModule_AddIntConstant(poModule, "REFINE_NEED_MORE_GOOD_SCROLL", REFINE_NEED_MORE_GOOD_SCROLL);

    PyModule_AddIntConstant(poModule, "REFINE_CANT_NO_DD_ITEM", REFINE_CANT_NO_DD_ITEM);
    PyModule_AddIntConstant(poModule, "REFINE_CANT_DD_ITEM", REFINE_CANT_DD_ITEM);

    PyModule_AddIntConstant(poModule, "REFINE_CANT_MAKE_SOCKET_ITEM", REFINE_CANT_MAKE_SOCKET_ITEM);
    PyModule_AddIntConstant(poModule, "REFINE_NOT_NEXT_GRADE_ITEM", REFINE_NOT_NEXT_GRADE_ITEM);
    PyModule_AddIntConstant(poModule, "REFINE_CANT_REFINE_METIN_TO_EQUIPMENT", REFINE_CANT_REFINE_METIN_TO_EQUIPMENT);
    PyModule_AddIntConstant(poModule, "REFINE_CANT_REFINE_ROD", REFINE_CANT_REFINE_ROD);
    PyModule_AddIntConstant(poModule, "ATTACH_METIN_CANT", ATTACH_METIN_CANT);
    PyModule_AddIntConstant(poModule, "ATTACH_METIN_OK", ATTACH_METIN_OK);
    PyModule_AddIntConstant(poModule, "ATTACH_METIN_NOT_MATCHABLE_ITEM", ATTACH_METIN_NOT_MATCHABLE_ITEM);
    PyModule_AddIntConstant(poModule, "ATTACH_METIN_NO_MATCHABLE_SOCKET", ATTACH_METIN_NO_MATCHABLE_SOCKET);
    PyModule_AddIntConstant(poModule, "ATTACH_METIN_NOT_EXIST_GOLD_SOCKET", ATTACH_METIN_NOT_EXIST_GOLD_SOCKET);
    PyModule_AddIntConstant(poModule, "ATTACH_METIN_CANT_ATTACH_TO_EQUIPMENT", ATTACH_METIN_CANT_ATTACH_TO_EQUIPMENT);
    PyModule_AddIntMacro(poModule, ATTACH_METIN_SEALED);

    PyModule_AddIntConstant(poModule, "DETACH_METIN_CANT", DETACH_METIN_CANT);
    PyModule_AddIntConstant(poModule, "DETACH_METIN_OK", DETACH_METIN_OK);

    // Party
    PyModule_AddIntConstant(poModule, "PARTY_STATE_NORMAL", CPythonPlayer::PARTY_ROLE_NORMAL);
    PyModule_AddIntConstant(poModule, "PARTY_STATE_LEADER", CPythonPlayer::PARTY_ROLE_LEADER);
    PyModule_AddIntConstant(poModule, "PARTY_STATE_ATTACKER", CPythonPlayer::PARTY_ROLE_ATTACKER);
    PyModule_AddIntConstant(poModule, "PARTY_STATE_TANKER", CPythonPlayer::PARTY_ROLE_TANKER);
    PyModule_AddIntConstant(poModule, "PARTY_STATE_BUFFER", CPythonPlayer::PARTY_ROLE_BUFFER);
    PyModule_AddIntConstant(poModule, "PARTY_STATE_SKILL_MASTER", CPythonPlayer::PARTY_ROLE_SKILL_MASTER);
    PyModule_AddIntConstant(poModule, "PARTY_STATE_BERSERKER", CPythonPlayer::PARTY_ROLE_BERSERKER);
    PyModule_AddIntConstant(poModule, "PARTY_STATE_DEFENDER", CPythonPlayer::PARTY_ROLE_DEFENDER);
    PyModule_AddIntConstant(poModule, "PARTY_STATE_MAX_NUM", CPythonPlayer::PARTY_ROLE_MAX_NUM);

    // Skill Index
    PyModule_AddIntConstant(poModule, "SKILL_INDEX_TONGSOL", c_iSkillIndex_Tongsol);
    PyModule_AddIntConstant(poModule, "SKILL_INDEX_FISHING", c_iSkillIndex_Fishing);
    PyModule_AddIntConstant(poModule, "SKILL_INDEX_MINING", c_iSkillIndex_Mining);
    PyModule_AddIntConstant(poModule, "SKILL_INDEX_MAKING", c_iSkillIndex_Making);
    PyModule_AddIntConstant(poModule, "SKILL_INDEX_COMBO", c_iSkillIndex_Combo);
    PyModule_AddIntConstant(poModule, "SKILL_INDEX_LANGUAGE1", c_iSkillIndex_Language1);
    PyModule_AddIntConstant(poModule, "SKILL_INDEX_LANGUAGE2", c_iSkillIndex_Language2);
    PyModule_AddIntConstant(poModule, "SKILL_INDEX_LANGUAGE3", c_iSkillIndex_Language3);
    PyModule_AddIntConstant(poModule, "SKILL_INDEX_POLYMORPH", c_iSkillIndex_Polymorph);
    PyModule_AddIntConstant(poModule, "SKILL_INDEX_RIDING", c_iSkillIndex_Riding);
    PyModule_AddIntConstant(poModule, "SKILL_INDEX_SUMMON", c_iSkillIndex_Summon);

    // PK Mode
    PyModule_AddIntConstant(poModule, "PK_MODE_PEACE", PK_MODE_PEACE);
    PyModule_AddIntConstant(poModule, "PK_MODE_REVENGE", PK_MODE_REVENGE);
    PyModule_AddIntConstant(poModule, "PK_MODE_FREE", PK_MODE_FREE);
    PyModule_AddIntConstant(poModule, "PK_MODE_PROTECT", PK_MODE_PROTECT);
    PyModule_AddIntConstant(poModule, "PK_MODE_GUILD", PK_MODE_GUILD);
    PyModule_AddIntConstant(poModule, "PK_MODE_MAX_NUM", PK_MODE_MAX_NUM);

    // Block Mode
    PyModule_AddIntConstant(poModule, "BLOCK_EXCHANGE", BLOCK_EXCHANGE);
    PyModule_AddIntConstant(poModule, "BLOCK_PARTY", BLOCK_PARTY_INVITE);
    PyModule_AddIntConstant(poModule, "BLOCK_GUILD", BLOCK_GUILD_INVITE);
    PyModule_AddIntConstant(poModule, "BLOCK_WHISPER", BLOCK_WHISPER);
    PyModule_AddIntConstant(poModule, "BLOCK_FRIEND", BLOCK_MESSENGER_INVITE);
    PyModule_AddIntConstant(poModule, "BLOCK_PARTY_REQUEST", BLOCK_PARTY_REQUEST);
    PyModule_AddIntConstant(poModule, "BLOCK_VIEW_EQUIP", BLOCK_VIEW_EQUIP);
    PyModule_AddIntConstant(poModule, "BLOCK_VIEW_SHOPS", BLOCK_VIEW_SHOPS);
    PyModule_AddIntConstant(poModule, "BLOCK_CHAT", BLOCK_CHAT);
    PyModule_AddIntConstant(poModule, "BLOCK_DUEL_REQUEST", BLOCK_DUEL_REQUEST);

    // Party
    PyModule_AddIntConstant(poModule, "PARTY_EXP_NON_DISTRIBUTION", PARTY_EXP_DISTRIBUTION_NON_PARITY);
    PyModule_AddIntConstant(poModule, "PARTY_EXP_DISTRIBUTION_PARITY", PARTY_EXP_DISTRIBUTION_PARITY);

    // Emotion
    PyModule_AddIntConstant(poModule, "EMOTION_CLAP", EMOTION_CLAP);
    PyModule_AddIntConstant(poModule, "EMOTION_CHEERS_1", EMOTION_CHEERS_1);
    PyModule_AddIntConstant(poModule, "EMOTION_CHEERS_2", EMOTION_CHEERS_2);
    PyModule_AddIntConstant(poModule, "EMOTION_DANCE_1", EMOTION_DANCE_1);
    PyModule_AddIntConstant(poModule, "EMOTION_DANCE_2", EMOTION_DANCE_2);
    PyModule_AddIntConstant(poModule, "EMOTION_DANCE_3", EMOTION_DANCE_3);
    PyModule_AddIntConstant(poModule, "EMOTION_DANCE_4", EMOTION_DANCE_4);
    PyModule_AddIntConstant(poModule, "EMOTION_DANCE_5", EMOTION_DANCE_5);
    PyModule_AddIntConstant(poModule, "EMOTION_DANCE_6", EMOTION_DANCE_6); // PSY 강남스타일
    PyModule_AddIntConstant(poModule, "EMOTION_DANCE_7", EMOTION_DANCE_7); // PSY 강남스타일
    PyModule_AddIntConstant(poModule, "EMOTION_CONGRATULATION", EMOTION_CONGRATULATION);
    PyModule_AddIntConstant(poModule, "EMOTION_FORGIVE", EMOTION_FORGIVE);
    PyModule_AddIntConstant(poModule, "EMOTION_ANGRY", EMOTION_ANGRY);
    PyModule_AddIntConstant(poModule, "EMOTION_ATTRACTIVE", EMOTION_ATTRACTIVE);
    PyModule_AddIntConstant(poModule, "EMOTION_SAD", EMOTION_SAD);
    PyModule_AddIntConstant(poModule, "EMOTION_SHY", EMOTION_SHY);
    PyModule_AddIntConstant(poModule, "EMOTION_CHEERUP", EMOTION_CHEERUP);
    PyModule_AddIntConstant(poModule, "EMOTION_BANTER", EMOTION_BANTER);
    PyModule_AddIntConstant(poModule, "EMOTION_JOY", EMOTION_JOY);
    PyModule_AddIntConstant(poModule, "EMOTION_THROW_MONEY", EMOTION_THROW_MONEY);

    PyModule_AddIntConstant(poModule, "EMOTION_PUSH_UP", EMOTION_PUSH_UP);
    PyModule_AddIntConstant(poModule, "EMOTION_EXERCISE", EMOTION_EXERCISE);
    PyModule_AddIntConstant(poModule, "EMOTION_DOZE", EMOTION_DOZE);
    PyModule_AddIntConstant(poModule, "EMOTION_SELFIE", EMOTION_SELFIE);
    PyModule_AddIntConstant(poModule, "EMOTION_CHARGING", EMOTION_CHARGING);
    PyModule_AddIntConstant(poModule, "EMOTION_WEATHER_1", EMOTION_WEATHER_1);
    PyModule_AddIntConstant(poModule, "EMOTION_WEATHER_2", EMOTION_WEATHER_2);
    PyModule_AddIntConstant(poModule, "EMOTION_WEATHER_3", EMOTION_WEATHER_3);
    PyModule_AddIntConstant(poModule, "EMOTION_HUNGRY", EMOTION_HUNGRY);
    PyModule_AddIntConstant(poModule, "EMOTION_SIREN", EMOTION_SIREN);
    PyModule_AddIntConstant(poModule, "EMOTION_LETTER", EMOTION_LETTER);
    PyModule_AddIntConstant(poModule, "EMOTION_CALL", EMOTION_CALL);
    PyModule_AddIntConstant(poModule, "EMOTION_CELEBRATION", EMOTION_CELEBRATION);
    PyModule_AddIntConstant(poModule, "EMOTION_ALCOHOL", EMOTION_ALCOHOL);
    PyModule_AddIntConstant(poModule, "EMOTION_BUSY", EMOTION_BUSY);
    PyModule_AddIntConstant(poModule, "EMOTION_NOSAY", EMOTION_NOSAY);
    PyModule_AddIntConstant(poModule, "EMOTION_WHIRL", EMOTION_WHIRL);

    PyModule_AddIntConstant(poModule, "EMOTION_KISS", EMOTION_KISS);
    PyModule_AddIntConstant(poModule, "EMOTION_FRENCH_KISS", EMOTION_FRENCH_KISS);
    PyModule_AddIntConstant(poModule, "EMOTION_SLAP", EMOTION_SLAP);

    //// 자동물약 타입
    PyModule_AddIntConstant(poModule, "AUTO_POTION_TYPE_HP", CPythonPlayer::AUTO_POTION_TYPE_HP);
    PyModule_AddIntConstant(poModule, "AUTO_POTION_TYPE_SP", CPythonPlayer::AUTO_POTION_TYPE_SP);

    // 용혼석
    PyModule_AddIntConstant(poModule, "DRAGON_SOUL_PAGE_SIZE", 32);
    PyModule_AddIntConstant(poModule, "DRAGON_SOUL_PAGE_COUNT", DRAGON_SOUL_GRADE_MAX);
    PyModule_AddIntConstant(poModule, "DRAGON_SOUL_SLOT_COUNT", DS_SLOT_MAX * DRAGON_SOUL_GRADE_MAX * 32);
    PyModule_AddIntConstant(poModule, "DRAGON_SOUL_EQUIPMENT_SLOT_START", DRAGON_SOUL_EQUIP_SLOT_START);
    PyModule_AddIntConstant(poModule, "DRAGON_SOUL_EQUIPMENT_PAGE_COUNT", DRAGON_SOUL_DECK_MAX_NUM);
    PyModule_AddIntConstant(poModule, "DRAGON_SOUL_EQUIPMENT_FIRST_SIZE", DS_SLOT_MAX);

    // 용혼석 개량창
    PyModule_AddIntConstant(poModule, "DRAGON_SOUL_REFINE_CLOSE", DS_SUB_HEADER_CLOSE);
    PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_DO_UPGRADE", DS_SUB_HEADER_DO_REFINE_GRADE);
    PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_DO_IMPROVEMENT", DS_SUB_HEADER_DO_REFINE_STEP);
    PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_DO_REFINE", DS_SUB_HEADER_DO_REFINE_STRENGTH);

#ifdef ENABLE_SKILLBOOK_COMBINATION
	PyModule_AddIntConstant(poModule, "SKILLBOOK_COMB_SLOT_MAX", 12);
#endif
    PyModule_AddIntConstant(poModule, "POINT_UPBRINGING_PET_HP", 0);
    PyModule_AddIntConstant(poModule, "POINT_UPBRINGING_PET_DEF_GRADE", 1);
    PyModule_AddIntConstant(poModule, "POINT_UPBRINGING_PET_SP", 2);
    PyModule_AddIntConstant(poModule, "POINT_UPBRINGING_PET_LEVEL", 0);
    PyModule_AddIntConstant(poModule, "POINT_UPBRINGING_PET_LEVEL_STEP", 1);
    PyModule_AddIntConstant(poModule, "POINT_UPBRINGING_PET_EXP", 2);
    PyModule_AddIntConstant(poModule, "POINT_UPBRINGING_PET_NEXT_EXP", 3);
    PyModule_AddIntConstant(poModule, "POINT_UPBRINGING_PET_EVOL_LEVEL", 6);
    PyModule_AddIntConstant(poModule, "POINT_UPBRINGING_PET_TYPE", 0);
    PyModule_AddIntConstant(poModule, "PET_FEED_SLOT_MAX", 9);
    // Flash Button events on pet
    PyModule_AddIntConstant(poModule, "FEED_LIFE_TIME_EVENT", 0);
    PyModule_AddIntConstant(poModule, "FEED_EVOL_EVENT", 1);
    PyModule_AddIntConstant(poModule, "FEED_EXP_EVENT", 2);
    PyModule_AddIntConstant(poModule, "FEED_BUTTON_MAX", 3);

    PyModule_AddIntConstant(poModule, "QUICK_SLOT_POS_ERROR", 0);
    PyModule_AddIntConstant(poModule, "QUICK_SLOT_ITEM_USE_SUCCESS", 1);
    PyModule_AddIntConstant(poModule, "QUICK_SLOT_IS_NOT_ITEM", 2);
    PyModule_AddIntConstant(poModule, "QUICK_SLOT_PET_ITEM_USE_SUCCESS", 3);
    PyModule_AddIntConstant(poModule, "QUICK_SLOT_PET_ITEM_USE_FAILED", 4);
    PyModule_AddIntConstant(poModule, "QUICK_SLOT_CAN_NOT_USE_PET_ITEM", 5);

    PyModule_AddIntConstant(poModule, "PET_GROWTH_SKILL_LEVEL_MAX", 20);
    PyModule_AddIntConstant(poModule, "PET_GROWTH_SKILL_OPEN_EVOL_LEVEL", 4);
    PyModule_AddIntConstant(poModule, "PET_GROWTH_EVOL_MAX", 4);
    PyModule_AddIntConstant(poModule, "SPECIAL_EVOL_MIN_AGE", 2592000);
    PyModule_AddIntConstant(poModule, "LIFE_TIME_FLASH_MIN_TIME", 3600);
    PyModule_AddIntConstant(poModule, "PET_SKILL_COUNT_MAX", 3);

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	PyModule_AddIntConstant(poModule, "EX_INVENTORY_PAGE_COUNT", 3);
	PyModule_AddIntConstant(poModule, "EX_INVENTORY_PAGE_START", 2);
	PyModule_AddIntConstant(poModule, "EX_INVENTORY_STAGE_MAX", 9);
#endif

    PyModule_AddIntConstant(poModule, "SKILLBOOK_COMB_SLOT_MAX", 10);

    PyModule_AddIntConstant(poModule, "BELT_INVENTORY", BELT_INVENTORY);

    PyModule_AddIntConstant(poModule, "COMB_WND_SLOT_MEDIUM", COMB_WND_SLOT_MEDIUM);
    PyModule_AddIntConstant(poModule, "COMB_WND_SLOT_BASE", COMB_WND_SLOT_BASE);
    PyModule_AddIntConstant(poModule, "COMB_WND_SLOT_MATERIAL", COMB_WND_SLOT_MATERIAL);
    PyModule_AddIntConstant(poModule, "COMB_WND_SLOT_RESULT", COMB_WND_SLOT_RESULT);
    PyModule_AddIntConstant(poModule, "COMB_WND_SLOT_MAX", COMB_WND_SLOT_MAX);

    PyModule_AddIntConstant(poModule, "ON_TOP_WND_NONE", ON_TOP_WND_NONE);
    PyModule_AddIntConstant(poModule, "ON_TOP_WND_SHOP", ON_TOP_WND_SHOP);
    PyModule_AddIntConstant(poModule, "ON_TOP_WND_EXCHANGE", ON_TOP_WND_EXCHANGE);
    PyModule_AddIntConstant(poModule, "ON_TOP_WND_SAFEBOX", ON_TOP_WND_SAFEBOX);
    PyModule_AddIntConstant(poModule, "ON_TOP_WND_PRIVATE_SHOP", ON_TOP_WND_PRIVATE_SHOP);
    PyModule_AddIntConstant(poModule, "ON_TOP_WND_ITEM_COMB", ON_TOP_WND_ITEM_COMB);
    PyModule_AddIntConstant(poModule, "ON_TOP_WND_PET_FEED", ON_TOP_WND_PET_FEED);
    PyModule_AddIntConstant(poModule, "ON_TOP_WND_SWITCHBOT", ON_TOP_WND_SWITCHBOT);
    PyModule_AddIntConstant(poModule, "ON_TOP_WND_REFINE", ON_TOP_WND_REFINE);
    PyModule_AddIntMacro(poModule, ON_TOP_WND_SHOP_BUILD);

    PyModule_AddIntConstant(poModule, "CHARACTER_NAME_MAX_LEN", CHARACTER_NAME_MAX_LEN);

    PyModule_AddIntConstant(poModule, "DIR_UP", CPythonPlayer::DIR_UP);
    PyModule_AddIntConstant(poModule, "DIR_DOWN", CPythonPlayer::DIR_DOWN);
    PyModule_AddIntConstant(poModule, "DIR_LEFT", CPythonPlayer::DIR_LEFT);
    PyModule_AddIntConstant(poModule, "DIR_RIGHT", CPythonPlayer::DIR_RIGHT);

    PyModule_AddIntConstant(poModule, "MINIGAME_TYPE_RUMI", 0);
    PyModule_AddIntConstant(poModule, "MINIGAME_ATTENDANCE", 1);
    PyModule_AddIntConstant(poModule, "MINIGAME_MONSTERBACK", 3);
    PyModule_AddIntConstant(poModule, "MINIGAME_FISH", 2);
    PyModule_AddIntConstant(poModule, "MINIGAME_YUTNORI", 4);
    PyModule_AddIntConstant(poModule, "MINIGAME_CATCHKING", 5);
    PyModule_AddIntConstant(poModule, "MINIGAME_FINDM", 6);
    PyModule_AddIntConstant(poModule, "MINIGAME_TYPE_MAX", 7);

    PyModule_AddIntConstant(poModule, "ROULETTE_GC_OPEN", 0);
    PyModule_AddIntConstant(poModule, "ROULETTE_GC_START", 1);
    PyModule_AddIntConstant(poModule, "ROULETTE_GC_REQUEST", 2);
    PyModule_AddIntConstant(poModule, "ROULETTE_GC_END", 3);
    PyModule_AddIntConstant(poModule, "ROULETTE_GC_CLOSE", 4);
}

void SetItemSlot(py::object p, uint32_t slot, const ClientItemData* item)
{
    UI::CSlotWindow *wnd = (UI::CSlotWindow *)PyCapsule_GetPointer((PyObject*)p.ptr(), nullptr);
    if (!wnd)
        return;

    if (item->vnum == 0) {
		wnd->ClearSlot(slot);
		return;
	}

    auto* proto = CItemManager::Instance().GetProto(item->vnum);
    if (!proto)
        return;

    DirectX::SimpleMath::Color diffuseColor(1.0, 1.0, 1.0, 1.0);
    wnd->SetSlot(slot, item->vnum, 1, proto->GetSize(), proto->GetIconImage(), diffuseColor);

    if (1 == item->count)
        wnd->SetSlotCount(slot, 0);
    else
        wnd->SetSlotCount(slot, item->count);

    if (item->transVnum != 0 && !(proto->GetType() == ITEM_TOGGLE && proto->GetSubType() == TOGGLE_LEVEL_PET))
    {
        wnd->SetSlotCoverImage(slot, "icon/item/ingame_convert_Mark.tga");
        wnd->EnableSlotCoverImage(slot, true);
    }
    else { wnd->EnableSlotCoverImage(slot, false); }

    if (proto->GetType() == ITEM_DS)
    {
        wnd->EnableSlot(slot);

        for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
        {
            const auto &limit = proto->GetLimit(i);
            if (!limit)
                continue;
            auto remain_time = 999;
            if (LIMIT_REAL_TIME == limit.value().bType)
                remain_time = item->sockets[0];
            else if (LIMIT_REAL_TIME_START_FIRST_USE == limit.value().bType)
                remain_time = item->sockets[0];
            else if (LIMIT_TIMER_BASED_ON_WEAR == limit.value().bType)
                remain_time = item->sockets[0];

            if (remain_time <= 0)
                wnd->DisableSlot(slot);
        }
    }

    if (proto->GetType() == ITEM_TOGGLE)
    {
        if (item->GetSocket(ITEM_SOCKET_TOGGLE_ACTIVE))
        {
            switch (proto->GetSubType())
            {
            case TOGGLE_PET:
                wnd->SetDiffuseColor(slot, UI::SLOT_COLOR_TYPE_GREEN);
                break;
            case TOGGLE_MOUNT:
                wnd->SetDiffuseColor(slot, UI::SLOT_COLOR_TYPE_SKY);
                break;
            case TOGGLE_AUTO_RECOVERY_SP:
                wnd->SetDiffuseColor(slot, UI::SLOT_COLOR_TYPE_BLUE);
                break;
            case TOGGLE_AUTO_RECOVERY_HP:
                wnd->SetDiffuseColor(slot, UI::SLOT_COLOR_TYPE_RED);
                break;

            default:
                wnd->SetDiffuseColor(slot, 8);
                break;
            }
            wnd->ActivateSlot(slot);
        }
        else {
            if (item->highlighted == 1)
            {
                wnd->SetDiffuseColor(slot, 8);
                wnd->ActivateSlot(slot);
            }
            else
            {
                wnd->DeactivateSlot(slot);
            }
        }
    }
    else
    {
        if(item->highlighted == 1)
            wnd->ActivateSlot(slot);
        else
            wnd->DeactivateSlot(slot);
    }
}

void OverItemSlot(py::object p, uint32_t slot, const ClientItemData* item)
{
    UI::CSlotWindow *wnd = (UI::CSlotWindow *)PyCapsule_GetPointer((PyObject*)p.ptr(), nullptr);
    if (!wnd)
        return;

    const auto proto = CItemManager::Instance().GetProto(item->vnum);
    if (!proto)
        return;

    if(item->highlighted)
    {
        if (proto->GetType() == ITEM_TOGGLE) {
            if(!item->GetSocket(ITEM_SOCKET_TOGGLE_ACTIVE)) {
                wnd->DeactivateSlot(slot);
            }
        } else {
            wnd->DeactivateSlot(slot);
        }
        const_cast<ClientItemData*>(item)->highlighted = 0;
    }

}


void init_player(py::module &m)
{
    using namespace pybind11::literals;

    py::module player = m.def_submodule("player", "Provides access to the player instance");
    py::module playerenums = player.def_submodule("enums", "Provides access to the player enums");

#define pybind11_enum_macro(c) value(#c, c)
    py::enum_<EOnTopWindows>(playerenums, "OnTopWindow")
        .pybind11_enum_macro(ON_TOP_WND_NONE)
        .pybind11_enum_macro(ON_TOP_WND_SHOP)
        .pybind11_enum_macro(ON_TOP_WND_EXCHANGE)
        .pybind11_enum_macro(ON_TOP_WND_SAFEBOX)
        .pybind11_enum_macro(ON_TOP_WND_PRIVATE_SHOP)
        .pybind11_enum_macro(ON_TOP_WND_ITEM_COMB)
        .pybind11_enum_macro(ON_TOP_WND_PET_FEED)
        .pybind11_enum_macro(ON_TOP_WND_SWITCHBOT)
        .pybind11_enum_macro(ON_TOP_WND_REFINE)
        .pybind11_enum_macro(ON_TOP_WND_SHOP_BUILD)
        .export_values();

#undef pybind11_enum_macro

    player.def("SetItemSlot", &SetItemSlot, py::arg("p"),  py::arg("slot"), py::arg("item"));
    player.def("OverItemSlot", &OverItemSlot);

    py::class_<TQuickslot>(player, "QuickSlot")
        .def(py::init<>())
        .def_readwrite("Type", &TQuickslot::type)
        .def_readwrite("Position", &TQuickslot::pos);

    py::class_<SwitchBotSlotData>(player, "SwitchbotSlotData")
        .def(py::init<>())
        .def_readwrite("slot", &SwitchBotSlotData::slot)
        .def_readwrite("status", &SwitchBotSlotData::status)
        .def_property_readonly(
            "attr", [](
            const SwitchBotSlotData &c) -> const std::array<std::array<TItemApply, ITEM_ATTRIBUTE_MAX_NUM>, 3>&
            {
                return reinterpret_cast<const std::array<std::array<TItemApply, ITEM_ATTRIBUTE_MAX_NUM>, 3> &>(c.attr);
            }
            );

    py::class_<HuntingMission>(player, "HuntingMission")
        .def(py::init<>())
        .def_readwrite("id", &HuntingMission::id)
        .def_readwrite("mobVnums", &HuntingMission::mobVnums)
        .def_readwrite("needCount", &HuntingMission::needCount)
        .def_readwrite("minLevel", &HuntingMission::minLevel)
        .def_readwrite("isDaily", &HuntingMission::isDaily)
        .def_readwrite("rewards", &HuntingMission::rewards)
        .def_readwrite("required", &HuntingMission::required)
                .def_property_readonly(
            "name", [](
            const HuntingMission &c) 
            {
                return py::bytes(c.name);
            }
            );
    ;

    py::class_<TPlayerHuntingMission>(player, "PlayerHuntingMission")
        .def(py::init<>())
        .def_readwrite("playerId", &TPlayerHuntingMission::playerId)
        .def_readwrite("missionId", &TPlayerHuntingMission::missionId)
        .def_readwrite("killCount", &TPlayerHuntingMission::killCount)
        .def_readwrite("day", &TPlayerHuntingMission::day)
        .def_readwrite("bCompleted", &TPlayerHuntingMission::bCompleted);

    py::class_<HuntingReward>(player, "HuntingReward")
        .def(py::init<>())
        .def_readwrite("vnum", &HuntingReward::vnum)
        .def_readwrite("count", &HuntingReward::count);

  py::class_<CPythonPlayer, std::unique_ptr<CPythonPlayer, py::nodelete>>(player, "playerInst")
        .def(py::init([]()
        {
            return std::unique_ptr<CPythonPlayer, py::nodelete>(CPythonPlayer::InstancePtr());
        }), pybind11::return_value_policy::reference_internal)
        .def("GetItemIndex", &CPythonPlayer::GetItemIndex)
        .def("GetItemVnum", &CPythonPlayer::GetItemIndex)

        .def("GetItemValue", &CPythonPlayer::GetItemValue)
        .def("GetItemCount", &CPythonPlayer::GetItemCount)
        .def("GetEmptyInventory", &CPythonPlayer::GetEmptyInventory, "size"_a, "type"_a, "except"_a = -1)
        .def("GetItemCountByVnum", &CPythonPlayer::GetItemCountByVnum)
        .def("GetItemMetinSocket", &CPythonPlayer::GetItemMetinSocket)
        .def("GetItemAttributes", &CPythonPlayer::GetItemAttributes)
        .def("GetItemAttribute", &CPythonPlayer::GetItemAttributeNew)
        .def("GetItemData", &CPythonPlayer::GetItemData, pybind11::return_value_policy::reference)
        .def("GetItemTypeBySlot", &CPythonPlayer::GetItemTypeBySlot)
        .def("GetItemSubTypeBySlot", &CPythonPlayer::GetItemSubTypeBySlot)
        .def("GetItemFlags", &CPythonPlayer::GetItemFlags)
        .def("GetItemLink", &CPythonPlayer::GetItemLink)
        .def("GetItemAntiFlags", &CPythonPlayer::GetItemAntiFlags)
        .def("GetMainCharacterIndex", &CPythonPlayer::GetMainCharacterIndex)
        .def("GetSkillSlotIndexNew", &CPythonPlayer::GetSkillSlotIndexNew)
            
        .def("GetSkillCurrentEfficientPercentage", &CPythonPlayer::GetSkillCurrentEfficientPercentage)
        .def("GetSkillNextEfficientPercentage", &CPythonPlayer::GetSkillNextEfficientPercentage)
        .def("GetSkillIndex", &CPythonPlayer::GetSkillIndex)
        .def("GetSkillGrade", &CPythonPlayer::GetSkillGrade)
        .def("GetSkillLevel", &CPythonPlayer::GetSkillLevel)
        .def("GetSkillDisplayLevel", &CPythonPlayer::GetSkillDisplayLevel)
        .def("GetSkillColor", &CPythonPlayer::GetSkillColor)
        .def("GetSkillCoolTime", &CPythonPlayer::GetSkillCoolTime)


            

        .def("PickCloseItem", &CPythonPlayer::PickCloseItem)
        .def("SetGameWindow", &CPythonPlayer::SetGameWindow)

        .def("CanRefine", &CPythonPlayer::CanRefine)
        .def("CanAttachMetin", &CPythonPlayer::CanAttachMetin)
        .def("CanDettachMetin", &CPythonPlayer::CanDetachMetin)
        .def("CanUnlockTreasure", &CPythonPlayer::CanUnlockTreasure)
        .def("CanSealItem", &CPythonPlayer::CanSealItem)
        .def("GetPoint", &CPythonPlayer::GetStatus)
        .def("GetGold", &CPythonPlayer::GetGold)
        .def("SetPoint", &CPythonPlayer::SetStatus)
        .def("IsFishing", &CPythonPlayer::IsFishing)

            

        .def("GetHuntingMissions", &CPythonPlayer::GetHuntingMissions, pybind11::return_value_policy::reference)
        .def("GetHuntingMission", &CPythonPlayer::GetHuntingMission, pybind11::return_value_policy::reference)

        .def("ChangeBlockMode", &CPythonPlayer::ChangeBlockMode)
        .def("GetBlockMode", &CPythonPlayer::GetBlockMode)
        .def("ShowPopUp", &CPythonPlayer::ShowPopUp)
        .def("SetPreviewTitle", &CPythonPlayer::SetTitlePreview)
        .def("GetTargetVID", &CPythonPlayer::GetTargetVID)
        .def("IsMainCharacterIndex", &CPythonPlayer::IsMainCharacterIndex)
        .def("GetGlobalQuickSlot", &CPythonPlayer::GetGlobalQuickSlot)
        .def("GetLocalQuickSlot", &CPythonPlayer::GetLocalQuickSlot)
        .def("GetSwitchbotSlotData", &CPythonPlayer::GetSwitchbotSlotData)
        .def("SetSwitchbotSlotAttribute", &CPythonPlayer::SetSwitchbotSlotAttribute)
        .def("SetSwitchbotSlotStatus", &CPythonPlayer::SetSwitchbotSlotStatus)
        .def("GetChangeLookWindowOpen", &CPythonPlayer::GetChangeLookWindowOpen)
        .def("SetChangeLookWindowOpen", &CPythonPlayer::SetChangeLookWindowOpen)
        .def("GetChangeLookData", &CPythonPlayer::GetChangeLookData)
        .def("GetChangeLookItemInvenSlot", &CPythonPlayer::GetChangeLookItemInvenSlot)
        .def("GetAcceItemData", &CPythonPlayer::GetAcceItemData, pybind11::return_value_policy::reference)
        .def("__repr__",
             [](const CPythonPlayer &a)
             {
                 return fmt::format("<pygame.player.playerInst named '{}'>", a.GetName());
             }
            )

        .def("GetMainCharacterPosition", [](const CPythonPlayer &a)
        {
            TPixelPosition kPPosMainActor;
            a.NEW_GetMainActorPosition(&kPPosMainActor);
            return py::make_tuple(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
        });
}
