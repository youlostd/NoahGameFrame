#include "constants.h"
#include "packet_info.h"

CPacketInfo::CPacketInfo()
    : m_pCurrentPacket(nullptr)
      , m_dwStartTime(0)
{
}

void CPacketInfo::Set(uint8_t header, int iSize, const char *c_pszName,
                      bool isSequence, bool dynamic)
{
    if (m_pPacketMap.find(header) != m_pPacketMap.end())
        return;

    TPacketElement &element = m_pPacketMap[header];
    element.iSize = iSize;
    element.stName.assign(c_pszName);
    element.iCalled = 0;
    element.dwLoad = 0;
    element.dynamic = dynamic;
    element.isSequence = isSequence;
}

void CPacketInfo::SetSequence(uint8_t header, bool bSeq)
{
    TPacketElement *pkElem = GetElement(header);
    if (pkElem)
        pkElem->isSequence = bSeq;
}

TPacketElement *CPacketInfo::GetElement(uint8_t header)
{
    auto it = m_pPacketMap.find(header);
    if (it == m_pPacketMap.end())
        return nullptr;

    return &it->second;
}

void CPacketInfo::Start(TPacketElement *info)
{
    m_pCurrentPacket = info;
    m_dwStartTime = get_dword_time();
}

void CPacketInfo::End()
{
    ++m_pCurrentPacket->iCalled;
    m_pCurrentPacket->dwLoad += get_dword_time() - m_dwStartTime;
}

void CPacketInfo::Log(const char *c_pszFileName)
{
    FILE *fp = fopen(c_pszFileName, "w");
    if (!fp)
        return;

    fprintf(fp, "Name             Called     Load       Ratio\n");

    auto it = m_pPacketMap.begin();
    while (it != m_pPacketMap.end())
    {
        TPacketElement &p = it->second;
        ++it;

        fprintf(fp, "%-16s %-10d %-10u %.2f\n", p.stName.c_str(), p.iCalled,
                p.dwLoad, p.iCalled != 0 ? (float)p.dwLoad / p.iCalled : 0.0f);
    }

    fclose(fp);
}

CPacketInfoCG::CPacketInfoCG()
{
    Set(HEADER_CG_PHASE_ACK, sizeof(CgPhaseAckPacket), "Handshake");
    Set(HEADER_CG_TEXT, sizeof(BlankPacket), "Text");
    Set(HEADER_CG_TIME_SYNC, sizeof(TPacketCGHandshake), "TimeSync");
    Set(HEADER_CG_REQ_TIMESYNC, sizeof(BlankPacket), "ReqTimeSync");
    Set(HEADER_CG_MARK_LOGIN, sizeof(TPacketCGMarkLogin), "MarkLogin");
    Set(HEADER_CG_MARK_IDXLIST, sizeof(TPacketCGMarkIDXList), "MarkIdxList");
    Set(HEADER_CG_MARK_CRCLIST, sizeof(TPacketCGMarkCRCList), "MarkCrcList");
    Set(HEADER_CG_MARK_UPLOAD, sizeof(TPacketCGMarkUpload), "MarkUpload");

    Set(HEADER_CG_GUILD_SYMBOL_UPLOAD, sizeof(TPacketCGGuildSymbolUpload), "SymbolUpload", false, true);
    Set(HEADER_CG_SYMBOL_CRC, sizeof(TPacketCGSymbolCRC), "SymbolCRC");
    Set(HEADER_CG_KEY_LOGIN, sizeof(CgKeyLoginPacket), "LoginKey");
    Set(HEADER_CG_ATTACK, sizeof(TPacketCGAttack), "Attack");
    Set(HEADER_CG_CHAT, sizeof(TPacketCGChat), "Chat", false, true);
    Set(HEADER_CG_WHISPER, sizeof(TPacketCGWhisper), "Whisper", false, true);

    Set(HEADER_CG_CHARACTER_SELECT, sizeof(TPacketCGPlayerSelect), "Select");
    Set(HEADER_CG_CHARACTER_CREATE, sizeof(TPacketCGPlayerCreate), "Create");
    Set(HEADER_CG_CHARACTER_DELETE, sizeof(TPacketCGPlayerDelete), "Delete");
    Set(HEADER_CG_ENTERGAME, sizeof(TPacketCGEnterGame), "EnterGame");

    Set(HEADER_CG_ITEM_USE, sizeof(TPacketCGItemUse), "ItemUse");
    Set(HEADER_CG_ITEM_USE_MULTIPLE, sizeof(TPacketCGItemUse), "ItemUseMult");

    Set(HEADER_CG_ITEM_DROP, sizeof(TPacketCGItemDrop), "ItemDrop");
    Set(HEADER_CG_ITEM_DROP2, sizeof(TPacketCGItemDrop2), "ItemDrop2");
    Set(HEADER_CG_ITEM_MOVE, sizeof(TPacketCGItemMove), "ItemMove");
    Set(HEADER_CG_ITEM_SPLIT, sizeof(TPacketCGItemSplit), "ItemSplit");

    Set(HEADER_CG_ITEM_PICKUP, sizeof(TPacketCGItemPickup), "ItemPickup");
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
    Set(HEADER_CG_SHOP_SEARCH, sizeof(TPacketCGShopSearch), "ShopSearch");
    Set(HEADER_CG_SHOP_SEARCH_SUB, sizeof(TPacketCGShopSearch), "ShopSearchSub");
    Set(HEADER_CG_SHOP_SEARCH_BUY, sizeof(TPacketCGShopSearchBuy), "ShopSearchBuy");
#endif

    Set(HEADER_CG_QUICKSLOT_ADD, sizeof(TPacketCGQuickslotAdd), "QuickslotAdd");
    Set(HEADER_CG_QUICKSLOT_DEL, sizeof(TPacketCGQuickslotDel), "QuickslotDel");
    Set(HEADER_CG_QUICKSLOT_SWAP, sizeof(TPacketCGQuickslotSwap), "QuickslotSwap");

    Set(HEADER_CG_SHOP, sizeof(TPacketCGShop), "Shop");

    Set(HEADER_CG_ON_CLICK, sizeof(TPacketCGOnClick), "OnClick");
    Set(HEADER_CG_EXCHANGE, sizeof(TPacketCGExchange), "Exchange");
    Set(HEADER_CG_SCRIPT_ANSWER, sizeof(TPacketCGScriptAnswer), "ScriptAnswer");
    Set(HEADER_CG_SCRIPT_BUTTON, sizeof(TPacketCGScriptButton), "ScriptButton");
    Set(HEADER_CG_QUEST_INPUT_STRING, sizeof(TPacketCGQuestInputString), "QuestInputString");
    Set(HEADER_CG_QUEST_CONFIRM, sizeof(TPacketCGQuestConfirm), "QuestConfirm");
    Set(HEADER_CG_QUEST_RECEIVE, sizeof(TPacketCGQuestRcv), "QuestReceive");

    Set(HEADER_CG_CHARACTER_MOVE, sizeof(TPacketCGMove), "Move");
    Set(HEADER_CG_SYNC_POSITION, sizeof(TPacketCGSyncPosition), "SyncPosition");

    Set(HEADER_CG_FLY_TARGETING, sizeof(TPacketCGFlyTargeting), "FlyTarget");
    Set(HEADER_CG_ADD_FLY_TARGETING, sizeof(TPacketCGFlyTargeting), "AddFlyTarget");
    Set(HEADER_CG_SHOOT, sizeof(TPacketCGShoot), "Shoot");

    Set(HEADER_CG_USE_SKILL, sizeof(TPacketCGUseSkill), "UseSkill");
    Set(HEADER_CG_CHANGE_SKILL_COLOR, sizeof(TPacketCGChangeSkillColorPacket), "ChangeSkillColor");
    Set(HEADER_CG_CHOOSE_SKILL_GROUP, sizeof(TPacketCGChooseSkillGroup), "ChooseSkillGroup");

    Set(HEADER_CG_ITEM_USE_TO_ITEM, sizeof(TPacketCGItemUseToItem), "UseItemToItem");
    Set(HEADER_CG_TARGET, sizeof(TPacketCGTarget), "Target");
    Set(HEADER_CG_TARGET, sizeof(TPacketCGTarget), "Target");
    Set(HEADER_CG_TARGET_LOAD, sizeof(TPacketCGTargetLoad), "TargetLoad");
    Set(HEADER_CG_MESSENGER, sizeof(TPacketCGMessenger), "Messenger");

    Set(HEADER_CG_PARTY_REMOVE, sizeof(TPacketCGPartyRemove), "PartyRemove");
    Set(HEADER_CG_PARTY_INVITE, sizeof(TPacketCGPartyInvite), "PartyInvite");
    Set(HEADER_CG_PARTY_INVITE_ANSWER, sizeof(TPacketCGPartyInviteAnswer), "PartyInviteAnswer");
    Set(HEADER_CG_PARTY_SET_STATE, sizeof(TPacketCGPartySetState), "PartySetState");
    Set(HEADER_CG_PARTY_USE_SKILL, sizeof(TPacketCGPartyUseSkill), "PartyUseSkill");
    Set(HEADER_CG_PARTY_PARAMETER, sizeof(TPacketCGPartyParameter), "PartyParam");

    Set(HEADER_CG_SAFEBOX_CHECKOUT, sizeof(TPacketCGSafeboxCheckout), "SafeboxCheckout");
    Set(HEADER_CG_SAFEBOX_CHECKIN, sizeof(TPacketCGSafeboxCheckin), "SafeboxCheckin");

    Set(HEADER_CG_SAFEBOX_ITEM_MOVE, sizeof(TPacketCGItemMove), "SafeboxItemMove");

    Set(HEADER_CG_GUILD, sizeof(TPacketCGGuild), "Guild", false, true);
    Set(HEADER_CG_ANSWER_MAKE_GUILD, sizeof(TPacketCGAnswerMakeGuild), "AnswerMakeGuild");

    Set(HEADER_CG_FISHING, sizeof(TPacketCGFishing), "Fishing");
    Set(HEADER_CG_ITEM_GIVE, sizeof(TPacketCGGiveItem), "ItemGive");
    Set(HEADER_CG_HACK, sizeof(TPacketCGHack), "Hack");
    Set(HEADER_CG_MYSHOP, sizeof(TPacketCGMyShop), "MyShop");

    Set(HEADER_CG_REFINE, sizeof(TPacketCGRefine), "Refine");
    Set(HEADER_CG_CHANGE_NAME, sizeof(CgChangeNamePacket), "ChangeName");
    Set(HEADER_CG_CHANGE_EMPIRE, sizeof(CgChangeEmpirePacket), "ChangeEmpire");
    Set(HEADER_CG_BLOCK_MODE, sizeof(CgBlockModePacket), "BlockMode");

    Set(HEADER_CG_PONG, sizeof(uint8_t), "Pong");
    Set(HEADER_CG_MALL_CHECKOUT, sizeof(TPacketCGSafeboxCheckout), "MallCheckout");

    Set(HEADER_CG_SCRIPT_SELECT_ITEM, sizeof(TPacketCGScriptSelectItem), "ScriptSelectItem");

    Set(HEADER_CG_DRAGON_SOUL_REFINE, sizeof(TPacketCGDragonSoulRefine), "DragonSoulRefine");

    //HGuard
    Set(HEADER_CG_HGUARD_PARAMETERS, sizeof(TPacketCGHGuardSend), "HGuardParams");
    Set(HEADER_CG_HGUARD_CODE, sizeof(TPacketCGHGuardCodeSend), "HGuardCode");
    Set(HEADER_CG_PIN, sizeof(CgPinResponsePacket), "PinResponse");

    //pets
    Set(HEADER_CG_MYSHOP_WARP, sizeof(BlankPacket), "MyShopWarp");          //Header-only
    Set(HEADER_CG_MYSHOP_OPEN, sizeof(BlankPacket), "MyShopOpen");          //Header-only
    Set(HEADER_CG_MYSHOP_OPEN_SEARCH, sizeof(uint8_t), "MyShopOpenSearch"); //Header-only

    Set(HEADER_CG_ACCE, sizeof(BlankDynamicPacketWithSub), "Acce", false, true);
    Set(HEADER_CG_CHANGELOOK, sizeof(BlankDynamicPacketWithSub), "Changelook", false, true);
    Set(HEADER_CG_SWITCHBOT, sizeof(BlankDynamicPacketWithSub), "Switchbot", false, true);
    Set(HEADER_CG_MYSHOP_DECO, sizeof(BlankDynamicPacket), "MyShopDeco", false, true);

#ifdef ENABLE_MOVE_COSTUME_ATTR
    Set(HEADER_CG_ITEM_COMBINATION, sizeof(TPacketCGItemCombiation), "ItemCombination");
#endif

#ifdef ENABLE_GUILD_STORAGE
	Set(HEADER_CG_GUILDSTORAGE_CHECKOUT, sizeof(TPacketCGGuildStorageCheckout), "GuildStorageCheckout");
	Set(HEADER_CG_GUILDSTORAGE_CHECKIN, sizeof(TPacketCGGuildStorageCheckin), "GuildStorageCheckin");
	Set(HEADER_CG_GUILDSTORAGE_ITEM_MOVE, sizeof(TPacketCGItemMove), "GuildStorageItemMove");
	Set(HEADER_CG_GUILDSTORAGE_REQUEST_LOG, sizeof(BlankPacket), "GuildStorageLogRequest");
#endif
#ifdef ENABLE_DESTROY_ITEM_SYSTTEM
    Set(HEADER_CG_ITEM_DESTROY, sizeof(TPacketCGItemDestroy), "ItemDestroy");
#endif

#if defined(WJ_COMBAT_ZONE)
	Set(HEADER_CG_COMBAT_ZONE_REQUEST_ACTION, sizeof(TPacketCGCombatZoneRequestAction), "TPacketCGCombatZoneRequestAction");
#endif

#ifdef __OFFLINE_SHOP__
    Set(HEADER_CG_OFFLINE_SHOP, sizeof(TPacketCGOfflineShop), "OfflineShop");
    Set(HEADER_CG_OFFLINE_SHOP_CREATE, sizeof(TPacketCGOfflineShopCreate), "OfflineShopCreate", false, true);
    Set(HEADER_CG_OFFLINE_SHOP_NAME, sizeof(TPacketCGOfflineShopName), "OfflineShopName");
#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
	Set(HEADER_CG_OFFLINE_SHOP_POSITION, sizeof(TPacketCGOfflineShopPosition), "OfflineShopPosition");
#endif
    Set(HEADER_CG_OFFLINE_SHOP_ITEM_ADD, sizeof(TPacketCGOfflineShopAddItem), "OfflineShopAddItem");
    Set(HEADER_CG_OFFLINE_SHOP_ITEM_MOVE, sizeof(TPacketCGOfflineShopMoveItem), "OfflineShopMoveItem");
    Set(HEADER_CG_OFFLINE_SHOP_ITEM_REMOVE, sizeof(TPacketCGOfflineShopRemoveItem), "OfflineShopRemoveItem");
    Set(HEADER_CG_OFFLINE_SHOP_ITEM_BUY, sizeof(TPacketCGOfflineShopBuyItem), "OfflineShopBuyItem");
    Set(HEADER_CG_OFFLINE_SHOP_WITHDRAW_GOLD, sizeof(TPacketCGOfflineShopWithdrawGold), "OfflineShopWithdrawGold");
    Set(HEADER_CG_OFFLINE_SHOP_CLICK, sizeof(TPacketCGOfflineShopClickShop), "OfflineShopClickShop");
    Set(HEADER_CG_OFFLINE_SHOP_REOPEN, sizeof(TPacketCGOfflineShopReopen), "OfflineShopReopen");
#endif

    Set(HEADER_GC_REMOVE_METIN, sizeof(CgRemoveMetinPacket), "ItemRemoveMetin");
    Set(HEADER_CG_WIKI_REQUEST, sizeof(CgRecvWikiPacket), "WikiRequest");
    Set(HEADER_CG_SET_TITLE, sizeof(CgSetTitlePacket), "SetTitle");

#ifdef ENABLE_BATTLE_PASS
    Set(HEADER_CG_BATTLE_PASS, sizeof(TPacketCGBattlePassAction), "BattlePass");
#endif
#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
	Set(HEADER_CG_CUBE_RENEWAL, sizeof(TPacketCGCubeRenewalSend), "CubeRenewalSend");
#endif

    Set(HEADER_CG_CHAT_FILTER, sizeof(TPacketCGChatFilter), "ChatFilter");
    Set(HEADER_CG_LEVEL_PET, sizeof(BlankDynamicPacketWithSub), "LevelPet", false, true);
}

CPacketInfoCG::~CPacketInfoCG() { Log("packet_info.txt"); }

////////////////////////////////////////////////////////////////////////////////
CPacketInfoGG::CPacketInfoGG()
{
}

CPacketInfoGG::~CPacketInfoGG() { Log("p2p_packet_info.txt"); }
