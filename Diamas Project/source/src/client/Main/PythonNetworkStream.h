#ifndef METIN2_CLIENT_MAIN_PYTHONNETWORKSTREAM_H
#define METIN2_CLIENT_MAIN_PYTHONNETWORKSTREAM_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif
#include <boost/asio/io_service.hpp>


#include "../EterLib/FuncObject.h"
#include "NetworkActorManager.h"
#include "InsultChecker.h"

#include <game/GamePacket.hpp>
#include <net/Socket.hpp>
#include <net/AsyncClientSocketAdapter.hpp>

class CInstanceBase;
class CNetworkActorManager;
struct SNetworkActorData;
struct SNetworkUpdateActorData;
extern int isInputEnabled;

template<typename T>
class CPacketHandlerFuncObject
{
	public:
		CPacketHandlerFuncObject()
		{
			Clear();
		}

		virtual ~CPacketHandlerFuncObject()
		{
		}

		void Clear()
		{
			m_pSelfObject = NULL;
			m_pFuncObject = NULL;
		}

		void Set(T* pSelfObject, bool (T::*pFuncObject)(const PacketHeader &header, const boost::asio::const_buffer &data))
		{
			m_pSelfObject = pSelfObject;
			m_pFuncObject = pFuncObject;
		}

		bool IsEmpty()
		{
			if (m_pSelfObject != NULL)
				return false;

			if (m_pFuncObject != NULL)
				return false;

			return true;
		}

		template <typename ...Params>
		bool Run(Params&&... params)
		{
			if (m_pSelfObject)
				if (m_pFuncObject)
					return std::invoke(m_pFuncObject, m_pSelfObject, std::forward<Params>(params)...);

			return false;
		}
	protected:
		T *	m_pSelfObject;
		bool (T::*m_pFuncObject) (const PacketHeader &, const boost::asio::const_buffer &);
};



class CPythonNetworkStream : public AsyncClientSocketAdapter<Socket>
{
public:
    enum
    {
        SERVER_COMMAND_LOG_OUT = 0,
        SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER = 1,
        SERVER_COMMAND_QUIT = 2,

        MAX_ACCOUNT_PLAYER
    };

    enum
    {
        ERROR_NONE,
        ERROR_UNKNOWN,
        ERROR_CONNECT_MARK_SERVER,
        ERROR_LOAD_MARK,
        ERROR_MARK_WIDTH,
        ERROR_MARK_HEIGHT,

        // MARK_BUG_FIX
        ERROR_MARK_UPLOAD_NEED_RECONNECT,
        ERROR_MARK_CHECK_NEED_RECONNECT,
        // END_OF_MARK_BUG_FIX
    };

    enum
    {
        ACCOUNT_CHARACTER_SLOT_ID,
        ACCOUNT_CHARACTER_SLOT_NAME,
        ACCOUNT_CHARACTER_SLOT_RACE,
        ACCOUNT_CHARACTER_SLOT_LEVEL,
        ACCOUNT_CHARACTER_SLOT_STR,
        ACCOUNT_CHARACTER_SLOT_DEX,
        ACCOUNT_CHARACTER_SLOT_HTH,
        ACCOUNT_CHARACTER_SLOT_INT,
        ACCOUNT_CHARACTER_SLOT_PLAYTIME,
        ACCOUNT_CHARACTER_SLOT_FORM,
        ACCOUNT_CHARACTER_SLOT_ADDR,
        ACCOUNT_CHARACTER_SLOT_PORT,
        ACCOUNT_CHARACTER_SLOT_GUILD_ID,
        ACCOUNT_CHARACTER_SLOT_GUILD_NAME,
        ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG,
        ACCOUNT_CHARACTER_SLOT_HAIR,
        ACCOUNT_CHARACTER_SLOT_ACCE,
        ACCOUNT_CHARACTER_SLOT_LAST_PLAYTIME,
    };

    enum
    {
        PHASE_WINDOW_LOGO,
        PHASE_WINDOW_LOGIN,
        PHASE_WINDOW_SELECT,
        PHASE_WINDOW_CREATE,
        PHASE_WINDOW_LOAD,
        PHASE_WINDOW_GAME,
        PHASE_WINDOW_EMPIRE,
        PHASE_WINDOW_NUM,
    };

public:
    CPythonNetworkStream(boost::asio::io_service &ioService);
    virtual ~CPythonNetworkStream();

    void StartGame();
    void Warp(uint32_t mapIndex, uint32_t x, uint32_t y);

    void NotifyHack(const char *c_szMsg);
    void SetWaitFlag();

    void SendEmoticon(UINT eEmoticon);

    void ExitApplication();
    void ExitGame();
    void LogOutGame();
    void AbsoluteExitGame();
    void AbsoluteExitApplication();

    bool IsInsultIn(const char *c_szMsg);

    uint32_t GetGuildID();

    UINT UploadMark(const char *c_szImageFileName);
    UINT UploadSymbol(const char *c_szImageFileName);

    bool LoadInsultList(const char *c_szInsultListFileName);

    bool IsSelectedEmpire();

    void ToggleGameDebugInfo();

    void SetMarkServer(const char *c_szAddr, UINT uPort);
    bool ConnectLoginServer(const char *c_szAddr, UINT uPort);
    void ConnectGameServer(UINT iChrSlot);

    std::string GetLoginId() const;
    void SetLogin(const std::string &login);
    void SetSessionId(uint64_t sessionId);

    void SetHandler(py::object poHandler);
    void SetPhaseWindow(UINT ePhaseWnd, py::object poPhaseWnd);
    py::object GetPhaseWindow(UINT ePhaseWnd);

    void ClearPhaseWindow(UINT ePhaseWnd, py::object poPhaseWnd);
    void SetServerCommandParserWindow(py::object poWnd);

    bool SendSyncPositionElementPacket(uint32_t dwVictimVID, uint32_t dwVictimX, uint32_t dwVictimY);

    bool SendAttackPacket(UINT uMotAttack, uint32_t dwVIDVictim, uint32_t motionKey, uint32_t x, uint32_t y, const Vector3& pushDest, const Vector3& victimPos);
    bool SendCharacterStatePacket(const TPixelPosition &c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg,
                                  uint32_t color = 0, uint32_t motionKey = 0);
    bool SendCharacterSkillStatePacket(const TPixelPosition &c_rkPPosDst, float fDstRot, UINT uArg, uint8_t loopCount,
                                       uint8_t isMovingSkill, uint32_t color, uint32_t motionKey);
    void SendChangeSkillColorPacket(uint32_t vnum, uint32_t color);
    void SendBlockModePacket(uint32_t blockMode);
    void SendSkillGroupSelectPacket(uint8_t skillGroup);

    bool SendUseSkillPacket(uint32_t dwSkillIndex, uint32_t dwTargetVID = 0);
    bool SendTargetPacket(uint32_t dwVID);

    bool SendItemUsePacket(TItemPos pos);
    bool SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos);
    bool SendRemoveMetinPacket(TItemPos pos, uint8_t slot);
    bool SendItemDropPacket(TItemPos pos, uint32_t elk);
    bool SendItemDropPacketNew(TItemPos pos, uint32_t elk, CountType count);
    bool SendItemDestroyPacket(TItemPos pos);
    bool SendItemMovePacket(TItemPos pos, TItemPos change_pos, CountType num);
    bool SendItemSplitPacket(TItemPos pos, CountType num);

    bool SendItemPickUpPacket(uint32_t vid);
    bool SendMyShopOpenPacket(const std::string &sign, const TItemPos &bundleItem,
                              const std::vector<TShopItemTable> &items);
    bool SendQuickSlotAddPacket(uint8_t wpos, uint8_t type, uint8_t pos);
    bool SendQuickSlotDelPacket(uint8_t wpos);
    bool SendQuickSlotMovePacket(uint8_t wpos, uint8_t change_pos);
#ifdef INGAME_WIKI
    bool SendWikiRequestInfo(unsigned long long retID, DWORD vnum, bool isMob);
#endif
    // PointReset 개 임시
    bool SendPointResetPacket();

    // Shop
    bool SendShopEndPacket();
    bool SendShopBuyPacket(uint16_t byCount, uint8_t amount = 1);
    bool SendShopSellPacket(TItemPos pos);
    bool SendShopSellPacketNew(TItemPos pos, CountType byCount);
    bool SendItemUseMultiplePacket(TItemPos pos);

    // Exchange
    bool SendExchangeStartPacket(uint32_t vid);
    bool SendExchangeItemAddPacket(TItemPos ItemPos, uint8_t byDisplayPos);
    bool SendExchangeElkAddPacket(Gold elk);
    bool SendExchangeItemDelPacket(uint8_t pos);
    bool SendExchangeAcceptPacket();
    bool SendExchangeExitPacket();

    // Quest
    bool SendScriptAnswerPacket(int iAnswer, int qIndex);
    bool SendScriptButtonPacket(unsigned int iIndex);
    bool SendAnswerMakeGuildPacket(const char * c_szName);
    bool SendQuestInputStringPacket(const char* c_szString, int32_t qIndex);
    bool SendQuestInputStringLongPacket(const char* c_szString, int32_t qIndex);
    bool SendQuestCommandPacket(const char * c_szCmd, uint32_t quest_index);
    bool SendQuestConfirmPacket(uint8_t byAnswer, uint32_t dwPID);

    // Event
    bool SendOnClickPacket(uint32_t vid);

    // Fly
    bool SendFlyTargetingPacket(uint32_t dwTargetVID, const TPixelPosition &kPPosTarget);
    bool SendAddFlyTargetingPacket(uint32_t dwTargetVID, const TPixelPosition &kPPosTarget);
    bool SendShootPacket(uint32_t motionKey, uint32_t skill);
    // Command
    bool ClientCommand(const char *c_szCommand);
    void ServerCommand(const char *c_szCommand);

    // Emoticon
    void RegisterEmoticonString(const char *pcEmoticonString);

    // Party
    bool SendPartyInvitePacket(uint32_t dwVID);
    bool SendPartyInviteAnswerPacket(uint32_t dwLeaderVID, uint8_t byAccept);
    bool SendPartyRemovePacket(uint32_t dwPID);
    bool SendPartySetStatePacket(uint32_t dwVID, uint8_t byState, uint8_t byFlag);
    bool SendPartyUseSkillPacket(uint8_t bySkillIndex, uint32_t dwVID);
    bool SendPartyParameterPacket(uint8_t byDistributeMode);

    // SafeBox
    bool SendSafeBoxMoneyPacket(uint8_t byState, uint32_t dwMoney);
    bool SendSafeBoxCheckinPacket(TItemPos InventoryPos, uint16_t bySafeBoxPos);
    bool SendSafeBoxCheckoutPacket(uint16_t bySafeBoxPos, TItemPos InventoryPos);
    bool SendSafeBoxItemMovePacket(uint16_t bySourcePos, uint16_t byTargetPos, CountType byCount);

    // Mall
    bool SendMallCheckoutPacket(uint8_t byMallPos, TItemPos InventoryPos);

    // Guild
    bool SendGuildAddMemberPacket(uint32_t dwVID);
    bool SendGuildRemoveMemberPacket(const std::string &name);
    bool SendGuildRemoveMemberPacket(uint32_t dwPID);
    bool SendGuildChangeGradeNamePacket(uint8_t byGradeNumber, const char *c_szName);
    bool SendGuildChangeGradeAuthorityPacket(uint8_t byGradeNumber, uint64_t byAuthority);
    bool SendGuildOfferPacket(PointValue exp);
    bool SendGuildPostCommentPacket(const char *c_szMessage);
    bool SendGuildDeleteCommentPacket(uint32_t dwIndex);
    bool SendGuildRefreshCommentsPacket(uint32_t dwHighestIndex);
    bool SendGuildChangeMemberGradePacket(uint32_t dwPID, uint8_t byGrade);
    bool SendGuildUseSkillPacket(uint32_t dwSkillID, uint32_t dwTargetVID);
    bool SendGuildChangeMemberGeneralPacket(uint32_t dwPID, uint8_t byFlag);
    bool SendGuildInviteAnswerPacket(uint32_t dwGuildID, uint8_t byAnswer);
    bool SendGuildChargeGSPPacket(uint32_t dwMoney);
    bool SendGuildDepositMoneyPacket(Gold dwMoney);
    bool SendGuildWithdrawMoneyPacket(Gold dwMoney);

    // ACCE
    bool SendAcceRefineCheckinPacket(TItemPos InventoryPos, unsigned char byAccePos, unsigned char byWindowType);
    bool SendAcceRefineCheckoutPacket(uint8_t byAccePos);
    bool SendAcceRefineAcceptPacket(uint8_t windowType);
    bool SendAcceRefineCancelPacket();
    bool SendChangeLookCheckinPacket(TItemPos InventoryPos, uint8_t byPos);
    bool SendChangeLookCancelPacket();
    bool SendChangeLookCheckoutPacket(uint8_t byPos);
    bool SendChangeLookAcceptPacket();
    // Shop Search
    bool SendPrivateShopSearchOpenFilter(int32_t filter);
    bool SendPrivateShopSearchOpenCategory(py::tuple category);
    bool SendPrivateShopSearchInfoSearch(py::tuple vnum);
    bool SendPrivateShopSearchOpenPage(uint16_t vnum);

    bool SendPrivateShopSearchBuyItem(uint32_t itemId);

    // Mall
    bool RecvMallOpenPacket(const TPacketCGSafeboxSize &p);
    bool RecvMallItemSetPacket(const GcItemSetPacket &p);
    bool RecvMallItemDelPacket(const TPacketGCItemDel &p);

    // Lover
    bool RecvLoverInfoPacket(const TPacketGCLoverInfo &p);
    bool RecvLovePointUpdatePacket(const TPacketGCLovePointUpdate &p);
    bool RecvDigMotionPacket(const TPacketGCDigMotion &p);

    // Dig

    // Fishing
    bool SendFishingPacket(int iRotation);
	bool SendFishingGamePacket(int hitCount, float time);

    bool SendGiveItemPacket(uint32_t dwTargetVID, TItemPos ItemPos, int iItemCount);

    // Refine
    bool SendRefinePacket(uint16_t byPos, uint8_t byType);
    bool SendSelectItemPacket(uint32_t dwItemPos);

    // CRC Report
    bool __SendCRCReportPacket();

    // 용홍석 강화
    bool SendDragonSoulRefinePacket(uint8_t bRefineType, TItemPos *pos);

    // Handshake
    bool RecvHandshakePacket(const TPacketGCHandshake &p);
    bool RecvHandshakeOKPacket(const BlankPacket &p);
    bool SendTimeSyncReqPacket();

    bool RecvMaintenance(const TPacketGCMaintenance &p);

#ifdef ENABLE_ADMIN_MANAGER
	public:
		bool SendAdminManagerHead(uint8_t bSubHeader);
		bool SendAdminManager(uint8_t bSubHeader, const void* c_pvData, int iSize = 0);
	protected:
		bool RecvAdminManagerLoad();
		bool RecvAdminManager();
#endif
public:
    // ETC
    uint32_t GetMainActorVID();
    uint32_t GetMainActorRace();
    uint32_t GetMainActorEmpire();
    uint32_t GetMainActorSkillGroup();
    void __TEST_SetSkillGroupFake(int iIndex);

    //////////////////////////////////////////////////////////////////////////
    // Phase 관련
    //////////////////////////////////////////////////////////////////////////
public:
    void SetOffLinePhase();
    void SetHandShakePhase();
    void SetLoginPhase();
    void SetSelectPhase();
    void SetLoadingPhase();
    void SetGamePhase();
    void ClosePhase();

    bool IsGamePhase() const
    {
        return m_strPhase == "Game";
    }

    void SameLoginPhase();

    // Login Phase
    bool SendLoginPacket(const char *login, uint64_t sessionId);

    bool SendEnterGame();

    // Select Phase
    const SimplePlayer &GetPlayerRef(uint8_t slot) const
    {
        return m_players[slot];
    }

    const SimplePlayer *GetPlayer(uint8_t slot) const
    {
        return &m_players[slot];
    }

    py::bytes GetGuildName(uint8_t slot) const
    {
        return m_astrGuildName[slot];
    }

    py::bytes GetPlayerName(uint8_t slot) const
    {
        return m_playerNames[slot];
    }

    bool SendSelectCharacterPacket(uint8_t account_Index);
    bool SendChangeNamePacket(uint8_t index, const char *name);
    bool SendChangeEmpirePacket(uint8_t slot, uint8_t empire);
    bool SendCreateCharacterPacket(uint8_t index, const char *name, uint8_t shape,
                                   uint8_t empire
                                   /* uint8_t byStat1, uint8_t byStat2, uint8_t byStat3, uint8_t byStat4*/, uint8_t
                                   job, uint8_t ht, uint8_t iq, uint8_t st, uint8_t dx);
    bool SendDestroyCharacterPacket(uint8_t index, const char *szPrivateCode);

    // Main Game Phase
    void SendSimpleChatPacket(const char *c_szChat)
    {
        SendChatPacket(c_szChat, CHAT_TYPE_TALKING);
    }

    bool SendChatPacket(const char *c_szChat, uint8_t byType = CHAT_TYPE_TALKING);
    bool SendWhisperPacket(const char *name, const char *c_szChat);
    bool SendCheckCountryFlagForWhisperPacket(const char *name);

    bool SendMessengerAddByVIDPacket(uint32_t vid);
    bool SendMessengerAddByNamePacket(const char *c_szName);
    bool SendMessengerRemovePacket(const char *c_szKey, const char *c_szName);
    bool SendMessengerSetBlockPacket(const char *name, uint16_t mode);

#ifdef ENABLE_MOVE_COSTUME_ATTR
    bool SendItemCombinationPacket(const TItemPos &itemPos, const TItemPos &basePos, const TItemPos &materialPos);

#endif

    bool SendMyShopDecoState(unsigned char state);
    bool SendMyShopDecoSet(uint32_t titleType, uint32_t shopVnum);
    bool RecvWikiPacket(const GcWikiPacket &p);
    bool RecvWikiMobPacket(const GcWikiMobPacket &p);

    bool SendUpdateSwitchbotAttributePacket(uint8_t slotIndex, uint8_t altIndex, uint8_t attrIndex,
                                            TItemApply attr);
    bool SendSwitchbotSlotActivatePacket(uint8_t slotIndex);
    bool SendSwitchbotSlotDeactivatePacket(uint8_t slotIndex);
    bool OpenMyShop();
    bool OpenMyShopSearch();
    bool SendLoadTargetInfo(uint32_t dwVID);
    bool SendOfflineShopWarpPacket();
    bool SendChatFilter(const std::vector<uint8_t> &bFilterList);
    bool SendSetTitlePacket(std::string title, uint32_t color);
    bool SendAttrPlusLevelPet(uint8_t index);
    bool SendOpenLevelPet(TItemPos pos);
    bool SendCloseLevelPet();
        bool Update(); // State들을 실제로 실행한다.


protected:
    bool OffLinePhase(const PacketHeader &header, const boost::asio::const_buffer &data);
    bool HandShakePhase(const PacketHeader &header, const boost::asio::const_buffer &data);
    bool LoginPhase(const PacketHeader &header, const boost::asio::const_buffer &data);
    bool SelectPhase(const PacketHeader &header, const boost::asio::const_buffer &data);
    bool LoadingPhase(const PacketHeader &header, const boost::asio::const_buffer &data);
    bool GamePhase(const PacketHeader &header, const boost::asio::const_buffer &data);

    bool __IsNotPing();

    void __DownloadMark();
    void __DownloadSymbol(const std::vector<uint32_t> &c_rkVec_dwGuildID);

    void __PlayInventoryItemUseSound(TItemPos uSlotPos);
    void __PlayInventoryItemDropSound(TItemPos uSlotPos);
    void __PlayAcceItemDropSound(UINT uSlotPos);
    void __PlayChangeLookItemDropSound(UINT uSlotPos);
    //void __PlayShopItemDropSound(UINT uSlotPos);
#ifdef ENABLE_GUILD_STORAGE
	void __PlayGuildStorageItemDropSound(UINT uSlotPos);
#endif
    void __PlaySafeBoxItemDropSound(UINT uSlotPos);
    void __PlayMallItemDropSound(UINT uSlotPos);

    bool __CanActMainInstance();

    enum REFRESH_WINDOW_TYPE
    {
        RefreshStatus = (1 << 0),
        RefreshAlignmentWindow = (1 << 1),
        RefreshCharacterWindow = (1 << 2),
        RefreshEquipmentWindow = (1 << 3),
        RefreshInventoryWindow = (1 << 4),
        RefreshExchangeWindow = (1 << 5),
        RefreshSkillWindow = (1 << 6),
        RefreshSafeboxWindow = (1 << 7),
        RefreshMessengerWindow = (1 << 8),
        RefreshGuildWindowInfoPage = (1 << 9),
        RefreshGuildWindowBoardPage = (1 << 10),
        RefreshGuildWindowMemberPage = (1 << 11),
        RefreshGuildWindowMemberPageGradeComboBox = (1 << 12),
        RefreshGuildWindowSkillPage = (1 << 13),
        RefreshGuildWindowGradePage = (1 << 14),
        RefreshTargetBoard = (1 << 15),
        RefreshMallWindow = (1 << 16),
    };

    void __RefreshStatus();
    void __RefreshAlignmentWindow();
    void __RefreshCharacterWindow();
    void __RefreshEquipmentWindow();
    void __RefreshHuntingMissions();
    void __RefreshInventoryWindow();
    void __RefreshShopInfoWindow();
    void __RefreshShopSearchWindow();
    void __RefreshExchangeWindow();
    void __RefreshSkillWindow();
    void __RefreshSafeboxWindow();
    void __RefreshMessengerWindow();
    void __RefreshGuildWindowInfoPage();
    void __RefreshGuildWindowBoardPage();
    void __RefreshGuildWindowMemberPage();
    void __RefreshGuildWindowMemberPageGradeComboBox();
    void __RefreshGuildWindowSkillPage();
    void __RefreshGuildWindowGradePage();
    void __RefreshTargetBoardByVID(uint32_t dwVID);
    void __RefreshTargetBoardByName(const char * c_szName);
    void __RefreshTargetBoard();
    void __RefreshMallWindow();
    void __RefreshAcceWindow();
#ifdef ENABLE_GUILD_STORAGE
	void __RefreshGuildStorageWindow();
	void __RefreshGuildStorageLogWindow();
#endif
    bool __SendHack(const char *c_szMsg);

protected:

    // Common
    bool RecvPingPacket(const TPacketGCPing& p);
    bool RecvPhasePacket(const TPacketGCPhase& p);

    // Login Phase
    bool __RecvLoginSuccessPacket(const TPacketGCLoginSuccess &p);
    bool __RecvLoginFailurePacket(const TPacketGCLoginFailure &p);

    // Select Phase
    bool __RecvPlayerCreateSuccessPacket(const TPacketGCPlayerCreateSuccess &p);
    bool __RecvPlayerCreateFailurePacket(const GcCharacterCreateFailurePacket &p);
    bool __RecvPlayerDestroySuccessPacket(const GcCharacterDeleteSuccessPacket &p);
    bool __RecvPlayerDestroyFailurePacket(const BlankPacket &p);
    bool __RecvPlayerPoints(const TPacketGCPoints &p);
    bool __RecvPlayerGold(const TPacketGCGold &p);

#ifdef ENABLE_GEM_SYSTEM
	bool __RecvPlayerGem();
#endif

    bool __RecvChangeName(const GcChangeNamePacket &p);
    bool __RecvChangeEmpire(const GcChangeEmpirePacket &p);

    // Loading Phase
    bool RecvMainCharacter(const GcMainCharacterPacket &p);
    bool RecvMainCharacter4_BGM_VOL();

    // Main Game Phase
    bool RecvWarpPacket(const TPacketGCWarp& p);
    bool RecvPVPPacket(const TPacketGCPVP &p);
    bool RecvDuelStartPacket(const TPacketGCDuelStart&p);
    bool RecvCharacterAppendPacket(const TPacketGCCharacterAdd &chrAddPacket);
    bool RecvCharacterAdditionalInfo(const TPacketGCCharacterAdditionalInfo &chrInfoPacket);
    bool RecvCharacterUpdatePacket(const TPacketGCCharacterUpdate &chrUpdatePacket);
    bool RecvCharacterDeletePacket(const TPacketGCCharacterDelete &chrDelPacket);
    bool RecvCharacterMovePacket(const TPacketGCMove &p);
    bool RecvOwnerShipPacket(const TPacketGCOwnership &p);
    bool RecvChatPacket(const TPacketGCChat&p);
    bool RecvSyncPositionPacket(const TPacketGCSyncPosition &p);
    bool RecvNpcUseSkill(const TPacketGCNPCUseSkill &packet);
    bool RecvSkillMotion(const GcSkillMotionPacket &p);
    bool RecvWhisperPacket(const TPacketGCWhisper &p);
    bool RecvExchangeInfoPacket(const TPacketGCExchageInfo &p);
    bool RecvPointChange(const TPacketGCPointChange& PointChange); // Alarm to python
    bool RecvGoldChange(const TPacketGCGoldChange& GoldChange);
    bool RecvChangeSpeedPacket();
#ifdef ENABLE_GEM_SYSTEM
	bool RecvGemChange();
#endif

    bool RecvStunPacket(const TPacketGCStun &p);
    bool RecvDeadPacket(const TPacketGCDead &p);

    bool RecvItemSetPacket(const GcItemSetPacket &p); // Alarm to python
    bool RecvItemDelPacket(const TPacketGCItemDel &p);
    bool RecvItemPickupInfoPacket(const GcItemPickupInfoPacket &p);

    bool RecvItemUsePacket(const TPacketGCItemUse &p); // Alarm to python

    bool RecvItemGroundAddPacket(const GcPacketItemGroundAdd &p);
    bool RecvItemGroundDelPacket(const TPacketGCItemGroundDel &p);
    bool RecvItemOwnership(const TPacketGCItemOwnership &p);

    bool RecvQuickSlotAddPacket(const packet_quickslot_add &p);   // Alarm to python
    bool RecvQuickSlotDelPacket(const packet_quickslot_del &p);   // Alarm to python
    bool RecvQuickSlotMovePacket(const packet_quickslot_swap &p); // Alarm to python

    bool RecvMotionPacket(const packet_motion &p);

    bool RecvShopPacket(const TPacketGCShop& p);
    bool RecvShopSignPacket(const TPacketGCShopSign &p);

    bool RecvExchangePacket(const packet_exchange &p);

    // Quest
    bool RecvScriptPacket(const packet_script &ScriptPacket);
    bool RecvQuestInfoPacket(const packet_quest_info &p);
    bool RecvQuestConfirmPacket(const TPacketGCQuestConfirm &p);
    bool RecvRequestMakeGuild(const BlankPacket &p);

    // Skill
    bool RecvSkillLevelNew(const TPacketGCSkillLevel &p);

    // Target
    bool RecvTargetPacket(const TPacketGCTarget &p);
    bool RecvTargetInfoPacket(const TPacketGCTargetInfo &p);
    bool RecvMountPacket(const GcMountPacket &p);
    bool RecvViewEquipPacket(const TPacketViewEquip &p);
    bool RecvDamageInfoPacket(const TPacketGCDamageInfo &p);

    // Fly
    bool RecvCreateFlyPacket(const TPacketGCCreateFly &p);
    bool RecvFlyTargetingPacket(const TPacketGCFlyTargeting &p);
    bool RecvAddFlyTargetingPacket(const TPacketGCFlyTargeting &p);

    // Messenger
    bool RecvMessenger(const TPacketGCMessenger &p);

    // Guild
    bool RecvGuild(const TPacketGCGuild& p);

    // Party
    bool RecvPartyInvite(const TPacketGCPartyInvite &p);
    bool RecvPartyAdd(const TPacketGCPartyAdd &kPartyAddPacket);
    bool RecvPartyUpdate(const TPacketGCPartyUpdate &p);
    bool RecvPartyPositionInfo(const TPacketGCPartyPositionInfo &p);
    bool RecvPartyRemove(const TPacketGCPartyRemove &p);
    bool RecvPartyLink(const TPacketGCPartyLink &p);
    bool RecvPartyUnlink(const TPacketGCPartyUnlink &p);
    bool RecvPartyParameter(const TPacketGCPartyParameter &p);

    // SafeBox
    bool RecvSafeBoxSetPacket(const GcItemSetPacket &kItemSet);
    bool RecvSafeBoxDelPacket(const TPacketGCItemDel &p);
    bool RecvSafeBoxWrongPasswordPacket(const TPacketCGSafeboxWrongPassword &p);
    bool RecvSafeBoxSizePacket(const TPacketCGSafeboxSize &p);

#ifdef ENABLE_GUILD_STORAGE
	//GuildStorage
	bool RecvGuildStorageItemSet();
	bool RecvGuildStorageItemDel();
	bool RecvGuildStorageOpenPacket();
	bool RecvGuildStorageLog();
#endif

    // Fishing
    bool RecvFishing(const TPacketGCFishing &p);

    // Dungeon
    bool RecvDungeon();

    // Time
    bool RecvTimePacket(const TPacketGCTime &p);

    // WalkMode
    bool RecvWalkModePacket(const TPacketGCWalkMode &p);

    // ChangeSkillGroup
    bool RecvChangeSkillGroupPacket(const TPacketGCChangeSkillGroup &p);

    // Refine
    bool RecvRefineInformationPacketNew(const TPacketGCRefineInformation &p);

    // Use Potion
    bool RecvSpecialEffect(const TPacketGCSpecialEffect &kSpecialEffect);

    // 서버에서 지정한 이팩트 발동 패킷.
    bool RecvSpecificEffect(const TPacketGCSpecificEffect &p);
    bool RecvSwitchbotSlotDataPacket(const GcSwitchbotSlotDataPacket &p);

    // 용혼석 관련
    bool RecvDragonSoulRefine(const TPacketGCDragonSoulRefine &p);
#ifdef ENABLE_BATTLE_PASS
public:
    bool SendBattlePassAction(BYTE bAction);
    bool RecvHuntingMissionPacket(const GcHuntingMissionPacket &p);
protected:
    bool RecvBattlePassPacket(const GcBattlePassPacket &p);
    bool RecvBattlePassRankingPacket(const TPacketGCBattlePassRanking &p);
    bool RecvBattlePassUpdatePacket(const GcBattlePassUpdatePacket &p);
    bool RecvUpdateChatFilterPacket(const TPacketGCUpdateChatFilter &p);
#endif

    // MiniMap Info
    bool RecvNPCList(const TPacketGCNPCPosition &p);
    bool RecvSHOPList(const TPacketGCShopPosition &p);
    bool RecvLandPacket(const TPacketGCLandList &p);
    bool RecvUpdateLandPacket(const TPacketGCGuildLandUpdate &p);
    bool RecvTargetCreatePacket(const TPacketGCTargetCreate &kTargetCreate);
    bool RecvTargetCreatePacketNew(const TPacketGCTargetCreate &kTargetCreate);
    bool RecvTargetUpdatePacket(const TPacketGCTargetUpdate &kTargetUpdate);
    bool RecvTargetDeletePacket(const TPacketGCTargetDelete &kTargetDelete);

    // Affect
    bool RecvAffectAddPacket(const GcAffectAddPacket &p);
    bool RecvAffectRemovePacket(const GcAffectDelPacket &p);

    // Channel
    bool RecvChannelPacket(const TPacketGCChannel &p);

    //NW
    bool RecvThreeWayStatus(const TPacketGCThreeWayStatus &p);
    bool RecvThreeWayLives(const TPacketGCThreeWayLives &p);

    // Item Seal
    bool RecvItemSealPacket(const TPacketGCSeal &p);
    bool RecvChangeLookPacket(const GcChangeLookPacket &p);

    // Acce
    bool RecvAccePacket(const TPacketGCAcce &p);

    bool RecvUpdateLastPlayPacket(const TGcUpdateLastPlay &p);

    // 
    bool RecvInputStatePacket(const TPacketGCInputState &p);
    bool RecvShopSearchSet(const GcShopSearchPacket &p);
    bool RecvHyperlinkItem(const CgHyperlinkItemPacket &p);

    bool RecvDungeonInfoPacket(const TPacketGCDungeonInfo &packet);
    bool RecvDungeonRankingSetPacket(const TPacketGCDungeonRanking &p);
    bool RecvDungeonUpdatePacket(const TPacketGCDungeonUpdate& p);

  public:
    bool SendRequestDungeonRanking(uint8_t bDungeonID, uint8_t bRankingType);
    bool SendRequestDungeonInfo();
    bool SendRequestDungeonWarp(uint8_t bDungeonID);

    void SwitchBotFinishCallback(const TItemPos &pos);
    void RegisterSlotCallback(const TItemPos &pos);
    void UnregisterSlotCallback(const TItemPos &pos);

    float GetAutoPotionScale() const
    {
        return m_autoPotionScale;
    }

protected:
    // 이모티콘
    bool ParseEmoticon(const char *pChatMsg, uint32_t *pdwEmoticon);

    // 파이썬으로 보내는 콜들
    void OnConnectFailure();
    void OnScriptEventStart(int iSkin, int iIndex);
    void OnQuestScriptEventStart(int iSkin, int iIndex);

    void OnRemoteDisconnect() override;
    void OnDisconnect() override;

    void SetGameOnline();
    void SetGameOffline();
    bool IsGameOnline();

protected:
    void RecvUnhandled(int header);

    bool RecvMultiStatus(const BlankPacket &p);

    void __InitializeGamePhase();

    bool __IsPlayerAttacking();
    bool __IsEquipItemInSlot(TItemPos Cell);

    void __ShowMapName(LONG lLocalX, LONG lLocalY);

    void __LeaveOfflinePhase()
    {
    }

    void __LeaveHandshakePhase()
    {
    }

    void __LeaveLoginPhase()
    {
    }

    void __LeaveSelectPhase()
    {
    }

    void __LeaveLoadingPhase()
    {
    }

    void __LeaveSentryPhase()
    {
    }

    void __LeaveGamePhase();

    void __ClearNetworkActorManager();

    void __ClearSelectCharacterData();

    // DELETEME
    //void __SendWarpPacket();

    void __ConvertEmpireText(uint32_t dwEmpireID, char *szText);

    void __RecvCharacterAppendPacket(SNetworkActorData *pkNetActorData);
    void __RecvCharacterUpdatePacket(SNetworkUpdateActorData *pkNetUpdateActorData);

    void __FilterInsult(char *szLine, UINT uLineLen);

    void __SetGuildID(uint32_t id);

protected:
    TPacketGCHandshake m_HandshakeData{};
    uint32_t m_dwChangingPhaseTime{};
    uint32_t m_dwBindupRetryCount{};
    uint32_t m_dwMainActorVID{};
    uint32_t m_dwMainActorRace{};
    uint32_t m_dwMainActorEmpire{};
    uint32_t m_dwMainActorSkillGroup{};
    bool m_firstHandShakeCompleted = false;
    bool m_delayedGameStart{};

    bool m_isGameOnline{};
    bool m_isChatEnable{};

    uint32_t m_dwGuildID{};
    uint32_t m_dwTempEmpireID{};
    uint8_t m_langCode;

    struct SServerTimeSync
    {
        uint32_t m_dwChangeServerTime;
        uint32_t m_dwChangeClientTime;
    } m_kServerTimeSync{};

    float m_autoPotionScale = 1.0f;

    void __ServerTimeSync_Initialize();
    //uint32_t m_dwBaseServerTime;
    //uint32_t m_dwBaseClientTime;

    uint32_t m_dwLastGamePingTime{};

    std::string m_login;
    std::string m_strLastCommand;
    std::string m_strPhase;
    SessionId m_sessionId;
    bool m_isWaitLoginKey;

    std::string m_stMarkIP;

    CPacketHandlerFuncObject<CPythonNetworkStream> m_phaseProcessFunc;
    CFuncObject<CPythonNetworkStream> m_phaseLeaveFunc;

    py::object m_poHandler;
    py::object m_apoPhaseWnd[PHASE_WINDOW_NUM]{};
    py::object m_poSerCommandParserWnd;

    SimplePlayer m_players[PLAYER_PER_ACCOUNT]{};
    uint32_t m_adwGuildID[PLAYER_PER_ACCOUNT]{};
    std::string m_astrGuildName[PLAYER_PER_ACCOUNT];
    std::string m_playerNames[PLAYER_PER_ACCOUNT];

    std::unique_ptr<CNetworkActorManager> m_rokNetActorMgr{new CNetworkActorManager};
    bool m_multiAllowed = false;

    bool m_isRefreshStatus{};
    bool m_isRefreshCharacterWnd{};
    bool m_isRefreshEquipmentWnd{};
    bool m_isRefreshInventoryWnd{};
    bool m_isRefreshExchangeWnd{};
    bool m_isRefreshSkillWnd{};
    bool m_isRefreshSafeboxWnd{};
    bool m_isRefreshMallWnd{};
    bool m_isRefreshMessengerWnd{};
    bool m_isRefreshGuildWndInfoPage{};
    bool m_isRefreshGuildWndBoardPage{};
    bool m_isRefreshGuildWndMemberPage{};
    bool m_isRefreshGuildWndMemberPageGradeComboBox{};
    bool m_isRefreshGuildWndSkillPage{};
    bool m_isRefreshGuildWndGradePage{};
    bool m_isRefreshAcceWnd{};
    bool m_isRefreshShopInfoWnd{};
    bool m_isRefreshShopSearchWnd{};
    bool m_isRefreshHuntingMissions{};

#ifdef ENABLE_GUILD_STORAGE
	bool m_isRefreshGuildStorageWnd{};
	bool m_isRefreshGuildStorageLogWnd{};
#endif
    // Emoticon
    std::vector<std::string> m_EmoticonStringVector;

    struct STextConvertTable
    {
        char acUpper[26];
        char acLower[26];
        uint8_t aacHan[5000][2];
    } m_aTextConvTable[3]{};

    std::string m_markAddr;
    std::string m_markPort;

    uint32_t m_selectedCharacterSlot{};
    bool m_autoSelectCharacter{};

    CInsultChecker m_kInsultChecker;

    bool m_bComboSkillFlag{};

    std::deque<std::string> m_kQue_stHack;

public:
    uint32_t EXPORT_GetBettingGuildWarValue(const char *c_szValueName);

private:
    struct SBettingGuildWar
    {
        uint32_t m_dwBettingMoney;
        uint32_t m_dwObserverCount;
    } m_kBettingGuildWar{};

    CInstanceBase *m_pInstTarget{};

    void __BettingGuildWar_Initialize();
    void __BettingGuildWar_SetObserverCount(UINT uObserverCount);
    void __BettingGuildWar_SetBettingMoney(UINT uBettingMoney);
private:

public:

    void SetPos(uint8_t bPos)
    {
        m_bPos = bPos;
    }

    uint8_t GetPos() const
    {
        return m_bPos;
    }

    void SetLastTime(uint8_t dwLastTime)
    {
        m_dwLastTime = dwLastTime;
    }

    uint8_t GetLastTime() const
    {
        return m_dwLastTime;
    }

    bool GetMultiState() const { return m_multiAllowed; }

    std::pair<uint32_t, std::string> isDetected() const
    {
        return {m_timeState, m_timeDesc};
    }

    void detected(uint8_t state, std::string desc);

protected:
    uint8_t m_bPos;
    uint32_t m_dwLastTime;
    std::array<std::string, 3> m_strHash;
    std::string m_timeDesc;
    uint32_t m_timeState;

protected:
    virtual void OnConnectSuccess() override final;
    virtual void OnConnectFailure(const boost::system::error_code &ec) override final;

    virtual bool HandlePacket(const PacketHeader &header,
                              const boost::asio::const_buffer &data) override final;

};
extern std::shared_ptr<CPythonNetworkStream> gPythonNetworkStream;

#endif
