#include "CharUtil.hpp"
#include "char.h"
#include "config.h"
#include "questmanager.h"
#include <fstream>
#include <sstream>

#ifdef WIN32
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif

namespace quest
{
std::array<std::string, QUEST_EVENT_COUNT> questEventNames = {
    "click",
    "kill",
    "timer",
    "levelup",
    "login",
    "logout",
    "button",
    "info",
    "chat",
    "attr_in",
    "attr_out",
    "item_use",
    "server_timer",
    "enter_state",
    "leave_state",
    "letter",
    "(item)take",
    "target",
    "party_kill",
    "unmount",
    "(item)pick",
    "sig_use",
    "die",
    "buy",
    "receive",
    "dungeon_stage_change",
    "dungeon_on_eliminated",
};

void SuspendedQuestStateError(PC& pc, int EventIndex)
{
    if (!gConfig.testServer)
        return;

    CQuestManager& mgr = CQuestManager::instance();
    QuestState* rqs = pc.GetRunningQuestState();

    std::string suspendedQuestName = mgr.GetQuestNameByIndex(rqs->iIndex);
    SPDLOG_ERROR("Quest {} is suspended at state {} with {}, can't run new "
                 "quest state (quest: {} pc: {}, event: {}:: {})",
                 suspendedQuestName.c_str(),
                 mgr.GetQuestStateName(suspendedQuestName, rqs->st),
                 rqs->suspend_state, pc.GetCurrentQuestName().c_str(),
                 mgr.GetCurrentCharacterPtr()
                     ? mgr.GetCurrentCharacterPtr()->GetName()
                     : "<none>",
                 EventIndex, questEventNames[EventIndex].c_str());
}

NPC::NPC()
{
    m_vnum = 0;
}

NPC::~NPC() {}

void NPC::Set(unsigned int vnum, const string& script_name)
{
    m_vnum = vnum;

    char buf[PATH_MAX];

    auto itEventName = CQuestManager::instance().m_mapEventName.begin();

    while (itEventName != CQuestManager::instance().m_mapEventName.end()) {
        auto it = itEventName;
        ++itEventName;

        int is = std::snprintf(buf, sizeof(buf), "data/quest/object/%s/%s/",
                               script_name.c_str(), it->first.c_str());

        if (is < 0 || is >= (int)sizeof(buf))
            is = sizeof(buf) - 1;

        // SPDLOG_TRACE( "XXX %s", buf);
        int event_index = it->second;

        if (fs::exists(buf)) {
            for (const auto& entry : fs::directory_iterator(buf)) {
                if (fs::is_regular_file(entry)) {
                    SPDLOG_TRACE("QUEST reading {0}",
                                 entry.path().filename().generic_string());
                    strlcpy(buf + is,
                            entry.path().filename().generic_string().c_str(),
                            sizeof(buf) - is);
                    LoadStateScript(
                        event_index, buf,
                        entry.path().filename().generic_string().c_str());
                }
            }
        }
    }
}

void NPC::LoadStateScript(int event_index, const char* filename,
                          const char* script_name)
{
    ifstream inf(filename);
    const string s(script_name);

    size_t i = s.find('.');

    CQuestManager& q = CQuestManager::instance();

    //
    // script_name examples:
    //   christmas_tree.start -> argument does not exist (can be identified by a
    //   single dot)
    //
    //   guild_manage.start.0.script -> argument exist
    //   guild_manage.start.0.when
    //   guild_manage.start.0.arg

    ///////////////////////////////////////////////////////////////////////////
    // Quest name
    const string stQuestName = s.substr(0, i);

    int quest_index = q.GetQuestIndexByName(stQuestName);

    if (quest_index == 0) {
        SPDLOG_ERROR("cannot find quest index for {}", stQuestName.c_str());
        assert(!"cannot find quest index");
        return;
    }

    ///////////////////////////////////////////////////////////////////////////
    // State name
    string stStateName;

    size_t j = i;
    i = s.find('.', i + 1);

    if (i == s.npos)
        stStateName = s.substr(j + 1, s.npos);
    else
        stStateName = s.substr(j + 1, i - j - 1);

    int state_index = q.GetQuestStateIndex(stQuestName, stStateName);
    ///////////////////////////////////////////////////////////////////////////

    SPDLOG_TRACE("QUEST loading {} : {} [STATE] {}", filename,
                 stQuestName.c_str(), stStateName.c_str());

    if (i == s.npos) {
        // like in example: christmas_tree.start
        istreambuf_iterator<char> ib(inf), ie;
        copy(ib, ie,
             back_inserter(
                 m_mapOwnQuest[event_index][quest_index]
                              [q.GetQuestStateIndex(stQuestName, stStateName)]
                                  .m_code));
    } else {
        //
        // like in example: guild_manage.start.0.blah
        // NOTE : currently, only CHAT script uses argument
        //

        ///////////////////////////////////////////////////////////////////////////
        // 순서 Index (여러개 있을 수 있으므로 있는 것임, 실제 index 값은 쓰지
        // 않음)
        j = i;
        i = s.find('.', i + 1);

        if (i == s.npos) {
            SPDLOG_ERROR("invalid QUEST STATE index [%s] [%s]", filename,
                         script_name);
            return;
        }

        const int index =
            strtol(s.substr(j + 1, i - j - 1).c_str(), nullptr, 10);
        ///////////////////////////////////////////////////////////////////////////
        // Type name
        j = i;
        i = s.find('.', i + 1);

        if (i != s.npos) {
            SPDLOG_ERROR("invalid QUEST STATE name [%s] [%s]", filename,
                         script_name);
            return;
        }

        const string type_name = s.substr(j + 1, i - j - 1);
        ///////////////////////////////////////////////////////////////////////////

        istreambuf_iterator<char> ib(inf), ie;

        m_mapOwnArgQuest[event_index][quest_index][state_index].resize(
            std::max<int>(
                index + 1,
                m_mapOwnArgQuest[event_index][quest_index][state_index]
                    .size()));

        if (type_name == "when") {
            copy(ib, ie,
                 back_inserter(m_mapOwnArgQuest[event_index][quest_index]
                                               [state_index][index]
                                                   .when_condition));
        } else if (type_name == "arg") {
            string s;
            getline(inf, s);
            m_mapOwnArgQuest[event_index][quest_index][state_index][index]
                .arg.clear();

            for (string::iterator it = s.begin(); it != s.end(); ++it) {
                m_mapOwnArgQuest[event_index][quest_index][state_index][index]
                    .arg += *it;
            }
        } else if (type_name == "script") {
            copy(ib, ie,
                 back_inserter(m_mapOwnArgQuest[event_index][quest_index]
                                               [state_index][index]
                                                   .script.m_code));
            m_mapOwnArgQuest[event_index][quest_index][state_index][index]
                .quest_index = quest_index;
            m_mapOwnArgQuest[event_index][quest_index][state_index][index]
                .state_index = state_index;
        }
    }
}

bool NPC::OnEnterState(PC& pc, uint32_t quest_index, int state, uint32_t itemId)
{
    return ExecuteEventScript(pc, QUEST_ENTER_STATE_EVENT, quest_index, state,
                              itemId);
}

bool NPC::OnLeaveState(PC& pc, uint32_t quest_index, int state)
{
    return ExecuteEventScript(pc, QUEST_LEAVE_STATE_EVENT, quest_index, state);
}

bool NPC::OnLetter(PC& pc, uint32_t quest_index, int state)
{
    return ExecuteEventScript(pc, QUEST_LETTER_EVENT, quest_index, state);
}

// Think - 25/04/14
bool NPC::OnReceive(PC& pc, uint32_t quest_index)
{
    PC::QuestInfoIterator itPCQuest = pc.quest_find(quest_index);

    int cState = 0;
    if (itPCQuest != pc.quest_end())
        cState = itPCQuest->second.st;

    return ExecuteEventScript(pc, QUEST_RECEIVE_EVENT, quest_index, cState);
}

// end

bool NPC::OnTarget(PC& pc, uint32_t dwQuestIndex, const char* c_pszTargetName,
                   const char* c_pszVerb, bool& bRet)
{
    SPDLOG_TRACE("OnTarget begin %s verb %s qi %u", c_pszTargetName, c_pszVerb,
                 dwQuestIndex);

    bRet = false;

    PC::QuestInfoIterator itPCQuest = pc.quest_find(dwQuestIndex);

    if (itPCQuest == pc.quest_end()) {
        SPDLOG_TRACE("no quest");
        return false;
    }

    int iState = itPCQuest->second.st;

    AArgQuestScriptType& r = m_mapOwnArgQuest[QUEST_TARGET_EVENT][dwQuestIndex];
    AArgQuestScriptType::iterator it = r.find(iState);

    if (it == r.end()) {
        SPDLOG_TRACE("no target event, state %d", iState);
        return false;
    }

    vector<AArgScript>::iterator it_vec = it->second.begin();

    int iTargetLen = strlen(c_pszTargetName);

    while (it_vec != it->second.end()) {
        AArgScript& argScript = *(it_vec++);
        const char* c_pszArg = argScript.arg.c_str();

        SPDLOG_WARN("OnTarget compare %s %d", c_pszArg, argScript.arg.length());

        if (strncmp(c_pszArg, c_pszTargetName, iTargetLen))
            continue;

        const char* c_pszArgVerb = strchr(c_pszArg, '.');

        if (!c_pszArgVerb)
            continue;

        if (strcmp(++c_pszArgVerb, c_pszVerb))
            continue;

        if (argScript.when_condition.size() > 0)
            SPDLOG_WARN("OnTarget when %s size %d",
                        &argScript.when_condition[0],
                        argScript.when_condition.size());

        if (argScript.when_condition.size() != 0 &&
            !IsScriptTrue(&argScript.when_condition[0],
                          argScript.when_condition.size()))
            continue;

        SPDLOG_WARN("OnTarget execute qi %u st %d code %s", dwQuestIndex,
                    iState, (const char*)argScript.script.GetCode());
        bRet = CQuestManager::instance().ExecuteQuestScript(
            pc, dwQuestIndex, iState, argScript.script.GetCode(),
            argScript.script.GetSize());
        bRet = true;
        return true;
    }

    return false;
}

bool NPC::OnAttrIn(PC& pc)
{
    return HandleEvent(pc, QUEST_ATTR_IN_EVENT);
}

bool NPC::OnAttrOut(PC& pc)
{
    return HandleEvent(pc, QUEST_ATTR_OUT_EVENT);
}

bool NPC::OnTakeItem(PC& pc, uint32_t itemId)
{
    return HandleEvent(pc, QUEST_ITEM_TAKE_EVENT, itemId);
}

bool NPC::OnUseItem(PC& pc, bool bReceiveAll, uint32_t itemId)
{
    bool result;
    if (bReceiveAll)
        result = HandleReceiveAllEvent(pc, QUEST_ITEM_USE_EVENT, itemId);
    else
        result = HandleEvent(pc, QUEST_ITEM_USE_EVENT, itemId);

    return result;
}

bool NPC::OnUseItemToItem(PC& pc, uint32_t itemId)
{
    return HandleEvent(pc, QUEST_ITEM_TO_ITEM_EVENT, itemId);
}

bool NPC::OnSIGUse(PC& pc, bool bReceiveAll, uint32_t itemId)
{
    bool result;
    if (bReceiveAll)
        result = HandleReceiveAllEvent(pc, QUEST_SIG_USE_EVENT, itemId);
    else
        result = HandleEvent(pc, QUEST_SIG_USE_EVENT, itemId);

    return result;
}

bool NPC::OnClick(PC& pc)
{
    return HandleEvent(pc, QUEST_CLICK_EVENT);
}

bool NPC::OnServerTimer(PC& pc)
{
    return HandleReceiveAllEvent(pc, QUEST_SERVER_TIMER_EVENT);
}

bool NPC::OnDungeonStageChange(PC& pc)
{
    return HandleEvent(pc, QUEST_DUNGEON_STAGE_CHANGE_EVENT);
}

bool NPC::OnDungeonEliminatedEvent(PC& pc)
{
    return HandleEvent(pc, QUEST_DUNGEON_ON_ELIMINATED_EVENT);
}

bool NPC::OnTimer(PC& pc)
{
    return HandleEvent(pc, QUEST_TIMER_EVENT);
}

bool NPC::OnKill(PC& pc)
{
    // PROF_UNIT puOnKill("quest::NPC::OnKill");
    if (m_vnum) {
        // PROF_UNIT puOnKill1("onk1");
        return HandleEvent(pc, QUEST_KILL_EVENT);
    } else {
        // PROF_UNIT puOnKill2("onk2");
        return HandleReceiveAllEvent(pc, QUEST_KILL_EVENT);
    }
}

bool NPC::OnDie(PC& pc)
{
    if (m_vnum)
        return HandleEvent(pc, QUEST_DIE_EVENT);
    else
        return HandleReceiveAllEvent(pc, QUEST_DIE_EVENT);
}

bool NPC::OnBuy(PC& pc, uint32_t itemId)
{
    if (m_vnum)
        return HandleEvent(pc, QUEST_BUY_EVENT);
    else
        return HandleReceiveAllEvent(pc, QUEST_BUY_EVENT, itemId);
}

bool NPC::OnPartyKill(PC& pc)
{
    if (m_vnum) {
        return HandleEvent(pc, QUEST_PARTY_KILL_EVENT);
    } else {
        return HandleReceiveAllEvent(pc, QUEST_PARTY_KILL_EVENT);
    }
}

bool NPC::OnLevelUp(PC& pc)
{
    return HandleReceiveAllEvent(pc, QUEST_LEVELUP_EVENT);
}

bool NPC::OnLogin(PC& pc, const char* c_pszQuestName)
{
    /*
       if (c_pszQuestName)
       {
       uint32_t dwQI =
       CQuestManager::instance().GetQuestIndexByName(c_pszQuestName);

       if (dwQI)
       {
       std::string stQuestName(c_pszQuestName);

       CQuestManager & q = CQuestManager::instance();

       QuestMapType::iterator qmit = m_mapOwnQuest[QUEST_LOGIN_EVENT].begin();

       while (qmit != m_mapOwnQuest[QUEST_LOGIN_EVENT].end())
       {
       if (qmit->first != dwQI)
       {
       ++qmit;
       continue;
       }

       int iState = pc.GetFlag(stQuestName + "__status");

       AQuestScriptType::iterator qsit;

       if ((qsit = qmit->second.find(iState)) != qmit->second.end())
       {
       return q.ExecuteQuestScript(pc, stQuestName, iState,
       qsit->second.GetCode(), qsit->second.GetSize(), NULL, true);
       }

       ++qmit;
       }

       SPDLOG_ERROR("Cannot find any code for %s", c_pszQuestName);
       }
       else
       SPDLOG_ERROR("Cannot find quest index by %s", c_pszQuestName);
       }
     */
    bool bRet = HandleReceiveAllNoWaitEvent(pc, QUEST_LOGIN_EVENT);
    HandleReceiveAllEvent(pc, QUEST_LETTER_EVENT);
    return bRet;
}

bool NPC::OnLogout(PC& pc)
{
    return HandleReceiveAllEvent(pc, QUEST_LOGOUT_EVENT);
}
bool NPC::OnDead(PC& pc)
{
    return HandleReceiveAllEvent(pc, QUEST_DEAD_EVENT);
}
bool NPC::OnUnmount(PC& pc)
{
    return HandleReceiveAllEvent(pc, QUEST_UNMOUNT_EVENT);
}

struct FuncMissHandleEvent {
    std::vector<uint32_t> vdwNewStartQuestIndices;
    int size;

    FuncMissHandleEvent()
        : vdwNewStartQuestIndices(0)
        , size(0)
    {
    }

    bool Matched() { return vdwNewStartQuestIndices.size() != 0; }

    void operator()(PC::QuestInfoIterator& itPCQuest,
                    NPC::QuestMapType::iterator& itQuestMap)
    {
        // 없으니 새로 시작
        uint32_t dwQuestIndex = itQuestMap->first;

        if (NPC::HasStartState(itQuestMap->second) &&
            CQuestManager::instance().CanStartQuest(dwQuestIndex)) {
            size++;
            vdwNewStartQuestIndices.push_back(dwQuestIndex);
        }
    }
};

struct FuncMatchHandleEvent {
    bool bMatched;

    std::vector<uint32_t> vdwQuesIndices;
    std::vector<int> viPCStates;
    std::vector<const char*> vcodes;
    std::vector<int> vcode_sizes;
    int size;

    // uint32_t dwQuestIndex;
    // int iPCState;
    // const char* code;
    // int code_size;

    FuncMatchHandleEvent()
        : bMatched(false)
        , vdwQuesIndices(0)
        , viPCStates(0)
        , vcodes(0)
        , vcode_sizes(0)
        , size(0)
    {
    }

    bool Matched() { return bMatched; }

    void operator()(PC::QuestInfoIterator& itPCQuest,
                    NPC::QuestMapType::iterator& itQuestMap)
    {
        NPC::AQuestScriptType::iterator itQuestScript;

        int iState = itPCQuest->second.st;
        if ((itQuestScript = itQuestMap->second.find(iState)) !=
            itQuestMap->second.end()) {
            bMatched = true;
            size++;
            vdwQuesIndices.push_back(itQuestMap->first);
            viPCStates.push_back(iState);
            vcodes.push_back(itQuestScript->second.GetCode());
            vcode_sizes.push_back(itQuestScript->second.GetSize());
        }
    }
};

bool NPC::HandleEvent(PC& pc, int EventIndex, uint32_t itemId)
{
    if (EventIndex < 0 || EventIndex >= QUEST_EVENT_COUNT) {
        SPDLOG_ERROR("QUEST invalid EventIndex : %d", EventIndex);
        return false;
    }

    if (pc.IsRunning()) {
        SuspendedQuestStateError(pc, EventIndex);
        return false;
    }

    CQuestManager& q = CQuestManager::instance();
    if (!q.CheckQuestLoaded(&pc)) {
        SPDLOG_ERROR("Quests not loaded for p#{} - EventIndex: {} ({}) - "
                     "CONTINUING",
                     pc.GetID(), questEventNames[EventIndex].c_str(),
                     EventIndex);
        // return false;
    }

    FuncMissHandleEvent fMiss;
    FuncMatchHandleEvent fMatch;
    MatchingQuest(pc, m_mapOwnQuest[EventIndex], fMatch, fMiss);

    bool r = false;
    if (fMatch.Matched()) {
        for (int i = 0; i < fMatch.size; i++) {
            CQuestManager::instance().ExecuteQuestScript(
                pc, fMatch.vdwQuesIndices[i], fMatch.viPCStates[i],
                fMatch.vcodes[i], fMatch.vcode_sizes[i], nullptr, true, itemId);
        }
        r = true;
    }

    if (fMiss.Matched()) {
        QuestMapType& rmapEventOwnQuest = m_mapOwnQuest[EventIndex];

        for (int i = 0; i < fMiss.size; i++) {
            AStateScriptType& script =
                rmapEventOwnQuest[fMiss.vdwNewStartQuestIndices[i]][0];
            CQuestManager::instance().ExecuteQuestScript(
                pc, fMiss.vdwNewStartQuestIndices[i], 0, script.GetCode(),
                script.GetSize(), nullptr, true, itemId);
        }
        r = true;
    } else {
        q.VerifyPC(pc);
        return r;
    }

    q.VerifyPC(pc);
    return true;
}

struct FuncMissHandleReceiveAllEvent {
    bool bHandled;
    uint32_t itemId;

    FuncMissHandleReceiveAllEvent(uint32_t itemId)
        : bHandled(false)
        , itemId(itemId)
    {
    }

    void operator()(PC::QuestInfoIterator& itPCQuest,
                    NPC::QuestMapType::iterator itQuestMap)
    {
        uint32_t dwQuestIndex = itQuestMap->first;

        if (NPC::HasStartState(itQuestMap->second) &&
            CQuestManager::instance().CanStartQuest(dwQuestIndex)) {
            const NPC::AQuestScriptType& QuestScript = itQuestMap->second;
            auto it = QuestScript.find(QUEST_START_STATE_INDEX);

            if (it != QuestScript.end()) {
                bHandled = true;
                CQuestManager::instance().ExecuteQuestScript(
                    *CQuestManager::instance().GetCurrentPC(), dwQuestIndex,
                    QUEST_START_STATE_INDEX, it->second.GetCode(),
                    it->second.GetSize(), nullptr, true, itemId);
            }
        }
    }
};

struct FuncMatchHandleReceiveAllEvent {
    bool bHandled;
    uint32_t itemId;

    FuncMatchHandleReceiveAllEvent(uint32_t itemId)
        : bHandled(false)
        , itemId(itemId)
    {
    }

    void operator()(PC::QuestInfoIterator& itPCQuest,
                    NPC::QuestMapType::iterator itQuestMap)
    {
        const NPC::AQuestScriptType& QuestScript = itQuestMap->second;
        int iPCState = itPCQuest->second.st;
        auto itQuestScript = QuestScript.find(iPCState);

        if (itQuestScript != QuestScript.end()) {
            bHandled = true;

            CQuestManager::instance().ExecuteQuestScript(
                *CQuestManager::instance().GetCurrentPC(), itQuestMap->first,
                iPCState, itQuestScript->second.GetCode(),
                itQuestScript->second.GetSize(), nullptr, true, itemId);
        }
    }
};

bool NPC::HandleReceiveAllEvent(PC& pc, int EventIndex, uint32_t itemId)
{
    if (EventIndex < 0 || EventIndex >= QUEST_EVENT_COUNT) {
        SPDLOG_ERROR("QUEST invalid EventIndex : %d", EventIndex);
        return false;
    }

    if (pc.IsRunning()) {
        SuspendedQuestStateError(pc, EventIndex);
        return false;
    }

    CQuestManager& q = CQuestManager::instance();
    if (!q.CheckQuestLoaded(&pc) && EventIndex != QUEST_SERVER_TIMER_EVENT && EventIndex != QUEST_DUNGEON_STAGE_CHANGE_EVENT && EventIndex != QUEST_DUNGEON_ON_ELIMINATED_EVENT) {
        SPDLOG_ERROR("Quests not loaded for pc#{} - Event: {} ({}) - "
                     "CONTINUING",
                     pc.GetID(), questEventNames[EventIndex].c_str(),
                     EventIndex);
        // return false;
    }

    FuncMissHandleReceiveAllEvent fMiss(itemId);
    FuncMatchHandleReceiveAllEvent fMatch(itemId);

    MatchingQuest(pc, m_mapOwnQuest[EventIndex], fMatch, fMiss);

    q.VerifyPC(pc);
    return fMiss.bHandled || fMatch.bHandled;
}

struct FuncDoNothing {
    void operator()(PC::QuestInfoIterator& itPCQuest,
                    NPC::QuestMapType::iterator& itQuestMap)
    {
    }
};

struct FuncMissHandleReceiveAllNoWaitEvent {
    bool bHandled;

    FuncMissHandleReceiveAllNoWaitEvent()
        : bHandled(false)
    {
    }

    void operator()(PC::QuestInfoIterator& itPCQuest,
                    NPC::QuestMapType::iterator& itQuestMap)
    {
        uint32_t dwQuestIndex = itQuestMap->first;

        if (NPC::HasStartState(itQuestMap->second) &&
            CQuestManager::instance().CanStartQuest(dwQuestIndex)) {
            const NPC::AQuestScriptType& QuestScript = itQuestMap->second;
            auto it = QuestScript.find(QUEST_START_STATE_INDEX);
            if (it != QuestScript.end()) {
                bHandled = true;
                PC* pPC = CQuestManager::instance().GetCurrentPC();
                if (CQuestManager::instance().ExecuteQuestScript(
                        *pPC, dwQuestIndex, QUEST_START_STATE_INDEX,
                        it->second.GetCode(), it->second.GetSize())) {
                    SPDLOG_ERROR("QUEST NOT END RUNNING on Login/Logout - %s",
                                 CQuestManager::instance()
                                     .GetQuestNameByIndex(itQuestMap->first)
                                     .c_str());

                    QuestState& rqs = *pPC->GetRunningQuestState();
                    CQuestManager::instance().CloseState(rqs);
                    pPC->EndRunning();
                }
            }
        }
    }
};

struct FuncMatchHandleReceiveAllNoWaitEvent {
    bool bHandled;

    FuncMatchHandleReceiveAllNoWaitEvent()
        : bHandled(false)
    {
    }

    void operator()(PC::QuestInfoIterator& itPCQuest,
                    NPC::QuestMapType::iterator& itQuestMap)
    {
        const NPC::AQuestScriptType& QuestScript = itQuestMap->second;
        int iPCState = itPCQuest->second.st;
        auto itQuestScript = QuestScript.find(iPCState);

        if (itQuestScript != QuestScript.end()) {
            PC* pPC = CQuestManager::instance().GetCurrentPC();

            if (CQuestManager::instance().ExecuteQuestScript(
                    *pPC, itQuestMap->first, iPCState,
                    itQuestScript->second.GetCode(),
                    itQuestScript->second.GetSize())) {
                SPDLOG_ERROR("QUEST NOT END RUNNING on Login/Logout - %s",
                             CQuestManager::instance()
                                 .GetQuestNameByIndex(itQuestMap->first)
                                 .c_str());

                QuestState& rqs = *pPC->GetRunningQuestState();
                CQuestManager::instance().CloseState(rqs);
                pPC->EndRunning();
            }
            bHandled = true;
        }
    }
};

bool NPC::HandleReceiveAllNoWaitEvent(PC& pc, int EventIndex)
{
    if (EventIndex < 0 || EventIndex >= QUEST_EVENT_COUNT) {
        SPDLOG_ERROR("QUEST invalid EventIndex : %d", EventIndex);
        return false;
    }

    // FuncDoNothing fMiss;
    FuncMissHandleReceiveAllNoWaitEvent fMiss;
    FuncMatchHandleReceiveAllNoWaitEvent fMatch;

    QuestMapType& rmapEventOwnQuest = m_mapOwnQuest[EventIndex];
    MatchingQuest(pc, rmapEventOwnQuest, fMatch, fMiss);

    return fMatch.bHandled || fMiss.bHandled;
}

bool NPC::OnInfo(PC& pc, unsigned int quest_index)
{
    const int EventIndex = QUEST_INFO_EVENT;

    if (pc.IsRunning()) {
        SuspendedQuestStateError(pc, EventIndex);
        return false;
    }

    PC::QuestInfoIterator itPCQuest = pc.quest_find(quest_index);
    if (pc.quest_end() == itPCQuest) {
        SPDLOG_ERROR("QUEST no quest by (quest %d)", quest_index);
        return false;
    }

    QuestMapType& rmapEventOwnQuest = m_mapOwnQuest[EventIndex];
    QuestMapType::iterator itQuestMap = rmapEventOwnQuest.find(quest_index);

    const char* questName =
        CQuestManager::instance().GetQuestNameByIndex(quest_index).c_str();

    if (itQuestMap == rmapEventOwnQuest.end()) {
        SPDLOG_ERROR("QUEST no info event (quest %s)", questName);
        return false;
    }

    AQuestScriptType::iterator itQuestScript =
        itQuestMap->second.find(itPCQuest->second.st);
    if (itQuestScript == itQuestMap->second.end()) {
        SPDLOG_ERROR("QUEST no info script by state %d (quest %s)",
                     itPCQuest->second.st, questName);
        return false;
    }

    CQuestManager::instance().ExecuteQuestScript(
        pc, quest_index, itPCQuest->second.st, itQuestScript->second.GetCode(),
        itQuestScript->second.GetSize());
    return true;
}

bool NPC::OnButton(PC& pc, unsigned int quest_index)
{
    const int EventIndex = QUEST_BUTTON_EVENT;

    if (pc.IsRunning()) {
        if (gConfig.testServer) {
            CQuestManager& mgr = CQuestManager::instance();

            SPDLOG_ERROR("QUEST There's suspended quest state, can't run new "
                         "quest state (quest: %s pc: %s)",
                         pc.GetCurrentQuestName().c_str(),
                         mgr.GetCurrentCharacterPtr()
                             ? mgr.GetCurrentCharacterPtr()->GetName()
                             : "<none>");
        }

        return false;
    }

    PC::QuestInfoIterator itPCQuest = pc.quest_find(quest_index);

    QuestMapType& rmapEventOwnQuest = m_mapOwnQuest[EventIndex];
    QuestMapType::iterator itQuestMap = rmapEventOwnQuest.find(quest_index);

    // 그런 퀘스트가 없음
    if (itQuestMap == rmapEventOwnQuest.end())
        return false;

    int iState = 0;

    if (itPCQuest != pc.quest_end()) {
        iState = itPCQuest->second.st;
    } else {
        // 새로 시작할까요?
        if (CQuestManager::instance().CanStartQuest(itQuestMap->first, pc) &&
            HasStartState(itQuestMap->second))
            iState = 0;
        else
            return false;
    }

    AQuestScriptType::iterator itQuestScript = itQuestMap->second.find(iState);

    if (itQuestScript == itQuestMap->second.end())
        return false;

    CQuestManager::instance().ExecuteQuestScript(
        pc, quest_index, iState, itQuestScript->second.GetCode(),
        itQuestScript->second.GetSize());
    return true;
}

struct FuncMissChatEvent {
    FuncMissChatEvent(vector<AArgScript*>& rAvailScript)
        : rAvailScript(rAvailScript)
    {
    }

    void operator()(PC::QuestInfoIterator& itPCQuest,
                    NPC::ArgQuestMapType::iterator& itQuestMap)
    {
        if (CQuestManager::instance().CanStartQuest(itQuestMap->first) &&
            NPC::HasStartState(itQuestMap->second)) {
            size_t i;
            for (i = 0; i < itQuestMap->second[QUEST_START_STATE_INDEX].size();
                 ++i) {
                if (itQuestMap->second[QUEST_START_STATE_INDEX][i]
                        .when_condition.empty() ||
                    IsScriptTrue(&itQuestMap->second[QUEST_START_STATE_INDEX][i]
                                      .when_condition[0],
                                 itQuestMap->second[QUEST_START_STATE_INDEX][i]
                                     .when_condition.size()))
                    rAvailScript.push_back(
                        &itQuestMap->second[QUEST_START_STATE_INDEX][i]);
            }
        }
    }

    vector<AArgScript*>& rAvailScript;
};

struct FuncMatchChatEvent {
    FuncMatchChatEvent(vector<AArgScript*>& rAvailScript)
        : rAvailScript(rAvailScript)
    {
    }

    void operator()(PC::QuestInfoIterator& itPCQuest,
                    NPC::ArgQuestMapType::iterator& itQuestMap)
    {
        int iState = itPCQuest->second.st;
        auto itQuestScript = itQuestMap->second.find(iState);
        if (itQuestScript != itQuestMap->second.end()) {
            size_t i;
            for (i = 0; i < itQuestMap->second[iState].size(); i++) {
                if (itQuestMap->second[iState][i].when_condition.empty() ||
                    IsScriptTrue(
                        &itQuestMap->second[iState][i].when_condition[0],
                        itQuestMap->second[iState][i].when_condition.size()))
                    rAvailScript.push_back(&itQuestMap->second[iState][i]);
            }
        }
    }

    vector<AArgScript*>& rAvailScript;
};

bool NPC::OnChat(PC& pc)
{
    const int EventIndex = QUEST_CHAT_EVENT;

    auto& mgr = CQuestManager::instance();
    if (pc.IsRunning()) {
        SuspendedQuestStateError(pc, EventIndex);
        return false;
    }

    std::vector<AArgScript*> AvailScript;

    FuncMatchChatEvent fMatch(AvailScript);
    FuncMissChatEvent fMiss(AvailScript);
    MatchingQuest(pc, m_mapOwnArgQuest[EventIndex], fMatch, fMiss);

    if (AvailScript.empty())
        return false;

    const Locale* locale = nullptr;
    if (mgr.GetCurrentCharacterPtr())
        locale = GetLocale(mgr.GetCurrentCharacterPtr());
    else
        locale = &GetLocaleService().GetDefaultLocale();

    {
        std::ostringstream os;
        os << "select(";
        os << '"' << ScriptToString(locale->name, AvailScript[0]->arg) << '"';
        for (size_t i = 1; i < AvailScript.size(); i++) {
            os << ",\"" << ScriptToString(locale->name, AvailScript[i]->arg)
               << '"';
        }
        os << ", '" << LC_TEXT_LC("닫기", locale) << "'";
        os << ")";

        mgr.ExecuteQuestScript(pc, "QUEST_CHAT_TEMP_QUEST", 0, os.str().c_str(),
                               os.str().size(), &AvailScript, false);
    }

    return true;
}

bool NPC::HasChat()
{
    return !m_mapOwnArgQuest[QUEST_CHAT_EVENT].empty();
}

bool NPC::ExecuteEventScript(PC& pc, int EventIndex, uint32_t dwQuestIndex,
                             int iState, uint32_t itemId)
{
    QuestMapType& rQuest = m_mapOwnQuest[EventIndex];

    auto itQuest = rQuest.find(dwQuestIndex);
    if (itQuest == rQuest.end()) {
        SPDLOG_TRACE("ExecuteEventScript ei %d qi %u is %d - NO QUEST",
                     EventIndex, dwQuestIndex, iState);
        return false;
    }

    AQuestScriptType& rScript = itQuest->second;
    auto itState = rScript.find(iState);
    if (itState == rScript.end()) {
        SPDLOG_TRACE("ExecuteEventScript ei %d qi %u is %d - NO STATE",
                     EventIndex, dwQuestIndex, iState);
        return false;
    }

    SPDLOG_TRACE("ExecuteEventScript ei %d qi %u is %d", EventIndex,
                 dwQuestIndex, iState);
    CQuestManager::instance().SetCurrentEventIndex(EventIndex);
    return CQuestManager::instance().ExecuteQuestScript(
        pc, dwQuestIndex, iState, itState->second.GetCode(),
        itState->second.GetSize(), nullptr, true, itemId);
}

bool NPC::OnPickupItem(PC& pc, uint32_t itemId)
{
    bool result;
    if (m_vnum == 0)
        result = HandleReceiveAllEvent(pc, QUEST_ITEM_PICK_EVENT, itemId);
    else
        result = HandleEvent(pc, QUEST_ITEM_PICK_EVENT, itemId);

    return result;
}
} // namespace quest
