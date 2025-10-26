#include "PythonNetworkStream.h"
#include "StdAfx.h"
#include <game/GamePacket.hpp>

#include "PythonApplication.h"
#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "PythonEventManager.h"
#include "PythonExchange.h"
#include "PythonGuild.h"
#include "PythonItem.h"
#include "PythonMessenger.h"
#include "PythonMiniMap.h"
#include "PythonPlayer.h"
#include "PythonQuest.h"
#include "PythonShop.h"
#include "PythonTextTail.h"

#include "../gamelib/ItemManager.h"

#include "InstanceBase.h"
#include "PythonFishingManager.h"

#include "PythonPrivateShopSearch.h"
#include <game/MotionConstants.hpp>
#include "PythonDungeonInfo.h"

#ifdef __OFFLINE_SHOP__
#include "PythonOfflineShop.h"
#endif

bool gs_bEmpireLanuageEnable = TRUE;

void CPythonNetworkStream::__RefreshAlignmentWindow()
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshAlignment");
}

void CPythonNetworkStream::__RefreshTargetBoardByVID(uint32_t dwVID)
{
    try {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "RefreshTargetBoardByVID", (int)dwVID);
    } catch (const py::error_already_set& e) {
        SPDLOG_ERROR("{}", e.what());
    }
}

void CPythonNetworkStream::__RefreshTargetBoardByName(const char* c_szName)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "RefreshTargetBoardByName", c_szName);
}

void CPythonNetworkStream::__RefreshTargetBoard()
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoar"
                                                            "d");
}

void CPythonNetworkStream::__RefreshGuildWindowGradePage()
{
    m_isRefreshGuildWndGradePage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowSkillPage()
{
    m_isRefreshGuildWndSkillPage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPageGradeComboBox()
{
    m_isRefreshGuildWndMemberPageGradeComboBox = true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPage()
{
    m_isRefreshGuildWndMemberPage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowBoardPage()
{
    m_isRefreshGuildWndBoardPage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowInfoPage()
{
    m_isRefreshGuildWndInfoPage = true;
}

void CPythonNetworkStream::__RefreshMessengerWindow()
{
    m_isRefreshMessengerWnd = true;
}

void CPythonNetworkStream::__RefreshSafeboxWindow()
{
    m_isRefreshSafeboxWnd = true;
}

void CPythonNetworkStream::__RefreshAcceWindow()
{
    m_isRefreshAcceWnd = true;
}

void CPythonNetworkStream::__RefreshShopSearchWindow()
{
    m_isRefreshShopSearchWnd = true;
}

void CPythonNetworkStream::__RefreshShopInfoWindow()
{
    m_isRefreshShopInfoWnd = true;
}

void CPythonNetworkStream::__RefreshMallWindow()
{
    m_isRefreshMallWnd = true;
}

void CPythonNetworkStream::__RefreshSkillWindow()
{
    m_isRefreshSkillWnd = true;
}

void CPythonNetworkStream::__RefreshExchangeWindow()
{
    m_isRefreshExchangeWnd = true;
}

void CPythonNetworkStream::__RefreshStatus()
{
    m_isRefreshStatus = true;
}

void CPythonNetworkStream::__RefreshCharacterWindow()
{
    m_isRefreshCharacterWnd = true;
}

void CPythonNetworkStream::__RefreshInventoryWindow()
{
    m_isRefreshInventoryWnd = true;
}

void CPythonNetworkStream::__RefreshEquipmentWindow()
{
    m_isRefreshEquipmentWnd = true;
}
void CPythonNetworkStream::__RefreshHuntingMissions()
{
    m_isRefreshHuntingMissions = true;
}

#ifdef ENABLE_GUILD_STORAGE
void CPythonNetworkStream::__RefreshGuildStorageWindow()
{
    m_isRefreshGuildStorageWnd = true;
}

void CPythonNetworkStream::__RefreshGuildStorageLogWindow()
{
    m_isRefreshGuildStorageLogWnd = true;
}
#endif // ENABLE_GUILD_STORAGE

void CPythonNetworkStream::__SetGuildID(uint32_t id)
{
    if (m_dwGuildID != id) {
        m_dwGuildID = id;
        CPythonPlayer& rkPlayer = CPythonPlayer::Instance();

        for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
            if (!strncmp(m_players[i].name, rkPlayer.GetName(),
                         CHARACTER_NAME_MAX_LEN)) {
                m_adwGuildID[i] = id;
                m_astrGuildName[i] =
                    CPythonGuild::Instance().GetGuildName(id).value_or("");
            }
    }
}

struct PERF_PacketInfo {
    uint32_t dwCount;
    uint32_t dwTime;

    PERF_PacketInfo()
    {
        dwCount = 0;
        dwTime = 0;
    }
};

#ifdef __PERFORMANCE_CHECK__

class PERF_PacketTimeAnalyzer
{
    public:
    ~PERF_PacketTimeAnalyzer()
    {
        FILE* fp = fopen("perf_dispatch_packet_result.txt", "w");

        for (std::map<uint32_t, PERF_PacketInfo>::iterator i =
                 m_kMap_kPacketInfo.begin();
             i != m_kMap_kPacketInfo.end(); ++i) {
            if (i->second.dwTime > 0)
                fprintf(fp, "header %d: count %d, time %d, tpc %d\n", i->first,
                        i->second.dwCount, i->second.dwTime,
                        i->second.dwTime / i->second.dwCount);
        }
        fclose(fp);
    }

    public:
    std::map<uint32_t, PERF_PacketInfo> m_kMap_kPacketInfo;
};

PERF_PacketTimeAnalyzer gs_kPacketTimeAnalyzer;

#endif

// Game Phase
// ---------------------------------------------------------------------------
bool CPythonNetworkStream::GamePhase(const PacketHeader& header,
                                     const boost::asio::const_buffer& data)
{
    if (!m_kQue_stHack.empty()) {
        __SendHack(m_kQue_stHack.front().c_str());
        m_kQue_stHack.pop_front();
    }

#define HANDLE_PACKET(id, fn, type)                                            \
    case id:                                                                   \
        ret = this->fn(ReadPacket<type>(data));                                \
        break

    bool ret = true;

    switch (header.id) {
        HANDLE_PACKET(HEADER_GC_WARP, RecvWarpPacket, TPacketGCWarp);
        HANDLE_PACKET(HEADER_GC_PVP, RecvPVPPacket, TPacketGCPVP);
        HANDLE_PACKET(HEADER_GC_DUEL_START, RecvDuelStartPacket,
                      TPacketGCDuelStart);
        HANDLE_PACKET(HEADER_GC_ADD_FLY_TARGETING, RecvAddFlyTargetingPacket,
                      TPacketGCFlyTargeting);
        HANDLE_PACKET(HEADER_GC_CHARACTER_ADD, RecvCharacterAppendPacket,
                      TPacketGCCharacterAdd);
        HANDLE_PACKET(HEADER_GC_CHAR_ADDITIONAL_INFO,
                      RecvCharacterAdditionalInfo,
                      TPacketGCCharacterAdditionalInfo);
        HANDLE_PACKET(HEADER_GC_CHARACTER_UPDATE, RecvCharacterUpdatePacket,
                      TPacketGCCharacterUpdate);
        HANDLE_PACKET(HEADER_GC_CHARACTER_DEL, RecvCharacterDeletePacket,
                      TPacketGCCharacterDelete);
        HANDLE_PACKET(HEADER_GC_CHAT, RecvChatPacket, TPacketGCChat);
        HANDLE_PACKET(HEADER_GC_SYNC_POSITION, RecvSyncPositionPacket,
                      TPacketGCSyncPosition);
        HANDLE_PACKET(HEADER_GC_OWNERSHIP, RecvOwnerShipPacket,
                      TPacketGCOwnership);
        HANDLE_PACKET(HEADER_GC_CHARACTER_MOVE, RecvCharacterMovePacket,
                      TPacketGCMove);
        HANDLE_PACKET(HEADER_GC_NPC_SKILL, RecvNpcUseSkill,
                      TPacketGCNPCUseSkill);
        HANDLE_PACKET(HEADER_GC_SKILL_MOTION, RecvSkillMotion,
                      GcSkillMotionPacket);
        HANDLE_PACKET(HEADER_GC_WHISPER, RecvWhisperPacket, TPacketGCWhisper);
        HANDLE_PACKET(HEADER_GC_EXCHANGE_INFO, RecvExchangeInfoPacket,
                      TPacketGCExchageInfo);
        HANDLE_PACKET(HEADER_GC_STUN, RecvStunPacket, TPacketGCStun);
        HANDLE_PACKET(HEADER_GC_DEAD, RecvDeadPacket, TPacketGCDead);
        HANDLE_PACKET(HEADER_GC_CHARACTER_POINT_CHANGE, RecvPointChange,
                      TPacketGCPointChange);
        HANDLE_PACKET(HEADER_GC_CHARACTER_GOLD_CHANGE, RecvGoldChange,
                      TPacketGCGoldChange);

        HANDLE_PACKET(HEADER_GC_ITEM_DEL, RecvItemDelPacket, TPacketGCItemDel);
        HANDLE_PACKET(HEADER_GC_ITEM_SET, RecvItemSetPacket, GcItemSetPacket);
        HANDLE_PACKET(HEADER_GC_SHOPSEARCH, RecvShopSearchSet,
                      GcShopSearchPacket);
        HANDLE_PACKET(HEADER_GC_ITEM_GROUND_ADD, RecvItemGroundAddPacket,
                      GcPacketItemGroundAdd);
        HANDLE_PACKET(HEADER_GC_ITEM_GROUND_DEL, RecvItemGroundDelPacket,
                      TPacketGCItemGroundDel);
        HANDLE_PACKET(HEADER_GC_ITEM_OWNERSHIP, RecvItemOwnership,
                      TPacketGCItemOwnership);
        HANDLE_PACKET(HEADER_GC_QUICKSLOT_ADD, RecvQuickSlotAddPacket,
                      packet_quickslot_add);
        HANDLE_PACKET(HEADER_GC_QUICKSLOT_DEL, RecvQuickSlotDelPacket,
                      packet_quickslot_del);
        HANDLE_PACKET(HEADER_GC_QUICKSLOT_SWAP, RecvQuickSlotMovePacket,
                      packet_quickslot_swap);

        HANDLE_PACKET(HEADER_GC_SHOP, RecvShopPacket, TPacketGCShop);
        HANDLE_PACKET(HEADER_GC_WIKI, RecvWikiPacket, GcWikiPacket);
        HANDLE_PACKET(HEADER_GC_PICKUP_INFO, RecvItemPickupInfoPacket,
                      GcItemPickupInfoPacket);
        HANDLE_PACKET(HEADER_GC_WIKI_MOB, RecvWikiMobPacket, GcWikiMobPacket);
        HANDLE_PACKET(HEADER_GC_SHOP_SIGN, RecvShopSignPacket,
                      TPacketGCShopSign);

        HANDLE_PACKET(HEADER_GC_EXCHANGE, RecvExchangePacket, packet_exchange);
        HANDLE_PACKET(HEADER_GC_QUEST_INFO, RecvQuestInfoPacket,
                      packet_quest_info);

        HANDLE_PACKET(HEADER_GC_REQUEST_MAKE_GUILD, RecvRequestMakeGuild,
                      BlankPacket);
        HANDLE_PACKET(HEADER_GC_SCRIPT, RecvScriptPacket, packet_script);
        HANDLE_PACKET(HEADER_GC_QUEST_CONFIRM, RecvQuestConfirmPacket,
                      TPacketGCQuestConfirm);
        HANDLE_PACKET(HEADER_GC_TARGET, RecvTargetPacket, TPacketGCTarget);
        HANDLE_PACKET(HEADER_GC_TARGET_INFO, RecvTargetInfoPacket,
                      TPacketGCTargetInfo);
        HANDLE_PACKET(HEADER_GC_DAMAGE_INFO, RecvDamageInfoPacket,
                      TPacketGCDamageInfo);
        HANDLE_PACKET(HEADER_GC_MOUNT, RecvMountPacket, GcMountPacket);
        HANDLE_PACKET(HEADER_GC_CHARACTER_POINTS, __RecvPlayerPoints,
                      TPacketGCPoints);
        HANDLE_PACKET(HEADER_GC_CHARACTER_GOLD, __RecvPlayerGold,
                      TPacketGCGold);
        HANDLE_PACKET(HEADER_GC_CREATE_FLY, RecvCreateFlyPacket,
                      TPacketGCCreateFly);
        HANDLE_PACKET(HEADER_GC_FLY_TARGETING, RecvFlyTargetingPacket,
                      TPacketGCFlyTargeting);

        HANDLE_PACKET(HEADER_GC_SKILL_LEVEL, RecvSkillLevelNew,
                      TPacketGCSkillLevel);
        HANDLE_PACKET(HEADER_GC_MESSENGER, RecvMessenger, TPacketGCMessenger);
        HANDLE_PACKET(HEADER_GC_GUILD, RecvGuild, TPacketGCGuild);

        HANDLE_PACKET(HEADER_GC_PARTY_INVITE, RecvPartyInvite,
                      TPacketGCPartyInvite);
        HANDLE_PACKET(HEADER_GC_PARTY_ADD, RecvPartyAdd, TPacketGCPartyAdd);
        HANDLE_PACKET(HEADER_GC_PARTY_UPDATE, RecvPartyUpdate,
                      TPacketGCPartyUpdate);
        HANDLE_PACKET(HEADER_GC_PARTY_POSITION_INFO, RecvPartyPositionInfo,
                      TPacketGCPartyPositionInfo);
        HANDLE_PACKET(HEADER_GC_PARTY_REMOVE, RecvPartyRemove,
                      TPacketGCPartyRemove);
        HANDLE_PACKET(HEADER_GC_PARTY_LINK, RecvPartyLink, TPacketGCPartyLink);
        HANDLE_PACKET(HEADER_GC_PARTY_UNLINK, RecvPartyUnlink,
                      TPacketGCPartyUnlink);
        HANDLE_PACKET(HEADER_GC_PARTY_PARAMETER, RecvPartyParameter,
                      TPacketGCPartyParameter);

        HANDLE_PACKET(HEADER_GC_SAFEBOX_SET, RecvSafeBoxSetPacket,
                      GcItemSetPacket);
        HANDLE_PACKET(HEADER_GC_SAFEBOX_DEL, RecvSafeBoxDelPacket,
                      TPacketGCItemDel);
        HANDLE_PACKET(HEADER_GC_SAFEBOX_WRONG_PASSWORD,
                      RecvSafeBoxWrongPasswordPacket,
                      TPacketCGSafeboxWrongPassword);
        HANDLE_PACKET(HEADER_GC_SAFEBOX_SIZE, RecvSafeBoxSizePacket,
                      TPacketCGSafeboxSize);

        HANDLE_PACKET(HEADER_GC_FISHING, RecvFishing, TPacketGCFishing);

        HANDLE_PACKET(HEADER_GC_WALK_MODE, RecvWalkModePacket,
                      TPacketGCWalkMode);

        HANDLE_PACKET(HEADER_GC_SKILL_GROUP, RecvChangeSkillGroupPacket,
                      TPacketGCChangeSkillGroup);
        HANDLE_PACKET(HEADER_GC_REFINE_INFORMATION,
                      RecvRefineInformationPacketNew,
                      TPacketGCRefineInformation);
        HANDLE_PACKET(HEADER_GC_SEPCIAL_EFFECT, RecvSpecialEffect,
                      TPacketGCSpecialEffect);
        HANDLE_PACKET(HEADER_GC_NPC_POSITION, RecvNPCList,
                      TPacketGCNPCPosition);
        HANDLE_PACKET(HEADER_GC_SHOP_POSITION, RecvSHOPList,
                      TPacketGCShopPosition);
        HANDLE_PACKET(HEADER_GC_CHANNEL, RecvChannelPacket, TPacketGCChannel);
        HANDLE_PACKET(HEADER_GC_VIEW_EQUIP, RecvViewEquipPacket,
                      TPacketViewEquip);
        HANDLE_PACKET(HEADER_GC_LAND_LIST, RecvLandPacket, TPacketGCLandList);
        HANDLE_PACKET(HEADER_GC_UPDATE_LAND, RecvUpdateLandPacket,
                      TPacketGCGuildLandUpdate);
        HANDLE_PACKET(HEADER_GC_TARGET_CREATE, RecvTargetCreatePacketNew,
                      TPacketGCTargetCreate);
        HANDLE_PACKET(HEADER_GC_TARGET_UPDATE, RecvTargetUpdatePacket,
                      TPacketGCTargetUpdate);
        HANDLE_PACKET(HEADER_GC_TARGET_DELETE, RecvTargetDeletePacket,
                      TPacketGCTargetDelete);
        HANDLE_PACKET(HEADER_GC_AFFECT_ADD, RecvAffectAddPacket,
                      GcAffectAddPacket);
        HANDLE_PACKET(HEADER_GC_AFFECT_REMOVE, RecvAffectRemovePacket,
                      GcAffectDelPacket);
        HANDLE_PACKET(HEADER_GC_MALL_OPEN, RecvMallOpenPacket,
                      TPacketCGSafeboxSize);

        HANDLE_PACKET(HEADER_GC_MALL_SET, RecvMallItemSetPacket,
                      GcItemSetPacket);
        HANDLE_PACKET(HEADER_GC_MALL_DEL, RecvMallItemDelPacket,
                      TPacketGCItemDel);
        HANDLE_PACKET(HEADER_GC_LOVER_INFO, RecvLoverInfoPacket,
                      TPacketGCLoverInfo);
        HANDLE_PACKET(HEADER_GC_LOVE_POINT_UPDATE, RecvLovePointUpdatePacket,
                      TPacketGCLovePointUpdate);
        HANDLE_PACKET(HEADER_GC_DIG_MOTION, RecvDigMotionPacket,
                      TPacketGCDigMotion);
        HANDLE_PACKET(HEADER_GC_HANDSHAKE, RecvHandshakePacket,
                      TPacketGCHandshake);
        HANDLE_PACKET(HEADER_GC_TIME_SYNC, RecvHandshakeOKPacket, BlankPacket);

        HANDLE_PACKET(HEADER_GC_SPECIFIC_EFFECT, RecvSpecificEffect,
                      TPacketGCSpecificEffect);
        HANDLE_PACKET(HEADER_GC_DRAGON_SOUL_REFINE, RecvDragonSoulRefine,
                      TPacketGCDragonSoulRefine);
        HANDLE_PACKET(HEADER_GC_BATTLE_PASS_OPEN, RecvBattlePassPacket,
                      GcBattlePassPacket);
        HANDLE_PACKET(HEADER_GC_BATTLE_PASS_UPDATE, RecvBattlePassUpdatePacket,
                      GcBattlePassUpdatePacket);
        HANDLE_PACKET(HEADER_GC_BATTLE_PASS_RANKING,
                      RecvBattlePassRankingPacket, TPacketGCBattlePassRanking);
        HANDLE_PACKET(HEADER_GC_HUNTING_MISSIONS, RecvHuntingMissionPacket,
                      GcHuntingMissionPacket);
        HANDLE_PACKET(HEADER_GC_THREEWAY_STATUS, RecvThreeWayStatus,
                      TPacketGCThreeWayStatus);
        HANDLE_PACKET(HEADER_GC_THREEWAY_LIVES, RecvThreeWayLives,
                      TPacketGCThreeWayLives);
        HANDLE_PACKET(HEADER_GC_INPUT_STATE, RecvInputStatePacket,
                      TPacketGCInputState);
        HANDLE_PACKET(HEADER_GC_SEAL, RecvItemSealPacket, TPacketGCSeal);
        HANDLE_PACKET(HEADER_GC_ACCE, RecvAccePacket, TPacketGCAcce);
        HANDLE_PACKET(HEADER_GC_CHANGELOOK, RecvChangeLookPacket,
                      GcChangeLookPacket);

        HANDLE_PACKET(HEADER_GC_UPDATE_LAST_PLAY, RecvUpdateLastPlayPacket,
                      TGcUpdateLastPlay);
        HANDLE_PACKET(HEADER_GC_MAINTENANCE, RecvMaintenance,
                      TPacketGCMaintenance);
        HANDLE_PACKET(HEADER_GC_UPDATE_CHAT_FILTER, RecvUpdateChatFilterPacket,
                      TPacketGCUpdateChatFilter);
        HANDLE_PACKET(HEADER_GC_SWITCHBOT_DATA, RecvSwitchbotSlotDataPacket,
                      GcSwitchbotSlotDataPacket);

        HANDLE_PACKET(HEADER_GC_HYPERLINK_ITEM, RecvHyperlinkItem,
                      CgHyperlinkItemPacket);

        HANDLE_PACKET(HEADER_GC_DUNGEON_INFO, RecvDungeonInfoPacket,
                      TPacketGCDungeonInfo);
        HANDLE_PACKET(HEADER_GC_DUNGEON_RANKING, RecvDungeonRankingSetPacket,
                      TPacketGCDungeonRanking);
        HANDLE_PACKET(HEADER_GC_DUNGEON_UPDATE, RecvDungeonUpdatePacket,
                      TPacketGCDungeonUpdate);

#ifdef __OFFLINE_SHOP__
        case HEADER_GC_OFFLINE_SHOP:
            ret = CPythonOfflineShop::Instance().ReceiveOfflineShopPacket(
                ReadPacket<TPacketGCOfflineShop>(data));
            break;

        case HEADER_GC_OFFLINE_SHOP_ADDITIONAL_INFO:
            ret = CPythonOfflineShop::instance().ReceiveAdditionalInfoPacket(
                ReadPacket<TPacketGCOfflineShopAdditionalInfo>(data));
            break;

        case HEADER_GC_OFFLINE_SHOP_GOLD:
            ret = CPythonOfflineShop::instance().ReceiveGoldPacket(
                ReadPacket<TPacketGCOfflineShopGold>(data));
            break;

        case HEADER_GC_OFFLINE_SHOP_ITEM_ADD:
            ret = CPythonOfflineShop::instance().ReceiveAddItemPacket(
                ReadPacket<TPacketGCOfflineShopAddItem>(data));
            break;

        case HEADER_GC_OFFLINE_SHOP_ITEM_MOVE:
            ret = CPythonOfflineShop::instance().ReceiveMoveItemPacket(
                ReadPacket<TPacketGCOfflineShopMoveItem>(data));
            break;

        case HEADER_GC_OFFLINE_SHOP_ITEM_REMOVE:
            ret = CPythonOfflineShop::instance().ReceiveRemoveItemPacket(
                ReadPacket<TPacketGCOfflineShopRemoveItem>(data));
            break;

        case HEADER_GC_OFFLINE_SHOP_NAME:
            ret = CPythonOfflineShop::instance().ReceiveNamePacket(
                ReadPacket<TPacketGCOfflineShopName>(data));
            break;

#if defined(__OFFLINE_SHOP_OPENING_TIME__) ||                                  \
    defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
        case HEADER_GC_OFFLINE_SHOP_LEFT_OPENING_TIME:
            ret = CPythonOfflineShop::instance().ReceiveLeftOpeningTimePacket(
                ReadPacket<TPacketGCOfflineShopLeftOpeningTime>(data));
            break;
#endif

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
        case HEADER_GC_OFFLINE_SHOP_EDITOR_POSITION_INFO:
            ret = CPythonOfflineShop::instance()
                      .ReceiveOfflineShopEditorPositionInfoPacket(
                          ReadPacket<TPacketGCOfflineShopEditorPositionInfo>(
                              data));
            break;
#endif
#endif

        default:
            break;
    }

    return ret;
}

void CPythonNetworkStream::__InitializeGamePhase()
{
    __ServerTimeSync_Initialize();

    m_isRefreshStatus = false;
    m_isRefreshCharacterWnd = false;
    m_isRefreshEquipmentWnd = false;
    m_isRefreshInventoryWnd = false;
    m_isRefreshShopInfoWnd = false;
    m_isRefreshShopSearchWnd = false;
    m_isRefreshExchangeWnd = false;
    m_isRefreshSkillWnd = false;
    m_isRefreshSafeboxWnd = false;
    m_isRefreshMallWnd = false;
    m_isRefreshMessengerWnd = false;
    m_isRefreshGuildWndInfoPage = false;
    m_isRefreshGuildWndBoardPage = false;
    m_isRefreshGuildWndMemberPage = false;
    m_isRefreshGuildWndMemberPageGradeComboBox = false;
    m_isRefreshGuildWndSkillPage = false;
    m_isRefreshGuildWndGradePage = false;
#ifdef ENABLE_GUILD_STORAGE
    m_isRefreshGuildStorageWnd = false;
    m_isRefreshGuildStorageLogWnd = false;
#endif // ENABLE_GUILD_STORAGE

    m_EmoticonStringVector.clear();

    m_pInstTarget = NULL;
}

void CPythonNetworkStream::Warp(uint32_t mapIndex, uint32_t x, uint32_t y)
{
    auto& background = CPythonBackground::Instance();

    background.Destroy();
    background.Create();

    if (!background.Warp(mapIndex, x, y))
        return;

    background.SelectViewDistanceNum(CPythonBackground::DISTANCE0);
    StartGame();
    CPythonApplication::AppInst().SetCenterPosition(x, y);
    __ShowMapName(x, y);
}

void CPythonNetworkStream::__ShowMapName(LONG x, LONG y)
{
    const std::string& c_rstrMapFileName =
        CPythonBackground::Instance().GetWarpMapName();
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ShowMapName",
                          py::bytes(c_rstrMapFileName), x, y);
}

void CPythonNetworkStream::SwitchBotFinishCallback(const TItemPos& pos)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_SwitchBotFinished", pos.window_type,
                          pos.cell);
}

void CPythonNetworkStream::RegisterSlotCallback(const TItemPos& pos)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_SwitchBotRegisterSlot", pos.window_type,
                          pos.cell);
}

void CPythonNetworkStream::UnregisterSlotCallback(const TItemPos& pos)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_SwitchBotUnregisterSlot", pos.window_type,
                          pos.cell);
}

void CPythonNetworkStream::__LeaveGamePhase()
{
    CInstanceBase::ClearPVPKeySystem();

    __ClearNetworkActorManager();

    m_bComboSkillFlag = FALSE;

    CPythonCharacterManager::Instance().Destroy();
    CPythonItem::Instance().Destroy();
    CPythonPlayer::Instance().LeaveGamePhase();
}

void CPythonNetworkStream::SetGamePhase()
{
    if ("Game" != m_strPhase)
        m_phaseLeaveFunc.Run();

    SPDLOG_DEBUG("");
    SPDLOG_DEBUG("## Network - Game Phase ##");
    SPDLOG_DEBUG("");

    m_strPhase = "Game";

    m_dwChangingPhaseTime = ELTimer_GetMSec();
    m_phaseProcessFunc.Set(this, &CPythonNetworkStream::GamePhase);
    m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveGamePhase);

    // Main Character µî·ÏO

    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
    rkPlayer.SetMainCharacterIndex(GetMainActorVID());

    __RefreshStatus();
}

bool CPythonNetworkStream::RecvWarpPacket(const TPacketGCWarp& p)
{

    m_autoSelectCharacter = true;
    Connect(p.lAddr, fmt::to_string(p.wPort));
    return true;
}

bool CPythonNetworkStream::RecvDuelStartPacket(const TPacketGCDuelStart& p)
{
    CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

    CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();
    if (!pkInstMain) {
        SPDLOG_ERROR("CPythonNetworkStream::RecvDuelStartPacket - "
                     "MainCharacter is NULL");
        return false;
    }
    uint32_t dwVIDSrc = pkInstMain->GetVirtualID();

    for (uint32_t i = 0; i < p.participants.size(); i++) {
        CInstanceBase::InsertDUELKey(dwVIDSrc, p.participants[i]);
    }

    if (p.participants.empty())
        pkInstMain->SetDuelMode(CInstanceBase::DUEL_CANNOTATTACK);
    else
        pkInstMain->SetDuelMode(CInstanceBase::DUEL_START);

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard");

    rkChrMgr.RefreshAllPCTextTail();

    return true;
}

bool CPythonNetworkStream::RecvPVPPacket(const TPacketGCPVP& p)
{
    CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();

    switch (p.bMode) {
        case PVP_MODE_AGREE:
            rkChrMgr.RemovePVPKey(p.dwVIDSrc, p.dwVIDDst);

            // 상대가 나(Dst)에게 동의를 구했을때
            if (rkPlayer.IsMainCharacterIndex(p.dwVIDDst))
                rkPlayer.RememberChallengeInstance(p.dwVIDSrc);

            // 상대에게 동의를 구한 동안에는 대결 불능
            if (rkPlayer.IsMainCharacterIndex(p.dwVIDSrc))
                rkPlayer.RememberCantFightInstance(p.dwVIDDst);
            break;
        case PVP_MODE_REVENGE: {
            rkChrMgr.RemovePVPKey(p.dwVIDSrc, p.dwVIDDst);

            DWORD dwKiller = p.dwVIDSrc;
            DWORD dwVictim = p.dwVIDDst;

            // 내(victim)가 상대에게 복수할 수 있을때
            if (rkPlayer.IsMainCharacterIndex(dwVictim))
                rkPlayer.RememberRevengeInstance(dwKiller);

            // 상대(victim)가 나에게 복수하는 동안에는 대결 불능
            if (rkPlayer.IsMainCharacterIndex(dwKiller))
                rkPlayer.RememberCantFightInstance(dwVictim);
            break;
        }

        case PVP_MODE_FIGHT:
            rkChrMgr.InsertPVPKey(p.dwVIDSrc, p.dwVIDDst);
            rkPlayer.ForgetInstance(p.dwVIDSrc);
            rkPlayer.ForgetInstance(p.dwVIDDst);
            break;
        case PVP_MODE_NONE:
            rkChrMgr.RemovePVPKey(p.dwVIDSrc, p.dwVIDDst);
            rkPlayer.ForgetInstance(p.dwVIDSrc);
            rkPlayer.ForgetInstance(p.dwVIDDst);
            break;
    }

    // NOTE : PVP 토글시 TargetBoard 를 업데이트 합니다.
    __RefreshTargetBoardByVID(p.dwVIDSrc);
    __RefreshTargetBoardByVID(p.dwVIDDst);

    return true;
}

void CPythonNetworkStream::NotifyHack(const char* c_szMsg)
{
    if (!m_kQue_stHack.empty())
        if (c_szMsg == m_kQue_stHack.back())
            return;

    m_kQue_stHack.push_back(c_szMsg);
}

bool CPythonNetworkStream::__SendHack(const char* c_szMsg)
{
    TPacketCGHack kPacketHack;
    kPacketHack.szBuf = c_szMsg;
    Send(HEADER_CG_HACK, kPacketHack);
    return true;
}

bool CPythonNetworkStream::SendMessengerAddByVIDPacket(uint32_t vid)
{
    TPacketCGMessenger packet;
    packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_VID;
    packet.vid = vid;
    Send(HEADER_CG_MESSENGER, packet);
    return true;
}

bool CPythonNetworkStream::SendMessengerAddByNamePacket(const char* c_szName)
{
    TPacketCGMessenger packet;
    packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_NAME;
    packet.name.emplace(c_szName);
    Send(HEADER_CG_MESSENGER, packet);

    SPDLOG_DEBUG(" SendMessengerAddByNamePacket : {0}", c_szName);
    return true;
}

bool CPythonNetworkStream::SendMessengerRemovePacket(const char* c_szKey,
                                                     const char* c_szName)
{
    TPacketCGMessenger packet;
    packet.subheader = MESSENGER_SUBHEADER_CG_REMOVE;
    packet.name.emplace(c_szKey);
    Send(HEADER_CG_MESSENGER, packet);
    __RefreshTargetBoardByName(c_szName);
    return true;
}

bool CPythonNetworkStream::SendMessengerSetBlockPacket(const char* name,
                                                       uint16_t mode)
{
    TPacketCGMessenger packet;
    packet.subheader = MESSENGER_SUBHEADER_CG_SET_BLOCK;

    PacketCGMessengerSetBlock p;
    p.name = name;
    p.mode = mode;
    packet.setBlock = p;

    Send(HEADER_CG_MESSENGER, packet);

    return true;
}

bool CPythonNetworkStream::SendCharacterStatePacket(
    const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg,
    uint32_t color, uint32_t motionKey)
{
    if (!__CanActMainInstance())
        return true;

    if (fDstRot < 0.0f)
        fDstRot = 360 + fDstRot;
    else if (fDstRot > 360.0f)
        fDstRot = fmodf(fDstRot, 360.0f);

    TPacketCGMove kStatePacket;
    kStatePacket.bFunc = eFunc;
    kStatePacket.bArg = uArg;
    kStatePacket.bRot = fDstRot / 5.0f;
    kStatePacket.lX = long(c_rkPPosDst.x);
    kStatePacket.lY = long(c_rkPPosDst.y);
    kStatePacket.dwTime = ELTimer_GetServerMSec();
    kStatePacket.color = color;
    kStatePacket.motionKey = motionKey;

    assert(kStatePacket.lX >= 0 && kStatePacket.lX < 204800);

    Send(HEADER_CG_CHARACTER_MOVE, kStatePacket);

    return true;
}

bool CPythonNetworkStream::SendCharacterSkillStatePacket(
    const TPixelPosition& c_rkPPosDst, float fDstRot, UINT uArg,
    uint8_t loopCount, uint8_t isMovingSkill, uint32_t color,
    uint32_t motionKey)
{
    if (!__CanActMainInstance())
        return true;

    if (fDstRot < 0.0f)
        fDstRot = 360 + fDstRot;
    else if (fDstRot > 360.0f)
        fDstRot = fmodf(fDstRot, 360.0f);

    TPacketCGMove kStatePacket;
    kStatePacket.bFunc = FUNC_SKILL;
    kStatePacket.bArg = uArg;
    kStatePacket.bRot = fDstRot / 5.0f;
    kStatePacket.lX = long(c_rkPPosDst.x);
    kStatePacket.lY = long(c_rkPPosDst.y);
    kStatePacket.dwTime = ELTimer_GetServerMSec();
    kStatePacket.color = color;
    kStatePacket.motionKey = motionKey;
    kStatePacket.loopCount = loopCount;
    kStatePacket.isMovingSkill = isMovingSkill;

    assert(kStatePacket.lX >= 0 && kStatePacket.lX < 204800);

    Send(HEADER_CG_CHARACTER_MOVE, kStatePacket);

    return true;
}

void CPythonNetworkStream::SendChangeSkillColorPacket(uint32_t vnum,
                                                      uint32_t color)
{
    TPacketCGChangeSkillColorPacket p;
    p.vnum = vnum;
    p.color = color;

    Send(HEADER_CG_CHANGE_SKILL_COLOR, p);
}

void CPythonNetworkStream::SendBlockModePacket(uint32_t blockMode)
{
    CgBlockModePacket p;
    p.blockMode = blockMode;
    Send(HEADER_CG_BLOCK_MODE, p);
}

void CPythonNetworkStream::SendSkillGroupSelectPacket(uint8_t skillGroup)
{
    TPacketCGChooseSkillGroup p = {};
    p.skillGroup = skillGroup;
    Send(HEADER_CG_CHOOSE_SKILL_GROUP, p);
}

// NOTE : SlotIndex´Â ÀÓ½Ã
bool CPythonNetworkStream::SendUseSkillPacket(uint32_t dwSkillIndex,
                                              uint32_t dwTargetVID)
{
    TPacketCGUseSkill UseSkillPacket;
    UseSkillPacket.dwVnum = dwSkillIndex;
    UseSkillPacket.dwVID = dwTargetVID;
    Send(HEADER_CG_USE_SKILL, UseSkillPacket);
    return true;
}

bool CPythonNetworkStream::SendChatPacket(const char* c_szChat, uint8_t byType)
{
    if (m_isChatEnable) {
        if (strlen(c_szChat) == 0)
            return true;

        if (strlen(c_szChat) >= 512)
            return true;

        if (c_szChat[0] == '/') {
            if (1 == strlen(c_szChat)) {
                if (!m_strLastCommand.empty())
                    c_szChat = m_strLastCommand.c_str();
            } else {
                m_strLastCommand = c_szChat;
            }
        }

        if (ClientCommand(c_szChat))
            return true;

        TPacketCGChat ChatPacket;
        ChatPacket.type = byType;
        ChatPacket.lang = LocaleService_GetLocaleID();
        ChatPacket.message = c_szChat;
        Send(HEADER_CG_CHAT, ChatPacket);

        return true;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
// Emoticon
void CPythonNetworkStream::RegisterEmoticonString(const char* pcEmoticonString)
{
    if (m_EmoticonStringVector.size() >= EMOTICON_NUM) {
        SPDLOG_ERROR("Can't register emoticon string... vector is full "
                     "(size:{0})",
                     m_EmoticonStringVector.size());
        return;
    }
    m_EmoticonStringVector.push_back(pcEmoticonString);
}

bool CPythonNetworkStream::ParseEmoticon(const char* pChatMsg,
                                         uint32_t* pdwEmoticon)
{
    for (uint32_t dwEmoticonIndex = 0;
         dwEmoticonIndex < m_EmoticonStringVector.size(); ++dwEmoticonIndex) {
        if (strlen(pChatMsg) > m_EmoticonStringVector[dwEmoticonIndex].size())
            continue;

        const char* pcFind =
            strstr(pChatMsg, m_EmoticonStringVector[dwEmoticonIndex].c_str());

        if (pcFind != pChatMsg)
            continue;

        *pdwEmoticon = dwEmoticonIndex;

        return true;
    }

    return false;
}

// Emoticon
//////////////////////////////////////////////////////////////////////////

void CPythonNetworkStream::__ConvertEmpireText(uint32_t dwEmpireID,
                                               char* szText)
{
    if (dwEmpireID < 1 || dwEmpireID > 3)
        return;

    UINT uHanPos;

    STextConvertTable& rkTextConvTable = m_aTextConvTable[dwEmpireID - 1];

    uint8_t* pbText = (uint8_t*)szText;
    while (*pbText) {
        if (*pbText & 0x80) {
            if (pbText[0] >= 0xb0 && pbText[0] <= 0xc8 && pbText[1] >= 0xa1 &&
                pbText[1] <= 0xfe) {
                uHanPos =
                    (pbText[0] - 0xb0) * (0xfe - 0xa1 + 1) + (pbText[1] - 0xa1);
                pbText[0] = rkTextConvTable.aacHan[uHanPos][0];
                pbText[1] = rkTextConvTable.aacHan[uHanPos][1];
            }
            pbText += 2;
        } else {
            if (*pbText >= 'a' && *pbText <= 'z') {
                *pbText = rkTextConvTable.acLower[*pbText - 'a'];
            } else if (*pbText >= 'A' && *pbText <= 'Z') {
                *pbText = rkTextConvTable.acUpper[*pbText - 'A'];
            }
            pbText++;
        }
    }
}

bool CPythonNetworkStream::RecvChatPacket(const TPacketGCChat& kChat)
{

    if (kChat.type >= CHAT_TYPE_MAX_NUM)
        return true;

    if (CHAT_TYPE_COMMAND == kChat.type) {
        ServerCommand(kChat.message.c_str());
        return true;
    }

    auto id = kChat.id;

    std::string line = kChat.message;

    CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

    if (kChat.type == 3 && !rkChrMgr.GetInstancePtr(kChat.id))
        id = 0;

    if (id != 0) {
        CInstanceBase* pkInstChatter = rkChrMgr.GetInstancePtr(kChat.id);
        if (NULL == pkInstChatter)
            return true;

        switch (kChat.type) {
            case CHAT_TYPE_TALKING: /* ±×³É Ã¤ÆÃ */
            case CHAT_TYPE_PARTY:   /* ÆÄÆ¼¸» */
            case CHAT_TYPE_GUILD:   /* ±æµå¸» */
            case CHAT_TYPE_SHOUT:   /* ¿ÜÄ¡±â */
            case CHAT_TYPE_WHISPER: // ¼­¹ö¿Í´Â ¿¬µ¿µÇÁö ¾Ê´Â Only Client Enum
            case CHAT_TYPE_EMPIRE:
            case CHAT_TYPE_TEAM: {
                char* p = nth_strchr(line.c_str(), ':', 2);

                if (p)
                    p += 2;
                else
                    p = const_cast<char*>(line.c_str());

                uint32_t dwEmoticon;

                if (ParseEmoticon(p, &dwEmoticon)) {
                    pkInstChatter->SetEmoticon(dwEmoticon);
                    return true;
                } else {
                    if (!pkInstChatter->IsNPC() && !pkInstChatter->IsEnemy()) {
                        __FilterInsult(p, strlen(p));
                    }

                    line = fmt::format("{}", p);
                }
            } break;
            case CHAT_TYPE_COMMAND: /* ¸í·É */
            case CHAT_TYPE_INFO: /* Á¤º¸ (¾ÆÀÌÅÛÀ» Áý¾ú´Ù, °æÇèÄ¡¸¦ ¾ò¾ú´Ù. µî)
                                  */
            case CHAT_TYPE_NOTICE: /* °øÁö»çÇ× */
            case CHAT_TYPE_BIG_NOTICE:
            case CHAT_TYPE_MAX_NUM:
            default:
                line = kChat.message;
                break;
        }

        if (CHAT_TYPE_SHOUT != kChat.type && CHAT_TYPE_TEAM != kChat.type &&
            CHAT_TYPE_EMPIRE != kChat.type) {
            CPythonTextTail::Instance().RegisterChatTail(kChat.id,
                                                         line.c_str());
        }

        if (pkInstChatter->IsPC())
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_AppendChat", kChat.type,
                                  py::bytes(kChat.message));
    } else {
        if (CHAT_TYPE_NOTICE == kChat.type) {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_SetTipMessage",
                                  py::bytes(kChat.message));
        } else if (CHAT_TYPE_BIG_NOTICE == kChat.type) {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_SetBigMessage",
                                  py::bytes(kChat.message));
        } else if (CHAT_TYPE_BIG_CONTROL == kChat.type) {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_SetBigControlMessage",
                                  py::bytes(kChat.message));
            if (!strcmp(kChat.message.c_str(), " ") ||
                !strcmp(kChat.message.c_str(), "#start") ||
                !strcmp(kChat.message.c_str(), "#end") ||
                !strcmp(kChat.message.c_str(), "#send"))
                return true;
        } else if (CHAT_TYPE_MISSION == kChat.type) {
            if (!kChat.message.empty())
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                      "BINARY_SetMissionMessage",
                                      py::bytes(kChat.message));
            else
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY"
                                                                        "_Clean"
                                                                        "Missio"
                                                                        "nMessa"
                                                                        "ge");
            return true;
        } else if (CHAT_TYPE_SUB_MISSION == kChat.type) {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_SetSubMissionMessage",
                                  py::bytes(kChat.message));
            return true;
        }

        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_AppendChat", (int)kChat.type,
                              py::bytes(kChat.message));
    }
    return true;
}

bool CPythonNetworkStream::RecvWhisperPacket(const TPacketGCWhisper& p)
{
    py::bytes from = p.szNameFrom;
    py::bytes msg = p.message;
    py::bytes whisperLang = GetLanguageNameByID(p.localeCode).value_or("");

    switch (p.bType) {
        case WHISPER_TYPE_NORMAL:
        case WHISPER_TYPE_GM: {
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "OnRecvWhisper", static_cast<int>(p.bType),
                                  whisperLang, from, msg);
        } break;

        case WHISPER_TYPE_SYSTEM:
        case WHISPER_TYPE_ERROR: {
            PyCallClassMemberFunc(
                m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperSystemMessage",
                static_cast<int>(p.bType), whisperLang, from, msg);
        } break;

        default: {
            PyCallClassMemberFunc(
                m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperError",
                static_cast<int>(p.bType), whisperLang, from, msg);
        }

        break;
    }

    return true;
}

bool CPythonNetworkStream::RecvExchangeInfoPacket(const TPacketGCExchageInfo& p)
{

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "OnRecvExchangeInfo", p.iUnixTime, p.bError,
                          py::bytes(p.message));
    return true;
}

bool CPythonNetworkStream::SendCheckCountryFlagForWhisperPacket(
    const char* name)
{
    if (strlen(name) > CHARACTER_NAME_MAX_LEN)
        return true;

    TPacketCGCheckCountryFlagWhisper pPack;
    pPack.szName = name;

    Send(HEADER_CG_CHECK_COUNTRY_WHISPER, pPack);

    return true;
}

bool CPythonNetworkStream::SendWhisperPacket(const char* name,
                                             const char* c_szChat)
{
    if (strlen(c_szChat) >= 255)
        return true;

    TPacketCGWhisper WhisperPacket;
    WhisperPacket.szNameTo = name;
    WhisperPacket.message = c_szChat;
    Send(HEADER_CG_WHISPER, WhisperPacket);

    return true;
}

bool CPythonNetworkStream::RecvGoldChange(const TPacketGCGoldChange& GoldChange)
{

    CInstanceBase* pInstance =
        CPythonCharacterManager::Instance().GetMainInstancePtr();
    if (pInstance) {
        if (GoldChange.dwVID == pInstance->GetVirtualID()) {
            CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
            rkPlayer.SetGold(GoldChange.value);

            if (GoldChange.amount > 0) {
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                      "OnPickMoney", GoldChange.amount);
            }
        }
    }
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus");

    return true;
}

#ifdef ENABLE_GEM_SYSTEM
bool CPythonNetworkStream::RecvGemChange()
{
    TPacketGCGemChange GemChange;

    if (!Recv(sizeof(TPacketGCGemChange), &GemChange)) {
        SPDLOG_DEBUG("Recv Gem Change Packet Error");
        return false;
    }

    CInstanceBase* pInstance =
        CPythonCharacterManager::Instance().GetMainInstancePtr();

    // ÀÚ½ÅÀÇ Point°¡ º¯°æµÇ¾úÀ» °æ¿ì..
    if (pInstance) {
        if (GemChange.dwVID == pInstance->GetVirtualID()) {
            CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
            rkPlayer.SetGem(GemChange.value);

            if (GemChange.amount > 0) {
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                      "OnPickGem", GemChange.amount);
            }
        }
    }
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus");

    return true;
}
#endif

bool CPythonNetworkStream::RecvPointChange(
    const TPacketGCPointChange& PointChange)
{
    CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

    CInstanceBase* ch = rkChrMgr.GetInstancePtr(PointChange.dwVID);
    if (!ch)
        return true;

    rkChrMgr.ShowPointEffect(PointChange.type, PointChange.dwVID);

    // ÀÚ½ÅÀÇ Point°¡ º¯°æµÇ¾úÀ» °æ¿ì..
    if (ch == CPythonCharacterManager::Instance().GetMainInstancePtr()) {
        CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
        rkPlayer.SetStatus(PointChange.type, PointChange.value);

        switch (PointChange.type) {
            case POINT_STAT_RESET_COUNT:
                __RefreshStatus();
                break;
            case POINT_PLAYTIME:
                m_players[m_selectedCharacterSlot].dwPlayMinutes =
                    PointChange.value;
                break;
            case POINT_LEVEL:
                m_players[m_selectedCharacterSlot].byLevel = PointChange.value;
                __RefreshStatus();
                __RefreshSkillWindow();
                break;
            case POINT_ST:
                m_players[m_selectedCharacterSlot].byST = PointChange.value;
                __RefreshStatus();
                __RefreshSkillWindow();
                break;
            case POINT_DX:
                m_players[m_selectedCharacterSlot].byDX = PointChange.value;
                __RefreshStatus();
                __RefreshSkillWindow();
                break;
            case POINT_HT:
                m_players[m_selectedCharacterSlot].byHT = PointChange.value;
                __RefreshStatus();
                __RefreshSkillWindow();
                break;
            case POINT_IQ:
                m_players[m_selectedCharacterSlot].byIQ = PointChange.value;
                __RefreshStatus();
                __RefreshSkillWindow();
                break;
            case POINT_SKILL:
            case POINT_SUB_SKILL:
            case POINT_HORSE_SKILL:
            case POINT_SKILLTREE_POINTS:
                __RefreshSkillWindow();
                break;
            case POINT_ENERGY:
                if (PointChange.value == 0) {
                    rkPlayer.SetStatus(POINT_ENERGY_END_TIME, 0);
                }
                __RefreshStatus();
                break;
            default:
                __RefreshStatus();
                break;
        }
    }

    switch (PointChange.type) {
        case POINT_LEVEL:
            ch->SetLevel(PointChange.value);
    }

    return true;
}

bool CPythonNetworkStream::RecvStunPacket(const TPacketGCStun& p)
{
    CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
    CInstanceBase* pkInstSel = rkChrMgr.GetInstancePtr(p.vid);

    if (pkInstSel) {
        if (CPythonCharacterManager::Instance().GetMainInstancePtr() ==
            pkInstSel)
            pkInstSel->Die();
        else
            pkInstSel->Stun();
    }

    return true;
}

bool CPythonNetworkStream::RecvDeadPacket(const TPacketGCDead& p)
{
    CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
    CInstanceBase* pkChrInstSel = rkChrMgr.GetInstancePtr(p.vid);
    if (pkChrInstSel) {
        CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();
        if (pkInstMain == pkChrInstSel) {
            SPDLOG_DEBUG("ÁÖÀÎ°ø »ç¸Á");
            if (false == pkInstMain->GetDuelMode()) {
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnGame"
                                                                        "Over");
            }
            CPythonPlayer::Instance().NotifyDeadMainCharacter();
        }

        pkChrInstSel->Die();
    }

    return true;
}

bool CPythonNetworkStream::SendOnClickPacket(uint32_t vid)
{
    TPacketCGOnClick OnClickPacket;
    OnClickPacket.vid = vid;

    Send(HEADER_CG_ON_CLICK, OnClickPacket);
    return true;
}

bool CPythonNetworkStream::SendLoadTargetInfo(uint32_t dwVID)
{
    TPacketCGTargetLoad kPacket;
    kPacket.dwVID = dwVID;

    Send(HEADER_CG_TARGET_LOAD, kPacket);
    return true;
}

bool CPythonNetworkStream::RecvMotionPacket(const packet_motion& p)
{
    CInstanceBase* pMainInstance =
        CPythonCharacterManager::Instance().GetInstancePtr(p.vid);
    CInstanceBase* pVictimInstance = NULL;

    if (0 != p.victim_vid)
        pVictimInstance =
            CPythonCharacterManager::Instance().GetInstancePtr(p.victim_vid);

    if (!pMainInstance)
        return false;

    pMainInstance->PushOnceMotion(p.motion);

    return true;
}

bool CPythonNetworkStream::RecvShopPacket(const TPacketGCShop& p)
{

    switch (p.subheader) {
        case SHOP_SUBHEADER_GC_START: {

            if (p.actionStart) {
                CPythonShop::Instance().Clear();

                auto p2 = p.actionStart.value();
                uint32_t dwVID = p2.owner_vid;

                for (uint8_t iItemIndex = 0;
                     iItemIndex < SHOP_HOST_ITEM_MAX_NUM; ++iItemIndex)
                    CPythonShop::Instance().SetItemData(iItemIndex,
                                                        p2.items[iItemIndex]);

                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                      "StartShop", dwVID);
            }

            break;
        }

        case SHOP_SUBHEADER_GC_START_EX: {
            if (p.actionStartEx) {
                auto pShopStartPacket = p.actionStartEx.value();
                CPythonShop::Instance().Clear();

                uint32_t dwVID = pShopStartPacket.owner_vid;
                uint8_t shop_tab_count = pShopStartPacket.shop_tab_count;
            //    SPDLOG_INFO("shop_tab_count %u", shop_tab_count);

                CPythonShop::instance().SetTabCount(shop_tab_count);

                for (size_t i = 0; i < shop_tab_count; i++) {
                    auto pPackTab = pShopStartPacket.tabs[i];

                    CPythonShop::instance().SetTabCoinType(i,
                                                           pPackTab.coin_type);
                    CPythonShop::instance().SetTabCoinVnum(i,
                                                           pPackTab.coin_vnum);

                    CPythonShop::instance().SetTabName(i,
                                                       pPackTab.name.c_str());

                    for (uint8_t j = 0; j < SHOP_HOST_ITEM_MAX_NUM; j++) {
                        auto item = &pPackTab.items[j];
                        CPythonShop::Instance().SetItemData(i, j, *item);
                    }
                }

                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                      "StartShop", dwVID);
            }

            break;
        }
        case SHOP_SUBHEADER_GC_END:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndShop");
            break;

        case SHOP_SUBHEADER_GC_UPDATE_ITEM: {
            if (p.actionUpdate)
                break;

            const auto& pShopUpdateItemPacket = p.actionUpdate;
            CPythonShop::Instance().SetItemData(pShopUpdateItemPacket->pos,
                                                pShopUpdateItemPacket->item);
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSho"
                                                                    "p");
            break;
        }

        case SHOP_SUBHEADER_GC_UPDATE_PRICE:
            if (!p.actionPriceUpdate)
                break;

            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "SetShopSellingPrice",
                                  p.actionPriceUpdate.value());
            break;

        case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "OnShopError", "NOT_ENOUGH_MONEY");
            break;
        case SHOP_SUBHEADER_GC_NOT_ENOUGH_CASH:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "OnShopError", "NOT_ENOUGH_CASH");
            break;

        case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX:
            if (!p.actionNothEnough)
                break;
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "OnShopCoinError",
                                  p.actionNothEnough.value());
            break;

        case SHOP_SUBHEADER_GC_SOLDOUT:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "OnShopError", "SOLDOUT");
            break;

        case SHOP_SUBHEADER_GC_INVENTORY_FULL:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "OnShopError", "INVENTORY_FULL");
            break;

        case SHOP_SUBHEADER_GC_INVALID_POS:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "OnShopError", "INVALID_POS");
            break;

        default:
            SPDLOG_ERROR("CPythonNetworkStream::RecvShopPacket: Unknown "
                         "subheader\n");
            break;
    }

    return true;
}

bool CPythonNetworkStream::RecvExchangePacket(const packet_exchange& p)
{

    switch (p.sub_header) {
        case EXCHANGE_SUBHEADER_GC_START:
            CPythonExchange::Instance().Clear();
            CPythonExchange::Instance().Start();

            {
                CInstanceBase* pCharacterInstance =
                    CPythonCharacterManager::Instance().GetInstancePtr(
                        p.arg.value());

                if (pCharacterInstance) {
                    CPythonExchange::Instance().SetTargetVID(
                        pCharacterInstance->GetVirtualID());
                }
            }

            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartExcha"
                                                                    "nge");
            break;

        case EXCHANGE_SUBHEADER_GC_ITEM_ADD:
            if (p.is_me && p.is_me.value()) {
                int iSlotIndex = p.itemPos.value().cell;
                CPythonExchange::Instance().SetItemToSelf(iSlotIndex,
                                                          p.itemData.value());
            } else {
                int iSlotIndex = p.itemPos.value().cell;
                CPythonExchange::Instance().SetItemToTarget(iSlotIndex,
                                                            p.itemData.value());
            }

            __RefreshExchangeWindow();
            __RefreshInventoryWindow();
            break;

        case EXCHANGE_SUBHEADER_GC_ITEM_DEL:
            if (p.is_me && p.is_me.value()) {
                CPythonExchange::Instance().DelItemOfSelf(
                    p.itemPos.value().cell);
            } else {
                CPythonExchange::Instance().DelItemOfTarget(
                    p.itemPos.value().cell);
            }
            __RefreshExchangeWindow();
            __RefreshInventoryWindow();
            break;

        case EXCHANGE_SUBHEADER_GC_GOLD_ADD:
            if (p.is_me && p.is_me.value())
                CPythonExchange::Instance().SetElkToSelf(p.arg.value());
            else
                CPythonExchange::Instance().SetElkToTarget(p.arg.value());

            __RefreshExchangeWindow();
            break;

        case EXCHANGE_SUBHEADER_GC_ACCEPT:
            if (p.is_me && p.is_me.value()) {
                CPythonExchange::Instance().SetAcceptToSelf(p.arg.value());
            } else {
                CPythonExchange::Instance().SetAcceptToTarget(p.arg.value());
            }
            __RefreshExchangeWindow();
            break;

        case EXCHANGE_SUBHEADER_GC_END:
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndExchang"
                                                                    "e");
            __RefreshInventoryWindow();
            CPythonExchange::Instance().End();
            break;

        case EXCHANGE_SUBHEADER_GC_ALREADY:
            SPDLOG_DEBUG("trade_already");
            break;

        case EXCHANGE_SUBHEADER_GC_LESS_GOLD:
            SPDLOG_DEBUG("trade_less_elk");
            break;
    };

    return true;
}

bool CPythonNetworkStream::RecvQuestInfoPacket(const packet_quest_info& p)
{

    const uint8_t& c_rFlag = p.flag;

    enum {
        QUEST_PACKET_TYPE_NONE,
        QUEST_PACKET_TYPE_BEGIN,
        QUEST_PACKET_TYPE_UPDATE,
        QUEST_PACKET_TYPE_END,
    };

    uint8_t byQuestPacketType = QUEST_PACKET_TYPE_NONE;

    if (0 != (c_rFlag & QUEST_SEND_ISBEGIN) && p.isBegin) {
        uint8_t isBegin = p.isBegin.value();

        if (isBegin)
            byQuestPacketType = QUEST_PACKET_TYPE_BEGIN;
        else
            byQuestPacketType = QUEST_PACKET_TYPE_END;
    } else {
        byQuestPacketType = QUEST_PACKET_TYPE_UPDATE;
    }

    CPythonQuest& rkQuest = CPythonQuest::Instance();

    // Process Start
    if (QUEST_PACKET_TYPE_END == byQuestPacketType) {
        rkQuest.DeleteQuestInstance(p.index);
    } else if (QUEST_PACKET_TYPE_UPDATE == byQuestPacketType) {
        if (!rkQuest.IsQuest(p.index)) {
            rkQuest.MakeQuest(p.index, p.c_index);
        }

        if (p.title)
            rkQuest.SetQuestTitle(p.index, p.title->c_str());
        if (p.clockName)
            rkQuest.SetQuestClockName(p.index, p.clockName->c_str());
        if (p.counterName)
            rkQuest.SetQuestCounterName(p.index, p.counterName->c_str());
        if (p.iconFile)
            rkQuest.SetQuestIconFileName(p.index, p.iconFile->c_str());

        if (c_rFlag & QUEST_SEND_CLOCK_VALUE && p.clockValue)
            rkQuest.SetQuestClockValue(p.index, p.clockValue.value());
        if (c_rFlag & QUEST_SEND_COUNTER_VALUE && p.counterValue)
            rkQuest.SetQuestCounterValue(p.index, p.counterValue.value());
    } else if (QUEST_PACKET_TYPE_BEGIN == byQuestPacketType) {
        CPythonQuest::SQuestInstance QuestInstance;
        QuestInstance.dwIndex = p.index;
        QuestInstance.strTitle = p.title.value_or("");
        QuestInstance.strClockName = p.clockName.value_or("");
        QuestInstance.iClockValue = p.clockValue.value_or(0);
        QuestInstance.strCounterName = p.counterName.value_or("");
        QuestInstance.iCounterValue = p.counterValue.value_or(0);
        QuestInstance.strIconFileName = p.iconFile.value_or("");
        rkQuest.RegisterQuestInstance(QuestInstance);
    }
    // Process Start End

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshQuest");
    return true;
}

bool CPythonNetworkStream::RecvQuestConfirmPacket(
    const TPacketGCQuestConfirm& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_OnQuestConfirm", py::bytes(p.msg), p.timeout,
                          p.requestPID);
    return true;
}

bool CPythonNetworkStream::RecvRequestMakeGuild(const BlankPacket& p)
{

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AskGuildName");

    return true;
}

void CPythonNetworkStream::ToggleGameDebugInfo()
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ToggleDebugInfo");
}

bool CPythonNetworkStream::SendExchangeStartPacket(uint32_t vid)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGExchange packet = {};
    packet.sub_header = EXCHANGE_SUBHEADER_CG_START;
    packet.arg1 = vid;

    Send(HEADER_CG_EXCHANGE, packet);
    return true;
}

bool CPythonNetworkStream::SendExchangeElkAddPacket(Gold elk)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGExchange packet = {};
    packet.sub_header = EXCHANGE_SUBHEADER_CG_ELK_SET;
    packet.arg1 = elk;

    Send(HEADER_CG_EXCHANGE, packet);
    return true;
}

bool CPythonNetworkStream::SendExchangeItemAddPacket(TItemPos ItemPos,
                                                     uint8_t byDisplayPos)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGExchange packet = {};
    packet.sub_header = EXCHANGE_SUBHEADER_CG_ITEM_ADD;
    packet.Pos = ItemPos;
    packet.arg2 = byDisplayPos;

    Send(HEADER_CG_EXCHANGE, packet);

    return true;
}

bool CPythonNetworkStream::SendExchangeItemDelPacket(uint8_t pos)
{
    // assert(!"Can't be called function -
    // CPythonNetworkStream::SendExchangeItemDelPacket"); return true;

    if (!__CanActMainInstance())
        return true;

    TPacketCGExchange packet = {};
    packet.sub_header = EXCHANGE_SUBHEADER_CG_ITEM_DEL;
    packet.arg1 = pos;

    Send(HEADER_CG_EXCHANGE, packet);

    return true;
}

bool CPythonNetworkStream::SendExchangeAcceptPacket()
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGExchange packet = {};
    packet.sub_header = EXCHANGE_SUBHEADER_CG_ACCEPT;

    Send(HEADER_CG_EXCHANGE, packet);
    return true;
}

bool CPythonNetworkStream::SendExchangeExitPacket()
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGExchange packet = {};
    packet.sub_header = EXCHANGE_SUBHEADER_CG_CANCEL;

    Send(HEADER_CG_EXCHANGE, packet);

    return true;
}

// PointReset °³ÀÓ½Ã
bool CPythonNetworkStream::SendPointResetPacket()
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartPointReset");
    return true;
}

bool CPythonNetworkStream::__IsPlayerAttacking()
{
    CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
    CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();
    if (!pkInstMain)
        return false;

    if (!pkInstMain->IsAttacking())
        return false;

    return true;
}

bool CPythonNetworkStream::RecvScriptPacket(const packet_script& ScriptPacket)
{
    //레터 퀘스트와 NPC 대화 구분
    bool bIsQuestInfo = false;
    if (ScriptPacket.quest_flag == 1)
        bIsQuestInfo = true;
    else
        bIsQuestInfo = false;

    int iIndex = CPythonEventManager::Instance().RegisterEventSetFromString(
        ScriptPacket.script, bIsQuestInfo);
    if (-1 != iIndex) {
        CPythonEventManager::Instance().SetVisibleLineCount(iIndex, 30);

        SPDLOG_DEBUG("RecvScriptPacket {0}", ScriptPacket.script);

        if (ScriptPacket.quest_flag == 1)
            OnQuestScriptEventStart(ScriptPacket.skin, iIndex);
        else
            OnScriptEventStart(ScriptPacket.skin, iIndex);
    }

    return true;
}

bool CPythonNetworkStream::SendScriptAnswerPacket(int iAnswer, int qIndex)
{
    TPacketCGScriptAnswer p;
    p.answer = (uint8_t)iAnswer;
    p.qIndex = qIndex;
    Send(HEADER_CG_SCRIPT_ANSWER, p);
    return true;
}

bool CPythonNetworkStream::SendScriptButtonPacket(unsigned int iIndex)
{
    TPacketCGScriptButton p;
    p.idx = iIndex;
    Send(HEADER_CG_SCRIPT_BUTTON, p);

    return true;
}

bool CPythonNetworkStream::SendAnswerMakeGuildPacket(const char* c_szName)
{
    TPacketCGAnswerMakeGuild p;
    p.guild_name = c_szName;
    Send(HEADER_CG_ANSWER_MAKE_GUILD, p);
    return true;
}

bool CPythonNetworkStream::SendQuestInputStringPacket(const char* c_szString,
                                                      int32_t qIndex)
{
    TPacketCGQuestInputString p;
    p.msg = c_szString;
    p.qIndex = qIndex;
    Send(HEADER_CG_QUEST_INPUT_STRING, p);

    return true;
}

bool CPythonNetworkStream::SendQuestInputStringLongPacket(
    const char* c_szString, int32_t qIndex)
{
    TPacketCGQuestInputString p;
    p.msg = c_szString;
    p.qIndex = qIndex;
    Send(HEADER_CG_QUEST_INPUT_STRING_LONG, p);
    return true;
}

bool CPythonNetworkStream::SendQuestCommandPacket(const char* c_szCmd,
                                                  uint32_t quest_index)
{
    TPacketCGQuestRcv p;
    p.questID = quest_index;
    p.msg = c_szCmd;
    Send(HEADER_CG_QUEST_RECEIVE, p);
    return true;
}

bool CPythonNetworkStream::SendQuestConfirmPacket(uint8_t byAnswer,
                                                  uint32_t dwPID)
{
    TPacketCGQuestConfirm p;
    p.answer = byAnswer;
    p.requestPID = dwPID;
    Send(HEADER_CG_QUEST_CONFIRM, p);

    return true;
}

bool CPythonNetworkStream::RecvSkillLevelNew(const TPacketGCSkillLevel& p)
{

    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();

    rkPlayer.SetSkill(7, 0);
    rkPlayer.SetSkill(8, 0);

    for (int i = 0; i < SKILL_MAX_NUM; ++i) {
        const TPlayerSkill& rPlayerSkill = p.skills[i];

        if (i >= 112 && i <= 115 && rPlayerSkill.bLevel)
            rkPlayer.SetSkill(7, i);

        if (i >= 116 && i <= 119 && rPlayerSkill.bLevel)
            rkPlayer.SetSkill(8, i);

#ifdef ENABLE_678TH_SKILL
        if (i >= 221 && i <= 229 && rPlayerSkill.bLevel)
            rkPlayer.SetSkill(7, i);

        if (i >= 236 && i <= 244 && rPlayerSkill.bLevel)
            rkPlayer.SetSkill(8, i);
#endif

        rkPlayer.SetSkillInfo(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel,
                              rPlayerSkill.color);
    }

    __RefreshSkillWindow();
    __RefreshStatus();
    // SPDLOG_DEBUG(" >> RecvSkillLevelNew\n");
    return true;
}

bool CPythonNetworkStream::RecvDamageInfoPacket(const TPacketGCDamageInfo& p)
{

    auto& chr = CPythonCharacterManager::Instance();

    CInstanceBase* pInstTarget = chr.GetInstancePtr(p.dwVID);
    bool bSelf = pInstTarget == chr.GetMainInstancePtr();
    bool bTarget = pInstTarget == m_pInstTarget;

    if (pInstTarget) {
        if (p.damage >= 0)
            pInstTarget->AddDamageEffect(p.damage, p.flag, bSelf, bTarget);
       // else
     //       SPDLOG_ERROR("Damage is equal or below 0.");
    }

    return true;
}

bool CPythonNetworkStream::RecvTargetPacket(const TPacketGCTarget& p)
{

    CInstanceBase* pInstPlayer =
        CPythonCharacterManager::Instance().GetMainInstancePtr();
    CInstanceBase* pInstTarget =
        CPythonCharacterManager::Instance().GetInstancePtr(p.dwVID);
    if (pInstPlayer && pInstTarget) {
        if (!pInstTarget->IsDead()) {
            if (!pInstTarget->IsShop()) {
                if (pInstTarget->IsPC() || pInstTarget->IsBuilding())
                    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                          "CloseTargetBoardIfDifferent",
                                          p.dwVID);
                else if (pInstPlayer->CanViewTargetHP(*pInstTarget)) {
                    try {
                        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                              "SetHPTargetBoard", p.dwVID,
                                              p.lHP, p.lMaxHP, p.prevHp);
                    } catch (py::error_already_set& e) {
                        e.restore();
                    }
                }

                else
                    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                          "CloseTargetBoard");
            }

            m_pInstTarget = pInstTarget;
        }
    } else {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoa"
                                                                "rd");
    }

    return true;
}

bool CPythonNetworkStream::RecvTargetInfoPacket(const TPacketGCTargetInfo& p)
{
    for (const auto& c_rDropInfo : p.drops) {
        PyCallClassMemberFunc(
            m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddDropInfo", p.dwVNum,
            c_rDropInfo.dwVNum, c_rDropInfo.count, c_rDropInfo.iMinLevel,
            c_rDropInfo.iMaxLevel, c_rDropInfo.rarity);
    }

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_"
                                                            "RefreshDropInfo");

    return true;
}

bool CPythonNetworkStream::RecvMountPacket(const GcMountPacket& p)
{
    CInstanceBase* ch =
        CPythonCharacterManager::Instance().GetInstancePtr(p.vid);
    if (!ch) {
        SPDLOG_ERROR("Failed to find VID {0}", p.vid);
        return true;
    }

    ch->Warp(p.x, p.y);

    if (0 != p.vnum)
        ch->MountHorse(p.vnum);
    else
        ch->DismountHorse();

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Recv

bool CPythonNetworkStream::SendAttackPacket(UINT uMotAttack,
                                            uint32_t dwVIDVictim,
                                            uint32_t motionKey, uint32_t x,
                                            uint32_t y, const Vector3& pushDest, const Vector3& victimPos)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGAttack kPacketAtk;
    kPacketAtk.bType = uMotAttack;
    kPacketAtk.dwVictimVID = dwVIDVictim;
    kPacketAtk.attackTime = ELTimer_GetServerMSec();
    kPacketAtk.motionKey = motionKey;
    kPacketAtk.x = x;
    kPacketAtk.y = y;
    kPacketAtk.pushX = pushDest.x;
    kPacketAtk.pushY = pushDest.y;
    kPacketAtk.victimX = victimPos.x;
    kPacketAtk.victimY = victimPos.y;

    Send(HEADER_CG_ATTACK, kPacketAtk);
    return true;
}

bool CPythonNetworkStream::RecvAddFlyTargetingPacket(
    const TPacketGCFlyTargeting& p)
{

    SPDLOG_DEBUG("VID [{0}]°¡ Å¸°ÙÀ» Ãß°¡ ¼³Á¤\n", p.dwShooterVID);

    CPythonCharacterManager& rpcm = CPythonCharacterManager::Instance();

    CInstanceBase* pShooter = rpcm.GetInstancePtr(p.dwShooterVID);

    if (!pShooter) {
#ifndef _DEBUG
        SPDLOG_ERROR("CPythonNetworkStream::RecvFlyTargetingPacket() - "
                     "dwShooterVID[{0}] NOT EXIST",
                     p.dwShooterVID);
#endif
        return true;
    }

    CInstanceBase* pTarget = rpcm.GetInstancePtr(p.dwTargetVID);

    if (p.dwTargetVID && pTarget) {
        pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(
            pTarget->GetGraphicThingInstancePtr());
    } else {
        float h = CPythonBackground::Instance().GetHeight(p.x, p.y) +
                  60.0f; // TEMPORARY HEIGHT
        pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(
            Vector3(p.x, p.y, h));
        // pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(kPacket.kPPosTarget.x,kPacket.kPPosTarget.y,);
    }

    return true;
}

bool CPythonNetworkStream::RecvFlyTargetingPacket(
    const TPacketGCFlyTargeting& p)
{
    CPythonCharacterManager& rpcm = CPythonCharacterManager::Instance();

    CInstanceBase* pShooter = rpcm.GetInstancePtr(p.dwShooterVID);

    if (!pShooter) {
#ifdef _DEBUG
        SPDLOG_ERROR("CPythonNetworkStream::RecvFlyTargetingPacket() - "
                     "dwShooterVID[{0}] NOT EXIST",
                     p.dwShooterVID);
#endif
        return true;
    }

    CInstanceBase* pTarget = rpcm.GetInstancePtr(p.dwTargetVID);

    if (p.dwTargetVID && pTarget) {
        pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(
            pTarget->GetGraphicThingInstancePtr());
    } else {
        float h = CPythonBackground::Instance().GetHeight(p.x, p.y) +
                  60.0f; // TEMPORARY HEIGHT
        pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(
            Vector3(p.x, p.y, h));
        // pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(kPacket.kPPosTarget.x,kPacket.kPPosTarget.y,);
    }

    return true;
}

bool CPythonNetworkStream::SendShootPacket(uint32_t motionKey, uint32_t skill)
{
    TPacketCGShoot pack;
    pack.type = skill;
    pack.motionKey = motionKey;

    Send(HEADER_CG_SHOOT, pack);
    return true;
}

bool CPythonNetworkStream::SendAddFlyTargetingPacket(
    uint32_t dwTargetVID, const TPixelPosition& kPPosTarget)
{
    TPacketCGFlyTargeting p;
    p.dwTargetVID = dwTargetVID;
    p.x = kPPosTarget.x;
    p.y = kPPosTarget.y;
    Send(HEADER_CG_ADD_FLY_TARGETING, p);
    return true;
}

bool CPythonNetworkStream::SendFlyTargetingPacket(
    uint32_t dwTargetVID, const TPixelPosition& kPPosTarget)
{
    TPacketCGFlyTargeting p;
    p.dwTargetVID = dwTargetVID;
    p.x = kPPosTarget.x;
    p.y = kPPosTarget.y;
    Send(HEADER_CG_FLY_TARGETING, p);
    return true;
}

bool CPythonNetworkStream::RecvCreateFlyPacket(const TPacketGCCreateFly& p)
{
    CFlyingManager& rkFlyMgr = CFlyingManager::Instance();
    CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

    CInstanceBase* pkStartInst = rkChrMgr.GetInstancePtr(p.dwStartVID);
    CInstanceBase* pkEndInst = rkChrMgr.GetInstancePtr(p.dwEndVID);
    if (!pkStartInst || !pkEndInst)
        return true;

    rkFlyMgr.CreateIndexedFly(p.bType,
                              pkStartInst->GetGraphicThingInstancePtr(),
                              pkEndInst->GetGraphicThingInstancePtr());

    return true;
}

bool CPythonNetworkStream::SendTargetPacket(uint32_t dwVID)
{
    TPacketCGTarget p;
    p.dwVID = dwVID;
    Send(HEADER_CG_TARGET, p);
    return true;
}

bool CPythonNetworkStream::RecvMessenger(const TPacketGCMessenger& p)
{

    switch (p.subheader) {
        case MESSENGER_SUBHEADER_GC_LIST: {
            if (!p.list)
                return true;
            for (const auto& on : p.list.value()) {
                if (on.connected)
                    CPythonMessenger::Instance().OnFriendLogin(on.name.c_str());
                else
                    CPythonMessenger::Instance().OnFriendLogout(
                        on.name.c_str());
            }

            break;
        }
        case MESSENGER_SUBHEADER_GC_BLOCK_LIST: {
            if (!p.listBlocked)
                return true;
            for (const auto& pc : p.listBlocked.value()) {
                if (pc.mode)
                    CPythonMessenger::Instance().AppendBlock(pc);
                else
                    CPythonMessenger::Instance().OnFriendLogout(pc.name);
            }
            break;
        }

        case MESSENGER_SUBHEADER_GC_GM_LIST: {
            if (!p.list)
                return true;
            for (const auto& on : p.list.value()) {
                if (on.connected)
                    CPythonMessenger::Instance().OnGamemasterLogin(
                        on.name.c_str(), on.lang);
                else
                    CPythonMessenger::Instance().OnGamemasterLogout(
                        on.name.c_str(), on.lang);
            }
            break;
        }

        case MESSENGER_SUBHEADER_GC_GM_LOGIN: {
            CPythonMessenger::Instance().OnGamemasterLogin(
                p.name.value().c_str(), p.lang.value());
            __RefreshTargetBoardByName(p.name.value().c_str());
            break;
        }

        case MESSENGER_SUBHEADER_GC_GM_LOGOUT: {
            CPythonMessenger::Instance().OnGamemasterLogout(
                p.name.value().c_str(), p.lang.value());
            break;
        }
        case MESSENGER_SUBHEADER_GC_LOGIN: {
            CPythonMessenger::Instance().OnFriendLogin(p.name.value().c_str());
            __RefreshTargetBoardByName(p.name.value().c_str());
            break;
        }

        case MESSENGER_SUBHEADER_GC_LOGOUT: {
            CPythonMessenger::Instance().OnFriendLogout(p.name.value().c_str());
            break;
        }

        case MESSENGER_SUBHEADER_GC_REMOVE_FRIEND: {
            CPythonMessenger::Instance().RemoveFriend(p.name.value().c_str());
            break;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// Party

bool CPythonNetworkStream::SendPartyInvitePacket(uint32_t dwVID)
{
    TPacketCGPartyInvite p;
    p.vid = dwVID;
    Send(HEADER_CG_PARTY_INVITE, p);
    return true;
}

bool CPythonNetworkStream::SendPartyInviteAnswerPacket(uint32_t dwLeaderVID,
                                                       uint8_t byAnswer)
{
    TPacketCGPartyInviteAnswer p;
    p.leader_vid = dwLeaderVID;
    p.accept = byAnswer;
    Send(HEADER_CG_PARTY_INVITE_ANSWER, p);

    return true;
}

bool CPythonNetworkStream::SendPartyRemovePacket(uint32_t dwPID)
{
    TPacketCGPartyRemove p;
    p.pid = dwPID;
    Send(HEADER_CG_PARTY_REMOVE, p);
    return true;
}

bool CPythonNetworkStream::SendPartySetStatePacket(uint32_t dwVID,
                                                   uint8_t byState,
                                                   uint8_t byFlag)
{
    TPacketCGPartySetState p;
    p.pid = dwVID;
    p.byRole = byState;
    p.flag = byFlag;
    Send(HEADER_CG_PARTY_SET_STATE, p);
    return true;
}

bool CPythonNetworkStream::SendPartyUseSkillPacket(uint8_t bySkillIndex,
                                                   uint32_t dwVID)
{
    TPacketCGPartyUseSkill p;
    p.bySkillIndex = bySkillIndex;
    p.vid = dwVID;
    Send(HEADER_CG_PARTY_USE_SKILL, p);
    return true;
}

bool CPythonNetworkStream::SendPartyParameterPacket(uint8_t byDistributeMode)
{
    TPacketCGPartyParameter p;
    p.bDistributeMode = byDistributeMode;
    Send(HEADER_CG_PARTY_PARAMETER, p);
    return true;
}

bool CPythonNetworkStream::RecvPartyInvite(const TPacketGCPartyInvite& p)
{
    CInstanceBase* pInstance =
        CPythonCharacterManager::Instance().GetInstancePtr(p.vid);
    if (!pInstance) {
        SPDLOG_ERROR(" CPythonNetworkStream::RecvPartyInvite - Failed to find "
                     "leader instance [{0}]\n",
                     p.vid);
        return true;
    }

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "RecvPartyInviteQuestion", p.vid,
                          py::bytes(pInstance->GetNameString()));
    SPDLOG_DEBUG(" >> RecvPartyInvite : {0}, {1}", p.vid,
                 pInstance->GetNameString());

    return true;
}

bool CPythonNetworkStream::RecvPartyAdd(
    const TPacketGCPartyAdd& kPartyAddPacket)
{
    if (kPartyAddPacket.name != CPythonPlayer::Instance().GetName())
        CPythonMiniMap::Instance().AddPartyMember(kPartyAddPacket.pid,
                                                  kPartyAddPacket.name.c_str());

    CPythonPlayer::Instance().AppendPartyMember(kPartyAddPacket.pid,
                                                kPartyAddPacket.name.c_str());
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddPartyMember",
                          kPartyAddPacket.pid, py::bytes(kPartyAddPacket.name),
                          kPartyAddPacket.mapIndex, kPartyAddPacket.channel,
                          kPartyAddPacket.race);
#else
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddPartyMember",
                          kPartyAddPacket.pid, kPartyAddPacket.name);
#endif
    SPDLOG_DEBUG(" >> RecvPartyAdd : {0}, {1}", kPartyAddPacket.pid,
                 kPartyAddPacket.name);

    return true;
}

bool CPythonNetworkStream::RecvPartyPositionInfo(
    const TPacketGCPartyPositionInfo& p)
{
    CPythonMiniMap::Instance().MovePartyMember(p.pid, p.x, p.y, p.rot);
    return true;
}

bool CPythonNetworkStream::RecvPartyUpdate(const TPacketGCPartyUpdate& p)
{
    CPythonPlayer::TPartyMemberInfo* pPartyMemberInfo;
    if (!CPythonPlayer::Instance().GetPartyMemberPtr(p.pid, &pPartyMemberInfo))
        return true;

    uint8_t byOldState = pPartyMemberInfo->byState;

    CPythonPlayer::Instance().UpdatePartyMemberInfo(p.pid, p.role,
                                                    p.percent_hp, p.is_leader, p.race);
    for (int i = 0; i < 7; ++i) {
        CPythonPlayer::Instance().UpdatePartyMemberAffect(p.pid, i,
                                                          p.affects[i]);
    }

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "UpdatePartyMemberInfo", p.pid);

    uint32_t dwVID;
    if (CPythonPlayer::Instance().PartyMemberPIDToVID(p.pid, &dwVID))
        if (byOldState != p.role) {
            __RefreshTargetBoardByVID(dwVID);
        }

    return true;
}

bool CPythonNetworkStream::RecvPartyRemove(const TPacketGCPartyRemove& p)
{
    CPythonMiniMap::Instance().RemovePartyMember(p.pid);

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RemovePartyMember",
                          p.pid);
    SPDLOG_DEBUG(" >> RecvPartyRemove : {}\n", p.pid);

    return true;
}

bool CPythonNetworkStream::RecvPartyLink(const TPacketGCPartyLink& p)
{

    CPythonPlayer::Instance().LinkPartyMember(p.pid, p.vid);

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "LinkPartyMember",
                          p.pid, p.vid, p.mapIndex, p.channel, p.race);
#else
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "LinkPartyMember",
                          kPartyLinkPacket.pid, kPartyLinkPacket.vid);

#endif
    SPDLOG_DEBUG(" >> RecvPartyLink : %d, %d\n", p.pid, p.vid);

    if (!CPythonPlayer::Instance().IsMainCharacterIndex(p.vid)) {
        CPythonPlayer::TPartyMemberInfo* pPartyMemberInfo;
        if (!CPythonPlayer::Instance().GetPartyMemberPtr(p.pid,
                                                         &pPartyMemberInfo))
            return true;

        CPythonMiniMap::Instance().AddPartyMember(
            p.pid, pPartyMemberInfo->strName.c_str());
    }

    return true;
}

bool CPythonNetworkStream::RecvPartyUnlink(const TPacketGCPartyUnlink& p)
{
    CPythonPlayer::Instance().UnlinkPartyMember(p.pid);

    if (CPythonPlayer::Instance().IsMainCharacterIndex(p.vid)) {
        CPythonMiniMap::Instance().ClearPartyMember();
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkAllParty"
                                                                "Member");
    } else {
        CPythonMiniMap::Instance().RemovePartyMember(p.pid);
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "UnlinkPartyMember", p.pid);
    }

    SPDLOG_DEBUG(" >> RecvPartyUnlink : %d, %d\n", p.pid, p.vid);

    return true;
}

bool CPythonNetworkStream::RecvPartyParameter(const TPacketGCPartyParameter& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "ChangePartyParameter", p.bDistributeMode);
    SPDLOG_DEBUG(" >> RecvPartyParameter : %d\n", p.bDistributeMode);

    return true;
}

// Party
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Guild

bool CPythonNetworkStream::SendGuildAddMemberPacket(uint32_t dwVID)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_ADD_MEMBER;
    p.vid = dwVID;
    Send(HEADER_CG_GUILD, p);
    return true;
}

bool CPythonNetworkStream::SendGuildRemoveMemberPacket(const std::string& name)
{
    CPythonGuild::TGuildMemberData* pGuildMemberData;
    if (!CPythonGuild::Instance().GetMemberDataPtrByName(name.c_str(),
                                                         &pGuildMemberData)) {
        return false;
    }

    return gPythonNetworkStream->SendGuildRemoveMemberPacket(
        pGuildMemberData->dwPID);
}

bool CPythonNetworkStream::SendGuildRemoveMemberPacket(uint32_t dwPID)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_REMOVE_MEMBER;
    p.pid = dwPID;
    Send(HEADER_CG_GUILD, p);
    return true;
}

bool CPythonNetworkStream::SendGuildChangeGradeNamePacket(uint8_t byGradeNumber,
                                                          const char* c_szName)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME;
    GuildRenameGrade grg;
    grg.index = byGradeNumber;
    grg.name = c_szName;
    p.changeGradeName = grg;
    Send(HEADER_CG_GUILD, p);
    return true;
}

bool CPythonNetworkStream::SendGuildChangeGradeAuthorityPacket(
    uint8_t gradeNumber, uint64_t authority)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY;
    GuildChangeAuthority gra;
    gra.index = gradeNumber;
    gra.auth = authority;
    p.changeGradeAuth = gra;
    Send(HEADER_CG_GUILD, p);

    return true;
}

bool CPythonNetworkStream::SendGuildOfferPacket(PointValue exp)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_OFFER;
    p.expOffer = exp;
    Send(HEADER_CG_GUILD, p);
    return true;
}

bool CPythonNetworkStream::SendGuildPostCommentPacket(const char* c_szMessage)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_POST_COMMENT;
    p.comment.emplace(c_szMessage);
    Send(HEADER_CG_GUILD, p);
    return true;
}

bool CPythonNetworkStream::SendGuildDeleteCommentPacket(uint32_t dwIndex)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_DELETE_COMMENT;
    p.commentIndex = dwIndex;
    Send(HEADER_CG_GUILD, p);

    return true;
}

bool CPythonNetworkStream::SendGuildRefreshCommentsPacket(
    uint32_t dwHighestIndex)
{
    static uint32_t s_LastTime = ELTimer_GetMSec() - 1001;

    if (ELTimer_GetMSec() - s_LastTime < 1000)
        return true;
    s_LastTime = ELTimer_GetMSec();

    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_REFRESH_COMMENT;
    Send(HEADER_CG_GUILD, p);

    return true;
}

bool CPythonNetworkStream::SendGuildChangeMemberGradePacket(uint32_t dwPID,
                                                            uint8_t byGrade)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE;

    GuildChangeMemberGrade gcmg;
    gcmg.pid = dwPID;
    gcmg.grade = byGrade;
    p.changeMemberGrade = gcmg;
    Send(HEADER_CG_GUILD, p);

    return true;
}

bool CPythonNetworkStream::SendGuildUseSkillPacket(uint32_t dwSkillID,
                                                   uint32_t dwTargetVID)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_USE_SKILL;

    GuildUseSkill gus;
    gus.skillIndex = dwSkillID;
    gus.targetVid = dwTargetVID;
    p.useSkill = gus;
    Send(HEADER_CG_GUILD, p);

    return true;
}

bool CPythonNetworkStream::SendGuildChangeMemberGeneralPacket(uint32_t dwPID,
                                                              uint8_t byFlag)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL;

    GuildChangeMemberGeneral gcg;
    gcg.pid = dwPID;
    gcg.flag = byFlag;
    p.changeGeneral = gcg;
    Send(HEADER_CG_GUILD, p);

    return true;
}

bool CPythonNetworkStream::SendGuildInviteAnswerPacket(uint32_t dwGuildID,
                                                       uint8_t byAnswer)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER;
    GuildInviteAnswer gia;
    gia.gid = dwGuildID;
    gia.flag = byAnswer;
    p.inviteAnswer = gia;
    Send(HEADER_CG_GUILD, p);

    SPDLOG_DEBUG(" SendGuildInviteAnswerPacket %d, %d\n", dwGuildID, byAnswer);
    return true;
}

bool CPythonNetworkStream::SendGuildChargeGSPPacket(uint32_t dwMoney)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_CHARGE_GSP;
    p.money = dwMoney;
    Send(HEADER_CG_GUILD, p);

    return true;
}

bool CPythonNetworkStream::SendGuildDepositMoneyPacket(Gold dwMoney)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_DEPOSIT_MONEY;
    p.money = dwMoney;
    Send(HEADER_CG_GUILD, p);
    return true;
}

bool CPythonNetworkStream::SendGuildWithdrawMoneyPacket(Gold dwMoney)
{
    TPacketCGGuild p;
    p.subheader = GUILD_SUBHEADER_CG_WITHDRAW_MONEY;
    p.money = dwMoney;
    Send(HEADER_CG_GUILD, p);
    return true;
}

bool CPythonNetworkStream::RecvGuild(const TPacketGCGuild& p)
{
    switch (p.subheader) {
        case GUILD_SUBHEADER_GC_LOGIN: {
            if (!p.pid)
                return true;
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_LOGIN");

            uint32_t dwPID = p.pid.value();

            // Messenger
            CPythonGuild::TGuildMemberData* pGuildMemberData;
            if (CPythonGuild::Instance().GetMemberDataPtrByPID(
                    dwPID, &pGuildMemberData))
                if (0 != pGuildMemberData->strName.compare(
                             CPythonPlayer::Instance().GetName()))
                    CPythonMessenger::Instance().LoginGuildMember(
                        pGuildMemberData->strName.c_str());

            SPDLOG_TRACE(" <Login> {}", dwPID);
            break;
        }
        case GUILD_SUBHEADER_GC_LOGOUT: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_LOGOUT");
            if (!p.pid)
                return true;
            uint32_t dwPID = p.pid.value();

            // Messenger
            CPythonGuild::TGuildMemberData* pGuildMemberData;
            if (CPythonGuild::Instance().GetMemberDataPtrByPID(
                    dwPID, &pGuildMemberData))
                if (0 != pGuildMemberData->strName.compare(
                             CPythonPlayer::Instance().GetName()))
                    CPythonMessenger::Instance().LogoutGuildMember(
                        pGuildMemberData->strName.c_str());

            SPDLOG_TRACE(" <Logout> {}", dwPID);

            break;
        }
        case GUILD_SUBHEADER_GC_REMOVE: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_REMOVE");
            if (!p.pid)
                return true;
            uint32_t dwPID = p.pid.value();

            // Main Player 일 경우 DeleteGuild
            if (CPythonGuild::Instance().IsMainPlayer(dwPID)) {
                CPythonGuild::Instance().Destroy();
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Delete"
                                                                        "Guil"
                                                                        "d");
                CPythonMessenger::Instance().RemoveAllGuildMember();
                __SetGuildID(0);
                __RefreshMessengerWindow();
                __RefreshTargetBoard();
                __RefreshCharacterWindow();
            } else {
                // Get Member Name
                std::string strMemberName = "";
                CPythonGuild::TGuildMemberData* pData;
                if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID,
                                                                   &pData)) {
                    strMemberName = pData->strName;
                    CPythonMessenger::Instance().RemoveGuildMember(
                        pData->strName.c_str());
                }

                CPythonGuild::Instance().RemoveMember(dwPID);

                // Refresh
                __RefreshTargetBoardByName(strMemberName.c_str());
                __RefreshGuildWindowMemberPage();
            }

            SPDLOG_TRACE(" <Remove> {0}", dwPID);
            break;
        }
        case GUILD_SUBHEADER_GC_LIST: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_LIST");

            if (!p.submembers) {
                break;
            }

            for (const auto& memberPacket : p.submembers.value()) {

                CPythonGuild::SGuildMemberData GuildMemberData;
                GuildMemberData.dwPID = memberPacket.pid;
                GuildMemberData.byGrade = memberPacket.byGrade;
                GuildMemberData.strName = memberPacket.name;
                GuildMemberData.byJob = memberPacket.byJob;
                GuildMemberData.byLevel = memberPacket.byLevel;
                GuildMemberData.dwOffer = memberPacket.dwOffer;
                GuildMemberData.byGeneralFlag = memberPacket.byIsGeneral;
                CPythonGuild::Instance().RegisterMember(GuildMemberData);

                // Messenger
                if (memberPacket.name != CPythonPlayer::Instance().GetName())
                    CPythonMessenger::Instance().AppendGuildMember(
                        memberPacket.name.c_str());

                __RefreshTargetBoardByName(memberPacket.name.c_str());
            }

            __RefreshGuildWindowInfoPage();
            __RefreshGuildWindowMemberPage();
            __RefreshMessengerWindow();
            __RefreshCharacterWindow();
            break;
        }
        case GUILD_SUBHEADER_GC_GRADE: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_GRADE");
            if (p.grades) {
                for (const auto& elem : p.grades.value()) {
                    CPythonGuild::Instance().SetGradeData(elem.first,
                                                          elem.second);
                }
            }

            __RefreshGuildWindowGradePage();
            __RefreshGuildWindowMemberPageGradeComboBox();
            break;
        }
        case GUILD_SUBHEADER_GC_GRADE_NAME: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_GRADE_NAME");
            if (p.oneGrade) {
                const auto& g = p.oneGrade.value();

                CPythonGuild::Instance().SetGradeName(g.grade,
                                                      g.grade_name.c_str());
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Refres"
                                                                        "hGuild"
                                                                        "Grad"
                                                                        "e");

                SPDLOG_TRACE(" <Change Grade Name> {0}, {1}", g.grade,
                             g.grade_name);
                __RefreshGuildWindowGradePage();
                __RefreshGuildWindowMemberPageGradeComboBox();
            }

            break;
        }
        case GUILD_SUBHEADER_GC_GRADE_AUTH: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_GRADE_AUTH");
            if (p.oneGradeAuth) {
                const auto& g = p.oneGradeAuth.value();

                CPythonGuild::Instance().SetGradeAuthority(g.grade, g.auth);
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Refres"
                                                                        "hGuild"
                                                                        "Grad"
                                                                        "e");

                SPDLOG_TRACE(" <Change Grade Authority> {0}, {1}", g.grade,
                             g.auth);
                __RefreshGuildWindowGradePage();
            }
            break;
        }
        case GUILD_SUBHEADER_GC_INFO: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_INFO");
            if (!p.info)
                return true;

            TPacketGCGuildInfo GuildInfo = p.info.value();

            CPythonGuild::Instance().EnableGuild();
            CPythonGuild::TGuildInfo& rGuildInfo =
                CPythonGuild::Instance().GetGuildInfoRef();
            storm::CopyStringSafe(rGuildInfo.szGuildName, GuildInfo.name);

            rGuildInfo.dwGuildID = GuildInfo.guild_id;
            rGuildInfo.dwMasterPID = GuildInfo.master_pid;
            rGuildInfo.dwGuildLevel = GuildInfo.level;
            rGuildInfo.dwCurrentExperience = GuildInfo.exp;
            rGuildInfo.dwCurrentMemberCount = GuildInfo.member_count;
            rGuildInfo.dwMaxMemberCount = GuildInfo.max_member_count;
            rGuildInfo.bHasLand = GuildInfo.hasLand;

            // Tracef(" <Info> %s, %d, %d : %d\n", GuildInfo.name,
            // GuildInfo.master_pid, GuildInfo.level, rGuildInfo.bHasLand);
            __RefreshGuildWindowInfoPage();
            break;
        }
        case GUILD_SUBHEADER_GC_COMMENTS: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_COMMENTS");

            if (!p.comments)
                break;

            CPythonGuild::Instance().ClearComment();

            for (const auto& comment : p.comments.value()) {
                CPythonGuild::Instance().RegisterComment(
                    comment.id, comment.player.c_str(),
                    comment.comment.c_str());
            }

            __RefreshGuildWindowBoardPage();
            break;
        }
        case GUILD_SUBHEADER_GC_CHANGE_EXP: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_CHANGE_EXP");
            if (p.status) {
                const auto& s = p.status.value();
                CPythonGuild::Instance().SetGuildEXP(s.lv, s.exp);
                SPDLOG_TRACE(" <ChangeEXP> {0}, {1}", s.lv, s.exp);
                __RefreshGuildWindowInfoPage();
            }
            break;
        }
        case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE");
            if (!p.updateMemberGrade)
                break;

            const auto& umg = p.updateMemberGrade.value();

            CPythonGuild::Instance().ChangeGuildMemberGrade(umg.pid, umg.grade);
            SPDLOG_TRACE(" <ChangeMemberGrade> {0}, {1}", umg.pid, umg.grade);
            __RefreshGuildWindowMemberPage();
            break;
        }
        case GUILD_SUBHEADER_GC_SKILL_INFO: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_SKILL_INFO");

            CPythonGuild::TGuildSkillData& rSkillData =
                CPythonGuild::Instance().GetGuildSkillDataRef();

            if (p.skillInfo) {
                const auto& skillInfo = p.skillInfo.value();
                rSkillData.bySkillPoint = skillInfo.skillPoints;
                std::memcpy(rSkillData.bySkillLevel, skillInfo.skillLevels,
                            GUILD_SKILL_COUNT);
                rSkillData.wGuildPoint = skillInfo.power;
                rSkillData.wMaxGuildPoint = skillInfo.maxPower;
            }

            SPDLOG_TRACE(" <SkillInfo> {0} / {1}, {2}", rSkillData.bySkillPoint,
                         rSkillData.wGuildPoint, rSkillData.wMaxGuildPoint);
            __RefreshGuildWindowSkillPage();
            break;
        }
        case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL");
            if (!p.updateGeneral)
                break;

            const auto& ug = p.updateGeneral.value();

            CPythonGuild::Instance().ChangeGuildMemberGeneralFlag(ug.pid,
                                                                  ug.isGeneral);
            SPDLOG_TRACE(" <ChangeMemberGeneralFlag> {0}, {1}", ug.pid,
                         ug.isGeneral);
            __RefreshGuildWindowMemberPage();
            break;
        }
        case GUILD_SUBHEADER_GC_GUILD_INVITE: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_GUILD_INVITE");
            if (!p.guildInvite)
                break;

            const auto& gi = p.guildInvite.value();

            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "RecvGuildInviteQuestion", gi.gid,
                                  py::bytes(gi.name));
            SPDLOG_TRACE(" <Guild Invite> {0}, {1}", gi.gid, gi.name);
            break;
        }
        case GUILD_SUBHEADER_GC_WAR: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_WAR");
            if (!p.guildWar)
                break;

            TPacketGCGuildWar kGuildWar = p.guildWar.value();

            switch (kGuildWar.bWarState) {
                case GUILD_WAR_SEND_DECLARE:
                    SPDLOG_TRACE(" >> GUILD_SUBHEADER_GC_WAR : "
                                 "GUILD_WAR_SEND_DECLARE");
                    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                          "BINARY_GuildWar_OnSendDeclare",
                                          kGuildWar.dwGuildOpp);
                    break;
                case GUILD_WAR_RECV_DECLARE:
#ifdef ENABLE_NEW_GUILD_WAR
                    SPDLOG_TRACE(" >> GUILD_SUBHEADER_GC_WAR : "
                                 "GUILD_WAR_RECV_DECLARE\n");
                    PyCallClassMemberFunc(
                        m_apoPhaseWnd[PHASE_WINDOW_GAME],
                        "BINARY_GuildWar_OnRecvDeclare", kGuildWar.dwGuildOpp,
                        kGuildWar.bType, kGuildWar.bScore, kGuildWar.bMinLevel,
                        kGuildWar.bMaxPlayer, kGuildWar.bBinekkullanimi,
                        kGuildWar.savasci, kGuildWar.ninja, kGuildWar.sura,
                        kGuildWar.shaman);
                    break;
#else
                case GUILD_WAR_RECV_DECLARE:
                    SPDLOG_TRACE(" >> GUILD_SUBHEADER_GC_WAR : "
                                 "GUILD_WAR_RECV_DECLARE\n");
                    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                          "BINARY_GuildWar_OnRecvDeclare",
                                          Py_BuildValue("(ii)",
                                                        kGuildWar.dwGuildOpp,
                                                        kGuildWar.bType));
                    break;
#endif
                case GUILD_WAR_ON_WAR:
                    SPDLOG_TRACE(" >> GUILD_SUBHEADER_GC_WAR : "
                                 "GUILD_WAR_ON_WAR : {0}, {1}",
                                 kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp);
                    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                          "BINARY_GuildWar_OnStart",
                                          kGuildWar.dwGuildSelf,
                                          kGuildWar.dwGuildOpp);
                    CPythonGuild::Instance().StartGuildWar(
                        kGuildWar.dwGuildOpp);
                    break;
                case GUILD_WAR_END:
                    SPDLOG_TRACE(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_END");
                    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                          "BINARY_GuildWar_OnEnd",
                                          kGuildWar.dwGuildSelf,
                                          kGuildWar.dwGuildOpp);
                    CPythonGuild::Instance().EndGuildWar(kGuildWar.dwGuildOpp);
                    break;
            }
            break;
        }
        case GUILD_SUBHEADER_GC_GUILD_NAME: {

            SPDLOG_TRACE("GUILD_SUBHEADER_GC_GUILD_NAME");
            if (!p.name)
                break;

            CPythonGuild::Instance().RegisterGuildName(p.name.value().id,
                                                       p.name.value().name);

            break;
        }
        case GUILD_SUBHEADER_GC_GUILD_WAR_LIST: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_GUILD_WAR_LIST");
            if (!p.guildWars)
                break;

            const auto& gwds = p.guildWars.value();

            for (const auto& guildWar : gwds) {
                SPDLOG_TRACE(" >> GulidWarList [{0} vs {1}]", guildWar.gid1,
                             guildWar.gid2);
                CInstanceBase::InsertGVGKey(guildWar.gid1, guildWar.gid2);
                CPythonCharacterManager::Instance().ChangeGVG(guildWar.gid1,
                                                              guildWar.gid2);
            }

            break;
        }
        case GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST");
            if (!p.guildWars)
                break;

            const auto& gwds = p.guildWars.value();

            for (const auto& guildWar : gwds) {
                SPDLOG_TRACE(" >> GulidWarEndList [{0} vs {1}]", guildWar.gid1,
                             guildWar.gid2);
                CInstanceBase::RemoveGVGKey(guildWar.gid1, guildWar.gid2);
                CPythonCharacterManager::Instance().ChangeGVG(guildWar.gid1,
                                                              guildWar.gid2);
            }
            break;
        }
        case GUILD_SUBHEADER_GC_WAR_SCORE: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_WAR_SCORE");
            if (!p.guildWarPoint)
                break;
            TPacketGuildWarPoint GuildWarPoint = p.guildWarPoint.value();

            PyCallClassMemberFunc(
                m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_GuildWar_OnRecvPoint",
                GuildWarPoint.dwGainGuildID, GuildWarPoint.dwOpponentGuildID,
                GuildWarPoint.lPoint);
            break;
        }
        case GUILD_SUBHEADER_GC_MONEY_CHANGE: {
            SPDLOG_TRACE("GUILD_SUBHEADER_GC_MONEY_CHANGE");
            if (!p.pid)
                break;

            const auto& moneyChange = p.pid.value();

            CPythonGuild::Instance().SetGuildMoney(moneyChange);

            __RefreshGuildWindowInfoPage();
            SPDLOG_TRACE(" >> Guild Money Change : {0}", p.pid.value());
            break;
        }
    }

    return true;
}

// Guild
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Nation War

bool CPythonNetworkStream::RecvThreeWayStatus(const TPacketGCThreeWayStatus& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_NationWar_Status", p.score[0], p.score[1],
                          p.score[2]);

    return true;
}

bool CPythonNetworkStream::RecvThreeWayLives(const TPacketGCThreeWayLives& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_NationWar_Lives", p.lives);

    return true;
}

// Nation War
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// Fishing

bool CPythonNetworkStream::SendFishingPacket(int iRotation)
{

    uint8_t byPacketRotation = iRotation / 5;
    Send(HEADER_CG_FISHING, byPacketRotation);

    return true;
}

bool CPythonNetworkStream::SendFishingGamePacket(int hitCount, float time)
{
    TPacketCGFishingGame p{static_cast<uint8_t>(hitCount), time};
    Send(HEADER_CG_FISHING_GAME, p);

    return true;
}

bool CPythonNetworkStream::SendGiveItemPacket(uint32_t dwTargetVID,
                                              TItemPos ItemPos, int iItemCount)
{
    TPacketCGGiveItem GiveItemPacket;
    GiveItemPacket.dwTargetVID = dwTargetVID;
    GiveItemPacket.ItemPos = ItemPos;
    GiveItemPacket.byItemCount = iItemCount;

    Send(HEADER_CG_ITEM_GIVE, GiveItemPacket);
    return true;
}

bool CPythonNetworkStream::RecvFishing(const TPacketGCFishing& p)
{
    CInstanceBase* pFishingInstance = NULL;
    if (FISHING_SUBHEADER_GC_FISH != p.subheader &&
        FISHING_SUBHEADER_GC_REACT != p.subheader) {
        pFishingInstance =
            CPythonCharacterManager::Instance().GetInstancePtr(p.info);
        if (!pFishingInstance)
            return true;
    }

    switch (p.subheader) {
        case FISHING_SUBHEADER_GC_START:
            pFishingInstance->StartFishing(float(p.dir) * 5.0f);
            break;
        case FISHING_SUBHEADER_GC_STOP:
            if (CPythonFishingManager::instance().FishingIsOnGoing() &&
                CPythonCharacterManager::instance().GetMainInstancePtr() ==
                    pFishingInstance) {
                CPythonFishingManager::instance().Quit();
            }
            if (pFishingInstance->IsFishing())
                pFishingInstance->StopFishing();
            break;
        case FISHING_SUBHEADER_GC_REACT: {
            if (!p.fi)
                break;

            const auto& fi = p.fi.value();
            pFishingInstance =
                CPythonCharacterManager::Instance().GetInstancePtr(fi.vid);
            if (!pFishingInstance)
                return true;

            if (pFishingInstance->IsFishing()) {
                CPythonFishingManager::instance().ReactFishing(
                    fi.maxHit, fi.x, fi.y); // Fish Emoticon
            }
            break;
        }

        case FISHING_SUBHEADER_GC_SUCCESS:
            pFishingInstance->CatchSuccess();
            pFishingInstance->SetFishEmoticon();
            break;
        case FISHING_SUBHEADER_GC_FAIL:
            pFishingInstance->CatchFail();
            if (pFishingInstance ==
                CPythonCharacterManager::Instance().GetMainInstancePtr()) {
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFish"
                                                                        "ingFai"
                                                                        "lure");
            }
            pFishingInstance->SetEmoticon(25);

            break;
        case FISHING_SUBHEADER_GC_FISH: {
            uint32_t dwFishID = p.info;

            if (0 == p.info) {
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFish"
                                                                        "ingNot"
                                                                        "ifyUnk"
                                                                        "nown");
                return true;
            }

            CItemData* pItemData;
            if (!CItemManager::Instance().GetItemDataPointer(dwFishID,
                                                             &pItemData))
                return true;

            CInstanceBase* pMainInstance =
                CPythonCharacterManager::Instance().GetMainInstancePtr();
            if (!pMainInstance)
                return true;

            if (pMainInstance->IsFishing()) {
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                      "OnFishingNotify",
                                      ITEM_FISH == pItemData->GetType(),
                                      py::bytes(pItemData->GetName()));
            } else {
                PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                      "OnFishingSuccess",
                                      ITEM_FISH == pItemData->GetType(),
                                      py::bytes(pItemData->GetName()));
            }
            break;
        }
    }

    return true;
}

// Fishing
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// MyShop
bool CPythonNetworkStream::SendMyShopOpenPacket(
    const std::string& sign, const TItemPos& bundleItem,
    const std::vector<TShopItemTable>& items)
{
    CgMyShopOpen packet;
    packet.bundleItem = bundleItem;
    packet.sign = sign;
    packet.count = items.size();
    packet.table = items;
    Send(HEADER_CG_MYSHOP, packet);

    return true;
}

bool CPythonNetworkStream::RecvShopSignPacket(const TPacketGCShopSign& p)
{
    CPythonPlayer& rkPlayer = CPythonPlayer::Instance();

    if (p.szSign.empty()) {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_PrivateShop_Disappear", p.dwVID);

        if (rkPlayer.IsMainCharacterIndex(p.dwVID))
            rkPlayer.ClosePrivateShop();
    } else {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_PrivateShop_Appear", p.dwVID,
                              py::bytes(p.szSign), p.type);

        if (rkPlayer.IsMainCharacterIndex(p.dwVID))
            rkPlayer.OpenPrivateShop();
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////

bool CPythonNetworkStream::RecvTimePacket(const TPacketGCTime& p)
{
    CPythonApplication& rkApp = CPythonApplication::AppInst();
    rkApp.SetServerTime(p.time);

    return true;
}

bool CPythonNetworkStream::RecvWalkModePacket(const TPacketGCWalkMode& p)
{

    CInstanceBase* pInstance =
        CPythonCharacterManager::Instance().GetInstancePtr(p.vid);
    if (pInstance) {
        if (WALKMODE_RUN == p.mode) {
            pInstance->SetRunMode();
        } else {
            pInstance->SetWalkMode();
        }
    }

    return true;
}

bool CPythonNetworkStream::RecvChangeSkillGroupPacket(
    const TPacketGCChangeSkillGroup& p)
{

    m_dwMainActorSkillGroup = p.skill_group;

    CPythonPlayer::Instance().NEW_ClearSkillData();
    __RefreshCharacterWindow();
    return true;
}

void CPythonNetworkStream::__TEST_SetSkillGroupFake(int iIndex)
{
    m_dwMainActorSkillGroup = uint32_t(iIndex);

    CPythonPlayer::Instance().NEW_ClearSkillData();
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter");
}

bool CPythonNetworkStream::SendRefinePacket(uint16_t byPos, uint8_t byType)
{
    TPacketCGRefine kRefinePacket;
    kRefinePacket.pos = byPos;
    kRefinePacket.type = byType;

    Send(HEADER_CG_REFINE, kRefinePacket);
    return true;
}

bool CPythonNetworkStream::SendSelectItemPacket(uint32_t dwItemPos)
{
    TPacketCGScriptSelectItem kScriptSelectItem;
    kScriptSelectItem.selection = dwItemPos;
    Send(HEADER_CG_SCRIPT_SELECT_ITEM, kScriptSelectItem);
    return true;
}

bool CPythonNetworkStream::RecvRefineInformationPacketNew(
    const TPacketGCRefineInformation& p)
{

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenRefineDialog",
                          p.pos, p.result_vnum, p.cost, p.prob, p.type);

    for (int i = 0; i < p.materials.size(); ++i) {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "AppendMaterialToRefineDialog",
                              p.materials[i].vnum, p.materials[i].count);
    }

    for (int i = 0; i < p.enhance_mat.size(); ++i) {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "AppendEnhanceMaterialToRefineDialog",
                              p.enhance_mat[i].vnum, p.enhance_mat[i].count,
                              p.enhance_mat[i].prob);
    }

#ifdef _DEBUG
    SPDLOG_DEBUG(" >> RecvRefineInformationPacketNew(pos=%d, result_vnum=%d, "
                 "cost=%d, prob=%d, type=%d)\n",
                 p.pos, p.result_vnum, p.cost, p.prob, p.type);
#endif

    return true;
}

bool CPythonNetworkStream::RecvNPCList(const TPacketGCNPCPosition& p)
{

    CPythonMiniMap::Instance().ClearAtlasMarkInfo();

    for (const auto& NPCPosition : p.positions) {
        const auto pRaceData =
            CRaceManager::instance().GetRaceDataPointer(NPCPosition.vnum);
        if (pRaceData)
            pRaceData.value()->LoadMotions();

        const auto* name =
            NpcManager::instance().GetMonsterName(NPCPosition.vnum);

        CPythonMiniMap::Instance().RegisterAtlasMark(
            NPCPosition.bType, name, NPCPosition.x, NPCPosition.y);
    }

    return true;
}

bool CPythonNetworkStream::RecvSHOPList(const TPacketGCShopPosition& p)
{
    CPythonMiniMap::Instance().ClearAtlasShopInfo(); // Clear existing shops

    for (const auto& SHOPPosition : p.positions) {
        CPythonMiniMap::Instance().RegisterAtlasMark(
            CActorInstance::TYPE_SHOP, SHOPPosition.name.c_str(),
            SHOPPosition.x, SHOPPosition.y);
    }

    return true;
}

bool CPythonNetworkStream::__SendCRCReportPacket()
{
    return true;
}

bool CPythonNetworkStream::RecvAffectAddPacket(const GcAffectAddPacket& p)
{

    auto& chrMgr = CPythonCharacterManager::Instance();
    auto instance = chrMgr.GetInstancePtr(p.vid);
    if (!instance) {
        SPDLOG_ERROR("Failed to find character {0}", p.vid);
        return true;
    }

    instance->AddAffect(p.data);
    SPDLOG_TRACE("AddAffect {}", p.data.type);
    if (instance != chrMgr.GetMainInstancePtr())
        return true;

    auto& player = CPythonPlayer::instance();
    const auto& affect = p.data;

    uint32_t skillSlot;
    if (player.GetSkillSlotIndex(affect.type, &skillSlot)) {
        CPythonSkill::TSkillData* pSkillData;
        if (CPythonSkill::Instance().GetSkillData(affect.type, &pSkillData)) {
            if (pSkillData->IsToggleSkill()) {
                player.ActivateSkillSlot(skillSlot);
            }
        }
    }

    if (affect.pointType == POINT_ENERGY) {
        // TODO: Don't use points for this...
        player.SetStatus(POINT_ENERGY_END_TIME,
                         CPythonApplication::AppInst().GetServerTimeStamp() +
                             affect.duration);
        __RefreshStatus();
    }

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_NEW_AddAffect", affect.type, affect.subType, affect.pointType,
                          affect.pointValue, affect.duration);
    return true;
}

bool CPythonNetworkStream::RecvAffectRemovePacket(const GcAffectDelPacket& p)
{
    auto& chrMgr = CPythonCharacterManager::Instance();
    auto instance = chrMgr.GetInstancePtr(p.vid);
    if (!instance) {
        SPDLOG_ERROR("Failed to find character {0}", p.vid);
        return true;
    }

    instance->RemoveAffect(p.type, p.pointType);

    if (instance != chrMgr.GetMainInstancePtr())
        return true;

    auto& player = CPythonPlayer::instance();

    uint32_t skillSlot;
    if (player.GetSkillSlotIndex(p.type, &skillSlot)) {
        CPythonSkill::TSkillData* pSkillData;
        if (CPythonSkill::Instance().GetSkillData(p.type, &pSkillData)) {
            if (pSkillData->IsToggleSkill()) {
                player.DeactivateSkillSlot(skillSlot);
            }
        }
    }

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_NEW_RemoveAffect", p.type, p.pointType);

    return true;
}

bool CPythonNetworkStream::RecvChannelPacket(const TPacketGCChannel& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "UpdateChannelDisplayInfo", p.channel);

    return true;
}

bool CPythonNetworkStream::RecvViewEquipPacket(const TPacketViewEquip& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "OpenEquipmentDialog", p.vid);
    for (int i = 0; i < WEAR_MAX_NUM; ++i) {
        auto& rItemSet = p.items[i];
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "SetEquipmentDialogItem", p.vid, i, rItemSet.vnum,
                              rItemSet.count, rItemSet.transVnum);
#else
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "SetEquipmentDialogItem", kViewEquipPacket.vid, i,
                              rItemSet.vnum, rItemSet.count);

#endif
        for (int j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "SetEquipmentDialogSocket", p.vid, i, j,
                                  rItemSet.sockets[j]);

        for (int k = 0; k < ITEM_ATTRIBUTE_MAX_NUM; ++k)
            PyCallClassMemberFunc(
                m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogAttr",
                p.vid, i, k, rItemSet.attrs[k].bType, rItemSet.attrs[k].sValue);
    }
    return true;
}

bool CPythonNetworkStream::RecvUpdateLandPacket(
    const TPacketGCGuildLandUpdate& p)
{

    std::vector<uint32_t> kVec_dwGuildID;

    CPythonMiniMap& rkMiniMap = CPythonMiniMap::Instance();

    rkMiniMap.UpdateGuildArea(p.landID, p.guildID);

    if (0 != p.guildID)
        kVec_dwGuildID.push_back(p.guildID);

    __DownloadSymbol(kVec_dwGuildID);

    return true;
}

bool CPythonNetworkStream::RecvLandPacket(const TPacketGCLandList& p)
{

    std::vector<uint32_t> kVec_dwGuildID;

    CPythonMiniMap& rkMiniMap = CPythonMiniMap::Instance();
    CPythonBackground& rkBG = CPythonBackground::Instance();
    CInstanceBase* pMainInstance =
        CPythonPlayer::Instance().NEW_GetMainActorPtr();

    rkMiniMap.ClearGuildArea();
    rkBG.ClearGuildArea();

    for (const auto& kElement : p.lands) {
        rkMiniMap.RegisterGuildArea(kElement.dwID, kElement.dwGuildID,
                                    kElement.x, kElement.y, kElement.width,
                                    kElement.height);

        if (pMainInstance) {
            if (kElement.dwGuildID == pMainInstance->GetGuildID()) {
                rkBG.RegisterGuildArea(kElement.x, kElement.y,
                                       kElement.x + kElement.width,
                                       kElement.y + kElement.height);
            }
        }

        if (0 != kElement.dwGuildID)
            kVec_dwGuildID.push_back(kElement.dwGuildID);
    }

    __DownloadSymbol(kVec_dwGuildID);

    return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacket(
    const TPacketGCTargetCreate& kTargetCreate)
{
    CPythonMiniMap& rkpyMiniMap = CPythonMiniMap::Instance();
    rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szName.c_str());

    //#ifdef _DEBUG
    //	char szBuf[256+1];
    //	_snprintf(szBuf, sizeof(szBuf), "Å¸°ÙÀÌ »ý¼º µÇ¾ú½À´Ï´Ù [%d:%s]",
    // kTargetCreate.lID,
    // kTargetCreate.szTargetName);
    // CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
    // SPDLOG_DEBUG("
    // >> RecvTargetCreatePacket %d : %s\n", kTargetCreate.lID,
    // kTargetCreate.szTargetName); #endif

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_"
                                                            "OpenAtlasWindow");
    return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacketNew(
    const TPacketGCTargetCreate& kTargetCreate)
{
    CPythonMiniMap& rkpyMiniMap = CPythonMiniMap::Instance();
    CPythonBackground& rkpyBG = CPythonBackground::Instance();
    if (CREATE_TARGET_TYPE_LOCATION == kTargetCreate.bType) {
        rkpyMiniMap.CreateTarget(kTargetCreate.lID,
                                 kTargetCreate.szName.c_str());
    } else {
        rkpyMiniMap.CreateTarget(kTargetCreate.lID,
                                 kTargetCreate.szName.c_str(),
                                 kTargetCreate.dwVID);
        rkpyBG.CreateTargetEffect(kTargetCreate.lID, kTargetCreate.dwVID);
    }

    //#ifdef _DEBUG
    //	char szBuf[256+1];
    //	_snprintf(szBuf, sizeof(szBuf), "Ä³¸¯ÅÍ Å¸°ÙÀÌ »ý¼º µÇ¾ú½À´Ï´Ù
    //[%d:%s:%d]", kTargetCreate.lID,
    // kTargetCreate.szTargetName, kTargetCreate.dwVID);
    // CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
    //	SPDLOG_DEBUG(" >> RecvTargetCreatePacketNew %d : %d/%d\n",
    // kTargetCreate.lID, kTargetCreate.byType,
    // kTargetCreate.dwVID); #endif

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_"
                                                            "OpenAtlasWindow");
    return true;
}

bool CPythonNetworkStream::RecvTargetUpdatePacket(
    const TPacketGCTargetUpdate& kTargetUpdate)
{

    CPythonMiniMap& rkpyMiniMap = CPythonMiniMap::Instance();
    rkpyMiniMap.UpdateTarget(kTargetUpdate.lID, kTargetUpdate.lX,
                             kTargetUpdate.lY);

    CPythonBackground& rkpyBG = CPythonBackground::Instance();
    rkpyBG.CreateTargetEffect(kTargetUpdate.lID, kTargetUpdate.lX,
                              kTargetUpdate.lY);

    //#ifdef _DEBUG
    //	char szBuf[256+1];
    //	_snprintf(szBuf, sizeof(szBuf), "Å¸°ÙÀÇ À§Ä¡°¡ °»½Å µÇ¾ú½À´Ï´Ù
    //[%d:%d/%d]", kTargetUpdate.lID, kTargetUpdate.lX,
    // kTargetUpdate.lY); 	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE,
    // szBuf); 	SPDLOG_DEBUG(" >> RecvTargetUpdatePacket %d : %d, %d\n",
    // kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY); #endif

    return true;
}

bool CPythonNetworkStream::RecvTargetDeletePacket(
    const TPacketGCTargetDelete& kTargetDelete)
{
    CPythonMiniMap& rkpyMiniMap = CPythonMiniMap::Instance();
    rkpyMiniMap.DeleteTarget(kTargetDelete.lID);

    CPythonBackground& rkpyBG = CPythonBackground::Instance();
    rkpyBG.DeleteTargetEffect(kTargetDelete.lID);

    //#ifdef _DEBUG
    //	SPDLOG_DEBUG(" >> RecvTargetDeletePacket %d\n", kTargetDelete.lID);
    //#endif

    return true;
}

bool CPythonNetworkStream::RecvLoverInfoPacket(const TPacketGCLoverInfo& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LoverInfo",
                          py::bytes(p.name), p.love_point);
#ifdef _DEBUG
    SPDLOG_DEBUG("RECV LOVER INFO : %s, %d\n", p.name, p.love_point);
#endif
    return true;
}

bool CPythonNetworkStream::RecvLovePointUpdatePacket(
    const TPacketGCLovePointUpdate& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_UpdateLovePoint", p.love_point);
#ifdef _DEBUG
    SPDLOG_DEBUG("RECV LOVE POINT UPDATE : %d\n", p.love_point);
#endif
    return true;
}

bool CPythonNetworkStream::RecvDigMotionPacket(const TPacketGCDigMotion& p)
{
#ifdef _DEBUG
    SPDLOG_DEBUG(" Dig Motion [{}/{}]", p.vid, p.count);
#endif

    auto& rkChrMgr = CPythonCharacterManager::Instance();
    CInstanceBase* pkInstMain = rkChrMgr.GetInstancePtr(p.vid);
    CInstanceBase* pkInstTarget = rkChrMgr.GetInstancePtr(p.target_vid);
    if (NULL == pkInstMain)
        return true;

    if (pkInstTarget)
        pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);

    for (int i = 0; i < p.count; ++i)
        pkInstMain->PushOnceMotion(MOTION_DIG);

    return true;
}

// ¿ëÈ¥¼® °­È­
bool CPythonNetworkStream::SendDragonSoulRefinePacket(uint8_t bRefineType,
                                                      TItemPos* pos)
{
    TPacketCGDragonSoulRefine pk;
    pk.bSubType = bRefineType;
    memcpy(pk.ItemGrid, pos, sizeof(TItemPos) * DS_REFINE_WINDOW_MAX_NUM);
    Send(HEADER_CG_DRAGON_SOUL_REFINE, pk);
    return true;
}

bool CPythonNetworkStream::RecvInputStatePacket(const TPacketGCInputState& p)
{
    isInputEnabled = p.isEnable;

    return true;
}

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
bool CPythonNetworkStream::RecvShopSearchSet(
    const GcShopSearchPacket& p)
{
    CPythonPrivateShopSearch::Instance().ClearItemData();

    if(p.setItems) {
        for(const auto& item : p.setItems.value())
            CPythonPrivateShopSearch::Instance().AddItemData(item);
    }

    if(p.pageInfo) {
        CPythonPrivateShopSearch::Instance().SetPageInfo(p.pageInfo.value());
    }

    __RefreshShopSearchWindow();
    return true;
}
#endif

bool CPythonNetworkStream::SendOfflineShopWarpPacket()
{
    uint8_t p = HEADER_CG_MYSHOP_WARP;
    Send(p, p);

    return true;
}

bool CPythonNetworkStream::OpenMyShop()
{
    BlankPacket header;
    Send(HEADER_CG_MYSHOP_OPEN, header);
    return true;
}

bool CPythonNetworkStream::OpenMyShopSearch()
{
    BlankPacket header;
    Send(HEADER_CG_MYSHOP_OPEN_SEARCH, header);
    return true;
}

bool CPythonNetworkStream::RecvUpdateLastPlayPacket(const TGcUpdateLastPlay& p)
{
    m_players[m_selectedCharacterSlot].last_play = p.lastPlay;
    return true;
}

bool CPythonNetworkStream::RecvMaintenance(const TPacketGCMaintenance& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_Maintenance", p.seconds, p.duration);

    return true;
}

bool CPythonNetworkStream::SendMyShopDecoState(uint8_t state)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGMyShopDecoState pState = state;

    TPacketCGMyShopDeco p;
    p.subheader = MYSHOP_DECO_SUBHEADER_CG_STATE;
    p.state = pState;

    Send(HEADER_CG_MYSHOP_DECO, p);
    return true;
}

bool CPythonNetworkStream::SendMyShopDecoSet(uint32_t titleType,
                                             uint32_t shopVnum)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGMyShopDecoSet p2;
    p2.titleType = titleType;
    p2.polyVnum = shopVnum;

    TPacketCGMyShopDeco p;
    p.subheader = MYSHOP_DECO_SUBHEADER_CG_SET;
    p.set = p2;

    Send(HEADER_CG_MYSHOP_DECO, p);

    return true;
}

// WIKI

#ifdef INGAME_WIKI
extern PyObject* wikiModule;

bool CPythonNetworkStream::RecvWikiPacket(const GcWikiPacket& p)
{
    CItemData* pData;
    if (!CItemManager::instance().GetItemDataPointer(p.vnum, &pData))
        return false;

    TWikiItemInfo* wikiInfo = pData->GetWikiTable();
    wikiInfo->isSet = true;
    wikiInfo->hasData = p.refineInfo.size() > 0 || p.chestInfos.size() > 0 ||
                        p.originInfos.size() > 0;
    wikiInfo->bIsCommon = p.itemInfo.bIsCommon;
    wikiInfo->dwOrigin = p.itemInfo.dwOrigin;

    wikiInfo->pRefineData = p.refineInfo;
    wikiInfo->pChestInfo = p.chestInfos;
    wikiInfo->pOriginInfo = p.originInfos;

    PyCallClassMemberFunc(wikiModule, "BINARY_LoadInfo",
                          Py_BuildValue("(Li)", (long long)p.ret_id, p.vnum));
    return true;
}

bool CPythonNetworkStream::RecvWikiMobPacket(const GcWikiMobPacket& p)
{
    NpcManager::TWikiInfoTable* mobData =
        NpcManager::instance().GetWikiTable(p.vnum);
    if (mobData) {
        if (!mobData->isSet) {
            mobData->isSet = true;
            for (auto mob : p.vnums)
                mobData->dropList.push_back(mob);
        }
    }

    PyCallClassMemberFunc(wikiModule, "BINARY_LoadInfo",
                          Py_BuildValue("(Li)", (long long)p.ret_id, p.vnum));
    return true;
}
#endif

#ifdef ENABLE_BATTLE_PASS
bool CPythonNetworkStream::SendBattlePassAction(BYTE bAction)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGBattlePassAction p;
    p.bAction = bAction;
    Send(HEADER_CG_BATTLE_PASS, p);
    return true;
}

bool CPythonNetworkStream::RecvHuntingMissionPacket(
    const GcHuntingMissionPacket& p)
{
    if (p.load) {
        CPythonPlayer::instance().ClearHuntingMissions();
        for (const auto& m : p.load.value()) {
            CPythonPlayer::instance().SetHuntingMission(m.missionId, m);
        }
    } else if (p.update) {
        auto m = p.update.value();
        CPythonPlayer::instance().SetHuntingMission(m.missionId, m);
    }

    __RefreshHuntingMissions();
    return true;
}

bool CPythonNetworkStream::RecvBattlePassPacket(const GcBattlePassPacket& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_"
                                                            "BattlePassClearMis"
                                                            "sion");

    for (const auto& missionInfo : p.missions) {
        PyCallClassMemberFunc(
            m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_BattlePassAddMission",
            missionInfo.bMissionType, missionInfo.dwMissionInfo[0],
            missionInfo.dwMissionInfo[1], missionInfo.dwMissionInfo[2]);

        for (int i = 0; i < MISSION_REWARD_COUNT; ++i)
            PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                                  "BINARY_BattlePassAddMissionReward",
                                  missionInfo.bMissionType,
                                  missionInfo.aRewardList[i].dwVnum,
                                  missionInfo.aRewardList[i].bCount);
    }

    for (const auto& rewardInfo : p.rewards) {

        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_BattlePassAddReward", rewardInfo.dwVnum,
                              rewardInfo.bCount);
    }

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_"
                                                            "BattlePassOpen");

    return true;
}

bool CPythonNetworkStream::RecvBattlePassRankingPacket(
    const TPacketGCBattlePassRanking& p)
{
    for (const auto& rankingInfo : p.elems) {

        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_BattlePassAddRanking", rankingInfo.bPos,
                              py::bytes(rankingInfo.playerName),
                              rankingInfo.dwFinishTime);
    }

    if (p.bIsGlobal)
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_"
                                                                "BattlePassOpen"
                                                                "Ranking");
    else
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_"
                                                                "BattlePassRefr"
                                                                "eshRanking");

    return true;
}

bool CPythonNetworkStream::RecvBattlePassUpdatePacket(
    const GcBattlePassUpdatePacket& p)
{
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_BattlePassUpdate", p.bMissionType,
                          p.dwNewProgress);

    return true;
}
#endif

bool CPythonNetworkStream::SendChatFilter(
    const std::vector<uint8_t>& bFilterList)
{
    TPacketCGChatFilter pack;
    pack.bFilterList = bFilterList;
    Send(HEADER_CG_CHAT_FILTER, pack);
    return true;
}

bool CPythonNetworkStream::RecvUpdateChatFilterPacket(
    const TPacketGCUpdateChatFilter& p)
{

    for (int i = 0; i < p.bFilterList.size(); i++)
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_SetChatFilter", i, p.bFilterList[i]);

    return true;
}
bool CPythonNetworkStream::RecvHyperlinkItem(const CgHyperlinkItemPacket& p)
{
    CItemManager::instance().RegisterHyperlinkItem(p.d.id, p.d);

    return true;
}

bool CPythonNetworkStream::RecvDungeonInfoPacket(
    const TPacketGCDungeonInfo& packet)
{
    // When the packet come from server it's clear that we already send a
    // request
    CPythonDungeonInfo::Instance().SetNoNeedRequestInfo();

    if (packet.infos.size() == 0) {
        CPythonDungeonInfo::Instance().ClearInfo();
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DungeonInfoOpen");
        return true;
    }

    CPythonDungeonInfo::Instance().ClearInfo();

    for (const auto& info : packet.infos) {
        CPythonDungeonInfo::Instance().AppendDungeonInfo(info);
    }

    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DungeonInfoOpen");
    return true;
}

bool CPythonNetworkStream::SendRequestDungeonInfo()
{
    if (!__CanActMainInstance())
        return true;

    BlankPacket bHeader;
    Send(HEADER_CG_DUNGEON_INFO, bHeader);
    return true;
}

bool CPythonNetworkStream::SendRequestDungeonWarp(uint8_t bDungeonID)
{
    if (!__CanActMainInstance())
        return true;

    TPacketCGDungeonWarp packet;
    packet.bDungeonID = bDungeonID;
    Send(HEADER_CG_DUNGEON_WARP, packet);

    return true;
}

bool CPythonNetworkStream::RecvDungeonRankingSetPacket(
    const TPacketGCDungeonRanking& p)
{
    // When the packet come from server it's clear that we already send a
    // request
    CPythonDungeonInfo::Instance().SetNoRankingRequestInfo(
        p.rankingSet.bDungeonID, p.rankingSet.bType);
    CPythonDungeonInfo::Instance().AppendDungeonRankingSet(p.rankingSet);

    PyCallClassMemberFunc(
        m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DungeonRankingOpen", p.rankingSet.bDungeonID, p.rankingSet.bType);

    return true;
}

bool CPythonNetworkStream::RecvDungeonUpdatePacket(
    const TPacketGCDungeonUpdate& p)
{

    CPythonDungeonInfo::Instance().UpdateDungeonCooldown(p.bDungeonID,
                                                         p.dwCooldownEnd);
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DungeonInfoRefresh");

    return true;
}

bool CPythonNetworkStream::SendRequestDungeonRanking(uint8_t bDungeonID,
                                                     uint8_t bRankingType)
{
    if (!__CanActMainInstance())
        return true;
    TPacketCGDungeonRanking packet;
    packet.bDungeonID = bDungeonID;
    packet.bRankingType = bRankingType;

    Send(HEADER_CG_DUNGEON_RANKING, packet);
    return true;
}