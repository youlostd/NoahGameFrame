#include "constants.h"
#include "questmanager.h"
#include <game/GamePacket.hpp>
#include "GBufferManager.h"
#include "char.h"
#include "DbCacheSocket.hpp"
#include "questevent.h"
#include "desc.h"

namespace quest
{
PC::PC()
    : m_bIsGivenReward(false), m_bShouldSendDone(false), m_dwID(0), m_RunningQuestState(nullptr),
      m_SuspendedQuestState(nullptr), m_iSendToClient(0), m_bLoaded(false), m_iLastState(0), m_dwWaitConfirmFromPID(0),
      m_bConfirmWait(false)
{
}

PC::~PC() { Destroy(); }

void PC::Destroy() { ClearTimer(); }

void PC::SetID(uint32_t dwID)
{
    m_dwID = dwID;
    m_bShouldSendDone = false;
}

const string &PC::GetCurrentQuestName() const { return m_stCurQuest; }

int PC::GetCurrentQuestIndex() { return CQuestManager::instance().GetQuestIndexByName(GetCurrentQuestName()); }

void PC::SetFlag(const string &name, int value, bool bSkipSave)
{
    SPDLOG_DEBUG("QUEST Setting flag {0} {1}", name.c_str(), value);

    if (value == 0)
    {
        DeleteFlag(name);
        return;
    }

    TFlagMap::iterator it = m_FlagMap.find(name);

    if (it == m_FlagMap.end())
        m_FlagMap.insert(make_pair(name, value));
    else if (it->second != value)
        it->second = value;
    else
        bSkipSave = true;

    if (!bSkipSave)
        SaveFlag(name, value);
}

bool PC::DeleteFlag(const string &name)
{
    TFlagMap::iterator it = m_FlagMap.find(name);

    if (it != m_FlagMap.end())
    {
        m_FlagMap.erase(it);
        SaveFlag(name, 0);
        return true;
    }

    return false;
}

int PC::GetFlag(const string &name)
{
    TFlagMap::iterator it = m_FlagMap.find(name);

    if (it != m_FlagMap.end())
    {
        SPDLOG_TRACE("QUEST getting flag {} {}", name.c_str(), it->second);
        return it->second;
    }
    return 0;
}

void PC::SaveFlag(const string &name, int value)
{
    TFlagMap::iterator it = m_FlagSaveMap.find(name);

    if (it == m_FlagSaveMap.end())
        m_FlagSaveMap.insert(make_pair(name, value));
    else if (it->second != value)
        it->second = value;
}

// only from lua call
void PC::SetCurrentQuestStateName(const string &state_name)
{
    SetFlag(m_stCurQuest + ".__status", CQuestManager::Instance().GetQuestStateIndex(m_stCurQuest, state_name));
}

void PC::SetQuestState(const string &quest_name, const string &state_name)
{
    SetQuestState(quest_name, CQuestManager::Instance().GetQuestStateIndex(quest_name, state_name));
}

void PC::SetQuestState(const string &quest_name, int new_state_index)
{
    int iNowState = GetFlag(quest_name + ".__status");

    if (iNowState != new_state_index)
        AddQuestStateChange(quest_name, iNowState, new_state_index);
}

void PC::AddQuestStateChange(const string &quest_name, int prev_state, int next_state)
{
    uint32_t dwQuestIndex = CQuestManager::instance().GetQuestIndexByName(quest_name);
    SPDLOG_INFO("QUEST reserve Quest State Change quest {}[{}] from  to {}", quest_name.c_str(), dwQuestIndex,
                prev_state, next_state);
    m_QuestStateChange.push_back(TQuestStateChangeInfo(dwQuestIndex, prev_state, next_state));
}

void PC::SetQuest(const string &quest_name, QuestState &qs)
{
    // SPDLOG_INFO( "PC SetQuest %s", quest_name.c_str());
    unsigned int qi = CQuestManager::instance().GetQuestIndexByName(quest_name);
    QuestInfo::iterator it = m_QuestInfo.find(qi);

    if (it == m_QuestInfo.end())
        m_QuestInfo.insert(make_pair(qi, qs));
    else
        it->second = qs;

    m_stCurQuest = quest_name;
    m_RunningQuestState = &m_QuestInfo[qi];
    m_iSendToClient = 0;

    m_iLastState = qs.st;
    SetFlag(quest_name + ".__status", qs.st);

    // m_RunningQuestState->iIndex = GetCurrentQuestBeginFlag();
    m_RunningQuestState->iIndex = qi;
    m_bShouldSendDone = false;
    // if (GetCurrentQuestBeginFlag())
    //{
    // m_bSendToClient = true;
    //}
}

void PC::AddTimer(const string &name, LPEVENT pEvent)
{
    RemoveTimer(name);
    m_TimerMap.insert(make_pair(name, pEvent));
}

void PC::RemoveTimerNotCancel(const string &name)
{
    TTimerMap::iterator it = m_TimerMap.find(name);

    if (it != m_TimerMap.end())
    {
        m_TimerMap.erase(it);
    }
}

void PC::RemoveTimer(const string &name)
{
    TTimerMap::iterator it = m_TimerMap.find(name);

    if (it != m_TimerMap.end())
    {
        CancelTimerEvent(&it->second);
        m_TimerMap.erase(it);
    }
}

void PC::ClearTimer()
{
    TTimerMap::iterator it = m_TimerMap.begin();

    while (it != m_TimerMap.end())
    {
        CancelTimerEvent(&it->second);
        ++it;
    }

    m_TimerMap.clear();
}

void PC::SetCurrentQuestStartFlag()
{
    if (!GetCurrentQuestBeginFlag())
    {
        SetCurrentQuestBeginFlag();
    }
}

void PC::SetCurrentQuestDoneFlag()
{
    if (GetCurrentQuestBeginFlag())
    {
        ClearCurrentQuestBeginFlag();
    }
}

void PC::SendQuestInfoPakcet()
{
    assert(m_iSendToClient);
    assert(m_RunningQuestState);

    packet_quest_info qi;
    qi.index = m_RunningQuestState->iIndex;
    qi.flag = m_iSendToClient;
    qi.c_index = CQuestManager::instance().GetQuestCategoryByQuestIndex(qi.index);

    TempBuffer buf;
    buf.write(&qi, sizeof(qi));

    if (m_iSendToClient & QUEST_SEND_ISBEGIN)
    {
        uint8_t temp = m_RunningQuestState->bStart ? 1 : 0;
        qi.isBegin = temp;

        SPDLOG_TRACE("QUEST BeginFlag {}", temp);
    }
    if (m_iSendToClient & QUEST_SEND_TITLE)
    {
        qi.title = m_RunningQuestState->_title;
        SPDLOG_TRACE("QUEST Title {}", m_RunningQuestState->_title.c_str());
    }
    if (m_iSendToClient & QUEST_SEND_CLOCK_NAME)
    {
        qi.clockName = m_RunningQuestState->_clock_name;
        SPDLOG_TRACE("QUEST Clock Name {}", m_RunningQuestState->_clock_name.c_str());
    }
    if (m_iSendToClient & QUEST_SEND_CLOCK_VALUE)
    {
        qi.clockValue = m_RunningQuestState->_clock_value;
        SPDLOG_TRACE("QUEST Clock Value {}", m_RunningQuestState->_clock_value);
    }
    if (m_iSendToClient & QUEST_SEND_COUNTER_NAME)
    {
        qi.counterName = m_RunningQuestState->_counter_name;
        SPDLOG_TRACE("QUEST Counter Name {}", m_RunningQuestState->_counter_name.c_str());
    }
    if (m_iSendToClient & QUEST_SEND_COUNTER_VALUE)
    {
        qi.counterValue = m_RunningQuestState->_counter_value;
        SPDLOG_TRACE("QUEST Counter Value {}", m_RunningQuestState->_counter_value);
    }
    if (m_iSendToClient & QUEST_SEND_ICON_FILE)
    {
        qi.iconFile = m_RunningQuestState->_icon_file;
        SPDLOG_TRACE("QUEST Icon File {}", m_RunningQuestState->_icon_file.c_str());
    }

    CQuestManager::instance().GetCurrentCharacterPtr()->GetDesc()->Send(HEADER_GC_QUEST_INFO, qi);

    m_iSendToClient = 0;
}

void PC::EndRunning()
{
    CQuestManager &q = CQuestManager::instance();

    // unlock locked npcs, just in case we forgot to unlock
    {
        CHARACTER *npc = q.GetCurrentNPCCharacterPtr();
        CHARACTER *ch = q.GetCurrentCharacterPtr();
        if (npc && !npc->IsPC())
        {
            if (ch->GetPlayerID() == npc->GetQuestNPCID())
            {
                npc->SetQuestNPCID(0);
                SPDLOG_ERROR("QUEST NPC lock wasn't unlocked : pid {}", ch->GetPlayerID());
                q.WriteRunningStateToSyserr();
            }
        }
    }

    // commit data
    if (HasReward())
    {
        Save();

        CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
        if (ch != nullptr)
        {
            Reward(ch);
            ch->Save();
        }
    }
    m_bIsGivenReward = false;

    if (m_iSendToClient)
    {
        SPDLOG_TRACE("QUEST end running {}", m_iSendToClient);
        SendQuestInfoPakcet();
    }

    if (m_RunningQuestState == nullptr)
    {
        SPDLOG_INFO("Entered PC::EndRunning() with invalid running quest state");
        return;
    }

    if (m_SuspendedQuestState && m_SuspendedQuestState->iIndex == m_RunningQuestState->iIndex)
    {
        m_SuspendedQuestState = nullptr; // Not running anymore
    }

    QuestState *pOldState = m_RunningQuestState;
    int iNowState = m_RunningQuestState->st;

    m_RunningQuestState = nullptr;

    if (m_iLastState != iNowState)
    {
        CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();
        uint32_t dwQuestIndex = CQuestManager::instance().GetQuestIndexByName(m_stCurQuest);
        if (ch)
        {
            SetFlag(m_stCurQuest + ".__status", m_iLastState);
            CQuestManager::instance().LeaveState(ch->GetPlayerID(), dwQuestIndex, m_iLastState);
            pOldState->st = iNowState;
            SetFlag(m_stCurQuest + ".__status", iNowState);
            CQuestManager::instance().EnterState(ch->GetPlayerID(), dwQuestIndex, iNowState);
            if (GetFlag(m_stCurQuest + ".__status") == iNowState)
                CQuestManager::instance().Letter(ch->GetPlayerID(), dwQuestIndex, iNowState);
        }
    }

    DoQuestStateChange();
}

void PC::DoQuestStateChange()
{
    CHARACTER *ch = CQuestManager::instance().GetCurrentCharacterPtr();

    std::vector<TQuestStateChangeInfo> vecQuestStateChange;
    m_QuestStateChange.swap(vecQuestStateChange);

    for (uint32_t i = 0; i < vecQuestStateChange.size(); ++i)
    {
        const TQuestStateChangeInfo &rInfo = vecQuestStateChange[i];
        if (rInfo.quest_idx == 0)
            continue;

        uint32_t dwQuestIdx = rInfo.quest_idx;
        QuestInfoIterator it = quest_find(dwQuestIdx);
        const string stQuestName = CQuestManager::instance().GetQuestNameByIndex(dwQuestIdx);

        if (it == quest_end())
        {
            QuestState qs;
            qs.st = 0;

            m_QuestInfo.insert(make_pair(dwQuestIdx, qs));
            SetFlag(stQuestName + ".__status", 0);

            it = quest_find(dwQuestIdx);
        }

        SPDLOG_INFO("QUEST change reserved Quest State Change quest {} from {} to {} ({} {})", dwQuestIdx,
                    rInfo.prev_state, rInfo.next_state, it->second.st, rInfo.prev_state);

        assert(it->second.st == rInfo.prev_state);

        CQuestManager::instance().LeaveState(ch->GetPlayerID(), dwQuestIdx, rInfo.prev_state);
        it->second.st = rInfo.next_state;
        SetFlag(stQuestName + ".__status", rInfo.next_state);

        CQuestManager::instance().EnterState(ch->GetPlayerID(), dwQuestIdx, rInfo.next_state);

        if (GetFlag(stQuestName + ".__status") == rInfo.next_state)
            CQuestManager::instance().Letter(ch->GetPlayerID(), dwQuestIdx, rInfo.next_state);
    }
}

void PC::CancelRunning()
{
    // cancel data
    m_RunningQuestState = nullptr;
    m_iSendToClient = 0;
    m_bShouldSendDone = false;
}

void PC::SetSendFlag(int idx) { m_iSendToClient |= idx; }

void PC::ClearCurrentQuestBeginFlag()
{
    // cerr << "iIndex " << m_RunningQuestState->iIndex << endl;
    SetSendFlag(QUEST_SEND_ISBEGIN);
    m_RunningQuestState->bStart = false;
    // SetFlag(m_stCurQuest+".__isbegin", 0);
}

void PC::SetCurrentQuestBeginFlag()
{
    CQuestManager &q = CQuestManager::instance();
    int iQuestIndex = q.GetQuestIndexByName(m_stCurQuest);
    m_RunningQuestState->bStart = true;
    m_RunningQuestState->iIndex = iQuestIndex;

    SetSendFlag(QUEST_SEND_ISBEGIN);
    // SetFlag(m_stCurQuest+".__isbegin", iQuestIndex);
}

int PC::GetCurrentQuestBeginFlag()
{
    return m_RunningQuestState ? m_RunningQuestState->iIndex : 0;
    // return GetFlag(m_stCurQuest+".__isbegin");
}

void PC::SetCurrentQuestTitle(const string &title)
{
    SetSendFlag(QUEST_SEND_TITLE);
    m_RunningQuestState->_title = title;
}

void PC::SetQuestTitle(const string &quest, const string &title)
{
    // SetSendFlag(QUEST_SEND_TITLE);
    QuestInfo::iterator it = m_QuestInfo.find(CQuestManager::instance().GetQuestIndexByName(quest));

    if (it != m_QuestInfo.end())
    {
        //(*it)->_title = title;
        QuestState *old = m_RunningQuestState;
        int old2 = m_iSendToClient;
        std::string oldquestname = m_stCurQuest;
        m_stCurQuest = quest;
        m_RunningQuestState = &it->second;
        m_iSendToClient = QUEST_SEND_TITLE;
        m_RunningQuestState->iIndex = GetCurrentQuestBeginFlag();

        SetCurrentQuestTitle(title);

        SendQuestInfoPakcet();

        m_stCurQuest = oldquestname;
        m_RunningQuestState = old;
        m_iSendToClient = old2;
    }
}

void PC::SetCurrentQuestClockName(const string &name)
{
    SetSendFlag(QUEST_SEND_CLOCK_NAME);
    m_RunningQuestState->_clock_name = name;
}

void PC::SetCurrentQuestClockValue(int value)
{
    SetSendFlag(QUEST_SEND_CLOCK_VALUE);
    m_RunningQuestState->_clock_value = value;
}

void PC::SetCurrentQuestCounterName(const string &name)
{
    SetSendFlag(QUEST_SEND_COUNTER_NAME);
    m_RunningQuestState->_counter_name = name;
}

void PC::SetCurrentQuestCounterValue(int value)
{
    SetSendFlag(QUEST_SEND_COUNTER_VALUE);
    m_RunningQuestState->_counter_value = value;
}

void PC::SetCurrentQuestIconFile(const string &icon_file)
{
    SetSendFlag(QUEST_SEND_ICON_FILE);
    m_RunningQuestState->_icon_file = icon_file;
}

void PC::Save()
{
    if (m_FlagSaveMap.empty())
        return;

    static std::vector<TQuestTable> s_table;
    s_table.resize(m_FlagSaveMap.size());

    int i = 0;

    TFlagMap::iterator it = m_FlagSaveMap.begin();

    while (it != m_FlagSaveMap.end())
    {
        const std::string &stComp = it->first;
        long lValue = it->second;

        ++it;

        int iPos = stComp.find(".");

        if (iPos < 0)
        {
            SPDLOG_ERROR("quest::PC::Save : cannot find . in FlagMap");
            continue;
        }

        string stName;
        stName.assign(stComp, 0, iPos);

        string stState;
        stState.assign(stComp, iPos + 1, stComp.length());

        if (stName.length() == 0 || stState.length() == 0)
        {
            SPDLOG_ERROR("quest::PC::Save : invalid quest data: {}", stComp.c_str());
            continue;
        }

        SPDLOG_TRACE("QUEST Save Flag {}, {} {} ({})", stName.c_str(), stState.c_str(), lValue, i);

        if (stName.length() >= QUEST_NAME_MAX_LEN)
        {
            SPDLOG_ERROR("quest::PC::Save : quest name overflow");
            continue;
        }

        if (stState.length() >= QUEST_STATE_MAX_LEN)
        {
            SPDLOG_ERROR("quest::PC::Save : quest state overflow");
            continue;
        }

        TQuestTable &r = s_table[i++];

        r.dwPID = m_dwID;
        CopyStringSafe(r.szName, stName);
        CopyStringSafe(r.szState, stState);
        r.lValue = lValue;
    }

    if (i > 0)
    {
        SPDLOG_TRACE("QuestPC::Save {}", i);
        db_clientdesc->DBPacketHeader(HEADER_GD_QUEST_SAVE, 0, sizeof(TQuestTable) * i);
        db_clientdesc->Packet(&s_table[0], sizeof(TQuestTable) * i);
    }

    m_FlagSaveMap.clear();
}

bool PC::HasQuest(const string &quest_name)
{
    unsigned int qi = CQuestManager::instance().GetQuestIndexByName(quest_name);
    return m_QuestInfo.find(qi) != m_QuestInfo.end();
}

QuestState &PC::GetQuest(const string &quest_name)
{
    unsigned int qi = CQuestManager::instance().GetQuestIndexByName(quest_name);
    return m_QuestInfo[qi];
}

void PC::GiveItem(const string &label, uint32_t dwVnum, int count)
{
    SPDLOG_TRACE("QUEST GiveItem {} {} {}", label.c_str(), dwVnum, count);
    if (!GetFlag(m_stCurQuest + "." + label))
    {
        m_vRewardData.push_back(RewardData(RewardData::REWARD_TYPE_ITEM, dwVnum, count));
        // SetFlag(m_stCurQuest+"."+label,1);
    }
    else
        m_bIsGivenReward = true;
}

void PC::GiveExp(const string &label, uint32_t exp)
{
    SPDLOG_TRACE("QUEST GiveExp {} {}", label.c_str(), exp);

    if (!GetFlag(m_stCurQuest + "." + label))
    {
        m_vRewardData.push_back(RewardData(RewardData::REWARD_TYPE_EXP, exp));
        // SetFlag(m_stCurQuest+"."+label,1);
    }
    else
        m_bIsGivenReward = true;
}

void PC::Reward(CHARACTER *ch)
{
    if (m_bIsGivenReward)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "<Äù½ºÆ®> ÀÌÀü¿¡ °°Àº º¸»óÀ» ¹ÞÀº ÀûÀÌ ÀÖ¾î ´Ù½Ã ¹ÞÁö ¾Ê½À´Ï´Ù.");
        m_bIsGivenReward = false;
    }

    for (auto &it : m_vRewardData)
    {
        switch (it.type)
        {
        case RewardData::REWARD_TYPE_EXP:
            SPDLOG_INFO("EXP cur {} add {} next {}", ch->GetExp(), it.value1, ch->GetNextExp());

            if (ch->GetExp() + it.value1 > ch->GetNextExp())
                ch->PointChange(POINT_EXP, ch->GetNextExp() - 1 - ch->GetExp());
            else
                ch->PointChange(POINT_EXP, it.value1);

            break;

        case RewardData::REWARD_TYPE_ITEM:
            ch->AutoGiveItem(it.value1, it.value2);
            break;

        case RewardData::REWARD_TYPE_NONE:
        default:
            SPDLOG_ERROR("Invalid RewardData type");
            break;
        }
    }

    m_vRewardData.clear();
}

void PC::Build()
{
    for (auto it = m_FlagMap.begin(); it != m_FlagMap.end(); ++it)
    {
        size_t firstSize = it->first.size();
        if (firstSize > 9 && it->first.compare(firstSize - 9, 9, ".__status") == 0)
        {
            uint32_t dwQuestIndex = CQuestManager::instance().GetQuestIndexByName(it->first.substr(0, firstSize - 9));
            int state = it->second;
            QuestState qs;
            qs.st = state;

            m_QuestInfo.insert(make_pair(dwQuestIndex, qs));
        }
    }
}

void PC::ClearQuest(const string &quest_name)
{
    string quest_name_with_dot = quest_name + '.';
    for (auto it = m_FlagMap.begin(); it != m_FlagMap.end();)
    {
        auto itNow = it++;
        if (itNow->second != 0 && itNow->first.compare(0, quest_name_with_dot.size(), quest_name_with_dot) == 0)
        {
            // m_FlagMap.erase(itNow);
            SetFlag(itNow->first, 0);
        }
    }

    ClearTimer();

    quest::PC::QuestInfoIterator it = quest_begin();
    unsigned int questindex = quest::CQuestManager::instance().GetQuestIndexByName(quest_name);

    while (it != quest_end())
    {
        if (it->first == questindex)
        {
            it->second.st = 0;
            break;
        }

        ++it;
    }
}

void PC::SendFlagList(CHARACTER *ch, const std::string &filter)
{
    for (auto it = m_FlagMap.begin(); it != m_FlagMap.end(); ++it)
    {
        if (!filter.empty() && it->first.find(filter) == std::string::npos)
            continue;

        size_t firstSize = it->first.size();
        if (firstSize > 9 && it->first.compare(firstSize - 9, 9, ".__status") == 0)
        {
            const string quest_name = it->first.substr(0, firstSize - 9);
            const char *state_name = CQuestManager::instance().GetQuestStateName(quest_name, it->second);
            ch->ChatPacket(CHAT_TYPE_INFO, "%s %s (%d)", quest_name.c_str(), state_name, it->second);
        }
        else
        {
            ch->ChatPacket(CHAT_TYPE_INFO, "%s %d", it->first.c_str(), it->second);
        }
    }
}
void PC::RestoreQuestState(QuestState *qs)
{
    if (qs)
        m_RunningQuestState = qs;
}

} // namespace quest
