#include "StdAfx.h"
#include <game/MotionConstants.hpp>
#include "PythonNetworkStream.h"
#include "../GameLib/NpcManager.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"
#include "InstanceBase.h"

#define ishan(ch)		(((ch) & 0xE0) > 0x90)
#define ishanasc(ch)	(isascii(ch) || ishan(ch))
#define ishanalp(ch)	(isalpha(ch) || ishan(ch))
#define isnhdigit(ch)	(!ishan(ch) && isdigit(ch))
#define isnhspace(ch)	(!ishan(ch) && isspace(ch))

#define LOWER(c)		(((c) >= 'A' && (c) <= 'Z') ? ((c) + ('a' - 'A')) : (c))
#define UPPER(c)		(((c) >= 'a' && (c) <= 'z') ? ((c) + ('A' - 'a')) : (c))

void SkipSpaces(char **string)
{
    for (; **string != '\0' && isnhspace((unsigned char) **string); ++(*string));
}

char *OneArgument(char *argument, char *first_arg)
{
    char mark = FALSE;

    if (!argument)
    {
        *first_arg = '\0';
        return NULL;
    }

    SkipSpaces(&argument);

    while (*argument)
    {
        if (*argument == '\"')
        {
            mark = !mark;
            ++argument;
            continue;
        }

        if (!mark && isnhspace((unsigned char) *argument))
            break;

        *(first_arg++) = LOWER(*argument);
        ++argument;
    }

    *first_arg = '\0';

    SkipSpaces(&argument);
    return (argument);
}

bool CPythonNetworkStream::ClientCommand(const char *c_szCommand)
{
    return false;
}

bool SplitToken(const char *c_szLine, CTokenVector *pstTokenVector, const char *c_szDelimeter = " ")
{
    pstTokenVector->reserve(10);
    pstTokenVector->clear();

    std::string strLine = c_szLine;

    uint32_t basePos = 0;

    do
    {
        int beginPos = strLine.find_first_not_of(c_szDelimeter, basePos);
        if (beginPos < 0)
            return false;

        int endPos;

        if (strLine[beginPos] == '"')
        {
            ++beginPos;
            endPos = strLine.find_first_of('\"', beginPos);

            if (endPos < 0)
                return false;

            basePos = endPos + 1;
        }
        else
        {
            endPos = strLine.find_first_of(c_szDelimeter, beginPos);
            basePos = endPos;
        }

        pstTokenVector->push_back(strLine.substr(beginPos, endPos - beginPos));

        // 추가 코드. 맨뒤에 탭이 있는 경우를 체크한다. - [levites]
        if (int(strLine.find_first_not_of(c_szDelimeter, basePos)) < 0)
            break;
    }
    while (basePos < strLine.length());

    return true;
}

void CPythonNetworkStream::ServerCommand(const char *c_szCommand)
{
    if (m_apoPhaseWnd[PHASE_WINDOW_GAME] && !py::isinstance<py::none>(m_apoPhaseWnd[PHASE_WINDOW_GAME]))
    {
        if (PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],"BINARY_ServerCommand_Run", py::bytes(c_szCommand)))
            return;
    }
    else if (m_poSerCommandParserWnd && !py::isinstance<py::none>(m_poSerCommandParserWnd))
    {
        if (PyCallClassMemberFunc(m_poSerCommandParserWnd,
                                  "BINARY_ServerCommand_Run",
                                  py::bytes(c_szCommand)))
            return;
    }

 // SPDLOG_ERROR("Unhandled Server Command: {}", c_szCommand);

    CTokenVector TokenVector;
    if (!SplitToken(c_szCommand, &TokenVector))
        return;
    if (TokenVector.empty())
        return;

    const char *szCmd = TokenVector[0].c_str();

    if (!strcmpi(szCmd, "quit"))
    {
        //SPDLOG_ERROR(" Command quit");
        PostQuitMessage(0);
    }
        /*else if (!strcmpi(szCmd, "BettingMoney"))
        {
            if (2 != TokenVector.size())
            {
                SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
                return;
            }
    
            //UINT uMoney= atoi(TokenVector[1].c_str());		
            
        }*/
        // GIFT NOTIFY
    else if (!strcmpi(szCmd, "gift"))
    {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Gift_Show");
    }
        // CUBE
    else if (!strcmpi(szCmd, "cube"))
    {
        if (TokenVector.size() < 2)
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand, TokenVector.size());
            return;
        }

        if ("open" == TokenVector[1])
        {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Open");
        }
        else if ("close" == TokenVector[1])
        {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Close");
        }
        else if ("success" == TokenVector[1])
        {
            if (4 != TokenVector.size())
            {
                SPDLOG_ERROR(
                    "CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                    c_szCommand);
                return;
            }

            UINT itemVnum = atoi(TokenVector[2].c_str());
            UINT count = atoi(TokenVector[3].c_str());
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Succeed", itemVnum, count);
        }
        else if ("fail" == TokenVector[1])
        {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Failed");
        }
    }
        // CUEBE_END
    else if (!strcmpi(szCmd, "ObserverCount"))
    {
        if (2 != TokenVector.size())
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand);
            return;
        }

        UINT uObserverCount = atoi(TokenVector[1].c_str());

        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_BettingGuildWar_UpdateObserverCount",
                              uObserverCount
            );
    }
    else if (!strcmpi(szCmd, "ShopSearchError"))
    {
        if (2 != TokenVector.size())
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand);
            return;
        }

        const auto errorCode = atoi(TokenVector[1].c_str());

        switch (errorCode)
        {
        case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", "NOT_ENOUGH_MONEY");
            break;

            //case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX:
            //	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY_EX"));
            //	break;

        case SHOP_SUBHEADER_GC_SOLDOUT:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", "SOLDOUT");
            break;

        case SHOP_SUBHEADER_GC_INVENTORY_FULL:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", "INVENTORY_FULL");
            break;

        case SHOP_SUBHEADER_GC_INVALID_POS:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", "INVALID_POS");
            break;
        }
    }
    else if (!strcmpi(szCmd, "ObserverMode"))
    {
        if (2 != TokenVector.size())
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand);
            return;
        }

        UINT uMode = atoi(TokenVector[1].c_str());

        CPythonPlayer &rkPlayer = CPythonPlayer::Instance();
        rkPlayer.SetObserverMode(uMode ? true : false);

        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_BettingGuildWar_SetObserverMode",
                              uMode
            );
    }
    else if (!strcmpi(szCmd, "ObserverTeamInfo"))
    {
    }
    else if (!strcmpi(szCmd, "StoneDetect"))
    {
        if (4 != TokenVector.size())
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand);
            return;
        }

        // vid distance(1-3) angle(0-360)
        uint32_t dwVID = atoi(TokenVector[1].c_str());
        uint8_t byDistance = atoi(TokenVector[2].c_str());
        float fAngle = atof(TokenVector[3].c_str());
        fAngle = fmod(540.0f - fAngle, 360.0f);
        SPDLOG_DEBUG("StoneDetect [VID:{0}] [Distance:{1}] [Angle:{2}->{3}]\n", dwVID, byDistance,
                     atoi(TokenVector[3].c_str()), fAngle);

        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();

        CInstanceBase *pInstance = rkChrMgr.GetInstancePtr(dwVID);
        if (!pInstance)
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Not Exist Instance", c_szCommand);
            return;
        }

        TPixelPosition PixelPosition;
        Vector3 v3Rotation(0.0f, 0.0f, fAngle);
        pInstance->NEW_GetPixelPosition(&PixelPosition);

        PixelPosition.y *= -1.0f;

        std::string filename;
        switch (byDistance)
        {
        case 0:
            filename = "d:/ymir work/effect/etc/firecracker/find_out.mse";
            break;
        case 1:
            filename = "d:/ymir work/effect/etc/compass/appear_small.mse";
            break;
        case 2:
            filename = "d:/ymir work/effect/etc/compass/appear_middle.mse";
            break;
        case 3:
            filename = "d:/ymir work/effect/etc/compass/appear_large.mse";
            break;
        default:
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Distance", c_szCommand);
            break;
        }

        if (!filename.empty())
        {
            auto &mgr = CEffectManager::Instance();

            mgr.RegisterEffect(filename.c_str());

            const auto index = mgr.CreateEffect(filename.c_str());

            Matrix mat = Matrix::CreateRotationZ(XMConvertToRadians(fAngle));
            mat._41 = PixelPosition.x;
            mat._42 = PixelPosition.y;
            mat._43 = PixelPosition.z;

            auto* effect = mgr.GetEffectInstance(index);
            if (effect)
                effect->SetGlobalMatrix(mat);
        }
    }

    else if (!strcmpi(szCmd, "messenger_auth"))
    {
        const std::string &c_rstrName = TokenVector[1];
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnMessengerAddFriendQuestion",
                              py::bytes(c_rstrName));
    }

    else if (!strcmpi(szCmd, "setblockmode"))
    {
        int iFlag = atoi(TokenVector[1].c_str());
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnBlockMode", iFlag);
    }
        // Emotion Start
    else if (!strcmpi(szCmd, "french_kiss"))
    {
        int iVID1 = atoi(TokenVector[1].c_str());
        int iVID2 = atoi(TokenVector[2].c_str());

        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CInstanceBase *pInstance1 = rkChrMgr.GetInstancePtr(iVID1);
        CInstanceBase *pInstance2 = rkChrMgr.GetInstancePtr(iVID2);
        if (pInstance1 && pInstance2)
            pInstance1->ActDualEmotion(*pInstance2, MOTION_FRENCH_KISS_START, MOTION_FRENCH_KISS_START);
    }
    else if (!strcmpi(szCmd, "kiss"))
    {
        int iVID1 = atoi(TokenVector[1].c_str());
        int iVID2 = atoi(TokenVector[2].c_str());

        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CInstanceBase *pInstance1 = rkChrMgr.GetInstancePtr(iVID1);
        CInstanceBase *pInstance2 = rkChrMgr.GetInstancePtr(iVID2);
        if (pInstance1 && pInstance2)
            pInstance1->ActDualEmotion(*pInstance2, MOTION_KISS_START, MOTION_KISS_START);
    }
    else if (!strcmpi(szCmd, "slap"))
    {
        int iVID1 = atoi(TokenVector[1].c_str());
        int iVID2 = atoi(TokenVector[2].c_str());

        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CInstanceBase *pInstance1 = rkChrMgr.GetInstancePtr(iVID1);
        CInstanceBase *pInstance2 = rkChrMgr.GetInstancePtr(iVID2);
        if (pInstance1 && pInstance2)
            pInstance1->ActDualEmotion(*pInstance2, MOTION_SLAP_HURT_START, MOTION_SLAP_HIT_START);
    }
    else if (!strcmpi(szCmd, "clap"))
    {
        int iVID = atoi(TokenVector[1].c_str());
        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CInstanceBase *pInstance = rkChrMgr.GetInstancePtr(iVID);
        if (pInstance)
            pInstance->ActEmotion(MOTION_CLAP);
    }
    else if (!strcmpi(szCmd, "cheer1"))
    {
        int iVID = atoi(TokenVector[1].c_str());
        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CInstanceBase *pInstance = rkChrMgr.GetInstancePtr(iVID);
        if (pInstance)
            pInstance->ActEmotion(MOTION_CHEERS_1);
    }
    else if (!strcmpi(szCmd, "cheer2"))
    {
        int iVID = atoi(TokenVector[1].c_str());
        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CInstanceBase *pInstance = rkChrMgr.GetInstancePtr(iVID);
        if (pInstance)
            pInstance->ActEmotion(MOTION_CHEERS_2);
    }
    else if (!strcmpi(szCmd, "dance1"))
    {
        int iVID = atoi(TokenVector[1].c_str());
        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CInstanceBase *pInstance = rkChrMgr.GetInstancePtr(iVID);
        if (pInstance)
            pInstance->ActEmotion(MOTION_DANCE_1);
    }
    else if (!strcmpi(szCmd, "dance2"))
    {
        int iVID = atoi(TokenVector[1].c_str());
        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CInstanceBase *pInstance = rkChrMgr.GetInstancePtr(iVID);
        if (pInstance)
            pInstance->ActEmotion(MOTION_DANCE_2);
    }
    else if (!strcmpi(szCmd, "dig_motion"))
    {
        int iVID = atoi(TokenVector[1].c_str());
        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CInstanceBase *pInstance = rkChrMgr.GetInstancePtr(iVID);
        if (pInstance)
            pInstance->ActEmotion(MOTION_DIG);
    }
        // Emotion End
    else if (!strcmpi(szCmd, "AutoPotionBig"))
    {
        m_autoPotionScale = 8.0f;
    }
    else if (!strcmpi(szCmd, "AutoPotionNormal"))
    {
        m_autoPotionScale = 1.0f;
    }

    else if (!strcmpi(szCmd, "highlight_item"))
    {
        if (3 != TokenVector.size())
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand);
            return;
        }

        int invenType = atoi(TokenVector[1].c_str());
        int invenCell = atoi(TokenVector[2].c_str());
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Highlight_Item", invenType, invenCell);
    }
    else if (!strcmpi(szCmd, "openUI12zi"))
    {
        if (6 != TokenVector.size())
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand);
            return;
        }
        auto yellowmark = atoi(TokenVector[1].c_str());
        auto greenmark = atoi(TokenVector[2].c_str());
        auto yellowreward = atoi(TokenVector[3].c_str());
        auto greenreward = atoi(TokenVector[4].c_str());
        auto goldreward = atoi(TokenVector[5].c_str());

        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenUI12zi", yellowmark, greenmark, yellowreward,
                              greenreward, goldreward);
    }
    else if (!strcmpi(szCmd, "env"))
    {
        if (2 != TokenVector.size())
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand);
            return;
        }

        int index = atoi(TokenVector[1].c_str());
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetEnvironment", index);
    }
    else if (!strcmpi(szCmd, "Show12ziJumpButton"))
    {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Show12ziJumpButton");
    }
    else if (!strcmpi(szCmd, "Hide12ziTimer"))
    {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Hide12ziTimer");
    }
    else if (!strcmpi(szCmd, "Refresh12ziTimer"))
    {
        if (5 != TokenVector.size())
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand);
            return;
        }

        int currentFloor = atoi(TokenVector[1].c_str());
        int jumpCount = atoi(TokenVector[2].c_str());
        int limitTime = atoi(TokenVector[3].c_str());
        int elapseTime = atoi(TokenVector[4].c_str());
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Refresh12ziTimer", currentFloor, jumpCount,
                              limitTime, elapseTime);
    }

    else if (!strcmpi(szCmd, "RefreshBeadTimer"))
    {
        if (2 != TokenVector.size())
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand);
            return;
        }

        int value = atoi(TokenVector[1].c_str());
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "NextBeadUpdateTime", value);
    }

    else if (!strcmpi(szCmd, "OpenReviveDialog"))
    {
        if (3 != TokenVector.size())
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand);
            return;
        }

        int vid = atoi(TokenVector[1].c_str());
        int itemcount = atoi(TokenVector[2].c_str());
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenReviveDialog", vid, itemcount);
    }
    else if (!strcmpi(szCmd, "OpenRestartDialog"))
    {
        if (2 != TokenVector.size())
        {
            SPDLOG_ERROR("CPythonNetworkStream::ServerCommand(c_szCommand={0}) - Strange Parameter Count : {1}",
                          c_szCommand);
            return;
        }

        int mapIdx = atoi(TokenVector[1].c_str());
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenRestartDialog", mapIdx);
    }
    else
    {
        static std::map<std::string, int> s_emotionDict;

        static bool s_isFirst = true;
        if (s_isFirst)
        {
            s_isFirst = false;

            s_emotionDict["dance3"] = MOTION_DANCE_3;
            s_emotionDict["dance4"] = MOTION_DANCE_4;
            s_emotionDict["dance5"] = MOTION_DANCE_5;
            s_emotionDict["dance6"] = MOTION_DANCE_6;
            s_emotionDict["dance7"] = MOTION_DANCE_7;
            s_emotionDict["congratulation"] = MOTION_CONGRATULATION;
            s_emotionDict["forgive"] = MOTION_FORGIVE;
            s_emotionDict["angry"] = MOTION_ANGRY;
            s_emotionDict["attractive"] = MOTION_ATTRACTIVE;
            s_emotionDict["sad"] = MOTION_SAD;
            s_emotionDict["shy"] = MOTION_SHY;
            s_emotionDict["cheerup"] = MOTION_CHEERUP;
            s_emotionDict["banter"] = MOTION_BANTER;
            s_emotionDict["joy"] = MOTION_JOY;
            s_emotionDict["throw_money"] = MOTION_THROW_MONEY;
            s_emotionDict["doze"] = MOTION_EMOTION_DOZE;
            s_emotionDict["exercise"] = MOTION_EMOTION_EXERCISE;
            s_emotionDict["selfie"] = MOTION_EMOTION_SELFIE;
            s_emotionDict["pushup"] = MOTION_EMOTION_PUSH_UP;
        }

        std::map<std::string, int>::iterator f = s_emotionDict.find(szCmd);
        if (f == s_emotionDict.end())
        {
            SPDLOG_DEBUG("Unknown Server Command {0} | {1}", c_szCommand, szCmd);
        }
        else
        {
            int emotionIndex = f->second;

            int iVID = atoi(TokenVector[1].c_str());
            CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
            CInstanceBase *pInstance = rkChrMgr.GetInstancePtr(iVID);

            if (pInstance)
                pInstance->ActEmotion(emotionIndex);
        }
    }
}
