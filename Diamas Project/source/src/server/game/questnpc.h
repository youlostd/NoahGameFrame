#ifndef METIN2_SERVER_GAME_QUESTNPC_H
#define METIN2_SERVER_GAME_QUESTNPC_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "questpc.h"

class CItem;

namespace quest
{
using namespace std;

enum
{
    QUEST_START_STATE_INDEX = 0,
    QUEST_CHAT_STATE_INDEX = -1,
    QUEST_FISH_REFINE_STATE_INDEX = -2,
};

class PC;

class NPC
{
public:
    // 인자가 없는 스크립트들
    // first: state number
    typedef map<int, AStateScriptType> AQuestScriptType;
    // first: quest number
    typedef map<unsigned int, AQuestScriptType> QuestMapType;

    // 인자가 있는 스크립트들
    // first: state number
    typedef map<int, vector<AArgScript>> AArgQuestScriptType;
    // first: quest number
    typedef map<unsigned int, AArgQuestScriptType> ArgQuestMapType;

    NPC();
    ~NPC();

    void Set(unsigned int vnum, const string &script_name);

    static bool HasStartState(const AQuestScriptType &q) { return q.find(0) != q.end(); }

    static bool HasStartState(const AArgQuestScriptType &q) { return q.find(0) != q.end(); }

    bool OnServerTimer(PC &pc);
    bool OnDungeonStageChange(PC &pc);
    bool OnDungeonEliminatedEvent(PC &pc);

    //New triggers
    bool OnDie(PC &pc);
    bool OnBuy(PC &pc, uint32_t itemId);
    bool OnReceive(PC &pc, uint32_t quest_index);

    //OLD
    bool OnClick(PC &pc);
    bool OnKill(PC &pc);
    bool OnPartyKill(PC &pc);
    bool OnTimer(PC &pc);
    bool OnLevelUp(PC &pc);
    bool OnLogin(PC &pc, const char *c_pszQuestName = nullptr);
    bool OnLogout(PC &pc);
    bool OnDead(PC& pc);
    bool OnButton(PC &pc, unsigned int quest_index);
    bool OnInfo(PC &pc, unsigned int quest_index);
    bool OnAttrIn(PC &pc);
    bool OnAttrOut(PC &pc);
    bool OnUseItem(PC &pc, bool bReceiveAll, uint32_t itemId);
    bool OnUseItemToItem(PC &pc, uint32_t itemId);
    bool OnTakeItem(PC &pc, uint32_t itemId);
    bool OnEnterState(PC &pc, uint32_t quest_index, int state, uint32_t itemId);
    bool OnLeaveState(PC &pc, uint32_t quest_index, int state);
    bool OnLetter(PC &pc, uint32_t quest_index, int state);
    bool OnChat(PC &pc);
    bool HasChat();

    bool OnTarget(PC &pc, uint32_t dwQuestIndex, const char *c_pszTargetName, const char *c_pszVerb, bool &bRet);
    bool OnUnmount(PC &pc);

    // ITEM_PICK EVENT
    bool OnPickupItem(PC &pc, uint32_t itemId);

    // Special item group USE EVENT
    bool OnSIGUse(PC &pc, bool bReceiveAll, uint32_t itemId);

    bool HandleEvent(PC &pc, int EventIndex, uint32_t itemId = 0);
    bool HandleReceiveAllEvent(PC &pc, int EventIndex,  uint32_t itemId = 0);
    bool HandleReceiveAllNoWaitEvent(PC &pc, int EventIndex);

    bool ExecuteEventScript(PC &pc, int EventIndex, uint32_t dwQuestIndex, int iState,  uint32_t itemId = 0);

    unsigned int GetVnum() { return m_vnum; }

protected:
    template <typename TQuestMapType, typename FuncMatch, typename FuncMiss>
    void MatchingQuest(PC &pc, TQuestMapType &QuestMap, FuncMatch &fMatch, FuncMiss &fMiss);

    // true if quest still running, false if ended

    void LoadStateScript(int idx, const char *filename, const char *script_name);

    unsigned int m_vnum;
    QuestMapType m_mapOwnQuest[QUEST_EVENT_COUNT];
    ArgQuestMapType m_mapOwnArgQuest[QUEST_EVENT_COUNT];
};

template <typename TQuestMapType, typename FuncMatch, typename FuncMiss>
void NPC::MatchingQuest(PC &pc, TQuestMapType &QuestMap, FuncMatch &fMatch, FuncMiss &fMiss)
{
    PC::QuestInfoIterator itPCQuest = pc.quest_begin();
    typename TQuestMapType::iterator itQuestMap = QuestMap.begin();

    while (itQuestMap != QuestMap.end())
    {
        if (itPCQuest == pc.quest_end() || itPCQuest->first > itQuestMap->first)
        {
            fMiss(itPCQuest, itQuestMap);
            ++itQuestMap;
        }
        else if (itPCQuest->first < itQuestMap->first) { ++itPCQuest; }
        else
        {
            fMatch(itPCQuest, itQuestMap);
            ++itPCQuest;
            ++itQuestMap;
        }
    }
}
}
#endif /* METIN2_SERVER_GAME_QUESTNPC_H */
