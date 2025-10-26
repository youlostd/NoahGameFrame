#ifndef METIN2_SERVER_GAME_TARGET_H
#define METIN2_SERVER_GAME_TARGET_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

enum ETargetTypes
{
    TARGET_TYPE_POS = (1 << 0),
    TARGET_TYPE_VID = (1 << 1),
#if defined(WJ_COMBAT_ZONE)
	TARGET_TYPE_COMBAT_ZONE = 4,
#endif
};

EVENTINFO(TargetInfo)
{
    // <Factor> Removed unsafe copy of CHARACTER* here
    //CHARACTER* pkChr;

    int iID;

    uint32_t dwPID;
    uint32_t dwQuestIndex;

    char szTargetName[32 + 1]; // 퀘스트에서 사용하는 이름
    char szTargetDesc[32 + 1]; // 실제 클라이언트에 전송되는 이름

    int iType;
    int iArg1;
    int iArg2;

    int iMapIndex;
    int iOldX;
    int iOldY;
    int iDistance;

    bool bSendToClient;

    TargetInfo()
        : iID(0)
          , dwPID(0)
          , dwQuestIndex(0)
          , iType(0)
          , iArg1(0)
          , iArg2(0)
          , iMapIndex(0)
          , iOldX(0)
          , iOldY(0)
          , bSendToClient(false)
          , iDistance(500)
    {
        ::memset(szTargetName, 0, 32 + 1);
        ::memset(szTargetDesc, 0, 32 + 1);
    }
};

class CTargetManager : public singleton<CTargetManager>
{
public:
    CTargetManager();
    virtual ~CTargetManager();

    void CreateTarget(uint32_t dwPID, uint32_t dwQuestIndex, const char *c_pszTargetName, int iType, int iArg1,
                      int iArg2, int iMapIndex, const char *c_pszTargetDesc = nullptr, int iSendFlag = 1,
                      int iDistance = 500);
    void DeleteTarget(uint32_t dwPID, uint32_t dwQuestIndex, const char *c_pszTargetName);

    void Logout(uint32_t dwPID);
    TargetInfo *GetTargetInfo(uint32_t dwPID, int iType, int iArg1);

    LPEVENT GetTargetEvent(uint32_t dwPID, uint32_t dwQuestIndex, const char *c_pszTargetName);

protected:
    // first: PID
    std::map<uint32_t, std::list<LPEVENT>> m_map_kListEvent;
    int m_iID;
};

#endif /* METIN2_SERVER_GAME_TARGET_H */
