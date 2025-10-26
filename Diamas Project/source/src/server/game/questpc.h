#ifndef METIN2_SERVER_GAME_QUESTPC_H
#define METIN2_SERVER_GAME_QUESTPC_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "quest.h"

class CHARACTER;

namespace quest
{
using namespace std;

struct RewardData
{
    enum RewardType
    {
        REWARD_TYPE_NONE,
        REWARD_TYPE_EXP,
        REWARD_TYPE_ITEM,
    } type;

    uint32_t value1;
    int value2;

    RewardData(RewardType t, uint32_t value1, int value2 = 0) : type(t), value1(value1), value2(value2) {}
};

class PC
{
  public:
    typedef map<unsigned int, QuestState> QuestInfo;
    typedef QuestInfo::iterator QuestInfoIterator;

    PC();
    ~PC();

    void Destroy();
    void SetID(uint32_t dwID);

    uint32_t GetID() { return m_dwID; }

    bool HasQuest(const string &quest_name);
    QuestState &GetQuest(const string &quest_name);

    inline QuestInfoIterator quest_begin();
    inline QuestInfoIterator quest_end();
    inline QuestInfoIterator quest_find(uint32_t quest_index);

    inline bool IsRunning();
    inline bool IsRunningQuest(int quest_index);

    void EndRunning();
    void CancelRunning();

    inline QuestState *GetRunningQuestState();

    void SetQuest(const string &quest_name, QuestState &qs);
    void SetCurrentQuestStateName(const string &state_name);
    void SetQuestState(const string &quest_name, const string &state_name);
    void SetQuestState(const string &quest_name, int new_state_index);

    void ClearQuest(const string &quest_name);

  private:
    void AddQuestStateChange(const string &quest_name, int prev_state, int next_state);
    void DoQuestStateChange();

    struct TQuestStateChangeInfo
    {
        uint32_t quest_idx;
        int prev_state;
        int next_state;

        TQuestStateChangeInfo(uint32_t _quest_idx, int _prev_state, int _next_state)
            : quest_idx(_quest_idx), prev_state(_prev_state), next_state(_next_state)
        {
        }
    };

    vector<TQuestStateChangeInfo> m_QuestStateChange;

  public:
    void SetFlag(const string &name, int value, bool bSkipSave = false);
    int GetFlag(const string &name);
    bool DeleteFlag(const string &name);

    const string &GetCurrentQuestName() const;
    int GetCurrentQuestIndex();

    void RemoveTimer(const string &name);
    void RemoveTimerNotCancel(const string &name);
    void AddTimer(const string &name, LPEVENT pEvent);
    void ClearTimer();

    void SetCurrentQuestStartFlag();
    void SetCurrentQuestDoneFlag();

    void SetQuestTitle(const string &quest, const string &title);

    void SetCurrentQuestTitle(const string &title);
    void SetCurrentQuestClockName(const string &name);
    void SetCurrentQuestClockValue(int value);
    void SetCurrentQuestCounterName(const string &name);
    void SetCurrentQuestCounterValue(int value);
    void SetCurrentQuestIconFile(const string &icon_file);

    bool IsLoaded() const { return m_bLoaded; }

    void SetLoaded() { m_bLoaded = true; }

    void Build();
    // DB에 저장
    void Save();

    bool HasReward() { return !m_vRewardData.empty() || m_bIsGivenReward; }

    void Reward(CHARACTER *ch);

    void GiveItem(const string &label, uint32_t dwVnum, int count);
    void GiveExp(const string &label, uint32_t exp);

    void SetSendDoneFlag() { m_bShouldSendDone = true; }

    bool GetAndResetDoneFlag()
    {
        bool temp = m_bShouldSendDone;
        m_bShouldSendDone = false;
        return temp;
    }

    void SendFlagList(CHARACTER *ch, const string &filter = "");

    void SetQuestState(const char *szQuestName, const char *szStateName);

    void SetConfirmWait(uint32_t dwPID)
    {
        m_bConfirmWait = true;
        m_dwWaitConfirmFromPID = dwPID;
    }

    void ClearConfirmWait() { m_bConfirmWait = false; }

    bool IsConfirmWait() const { return m_bConfirmWait; }

    bool IsConfirmWait(uint32_t dwPID) const { return m_bConfirmWait && dwPID == m_dwWaitConfirmFromPID; }

    void SetCommand(const char *cmd) { szCmd = cmd; }

    std::string GetCommand() const { return szCmd; }

    void ClearCommand() { szCmd.clear(); }

  private:
    void SetSendFlag(int idx);

    void ClearSendFlag() { m_iSendToClient = 0; }

    void SaveFlag(const string &name, int value);

    void ClearCurrentQuestBeginFlag();
    void SetCurrentQuestBeginFlag();
    int GetCurrentQuestBeginFlag();

    void SendQuestInfoPakcet();

  public:
    void RestoreQuestState(QuestState *qs);
    void SetSuspendedState(QuestState *qs) { m_SuspendedQuestState = qs; };
    QuestState *GetSuspendedState() const { return m_SuspendedQuestState; };

  private:
    vector<RewardData> m_vRewardData;
    bool m_bIsGivenReward;

    bool m_bShouldSendDone;

    uint32_t m_dwID;

    QuestInfo m_QuestInfo;

    QuestState *m_RunningQuestState;
    QuestState *m_SuspendedQuestState;
    string m_stCurQuest;

    typedef map<string, int> TFlagMap;
    TFlagMap m_FlagMap;

    TFlagMap m_FlagSaveMap;

    typedef map<string, LPEVENT> TTimerMap;
    TTimerMap m_TimerMap;

    int m_iSendToClient;
    bool m_bLoaded; // 로드는 한번만 한다.

    int m_iLastState;

    uint32_t m_dwWaitConfirmFromPID;
    bool m_bConfirmWait;

    std::string szCmd;
};

inline PC::QuestInfoIterator PC::quest_begin() { return m_QuestInfo.begin(); }

inline PC::QuestInfoIterator PC::quest_end() { return m_QuestInfo.end(); }

inline PC::QuestInfoIterator PC::quest_find(uint32_t quest_index) { return m_QuestInfo.find(quest_index); }

inline bool PC::IsRunning() { return m_RunningQuestState != nullptr; }

inline QuestState *PC::GetRunningQuestState() { return m_RunningQuestState; }
inline bool PC::IsRunningQuest(int quest_index)
{
    return m_RunningQuestState && m_RunningQuestState->iIndex == quest_index;
}
} // namespace quest

#endif /* METIN2_SERVER_GAME_QUESTPC_H */
