#ifndef METIN2_SERVER_GAME_OXEVENT_H
#define METIN2_SERVER_GAME_OXEVENT_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "event.h"

struct tag_Quiz
{
    uint8_t level;
    char Quiz[256];
    bool answer;
};

enum OXEventStatus
{
    OXEVENT_FINISH = 0,
    // OX이벤트가 완전히 끝난 상태
    OXEVENT_OPEN = 1,
    // OX이벤트가 시작됨. 을두지(20012)를 통해서 입장가능
    OXEVENT_CLOSE = 2,
    // OX이벤트의 참가가 끝남. 을두지(20012)를 통한 입장이 차단됨
    OXEVENT_QUIZ = 3,
    // 퀴즈를 출제함.

    OXEVENT_ERR = 0xff
};

class COXEventManager : public singleton<COXEventManager>
{
private :
    std::map<uint32_t, uint32_t> m_map_char;
    std::map<uint32_t, uint32_t> m_map_attender;
    std::map<uint32_t, uint32_t> m_map_miss;

    std::vector<std::vector<tag_Quiz>> m_vec_quiz;
    std::unordered_set<std::string> m_list_iplist;

    LPEVENT m_timedEvent;
    LPEVENT m_eAutoEvent;
protected :
    bool CheckAnswer();

    bool EnterAudience(CHARACTER *pChar);
    bool EnterAttender(CHARACTER *pChar);

public :
    bool Initialize();
    void Destroy();

    //Determine whether it is an OX Event mapindex or not
    static inline bool IsEventMap(int mapindex) { return mapindex == OXEVENT_MAP_INDEX; }

    OXEventStatus GetStatus();
    void SetStatus(OXEventStatus status);

    bool LoadQuizScript(const char *szFileName);
    bool CheckIpAddress(CHARACTER *ch);
    void RemoveFromAttenderList(uint32_t dwPID);

    bool Enter(CHARACTER *pChar);

    bool CloseEvent();

    void ClearQuiz();
    void ClearAttenders();
    bool AddQuiz(uint8_t level, const char *pszQuestion, bool answer);
    bool ShowQuizList(CHARACTER *pChar);

    bool Quiz(unsigned char level, int timelimit);
    bool GiveItemToAttender(uint32_t dwItemVnum, uint8_t count);

    bool CheckAnswer(bool answer);
    void WarpToAudience();

    bool LogWinner();
    CHARACTER *GetWinner();

    uint32_t GetAttenderCount() { return m_map_attender.size(); }

    void StartAutomaticOX();
};

#endif /* METIN2_SERVER_GAME_OXEVENT_H */
