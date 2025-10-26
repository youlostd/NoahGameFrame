#include "questmanager.h"
#include "CharUtil.hpp"
#include "ChatUtil.hpp"
#include "DbCacheSocket.hpp"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "desc.h"
#include "desc_manager.h"
#include "dungeon.h"
#include "guild.h"
#include "item.h"
#include "item_manager.h"
#include "lzo_manager.h"
#include "party.h"
#include "target.h"
#include "xmas_event.h"
#include <boost/tokenizer.hpp>
#include <fstream>
#include <game/GamePacket.hpp>
#ifdef WIN32
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif
namespace quest
{
using namespace std;

CQuestManager::CQuestManager()
    : m_pSelectedDungeon(nullptr)
      , m_dwServerTimerArg(0)
      , m_iRunningEventIndex(0)
      , L(nullptr)
      , m_bNoSend(false)
      , m_CurrentRunningState(nullptr)
      , m_pCurrentCharacter(nullptr)
      , m_pCurrentNPCCharacter(nullptr)
      , m_pCurrentPartyMember(nullptr)
      , m_pCurrentPC(nullptr)
      , m_bQuestInfoFlag{false}, m_iCurrentSkin(0)
      , m_bError(false)
      , m_pOtherPCBlockRootPC(nullptr)
{
}

CQuestManager::~CQuestManager()
{
    Destroy();
}

void CQuestManager::Destroy()
{
    if (L) {
        lua_close(L);
        L = nullptr;
    }
}

bool CQuestManager::Initialize()
{
    if (gConfig.authServer)
        return true;

    if (!InitializeLua())
        return false;

    m_pSelectedDungeon = nullptr;

    m_mapEventName.insert({"click", QUEST_CLICK_EVENT}); // NPC를 클릭
    m_mapEventName.insert({"kill", QUEST_KILL_EVENT});   // Mob을 사냥
    m_mapEventName.insert(
        {"timer", QUEST_TIMER_EVENT}); // 미리 지정해둔 시간이 지남
    m_mapEventName.insert({"levelup", QUEST_LEVELUP_EVENT}); // 레벨업을 함
    m_mapEventName.insert({"login", QUEST_LOGIN_EVENT});     // 로그인 시
    m_mapEventName.insert({"logout", QUEST_LOGOUT_EVENT});   // 로그아웃 시
    m_mapEventName.insert({"button", QUEST_BUTTON_EVENT}); // 퀘스트 버튼을 누름
    m_mapEventName.insert({"info", QUEST_INFO_EVENT}); // 퀘스트 정보창을 염
    m_mapEventName.insert(
        {"chat", QUEST_CHAT_EVENT}); // 특정 키워드로 대화를 함
    m_mapEventName.insert(
        {"in", QUEST_ATTR_IN_EVENT}); // 맵의 특정 속성에 들어감
    m_mapEventName.insert(
        {"out", QUEST_ATTR_OUT_EVENT}); // 맵의 특정 속성에서 나옴
    m_mapEventName.insert(
        {"use", QUEST_ITEM_USE_EVENT}); // 퀘스트 아이템을 사용
    m_mapEventName.insert(
        {"server_timer",
         QUEST_SERVER_TIMER_EVENT}); // 서버 타이머 (아직 테스트 안됐음)
    m_mapEventName.insert(
        {"enter", QUEST_ENTER_STATE_EVENT}); // 현재 스테이트가 됨
    m_mapEventName.insert(
        {"leave",
         QUEST_LEAVE_STATE_EVENT}); // 현재 스테이트에서 다른 스테이트로 바뀜
    m_mapEventName.insert(
        {"letter", QUEST_LETTER_EVENT}); // 로긴 하거나 스테이트가 바껴 새로
                                         // 정보를 세팅해줘야함
    m_mapEventName.insert({"take", QUEST_ITEM_TAKE_EVENT}); // 아이템을 받음
    m_mapEventName.insert({"target", QUEST_TARGET_EVENT}); // 타겟
    m_mapEventName.insert(
        {"party_kill",
         QUEST_PARTY_KILL_EVENT}); // 파티 멤버가 몬스터를 사냥 (리더에게 옴)
    m_mapEventName.insert({"unmount", QUEST_UNMOUNT_EVENT});
    m_mapEventName.insert(
        {"pick", QUEST_ITEM_PICK_EVENT}); // 떨어져있는 아이템을 습득함.
    m_mapEventName.insert(
        {"sig_use",
         QUEST_SIG_USE_EVENT}); // Special item group에 속한 아이템을 사용함.

    // Martin - 2014
    m_mapEventName.insert({"die", QUEST_DIE_EVENT});
    m_mapEventName.insert({"buy", QUEST_BUY_EVENT});

    m_mapEventName.insert({"receive", QUEST_RECEIVE_EVENT});
    m_mapEventName.insert(
        {"dungeon_stage_change", QUEST_DUNGEON_STAGE_CHANGE_EVENT});
    m_mapEventName.insert(
        {"dungeon_on_eliminated", QUEST_DUNGEON_ON_ELIMINATED_EVENT});
    m_mapEventName.insert({"dead", QUEST_DEAD_EVENT});
    m_mapEventName.insert(
        {"use_to_item", QUEST_ITEM_TO_ITEM_EVENT}); // 퀘스트 아이템을 사용
    m_bNoSend = false;

    m_iCurrentSkin = QUEST_SKIN_NORMAL;

    {
        ifstream inf("data/quest/questnpc.txt");

        if (!inf.is_open())
            SPDLOG_ERROR("QUEST Cannot open 'questnpc.txt'");
        else
            SPDLOG_TRACE("QUEST can open 'data/quest/questnpc.txt'");

        typedef boost::tokenizer<boost::char_separator<char>> Tokenizer;
        boost::char_separator<char> sep{" "};
        vector<string> vec;
        string line;

        while (getline(inf, line)) {
            Tokenizer tok(line, sep);
            vec.assign(tok.begin(), tok.end());
            if (vec.size() < 2)
                continue;

            auto vnum = std::stoul(vec[0]);
            auto npcName = vec[1];

            m_mapNPC[vnum].Set(vnum, npcName);
            m_mapNPCNameID[npcName] = vnum;

            if (gConfig.testServer)
                SPDLOG_TRACE("QUEST reading script of {0}({1})",
                             npcName.c_str(), vnum);
        }

        // notarget quest
        m_mapNPC[0].Set(0, "notarget");
    }

    ReadQuestCategoryToDict();
    return true;
}

unsigned int CQuestManager::FindNPCIDByName(const string& name)
{
    map<string, unsigned int>::iterator it = m_mapNPCNameID.find(name);
    return it != m_mapNPCNameID.end() ? it->second : 0;
}

void CQuestManager::SelectItem(unsigned int pc, unsigned int selection)
{
    PC* pPC = GetPC(pc);

    if (!pPC) {
        SPDLOG_ERROR("no pc! : {}", pc);
        return;
    }

    quest::QuestState* state = pPC->GetRunningQuestState();
    if (pPC->IsRunning() && state->suspend_state == SUSPEND_STATE_SELECT_ITEM) {
        state->suspend_state = SUSPEND_STATE_NONE;
        pPC->SetSendDoneFlag();
        state->args = 1;
        lua_pushnumber(state->co, selection);

        if (!RunState(*state)) {
            CloseState(*state);
            pPC->EndRunning();
        }
    }
}

void CQuestManager::Confirm(unsigned int pc, EQuestConfirmType confirm,
                            unsigned int pc2)
{
    PC* pPC = GetPC(pc);

    if (!pPC->IsRunning()) {
        SPDLOG_ERROR("no quest running for pc, cannot process input : %u", pc);
        return;
    }

    QuestState* state = pPC->GetRunningQuestState();
    if (state->suspend_state != SUSPEND_STATE_CONFIRM) {
        SPDLOG_ERROR("not wait for a confirm : %u %d", pc,
                     state->suspend_state);
        return;
    }

    if (pc2 && !pPC->IsConfirmWait(pc2)) {
        SPDLOG_ERROR("not wait for a confirm : %u %d", pc,
                     state->suspend_state);
        return;
    }

    pPC->ClearConfirmWait();

    pPC->SetSendDoneFlag();

    state->args = 1;
    lua_pushnumber(state->co, confirm);

    AddScript("[END_CONFIRM_WAIT]");
    SetSkinStyle(QUEST_SKIN_NOWINDOW);
    SendScript();

    if (!RunState(*state)) {
        CloseState(*state);
        pPC->EndRunning();
    }
}

/****
 * Get QuestCategoryIndex by Quest if available
 */
int CQuestManager::GetQuestCategoryByQuestIndex(uint16_t q_index)
{
    if (QuestCategoryIndexMap.find(q_index) != QuestCategoryIndexMap.end())
        return QuestCategoryIndexMap[q_index];
    else
        return 6; // Default category if not found
}

void CQuestManager::ReadQuestCategoryToDict()
{
    // Clear the map to avoid old data!
    // MartPwnS 08.03.2014 16:00
    if (!QuestCategoryIndexMap.empty())
        QuestCategoryIndexMap.clear();

    ifstream inf("data/quest/questcategory.txt");

    if (!inf.is_open()) {
        SPDLOG_ERROR("QUEST Cannot open 'questcategory.txt'");
        return;
    }

    string lineFromFile;
    while (getline(inf, lineFromFile)) {
        if (lineFromFile.empty()) // Skip empty lines
            continue;

        boost::tokenizer<boost::escaped_list_separator<char>> token(
            lineFromFile,
            boost::escaped_list_separator<char>('\\', '\t', '\"'));
        vector<string> data(token.begin(), token.end());

        int category_num = atoi(data[0].c_str());
        string quest_name = data[1];

        unsigned int quest_index =
            CQuestManager::instance().GetQuestIndexByName(quest_name);

        if (gConfig.testServer)
            SPDLOG_TRACE("QUEST_CATEGORY_LINE: {0} => {1}, {2}",
                         lineFromFile.c_str(), data[0].c_str(),
                         quest_name.c_str());

        if (quest_index != 0)
            QuestCategoryIndexMap[quest_index] = category_num;
        else
            SPDLOG_ERROR("QUEST couldnt find QuestIndex for name Quest: "
                         "{0}({1})",
                         quest_name.c_str(), category_num);
    }
}

void CQuestManager::Input(unsigned int pc, const char* msg, int qIndex)
{
    PC* pPC = GetPC(pc);

    if (!pPC) {
        SPDLOG_ERROR("no pc! : {}", pc);
        return;
    }

    const auto it = pPC->quest_find(qIndex);
    if (it == pPC->quest_end()) {
        SPDLOG_ERROR("quest not running for pc, cannot process input: p#%u",
                     qIndex, pc);
        return;
    }

    QuestState& qs = it->second;
    if (qs.suspend_state != SUSPEND_STATE_INPUT) {
        SPDLOG_ERROR("not waiting for a input: p#%u, state %d", qIndex, pc,
                     qs.suspend_state);
        return;
    }

    pPC->RestoreQuestState(&qs);
    pPC->SetSendDoneFlag();

    qs.args = 1;
    lua_pushstring(qs.co, msg);

    if (!RunState(qs)) {
        CloseState(qs);
        pPC->EndRunning();
    }
}

void CQuestManager::Select(unsigned int pc, unsigned int selection, int qIndex)
{
    PC* pPC = GetPC(pc);
    if (!pPC) {
        SPDLOG_ERROR("no pc! : {}", pc);
        return;
    }

    const auto it = pPC->quest_find(qIndex);
    if (it != pPC->quest_end()) {
        QuestState& qs = it->second;
        if (qs.suspend_state == SUSPEND_STATE_SELECT) {
            pPC->RestoreQuestState(&qs);
            qs.suspend_state = SUSPEND_STATE_NONE;
            pPC->SetSendDoneFlag();

            if (!qs.chat_scripts.empty()) {
                CloseState(qs);

                if (selection >= qs.chat_scripts.size()) {
                    pPC->SetSendDoneFlag();
                    GotoEndState(qs);
                    pPC->EndRunning();
                } else {
                    AArgScript* pas = qs.chat_scripts[selection];
                    ExecuteQuestScript(*pPC, pas->quest_index, pas->state_index,
                                       pas->script.GetCode(),
                                       pas->script.GetSize());
                }
            } else {
                // The selection data we receive is 0-based and quests work with
                // 1-based
                selection = selection + 1;

                qs.args = 1;
                lua_pushnumber(qs.co, selection);

                if (!RunState(qs)) {
                    CloseState(qs);
                    pPC->EndRunning();
                }
            }

            return;
        } else {
            SPDLOG_ERROR("wrong QUEST_SELECT request! p#{}, quest {}({}) is "
                         "not on select state, but on {} state",
                         pc, qIndex, GetQuestNameByIndex(qIndex).c_str(),
                         qs.suspend_state);
        }
    } else {
        SPDLOG_ERROR("wrong QUEST_SELECT request! p#{}, quest {}({}), could "
                     "not be found",
                     pc, qIndex, GetQuestNameByIndex(qIndex).c_str());
    }
}

void CQuestManager::Resume(unsigned int pc, int qIndex)
{
    PC* pPC = GetPC(pc);
    if (!pPC) {
        SPDLOG_ERROR("no pc! : {}", pc);
        return;
    }

    const auto it = pPC->quest_find(qIndex);
    if (it != pPC->quest_end()) {
        QuestState& qs = it->second;
        if (qs.suspend_state == SUSPEND_STATE_PAUSE) {
            pPC->RestoreQuestState(&qs);
            qs.suspend_state = SUSPEND_STATE_NONE;
            pPC->SetSendDoneFlag();
            qs.args = 0;

            if (!RunState(qs)) {
                CloseState(qs);
                pPC->EndRunning();
            }

            return;
        }
    }

    SPDLOG_ERROR("wrong QUEST_WAIT request! p#{}, quest {}", pc, qIndex);
}

void CQuestManager::EnterState(uint32_t pc, uint32_t quest_index, int state,
                               CItem* qItem /*= nullptr*/)
{
    PC* pPC;
    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return;

        m_mapNPC[QUEST_NO_NPC].OnEnterState(*pPC, quest_index, state, qItem ? qItem->GetID() : 0);
    } else
        SPDLOG_ERROR("QUEST no such pc id : {}", pc);
}

void CQuestManager::LeaveState(uint32_t pc, uint32_t quest_index, int state)
{
    PC* pPC;
    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return;

        m_mapNPC[QUEST_NO_NPC].OnLeaveState(*pPC, quest_index, state);
    } else
        SPDLOG_ERROR("QUEST no such pc id : {}", pc);
}

void CQuestManager::Receive(unsigned int pc, const char* cmd,
                            uint32_t quest_index)
{
    PC* pPC = GetPC(pc);
    if (!pPC) {
        SPDLOG_ERROR("no pc! : {}", pc);
        return;
    }

    pPC->SetCommand(cmd);
    m_mapNPC[QUEST_NO_NPC].OnReceive(*pPC, quest_index);
    pPC->ClearCommand();
}

void CQuestManager::Letter(uint32_t pc, uint32_t quest_index, int state)
{
    PC* pPC;
    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return;

        m_mapNPC[QUEST_NO_NPC].OnLetter(*pPC, quest_index, state);
    } else
        SPDLOG_ERROR("QUEST no such pc id : {}", pc);
}

void CQuestManager::LogoutPC(CHARACTER* ch)
{
    PC* pPC = GetPC(ch->GetPlayerID());

    if (pPC && pPC->IsRunning()) {
        CloseState(*pPC->GetRunningQuestState());
        pPC->CancelRunning();
    }

    // 지우기 전에 로그아웃 한다.
    Logout(ch->GetPlayerID());

    if (ch == m_pCurrentCharacter) {
        m_pCurrentCharacter = nullptr;
        m_pCurrentPC = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////
// NEW QUEST TRIGGERS
//////////////////////////////////////////////////////////////////////////
void CQuestManager::Die(unsigned int pc, unsigned int npc)
{
    PC* pPC;

    SPDLOG_TRACE("CQuestManager::OnDie QUEST_DIE_EVENT (pc=%d, npc=%d)", pc,
                 npc);

    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return;

        if (npc != QUEST_NO_NPC)
            m_mapNPC[npc].OnDie(*pPC);

        m_mapNPC[QUEST_NO_NPC].OnDie(*pPC);
    } else
        SPDLOG_ERROR("QUEST: no such pc id : %d", pc);
}

// Triggers if you buy an item
//	quest::CQuestManager::instance().OnBuy(ch->GetPlayerID(), r_item);
void CQuestManager::Buy(unsigned int pc, CItem* item)
{
    PC* pPC;

    SPDLOG_TRACE("CQuestManager::OnBuy QUEST_BUY_EVENT (pc=%d,)", pc);

    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return;

        m_mapNPC[item->GetVnum()].OnBuy(*pPC,item ? item->GetID() : 0);
        m_mapNPC[QUEST_NO_NPC].OnBuy(*pPC, item ? item->GetID() : 0);
    } else
        SPDLOG_ERROR("QUEST: no such pc id : %d", pc);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// BASIC QUEST TRIGGERS
//
///////////////////////////////////////////////////////////////////////////////////////////
void CQuestManager::Login(unsigned int pc, const char* c_pszQuest)
{
    PC* pPC;

    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return;

        m_mapNPC[QUEST_NO_NPC].OnLogin(*pPC, c_pszQuest);
    } else {
        SPDLOG_ERROR("QUEST no such pc id : {}", pc);
    }
}

void CQuestManager::Dead(unsigned int pc)
{
    PC* pPC;

    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return;

        m_mapNPC[QUEST_NO_NPC].OnDead(*pPC);
    } else
        SPDLOG_ERROR("QUEST no such pc id : {}", pc);
}

void CQuestManager::Logout(unsigned int pc)
{
    PC* pPC;

    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return;

        m_mapNPC[QUEST_NO_NPC].OnLogout(*pPC);
    } else
        SPDLOG_ERROR("QUEST no such pc id : {}", pc);
}

struct FuncPartyKill {
    quest::NPC& questNPC;
    FuncPartyKill(NPC& npc)
        : questNPC(npc)
    {
    }

    void operator()(CHARACTER* ch)
    {
        questNPC.OnPartyKill(
            *(quest::CQuestManager::instance().GetPC(ch->GetPlayerID())));
    }
};

void CQuestManager::VerifyPC(PC& expectedPC)
{

    PC* pPC = GetCurrentPC();
    CHARACTER* pChar = GetCurrentCharacterPtr();
    std::string qName = GetCurrentQuestName();
    if (pPC && pChar) {
        if (pPC->GetID() != pChar->GetPlayerID() ||
            pPC->GetID() != expectedPC.GetID()) {
            SPDLOG_ERROR("Quest: {}  - Unexpected character. PC:{}, CPID:{}, "
                         "expected:{}.",
                         qName.c_str(), pPC->GetID(), pChar->GetPlayerID(),
                         expectedPC.GetID());
        }
    } else if (expectedPC.GetID() != 0) {
        SPDLOG_ERROR("Quest: {} - Invalid pc/lpc: {:p}/{:p} [expected: {}]",
                     qName.c_str(), fmt::ptr(pPC), fmt::ptr(pChar),
                     expectedPC.GetID(), qName.c_str());
    }
}

void CQuestManager::Kill(unsigned int pc, unsigned int npc)
{
    // m_CurrentNPCRace = npc;
    PC* pPC;

    // SPDLOG_TRACE("CQuestManager::Kill QUEST_KILL_EVENT (pc={}, npc={})", pc,
    // npc);

    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return;

        if (npc != QUEST_NO_NPC) // Prevent duplicate QUEST_NO_NPC calls!
            m_mapNPC[npc].OnKill(*pPC);

        m_mapNPC[QUEST_NO_NPC].OnKill(*pPC);

        auto* ch = GetCurrentCharacterPtr();
        auto* pParty = ch->GetParty();

        if (pParty) {
            if (npc != QUEST_NO_NPC) {
                FuncPartyKill f(m_mapNPC[npc]);
                pParty->ForEachOnlineMember(f);
            }

            FuncPartyKill f(m_mapNPC[QUEST_NO_NPC]);
            pParty->ForEachOnlineMember(f);

            // Restore PC
            pPC = GetPC(pc);
        } else {
            // Execute all the party_kill triggers for ourselves if we have no
            // party
            if (npc != QUEST_NO_NPC) {
                m_mapNPC[npc].OnPartyKill(*pPC);
            }

            m_mapNPC[QUEST_NO_NPC].OnPartyKill(*pPC);
        }

    } else
        SPDLOG_ERROR("QUEST: no such pc id : {}", pc);
}

bool CQuestManager::ServerTimer(unsigned int npc, unsigned int arg)
{
    SetServerTimerArg(arg);
    m_pCurrentPC = GetPCForce(0);
    m_pCurrentCharacter = nullptr;
    m_pSelectedDungeon = nullptr;
    return m_mapNPC[npc].OnServerTimer(*m_pCurrentPC);
}

bool CQuestManager::DungeonStageChange(uint32_t npc, QuestDungeon* dungeon)
{
    m_pCurrentPC = GetPCForce(0);
    m_pCurrentCharacter = nullptr;
    m_pSelectedDungeon = dungeon;
    return m_mapNPC[npc].OnDungeonStageChange(*m_pCurrentPC);
}

bool CQuestManager::DungeonEliminatedEvent(uint32_t npc, QuestDungeon* dungeon)
{
    m_pCurrentPC = GetPCForce(0);
    m_pCurrentCharacter = nullptr;
    m_pSelectedDungeon = dungeon;
    return m_mapNPC[npc].OnDungeonEliminatedEvent(*m_pCurrentPC);
}

bool CQuestManager::Timer(unsigned int pc, unsigned int npc)
{
    PC* pPC;

    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC)) {
            return false;
        }
        // call script
        return m_mapNPC[npc].OnTimer(*pPC);
    } else {
        // cout << "no such pc id : " << pc;
        SPDLOG_ERROR("QUEST TIMER_EVENT no such pc id : %d", pc);
        return false;
    }
    // cerr << "QUEST TIMER" << endl;
}

void CQuestManager::LevelUp(unsigned int pc)
{
    PC* pPC;

    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return;

        m_mapNPC[QUEST_NO_NPC].OnLevelUp(*pPC);
    } else {
        SPDLOG_ERROR("QUEST LEVELUP_EVENT no such pc id : %d", pc);
    }
}

void CQuestManager::AttrIn(unsigned int pc, CHARACTER* ch, int attr)
{
    PC* pPC;
    if ((pPC = GetPC(pc))) {
        m_pCurrentPartyMember = ch;
        if (!CheckQuestLoaded(pPC))
            return;

        // call script
        m_mapNPC[attr + QUEST_ATTR_NPC_START].OnAttrIn(*pPC);
    } else {
        // cout << "no such pc id : " << pc;
        SPDLOG_ERROR("QUEST no such pc id : {}", pc);
    }
}

void CQuestManager::AttrOut(unsigned int pc, CHARACTER* ch, int attr)
{
    PC* pPC;
    if ((pPC = GetPC(pc))) {
        // m_pCurrentCharacter = ch;
        m_pCurrentPartyMember = ch;
        if (!CheckQuestLoaded(pPC))
            return;

        // call script
        m_mapNPC[attr + QUEST_ATTR_NPC_START].OnAttrOut(*pPC);
    } else {
        // cout << "no such pc id : " << pc;
        SPDLOG_ERROR("QUEST no such pc id : {}", pc);
    }
}

bool CQuestManager::Target(unsigned int pc, uint32_t dwQuestIndex,
                           const char* c_pszTargetName, const char* c_pszVerb)
{
    PC* pPC;

    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return false;

        bool bRet;
        return m_mapNPC[QUEST_NO_NPC].OnTarget(
            *pPC, dwQuestIndex, c_pszTargetName, c_pszVerb, bRet);
    }

    return false;
}

void CQuestManager::QuestInfo(unsigned int pc, unsigned int quest_index)
{
    PC* pPC;

    if ((pPC = GetPC(pc))) {
        // call script
        if (!CheckQuestLoaded(pPC)) {
            CHARACTER* ch = g_pCharManager->FindByPID(pc);

            if (ch)
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "퀘스트를 로드하는 중입니다. 잠시만 기다려 "
                                   "주십시오.");

            return;
        }

        // Flag to distinguish between clicking on the quest and clicking on the
        // NPC in the quest window
        m_bQuestInfoFlag = 1;
        m_mapNPC[QUEST_NO_NPC].OnInfo(*pPC, quest_index);
    } else {
        // cout << "no such pc id : " << pc;
        SPDLOG_ERROR("QUEST INFO_EVENT no such pc id : %d", pc);
    }
}

void CQuestManager::QuestButton(unsigned int pc, unsigned int quest_index)
{
    PC* pPC;
    if ((pPC = GetPC(pc))) {
        // call script
        if (!CheckQuestLoaded(pPC)) {
            CHARACTER* ch = g_pCharManager->FindByPID(pc);
            if (ch) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "퀘스트를 로드하는 중입니다. 잠시만 기다려 "
                                   "주십시오.");
            }
            return;
        }
        m_mapNPC[QUEST_NO_NPC].OnButton(*pPC, quest_index);
    } else {
        // cout << "no such pc id : " << pc;
        SPDLOG_ERROR("QUEST CLICK_EVENT no such pc id : %d", pc);
    }
}

bool CQuestManager::TakeItem(unsigned int pc, unsigned int npc, CItem* item)
{
    // m_CurrentNPCRace = npc;
    PC* pPC;

    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC)) {
            CHARACTER* ch = g_pCharManager->FindByPID(pc);
            if (ch) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "퀘스트를 로드하는 중입니다. 잠시만 기다려 "
                                   "주십시오.");
            }
            return false;
        }
        // call script
        return m_mapNPC[npc].OnTakeItem(*pPC, item ? item->GetID() : 0);
    } else {
        // cout << "no such pc id : " << pc;
        SPDLOG_ERROR("QUEST USE_ITEM_EVENT no such pc id : %d", pc);
        return false;
    }
}

bool CQuestManager::UseItemToItem(unsigned int pc, unsigned int npc, CItem* item)
{
    if (gConfig.testServer)
        SPDLOG_TRACE("questmanager::UseItem Start : itemVnum : %d PC : %d",
                     item->GetOriginalVnum(), pc);
    PC* pPC;
    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC)) {
            CHARACTER* ch = g_pCharManager->FindByPID(pc);
            if (ch) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "퀘스트를 로드하는 중입니다. 잠시만 기다려 "
                                   "주십시오.");
            }
            return false;
        }
        return m_mapNPC[npc].OnUseItemToItem(*pPC, item ? item->GetID() : 0);
    } else {
        // cout << "no such pc id : " << pc;
        SPDLOG_ERROR("QUEST USE_ITEM_EVENT no such pc id : %d", pc);
        return false;
    }
}

bool CQuestManager::UseItem(unsigned int pc, CItem* item, bool bReceiveAll)
{
    if (gConfig.testServer)
        SPDLOG_TRACE("questmanager::UseItem Start : itemVnum : %d PC : %d",
                     item->GetOriginalVnum(), pc);
    PC* pPC;
    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC)) {
            CHARACTER* ch = g_pCharManager->FindByPID(pc);
            if (ch) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "퀘스트를 로드하는 중입니다. 잠시만 기다려 "
                                   "주십시오.");
            }
            return false;
        }
        return m_mapNPC[item->GetVnum()].OnUseItem(*pPC, bReceiveAll, item ? item->GetID() : 0);
    } else {
        // cout << "no such pc id : " << pc;
        SPDLOG_ERROR("QUEST USE_ITEM_EVENT no such pc id : %d", pc);
        return false;
    }
}

// Speical Item Group에 정의된 Group Use
bool CQuestManager::SIGUse(unsigned int pc, uint32_t sig_vnum, CItem* item,
                           bool bReceiveAll)
{
    if (gConfig.testServer)
        SPDLOG_TRACE("questmanager::SIGUse Start : itemVnum : %d PC : %d",
                     item->GetOriginalVnum(), pc);
    PC* pPC;
    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC)) {
            CHARACTER* ch = g_pCharManager->FindByPID(pc);
            if (ch) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "퀘스트를 로드하는 중입니다. 잠시만 기다려 "
                                   "주십시오.");
            }
            return false;
        }
        // call script
        return m_mapNPC[sig_vnum].OnSIGUse(*pPC, bReceiveAll, item ? item->GetID() : 0);
    } else {
        // cout << "no such pc id : " << pc;
        SPDLOG_ERROR("QUEST USE_ITEM_EVENT no such pc id : %d", pc);
        return false;
    }
}

bool CQuestManager::GiveItemToPC(unsigned int pc, CHARACTER* pkChr)
{
    if (!pkChr->IsPC())
        return false;

    PC* pPC = GetPC(pc);

    if (pPC) {
        if (!CheckQuestLoaded(pPC))
            return false;

        TargetInfo* pInfo = CTargetManager::instance().GetTargetInfo(
            pc, TARGET_TYPE_VID, pkChr->GetVID());

        if (pInfo) {
            bool bRet;

            if (m_mapNPC[QUEST_NO_NPC].OnTarget(*pPC, pInfo->dwQuestIndex,
                                                pInfo->szTargetName, "click",
                                                bRet))
                return true;
        }
    }

    return false;
}

bool CQuestManager::Click(unsigned int pc, CHARACTER* pkChrTarget)
{
    PC* pPC = GetPC(pc);

    if (pPC) {
        if (!CheckQuestLoaded(pPC)) {
            CHARACTER* ch = g_pCharManager->FindByPID(pc);

            if (ch)
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "퀘스트를 로드하는 중입니다. 잠시만 기다려 "
                                   "주십시오.");

            return false;
        }

        TargetInfo* pInfo = CTargetManager::instance().GetTargetInfo(
            pc, TARGET_TYPE_VID, pkChrTarget->GetVID());
        if (gConfig.testServer) {
            SPDLOG_TRACE("CQuestManager::Click(pid={}, npc_name={}) - "
                         "target_info({:p})",
                         pc, pkChrTarget->GetName(), (void*)pInfo);
        }

        if (pInfo) {
            bool bRet;
            if (m_mapNPC[QUEST_NO_NPC].OnTarget(*pPC, pInfo->dwQuestIndex,
                                                pInfo->szTargetName, "click",
                                                bRet))
                return bRet;
        }

        uint32_t dwCurrentNPCRace = pkChrTarget->GetRaceNum();

        if (pkChrTarget->IsNPC()) {
            map<unsigned int, NPC>::iterator it =
                m_mapNPC.find(dwCurrentNPCRace);

            if (it == m_mapNPC.end()) {
                SPDLOG_TRACE("CQuestManager::Click(pid=%d, target_npc_name=%s) "
                             "- NOT EXIST NPC RACE VNUM[%d]",
                             pc, pkChrTarget->GetName(), dwCurrentNPCRace);
                return false;
            }
            quest::NPC* myNPC = &it->second;

            // call script
            if (myNPC->HasChat()) {
                // if have chat, give chat
                if (gConfig.testServer)
                    SPDLOG_TRACE("CQuestManager::Click->OnChat");

                if (!myNPC->OnChat(*pPC)) {
                    if (gConfig.testServer)
                        SPDLOG_TRACE("CQuestManager::Click->OnChat Failed");

                    return myNPC->OnClick(*pPC);
                }

                return true;
            } else {
                // else click
                return myNPC->OnClick(*pPC);
            }
        }
        return false;
    } else {
        // cout << "no such pc id : " << pc;
        SPDLOG_ERROR("QUEST CLICK_EVENT no such pc id : %d", pc);
        return false;
    }
    // cerr << "QUEST CLICk" << endl;
}

void CQuestManager::Unmount(unsigned int pc)
{
    PC* pPC;

    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC))
            return;

        m_mapNPC[QUEST_NO_NPC].OnUnmount(*pPC);
    } else
        SPDLOG_ERROR("QUEST no such pc id : {}", pc);
}

///////////////////////////////////////////////////////////////////////////////////////////
// END OF 퀘스트 이벤트 처리
///////////////////////////////////////////////////////////////////////////////////////////

/**
 * Update the game item linked to the running quest state. This is the item
 * that the quest has access to.
 *
 * @param ch The player whose quest state will be updated
 * @param item The item to be binded to the running state (and quest pc)
 */
void CQuestManager::UpdateQuestItem(CHARACTER* ch, CItem* item)
{
    PC* pPC = GetPC(ch->GetPlayerID());

    if (pPC && pPC->IsRunning()) {
        pPC->GetRunningQuestState()->item = item ? item->GetID() : 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
void CQuestManager::LoadStartQuest(const string& quest_name, unsigned int idx)
{
    string full_name = "data/quest/object/begin_condition/" + quest_name;
    ifstream inf(full_name.c_str());

    if (inf.is_open()) {
        SPDLOG_TRACE("QUEST loading begin condition for %s",
                     quest_name.c_str());

        istreambuf_iterator<char> ib(inf), ie;
        copy(ib, ie, back_inserter(m_hmQuestStartScript[idx]));
    }
}

bool CQuestManager::CanStartQuest(unsigned int quest_index, const PC& pc)
{
    return CanStartQuest(quest_index);
}

bool CQuestManager::CanStartQuest(unsigned int quest_index)
{
    THashMapQuestStartScript::iterator it;

    if ((it = m_hmQuestStartScript.find(quest_index)) ==
        m_hmQuestStartScript.end())
        return true;
    else {
        int x = lua_gettop(L);
        lua_dobuffer(L, &(it->second[0]), it->second.size(), "StartScript");
        int bStart = lua_toboolean(L, -1);
        lua_settop(L, x);
        return bStart != 0;
    }
}

bool CQuestManager::CanEndQuestAtState(const string& quest_name,
                                       const string& state_name)
{
    return false;
}

void CQuestManager::DisconnectPC(CHARACTER* ch)
{
    m_mapPC.erase(ch->GetPlayerID());
}

PC* CQuestManager::GetPCForce(unsigned int pc)
{
    PCMap::iterator it;
    if ((it = m_mapPC.find(pc)) == m_mapPC.end()) {
        PC* pPC = &m_mapPC[pc];
        pPC->SetID(pc);
        return pPC;
    }

    return &it->second;
}

PC* CQuestManager::GetPC(unsigned int pc)
{
    auto* pkChr = g_pCharManager->FindByPID(pc);

    if (!pkChr)
        return nullptr;

    m_pCurrentPC = GetPCForce(pc);
    m_pCurrentCharacter = pkChr;
    m_pSelectedDungeon = nullptr;
    return (m_pCurrentPC);
}

// void CQuestManager::SetPC(PC* pPC)
//{
//	CHARACTER* pkChr = CHARACTER_MANAGER::instance().FindByPID(pPC->GetID());

//	if (!pkChr)
//		return;

//	m_pCurrentPC = pPC;
//	m_pCurrentCharacter = pkChr;
//	m_pSelectedDungeon = NULL;
//}

void CQuestManager::ClearScript()
{
    m_strScript.clear();
    m_iCurrentSkin = QUEST_SKIN_NORMAL;
}

void CQuestManager::AddScript(const string& str)
{
    m_strScript += str;
}

void CQuestManager::SendScript()
{
    if (!GetCurrentCharacterPtr()) {
        ClearScript();
        return;
    }

    if (m_bNoSend) {
        m_bNoSend = false;
        ClearScript();
        return;
    }

    if (m_strScript == "[DONE]" || m_strScript == "[NEXT]") {
        if (m_pCurrentPC && !m_pCurrentPC->GetAndResetDoneFlag() &&
            m_strScript == "[DONE]" && m_iCurrentSkin == QUEST_SKIN_NORMAL &&
            !IsError()) {
            ClearScript();
            return;
        }
        m_iCurrentSkin = QUEST_SKIN_NOWINDOW;
    }

    packet_script p;
    p.skin = m_iCurrentSkin;
    p.script = m_strScript;
    p.quest_flag = 0;

    // Flag to distinguish between clicking on the quest and clicking on the NPC
    // in the quest window
    if (m_bQuestInfoFlag == 1)
        p.quest_flag = 1;

    GetCurrentCharacterPtr()->GetDesc()->Send(HEADER_GC_SCRIPT, p);

    ClearScript();
    m_bQuestInfoFlag = 0;
}

const char* CQuestManager::GetQuestStateName(const string& quest_name,
                                             const int state_index)
{
    int x = lua_gettop(L);
    lua_getglobal(L, quest_name.c_str());
    if (lua_isnil(L, -1)) {
        SPDLOG_ERROR("QUEST wrong quest state file %s.%d", quest_name.c_str(),
                     state_index);
        lua_settop(L, x);
        return "";
    }
    lua_pushnumber(L, state_index);
    lua_gettable(L, -2);

    const char* str = lua_tostring(L, -1);
    lua_settop(L, x);
    return str;
}

int CQuestManager::GetQuestStateIndex(const string& quest_name,
                                      const string& state_name)
{
    int x = lua_gettop(L);
    lua_getglobal(L, quest_name.c_str());
    if (lua_isnil(L, -1)) {
        SPDLOG_ERROR("QUEST wrong quest state file {0}.{1}", quest_name.c_str(),
                     state_name.c_str());
        lua_settop(L, x);
        return 0;
    }
    lua_pushstring(L, state_name.c_str());
    lua_gettable(L, -2);

    int v = static_cast<int>(lua_tonumber(L, -1));
    lua_settop(L, x);
    if (gConfig.testServer)
        SPDLOG_TRACE("[QUESTMANAGER] GetQuestStateIndex x({0}) v({1}) {2} {2}",
                     v, x, quest_name.c_str(), state_name.c_str());
    return v;
}

void CQuestManager::SetSkinStyle(int iStyle)
{
    if (iStyle < 0 || iStyle >= QUEST_SKIN_COUNT) {
        m_iCurrentSkin = QUEST_SKIN_NORMAL;
    } else
        m_iCurrentSkin = iStyle;
}

unsigned int CQuestManager::LoadDungeonScript(const string& name)
{
    map<string, unsigned int>::iterator it;
    if ((it = m_mapDungeonID.find(name)) != m_mapDungeonID.end()) {
        return it->second;
    } else {
        unsigned int new_id =
            UINT_MAX - m_mapTimerID.size() - m_mapDungeonID.size();

        m_mapNPC[new_id].Set(new_id, name);
        m_mapDungeonID.insert(make_pair(name, new_id));

        return new_id;
    }
}

unsigned int CQuestManager::LoadTimerScript(const string& name)
{
    map<string, unsigned int>::iterator it;
    if ((it = m_mapTimerID.find(name)) != m_mapTimerID.end()) {
        return it->second;
    } else {
        unsigned int new_id = UINT_MAX - m_mapTimerID.size();

        m_mapNPC[new_id].Set(new_id, name);
        m_mapTimerID.insert(make_pair(name, new_id));

        return new_id;
    }
}

unsigned int CQuestManager::GetCurrentNPCRace()
{
    return GetCurrentNPCCharacterPtr()
               ? GetCurrentNPCCharacterPtr()->GetRaceNum()
               : 0;
}

CItem* CQuestManager::GetCurrentItem()
{
    return ITEM_MANAGER::instance().Find(GetCurrentState()->item);
}

CHARACTER* CQuestManager::GetCurrentNPCCharacterPtr()
{
    return GetCurrentCharacterPtr() ? GetCurrentCharacterPtr()->GetQuestNPC()
                                    : nullptr;
}

const string& CQuestManager::GetCurrentQuestName()
{
    return GetCurrentPC()->GetCurrentQuestName();
}

QuestDungeon* CQuestManager::GetCurrentDungeon()
{
    if (m_pSelectedDungeon)
        return m_pSelectedDungeon;

    CHARACTER* ch = GetCurrentCharacterPtr();
    if (!ch)
        return nullptr;

    const auto map = ch->GetWarpMap();
    if (map > 10000)
        return CDungeonManager::instance().FindByMapIndex<QuestDungeon>(map);

    // TODO: Don't use dynamic_cast here.
    return dynamic_cast<QuestDungeon*>(ch->GetDungeon());
}

void CQuestManager::RegisterQuest(const string& stQuestName, unsigned int idx)
{
    assert(idx > 0);

    if (m_hmQuestName.find(stQuestName) != m_hmQuestName.end())
        return;

    m_hmQuestName.emplace(stQuestName, idx);
    LoadStartQuest(stQuestName, idx);
    m_mapQuestNameByIndex.insert(make_pair(idx, stQuestName));

    SPDLOG_TRACE("QUEST: Register {0} {1}", idx, stQuestName.c_str());
}

unsigned int CQuestManager::GetQuestIndexByName(const string& name)
{
    auto it = m_hmQuestName.find(name);
    if (it == m_hmQuestName.end())
        return 0; // RESERVED

    return it->second;
}

const string& CQuestManager::GetQuestNameByIndex(unsigned int idx)
{
    auto it = m_mapQuestNameByIndex.find(idx);
    if (it == m_mapQuestNameByIndex.end()) {
        //  SPDLOG_ERROR("cannot find quest name by index {0}", idx);
        // assert(!"cannot find quest name by index");

        static std::string st = "";
        return st;
    }

    return it->second;
}

void CQuestManager::SendEventFlagList(CHARACTER* ch, std::string filter)
{
    bool found = false;
    for (const auto& it : m_mapEventFlag) {
        const string& flagname = it.first;
        int value = it.second;

        if (filter != "" && flagname.find(filter) == std::string::npos)
            continue;

        if (!gConfig.testServer && value == 1 && flagname == "valentine_drop")
            ch->ChatPacket(CHAT_TYPE_INFO, "%s %d prob 800", flagname.c_str(),
                           value);
        else if (!gConfig.testServer && value == 1 &&
                 flagname == "newyear_wonso")
            ch->ChatPacket(CHAT_TYPE_INFO, "%s %d prob 500", flagname.c_str(),
                           value);
        else if (!gConfig.testServer && value == 1 &&
                 flagname == "newyear_fire")
            ch->ChatPacket(CHAT_TYPE_INFO, "%s %d prob 1000", flagname.c_str(),
                           value);
        else
            ch->ChatPacket(CHAT_TYPE_INFO, "%s %d", flagname.c_str(), value);

        if (filter != "")
            found = true;
    }

    if (filter != "" && !found) {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "NO_EVENTFLAG_MATCHED_FILTER_%s",
                           filter.c_str());
    }
}

void CQuestManager::RequestSetEventFlag(const string& name, int value)
{
    TPacketSetEventFlag p;
    strlcpy(p.szFlagName, name.c_str(), sizeof(p.szFlagName));
    p.lValue = value;
    db_clientdesc->DBPacket(HEADER_GD_SET_EVENT_FLAG, 0, &p,
                            sizeof(TPacketSetEventFlag));
}

void CQuestManager::SetHwidFlag(const string& hwid, const string& name,
                                int value)
{
    int prev_value = 0;

    using const_ref =
        std::multimap<std::string, THwidDataPair>::const_reference;
    auto it = std::find_if(m_mapHwidFlag.begin(), m_mapHwidFlag.end(),
                           [&](const_ref a) -> bool {
                               return a.first == name && a.second.first == hwid;
                           });

    if (it != m_mapHwidFlag.end())
        prev_value = it->second.second;

    if (it == m_mapHwidFlag.end()) {
        m_mapHwidFlag.insert(std::make_pair(name, std::make_pair(hwid, value)));
    } else if (it->second.second != value) {
        it->second.second = value;
    }

    SPDLOG_TRACE("QUEST hwidflag %s %s %d prev_value %d", hwid.c_str(),
                 name.c_str(), value, prev_value);
}

int CQuestManager::GetHwidFlag(const string& hwid, const string& name)
{
    using const_ref =
        std::multimap<std::string, THwidDataPair>::const_reference;
    auto it = std::find_if(m_mapHwidFlag.begin(), m_mapHwidFlag.end(),
                           [&](const_ref a) -> bool {
                               return a.first == name && a.second.first == hwid;
                           });

    if (it == m_mapHwidFlag.end())
        return 0;

    return it->second.second;
}

void CQuestManager::SetEventFlag(const string& name, int value)
{
    int prev_value = m_mapEventFlag[name];

    SPDLOG_TRACE("QUEST eventflag {} {} prev_value {}", name.c_str(), value,
                 m_mapEventFlag[name]);
    m_mapEventFlag[name] = value;

    if (name == "mob_item") {
        g_pCharManager->SetMobItemRate(value);
    } else if (name == "mob_dam") {
        g_pCharManager->SetMobDamageRate(value);
    } else if (name == "mob_gold") {
        if (value / 200 == 2) {
            SendI18nNotice("The Double-Yang Event is active.");
        }
        g_pCharManager->SetMobGoldAmountRate(value);
    } else if (name == "mob_gold_pct") {
        g_pCharManager->SetMobGoldDropRate(value);
    } else if (name == "user_dam") {
        g_pCharManager->SetUserDamageRate(value);
    } else if (name == "user_dam_buyer") {
        g_pCharManager->SetUserDamageRatePremium(value);
    } else if (name == "mob_exp") {
        g_pCharManager->SetMobExpRate(value);
    } else if (name == "mob_item_buyer") {
        g_pCharManager->SetMobItemRatePremium(value);
    } else if (name == "mob_exp_buyer") {
        g_pCharManager->SetMobExpRatePremium(value);
    } else if (name == "mob_gold_buyer") {
        g_pCharManager->SetMobGoldAmountRatePremium(value);
    } else if (name == "mob_gold_pct_buyer") {
        g_pCharManager->SetMobGoldDropRatePremium(value);
    } else if (name == "damage_debug") {
        gConfig.damageDebug = value;
    } else if (name == "switchbot_enabled") {
        gConfig.enableSwitchbot = value;
    } else if (!name.compare(0, 5, "xmas_")) {
        xmas::ProcessEventFlag(name, prev_value, value);
    } else if (name == "newyear_boom") {
        const DESC_MANAGER::DESC_SET& c_ref_set =
            DESC_MANAGER::instance().GetClientSet();

        for (auto it = c_ref_set.begin(); it != c_ref_set.end(); ++it) {
            CHARACTER* ch = (*it)->GetCharacter();
            if (!ch)
                continue;

            ch->ChatPacket(CHAT_TYPE_COMMAND, "newyear_boom %s", value);
        }
    } else if (name == "eclipse") {
        std::string mode = value == 1 ? "dark" : "light";
        const DESC_MANAGER::DESC_SET& c_ref_set =
            DESC_MANAGER::instance().GetClientSet();

        for (auto it = c_ref_set.begin(); it != c_ref_set.end(); ++it) {
            CHARACTER* ch = (*it)->GetCharacter();
            if (!ch)
                continue;

            ch->ChatPacket(CHAT_TYPE_COMMAND, "DayMode %s", mode.c_str());
        }
    } else if (name == "new_xmas_event") {
        // 20126 new산타.
        static uint32_t new_santa = 20126;
        if (value != 0) {
            bool map1_santa_exist = false;
            bool map21_santa_exist = false;
            bool map41_santa_exist = false;

            const auto chars =
                g_pCharManager->GetCharactersByRaceNum(new_santa);
            for (auto ch : chars) {
                if (ch->GetMapIndex() == 1)
                    map1_santa_exist = true;
                else if (ch->GetMapIndex() == 21)
                    map21_santa_exist = true;
                else if (ch->GetMapIndex() == 41)
                    map41_santa_exist = true;
            }

            if (gConfig.IsHostingMap(1) && !map1_santa_exist) {
                g_pCharManager->SpawnMob(new_santa, 1, 60800, 61700, 0, false,
                                         90, true);
            }
            if (gConfig.IsHostingMap(21) && !map21_santa_exist) {
                g_pCharManager->SpawnMob(new_santa, 21, 59600, 61000, 0, false,
                                         110, true);
            }
            if (gConfig.IsHostingMap(41) && !map41_santa_exist) {
                g_pCharManager->SpawnMob(new_santa, 41, 35700, 74300, 0, false,
                                         140, true);
            }
        } else {
            auto chars = g_pCharManager->GetCharactersByRaceNum(new_santa);
            for (auto* ch : chars)
                M2_DESTROY_CHARACTER(ch);
        }
    }
}

int CQuestManager::GetEventFlag(const string& name)
{
    auto it = m_mapEventFlag.find(name);

    if (it == m_mapEventFlag.end())
        return 0;

    return it->second;
}

	void CQuestManager::RequestSetEventFlagBR(const string& name1, const string& name2, int value)
	{
		const string& name = name1 + name2;
		TPacketSetEventFlag p;
		strlcpy(p.szFlagName, name.c_str(), sizeof(p.szFlagName));
		p.lValue = value;
		db_clientdesc->DBPacket(HEADER_GD_SET_EVENT_FLAG, 0, &p, sizeof(TPacketSetEventFlag));
	}

	int	CQuestManager::GetEventFlagBR(const string& name1, const string& name2)
	{
		const string& name = name1 + name2;

		auto it = m_mapEventFlag.find(name);

		if (it == m_mapEventFlag.end())
			return 0;

		return it->second;
	}

void CQuestManager::BroadcastEventFlagOnLogin(CHARACTER* ch)
{
    // Generic flag behaviour
    std::vector<std::string> flags = {"xmas_snow", "xmas_boom", "xmas_tree",
                                      "newyear_boom"};
    for (const auto& flag : flags) {
        if (const auto iEventFlagValue =
                quest::CQuestManager::instance().GetEventFlag(flag);
            iEventFlagValue) {
            ch->ChatPacket(CHAT_TYPE_COMMAND, "%s %d", flag.c_str(),
                           iEventFlagValue);
        }
    }

    // Eclipse
    if (const auto iEventFlagValue =
            quest::CQuestManager::instance().GetEventFlag("eclipse");
        iEventFlagValue) {
        ch->ChatPacket(CHAT_TYPE_COMMAND, "DayMode %s",
                       iEventFlagValue == 1 ? "dark" : "light");
    }
}

void CQuestManager::Reload()
{
    lua_close(L);
    m_mapNPC.clear();
    m_mapNPCNameID.clear();
    m_hmQuestName.clear();
    m_mapTimerID.clear();
    m_hmQuestStartScript.clear();
    m_mapEventName.clear();
    L = nullptr;
    Initialize();

    for (const auto& vnum : m_registeredNPCVnum) {
        m_mapNPC[vnum].Set(vnum, fmt::format("{}", vnum));
    }
}

bool CQuestManager::ExecuteQuestScript(PC& pc, uint32_t quest_index,
                                       const int state, const char* code,
                                       const int code_size,
                                       vector<AArgScript*>* pChatScripts,
                                       bool bUseCache, uint32_t itemId)
{
    return ExecuteQuestScript(pc, GetQuestNameByIndex(quest_index), state, code,
                              code_size, pChatScripts, bUseCache, itemId);
}

bool CQuestManager::ExecuteQuestScript(PC& pc, const string& quest_name,
                                       const int state, const char* code,
                                       const int code_size,
                                       vector<AArgScript*>* pChatScripts,
                                       bool bUseCache, uint32_t itemId)
{
    const Locale* locale;
    if (m_pCurrentCharacter)
        locale = GetLocale(m_pCurrentCharacter);
    else
        locale = &GetLocaleService().GetDefaultLocale();

    // Generate a running space
    QuestState qs = OpenState(quest_name, state, locale->name, itemId);
    if (pChatScripts)
        qs.chat_scripts.swap(*pChatScripts);

    // Load from cache. If not in cache, save it.
    if (bUseCache) {
        lua_getglobal(qs.co, "__codecache");
        // stack : __codecache
        lua_pushnumber(qs.co, reinterpret_cast<long long>(code));
        // stack : __codecache (codeptr)
        lua_rawget(qs.co, -2);
        // stack : __codecache (compiled-code)
        if (lua_isnil(qs.co, -1)) {
            // cache miss

            // load code to lua,
            // save it to cache
            // and only function remain in stack
            lua_pop(qs.co, 1);
            // stack : __codecache
            luaL_loadbuffer(qs.co, code, code_size, quest_name.c_str());
            // stack : __codecache (compiled-code)
            lua_pushnumber(qs.co, reinterpret_cast<long long>(code));
            // stack : __codecache (compiled-code) (codeptr)
            lua_pushvalue(qs.co, -2);
            // stack : __codecache (compiled-code) (codeptr) (compiled_code)
            lua_rawset(qs.co, -4);
            // stack : __codecache (compiled-code)
            lua_remove(qs.co, -2);
            // stack : (compiled-code)
        } else {
            // cache hit
            lua_remove(qs.co, -2);
            // stack : (compiled-code)
        }
    } else
        luaL_loadbuffer(qs.co, code, code_size, quest_name.c_str());

    // Connect the quest state with the pc.
    pc.SetQuest(quest_name, qs);

    // Run!
    auto& rqs = *pc.GetRunningQuestState();
    if (!RunState(rqs)) {
        CloseState(rqs);
        pc.EndRunning();
        return false;
    }
    return true;
}

void CQuestManager::RegisterNPCVnum(uint32_t dwVnum)
{
    if (m_registeredNPCVnum.find(dwVnum) != m_registeredNPCVnum.end())
        return;

    m_registeredNPCVnum.insert(dwVnum);

    std::string buf = fmt::format("data/quest/object/{}", dwVnum);

    SPDLOG_DEBUG("{}", buf);
    if (fs::exists(buf)) {
        buf = fmt::format("{}", dwVnum);
        SPDLOG_TRACE("{}", buf);
        m_mapNPC[dwVnum].Set(dwVnum, buf);
    }
}

void CQuestManager::WriteRunningStateToSyserr()
{
    const char* state_name =
        GetQuestStateName(GetCurrentQuestName(), GetCurrentState()->st);

    string event_index_name = "";

    for (const auto& elem : m_mapEventName) {
        if (elem.second == m_iRunningEventIndex) {
            event_index_name = elem.first;
            break;
        }
    }

    SPDLOG_ERROR("LUA_ERROR: quest  {0}.{1} {2}", GetCurrentQuestName().c_str(),
                 state_name, event_index_name.c_str());
    if (GetCurrentCharacterPtr() && gConfig.testServer)
        GetCurrentCharacterPtr()->ChatPacket(
            CHAT_TYPE_PARTY, "LUA_ERROR: quest %s.%s %s",
            GetCurrentQuestName().c_str(), state_name,
            event_index_name.c_str());
}

void CQuestManager::AddServerTimer(const std::string& name, uint32_t arg,
                                   LPEVENT event)
{
    if (m_mapServerTimer.find(make_pair(name, arg)) != m_mapServerTimer.end()) {
        SPDLOG_ERROR("QuestManager: Server timer already registered (name: {} "
                     "arg: {})",
                     name, arg);
        return;
    }
    m_mapServerTimer.insert(make_pair(make_pair(name, arg), event));
}

void CQuestManager::ClearServerTimerNotCancel(const std::string& name,
                                              uint32_t arg)
{
    m_mapServerTimer.erase(make_pair(name, arg));
}

void CQuestManager::ClearServerTimer(const std::string& name, uint32_t arg)
{
    auto it = m_mapServerTimer.find(make_pair(name, arg));
    if (it != m_mapServerTimer.end()) {
        LPEVENT event = it->second;
        event_cancel(&event);
        m_mapServerTimer.erase(it);
    }
}

void CQuestManager::CancelServerTimers(uint32_t arg)
{
    auto it = m_mapServerTimer.begin();
    for (; it != m_mapServerTimer.end();) {
        if (it->first.second == arg) {
            LPEVENT event = it->second;
            event_cancel(&event);
            m_mapServerTimer.erase(it++);
        } else {
            ++it;
        }
    }
}

void CQuestManager::SetServerTimerArg(uint32_t dwArg)
{
    m_dwServerTimerArg = dwArg;
}

uint32_t CQuestManager::GetServerTimerArg()
{
    return m_dwServerTimerArg;
}

void CQuestManager::SelectDungeon(QuestDungeon* pDungeon)
{
    m_pSelectedDungeon = pDungeon;
}

bool CQuestManager::PickupItem(unsigned int pc, CItem* item)
{
    if (gConfig.testServer)
        SPDLOG_TRACE("questmanager::PickupItem Start : itemVnum : %d PC : %d",
                     item->GetOriginalVnum(), pc);

    PC* pPC;
    if ((pPC = GetPC(pc))) {
        if (!CheckQuestLoaded(pPC)) {
            CHARACTER* ch = g_pCharManager->FindByPID(pc);
            if (ch) {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO,
                                   "퀘스트를 로드하는 중입니다. 잠시만 기다려 "
                                   "주십시오.");
            }
            return false;
        }
        // call script
        return m_mapNPC[item->GetVnum()].OnPickupItem(*pPC, item ? item->GetID() : 0);
    } else {
        SPDLOG_ERROR("QUEST PICK_ITEM_EVENT no such pc id : %d", pc);
        return false;
    }
}

void CQuestManager::BeginOtherPCBlock(uint32_t pid)
{
    CHARACTER* ch = GetCurrentCharacterPtr();
    if (nullptr == ch) {
        SPDLOG_ERROR("NULL?");
        return;
    }
    /*
    # 1. current pid = pid0 <- It will be m_pOtherPCBlockRootPC.
    begin_other_pc_block(pid1)
        # 2. current pid = pid1
        begin_other_pc_block(pid2)
            # 3. current_pid = pid2
        end_other_pc_block()
    end_other_pc_block()
    */
    // when begin_other_pc_block(pid1)
    if (m_vecPCStack.empty()) {
        m_pOtherPCBlockRootPC = GetCurrentPC();
    }
    m_vecPCStack.push_back(GetCurrentCharacterPtr()->GetPlayerID());
    GetPC(pid);
}

void CQuestManager::EndOtherPCBlock()
{
    if (m_vecPCStack.empty()) {
        SPDLOG_ERROR("m_vecPCStack is alread empty. CurrentQuest{Name(%s), "
                     "State(%s)}",
                     GetCurrentQuestName().c_str(),
                     GetCurrentState()->_title.c_str());
        return;
    }
    uint32_t pc = m_vecPCStack.back();
    m_vecPCStack.pop_back();
    GetPC(pc);

    if (m_vecPCStack.empty()) {
        m_pOtherPCBlockRootPC = nullptr;
    }
}

bool CQuestManager::IsInOtherPCBlock()
{
    return !m_vecPCStack.empty();
}

PC* CQuestManager::GetOtherPCBlockRootPC()
{
    return m_pOtherPCBlockRootPC;
}
} // namespace quest
