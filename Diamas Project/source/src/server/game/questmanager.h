#ifndef METIN2_SERVER_GAME_QUESTMANAGER_H
#define METIN2_SERVER_GAME_QUESTMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <unordered_map>

#include "questnpc.h"
#include "dungeon.h"
#include <base/robin_hood.h>
class CItem;
class CHARACTER;
class QuestDungeon;

namespace quest
{
using namespace std;

bool IsScriptTrue(const char *code, int size);
string ScriptToString(const string &locale, const string &str);

class CQuestManager : public singleton<CQuestManager>
{
public:
    enum
    {
        QUEST_SKIN_NOWINDOW,
        QUEST_SKIN_NORMAL,
        // QUEST_SKIN_CINEMATIC,
        QUEST_SKIN_SCROLL = 4,
        QUEST_SKIN_CINEMATIC = 5,
        QUEST_SKIN_COUNT
    };

    typedef map<string, int> TEventNameMap;
    typedef map<unsigned int, PC> PCMap;

public:
    CQuestManager();
    virtual ~CQuestManager();

    bool Initialize();
    void Destroy();

    bool InitializeLua();

    lua_State *GetLuaState() { return L; }

    void AddLuaFunctionTable(const char *c_pszName, luaL_reg *preg);

    TEventNameMap m_mapEventName;

    QuestState OpenState(const string &quest_name, int state_index, const std::string &locale, uint32_t itemId);
    void CloseState(QuestState &qs) const;
    bool RunState(QuestState &qs);

    PC *GetPC(unsigned int pc);
    PC *GetPCForce(unsigned int pc); // 현재 PC를 바꾸지 않고 PC 포인터를 가져온다.

    unsigned int GetCurrentNPCRace();
    const string &GetCurrentQuestName();
    unsigned int FindNPCIDByName(const string &name);

    // void		SetCurrentNPCCharacterPtr(CHARACTER* ch) { m_pkCurrentNPC = ch; }
    CHARACTER *GetCurrentNPCCharacterPtr();

    void SetCurrentEventIndex(int index) { m_iRunningEventIndex = index; }

    bool UseItem(unsigned int pc, CItem *item, bool bReceiveAll);
    bool PickupItem(unsigned int pc, CItem *item);
    bool SIGUse(unsigned int pc, uint32_t sig_vnum, CItem *item, bool bReceiveAll);
    bool TakeItem(unsigned int pc, unsigned int npc, CItem *item);
    bool UseItemToItem(unsigned pc, unsigned npc, CItem *item);
    CItem *GetCurrentItem();
    void UpdateQuestItem(CHARACTER *ch, CItem *item);
    void AddServerTimer(const string &name, uint32_t arg, LPEVENT event);
    void ClearServerTimer(const string &name, uint32_t arg);
    void ClearServerTimerNotCancel(const string &name, uint32_t arg);
    void CancelServerTimers(uint32_t arg);

    void SetServerTimerArg(uint32_t dwArg);
    uint32_t GetServerTimerArg();

    // event over state and stae
    bool ServerTimer(unsigned int npc, unsigned int arg);
    bool DungeonStageChange(uint32_t npc, QuestDungeon *dungeon);
    bool DungeonEliminatedEvent(uint32_t npc, QuestDungeon *dungeon);

    //////////////////////////////////
    //		NEW QUEST TRIGGERS		//
    //////////////////////////////////
    void Die(unsigned int pc, unsigned int pc2);
    void Buy(unsigned int ch, CItem *item);

    //////////////////////////////////
    //	   NORMAL QUEST TRIGGERS	//
    //////////////////////////////////
    void Login(unsigned int pc, const char * c_pszQuestName = nullptr);
void Dead(unsigned pc);
void Logout(unsigned int pc);
    void VerifyPC(PC& expectedPC);
    bool Timer(unsigned int pc, unsigned int npc);
    bool Click(unsigned int pc, CHARACTER * pkNPC);
    void Kill(unsigned int pc, unsigned int npc);
    void LevelUp(unsigned int pc);
    void AttrIn(unsigned int pc, CHARACTER * ch, int attr);
    void AttrOut(unsigned int pc, CHARACTER * ch, int attr);
    bool Target(unsigned int pc, uint32_t dwQuestIndex, const char * c_pszTargetName, const char * c_pszVerb);
    bool GiveItemToPC(unsigned int pc, CHARACTER * pkChr);
    void Unmount(unsigned int pc);

    void QuestButton(unsigned int pc, unsigned int quest_index);
    void QuestInfo(unsigned int pc, unsigned int quest_index);

    void EnterState(uint32_t pc, uint32_t quest_index, int state, CItem *item = nullptr);
    void LeaveState(uint32_t pc, uint32_t quest_index, int state);

    void Letter(uint32_t pc);
    void Letter(uint32_t pc, uint32_t quest_index, int state);

    //

    bool CheckQuestLoaded(PC *pc) { return pc && pc->IsLoaded(); }

    // event occurs in one state
    void Select(unsigned pc, unsigned selection, int qIndex);
    void Resume(unsigned pc, int qIndex);

    // int		ReadQuestCategoryFile(uint16_t q_index);
    void Input(unsigned pc, const char* msg, int qIndex);
    void Confirm(unsigned int pc, EQuestConfirmType confirm, unsigned int pc2 = 0);
    void SelectItem(unsigned int pc, unsigned int selection);
    void Receive(unsigned int ch, const char * cmd, uint32_t quest_index);

    void LogoutPC(CHARACTER *ch);
    void DisconnectPC(CHARACTER *ch);

    QuestState *GetCurrentState() { return m_CurrentRunningState; }

    void LoadStartQuest(const string &quest_name, unsigned int idx);
    // bool		CanStartQuest(const string& quest_name, const PC& pc);
    bool CanStartQuest(unsigned int quest_index, const PC &pc);
    bool CanStartQuest(unsigned int quest_index);
    bool CanEndQuestAtState(const string &quest_name, const string &state_name);

    CHARACTER *GetCurrentCharacterPtr() { return m_pCurrentCharacter; }

    CHARACTER *GetCurrentPartyMember() { return m_pCurrentPartyMember; }

    PC *GetCurrentPC() { return m_pCurrentPC; }

    QuestDungeon *GetCurrentDungeon();
    void SelectDungeon(QuestDungeon *pDungeon);

    void ClearScript();
    void SendScript();
    void AddScript(const string &str);

    void BuildStateIndexToName(const char *questName);

    int GetQuestStateIndex(const string &quest_name, const string &state_name);
    const char *GetQuestStateName(const string &quest_name, const int state_index);

    void SetSkinStyle(int iStyle);
    unsigned LoadDungeonScript(const string &name);

    void SetNoSend() { m_bNoSend = true; }

    unsigned int LoadTimerScript(const string &name);

    // unsigned int	RegisterQuestName(const string& name);

    void RegisterQuest(const string &name, unsigned int idx);
    unsigned int GetQuestIndexByName(const string &name);
    const string &GetQuestNameByIndex(unsigned int idx);

    void RequestSetEventFlag(const string &name, int value);
    void SetHwidFlag(const string &hwid, const string &name, int value);

    void SetEventFlag(const string &name, int value);
    int GetHwidFlag(const string &hwid, const string &name);
    int GetEventFlag(const string &name);
    void BroadcastEventFlagOnLogin(CHARACTER *ch);
	int GetEventFlagBR(const string& name1, const string& name2);
	void RequestSetEventFlagBR(const string& name1, const string& name2, int value);
    void SendEventFlagList(CHARACTER *ch, std::string filter);

    void Reload();

    // void		CreateAllButton(const string& quest_name, const string& button_name);
    void SetError() { m_bError = true; }

    void ClearError() { m_bError = false; }

    bool IsError() { return m_bError; }

    void WriteRunningStateToSyserr();

    void RegisterNPCVnum(uint32_t dwVnum);

private:
    QuestDungeon *m_pSelectedDungeon;
    uint32_t m_dwServerTimerArg;

    map<pair<string, uint32_t>, LPEVENT> m_mapServerTimer;

    int m_iRunningEventIndex;

    robin_hood::unordered_map<string, int> m_mapEventFlag;

    using THwidDataPair = std::pair<std::string, long>;
    using THWidFlagMap = std::multimap<std::string, THwidDataPair>;
    THWidFlagMap m_mapHwidFlag;

    void GotoSelectState(QuestState &qs);
    void GotoPauseState(QuestState &qs);
    void GotoEndState(QuestState &qs);
    void GotoInputState(QuestState &qs);
    void GotoConfirmState(QuestState &qs);
    void GotoSelectItemState(QuestState &qs);

    lua_State *L;

    bool m_bNoSend;

    unordered_set<unsigned int> m_registeredNPCVnum;
    map<unsigned int, NPC> m_mapNPC;
    map<string, unsigned int> m_mapNPCNameID;
    map<string, unsigned int> m_mapTimerID;
    map<string, unsigned int> m_mapDungeonID;

    QuestState *m_CurrentRunningState;

    PCMap m_mapPC;

    CHARACTER *m_pCurrentCharacter;
    CHARACTER *m_pCurrentNPCCharacter;
    CHARACTER *m_pCurrentPartyMember;
    PC *m_pCurrentPC;

    string m_strScript;
    bool m_bQuestInfoFlag;

    int m_iCurrentSkin;

    struct stringhash
    {
        size_t operator()(const string &str) const
        {
            const unsigned char *s = (const unsigned char *)str.c_str();
            const unsigned char *end = s + str.size();
            size_t h = 0;

            while (s < end)
            {
                h *= 16777619;
                h ^= (unsigned char)*(unsigned char *)(s++);
            }

            return h;
        }
    };

    typedef robin_hood::unordered_map<string, int, stringhash> THashMapQuestName;
    typedef std::unordered_map<unsigned int, vector<char>> THashMapQuestStartScript;

    THashMapQuestName m_hmQuestName;
    THashMapQuestStartScript m_hmQuestStartScript;
    map<unsigned int, string> m_mapQuestNameByIndex;

    bool m_bError;

public:
    bool ExecuteQuestScript(PC &pc, const string &quest_name, const int state, const char *code, const int code_size,
                            vector<AArgScript *> *pChatScripts = nullptr, bool bUseCache = true, uint32_t itemId = 0);
    bool ExecuteQuestScript(PC &pc, uint32_t quest_index, const int state, const char *code, const int code_size,
                            vector<AArgScript *> *pChatScripts = nullptr, bool bUseCache = true, uint32_t itemId = 0);

    // begin_other_pc_blcok, end_other_pc_block을 위한 객체들.
public:
    void BeginOtherPCBlock(uint32_t pid);
    void EndOtherPCBlock();
    bool IsInOtherPCBlock();
    PC *GetOtherPCBlockRootPC();

private:
    PC *m_pOtherPCBlockRootPC;
    std::vector<uint32_t> m_vecPCStack;

public:
    std::map<uint16_t, unsigned int> QuestCategoryIndexMap;
    void ReadQuestCategoryToDict();
    int GetQuestCategoryByQuestIndex(uint16_t q_index);
};
}; // namespace quest

#endif /* METIN2_SERVER_GAME_QUESTMANAGER_H */
