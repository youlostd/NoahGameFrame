#ifndef METIN2_SERVER_GAME_INPUT_H
#define METIN2_SERVER_GAME_INPUT_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/DbPackets.hpp>
#include <game/GamePacket.hpp>

struct PacketHeader;
class CHARACTER;
class DESC;
void LoginFailure(DESC *d, const char *c_pszStatus);

class CInputProcessor
{
  public:
    CInputProcessor();

    virtual ~CInputProcessor() {}

    virtual bool Analyze(DESC *d, const PacketHeader &header, const boost::asio::const_buffer &data);

    void Pong(DESC *d);
    int Text(DESC *d, const std::string &c_pData);
    void Handshake(DESC *d, const TPacketCGHandshake &p);
};

class CInputClose : public CInputProcessor
{
  protected:
    bool Analyze(DESC *d, const PacketHeader &header, const boost::asio::const_buffer &data) override { return true; }
};

class CInputPhase : public CInputProcessor
{
  protected:
    bool Analyze(DESC *d, const PacketHeader &header, const boost::asio::const_buffer &data) override;

  private:
    void RecvPhaseAck(DESC *d);
};

class CInputHandshake : public CInputProcessor
{
  public:
    CInputHandshake();
    virtual ~CInputHandshake();

  protected:
    bool Analyze(DESC *d, const PacketHeader &header, const boost::asio::const_buffer &data) override;
    void GuildMarkLogin(DESC *d, const char *c_pData);

};

class CInputLogin : public CInputProcessor
{
  protected:
    bool Analyze(DESC *d, const PacketHeader &header, const boost::asio::const_buffer &data) override;

  protected:
    void Login(DESC *d, const char *data);
    void LoginByKey(DESC *d, const char *data);

    void CharacterSelect(DESC *d, const TPacketCGPlayerSelect &p);
    void CharacterCreate(DESC *d, const TPacketCGPlayerCreate &p);
    void CharacterDelete(DESC *d, const TPacketCGPlayerDelete &p);
    void Entergame(DESC *d, const TPacketCGEnterGame &p);
    void GuildMarkCRCList(DESC *d, const TPacketCGMarkCRCList &pCG);
    // MARK_BUG_FIX
    void GuildMarkIDXList(DESC *d, const TPacketCGMarkIDXList &p);
    // END_OF_MARK_BUG_FIX
    void GuildMarkUpload(DESC *d, TPacketCGMarkUpload p);
    int GuildSymbolUpload(DESC *d, const TPacketCGGuildSymbolUpload &p);
    void GuildSymbolCRC(DESC *d, const TPacketCGSymbolCRC &p);
    void ChangeName(DESC *d, const CgChangeNamePacket &p);
    void LoginByKey(DESC *d, const CgKeyLoginPacket &p);
    void ChangeEmpire(DESC *d, const CgChangeEmpirePacket &p);
};

class CInputMain : public CInputProcessor
{
  protected:
    bool Analyze(DESC *d, const PacketHeader &header, const boost::asio::const_buffer &data) override;

    bool SyncPosition(CHARACTER *ch, const TPacketCGSyncPosition &p);

  protected:
    void Attack(CHARACTER *ch, const TPacketCGAttack &p);
    void Shoot(CHARACTER *ch, const TPacketCGShoot &p);

  void SendOfflineMessage(CHARACTER* ch, const TPacketCGWhisper& p);
    bool Whisper(CHARACTER *ch, const TPacketCGWhisper &p);
    bool Chat(CHARACTER *ch, const TPacketCGChat &p);
    void SetTitle(CHARACTER *ch, const CgSetTitlePacket &p);
    void ItemUse(CHARACTER *ch, const TPacketCGItemUse &p);
    void ItemUseMultiple(CHARACTER *ch, const TPacketCGItemUse &p);

    void ItemDrop(CHARACTER *ch, const TPacketCGItemDrop &p);
    void ItemDrop2(CHARACTER *ch, const TPacketCGItemDrop2 &p);
#ifdef ENABLE_DESTROY_ITEM_SYSTTEM
    void ItemDestroy(CHARACTER *ch, const TPacketCGItemDestroy &p);
    void ItemRemoveMetin(CHARACTER *ch, const CgRemoveMetinPacket &p);
#endif
    void ItemMove(CHARACTER *ch, const TPacketCGItemMove &p);
    void ItemSplit(CHARACTER *ch, const TPacketCGItemSplit &p);
    void ItemPickup(CHARACTER *ch, const TPacketCGItemPickup &p);
    void ItemToItem(CHARACTER *ch, const TPacketCGItemUseToItem &p);
    void QuickslotAdd(CHARACTER *ch, const TPacketCGQuickslotAdd &p);
    void QuickslotDelete(CHARACTER *ch, const TPacketCGQuickslotDel &p);
    void QuickslotSwap(CHARACTER *ch, const TPacketCGQuickslotSwap &p);
    void Shop(CHARACTER *ch, const TPacketCGShop &p);
    bool OnClick(CHARACTER *ch, const TPacketCGOnClick &p);
    bool Exchange(CHARACTER *ch, const TPacketCGExchange &p);

    void Move(CHARACTER *ch, const TPacketCGMove &p);
    void FlyTarget(CHARACTER *ch, const TPacketCGFlyTargeting &p, uint8_t header);
    void UseSkill(CHARACTER *ch, const TPacketCGUseSkill &p);
#ifdef ENABLE_BATTLE_PASS
    void BattlePass(CHARACTER *ch, const TPacketCGBattlePassAction &p);
#endif

    void ChooseSkillGroup(CHARACTER *ch, const TPacketCGChooseSkillGroup &p);
    void ChangeSkillColor(CHARACTER *ch, const TPacketCGChangeSkillColorPacket &p);

    void ScriptAnswer(CHARACTER *ch, const TPacketCGScriptAnswer &p);
    void ScriptButton(CHARACTER *ch, const TPacketCGScriptButton &p);
    void ScriptSelectItem(CHARACTER *ch, const TPacketCGScriptSelectItem &p);

    void QuestInputString(CHARACTER *ch, const TPacketCGQuestInputString &p);
    void QuestConfirm(CHARACTER *ch, const TPacketCGQuestConfirm &p);
    void QuestReceive(CHARACTER *ch, const TPacketCGQuestRcv &p);

    void Target(CHARACTER *ch, const TPacketCGTarget &p);
    void LoadTargetInfo(CHARACTER *pkCharacter, const TPacketCGTargetLoad &p);
    void SafeboxCheckin(CHARACTER *ch, const TPacketCGSafeboxCheckin &p);
    void SafeboxCheckout(CHARACTER *ch, const TPacketCGSafeboxCheckout &p, bool bMall);
    void SafeboxItemMove(CHARACTER *ch, const TPacketCGItemMove &p);

#ifdef ENABLE_GUILD_STORAGE
    void GuildStorageCheckin(CHARACTER *ch, const char *c_pData);
    void GuildStorageCheckout(CHARACTER *ch, const char *c_pData);
    void GuildStorageItemMove(CHARACTER *ch, const char *data);
    void GuildStorageRequestLogs(CHARACTER *ch, const char *c_pData);
#endif
    bool Messenger(CHARACTER *ch, const TPacketCGMessenger &p);
    void BlockMode(CHARACTER *ch, const CgBlockModePacket &p);

    void PartyInvite(CHARACTER *ch, const TPacketCGPartyInvite &p);
    void PartyInviteAnswer(CHARACTER *ch, const TPacketCGPartyInviteAnswer &p);
    void PartyRemove(CHARACTER *ch, const TPacketCGPartyRemove &p);
    void PartySetState(CHARACTER *ch, const TPacketCGPartySetState &p);
    void PartyUseSkill(CHARACTER *ch, const TPacketCGPartyUseSkill &p);
    void PartyParameter(CHARACTER *ch, const TPacketCGPartyParameter &p);

    int Guild(CHARACTER *ch, const TPacketCGGuild &p);
    void FishingGame(CHARACTER *ch, const TPacketCGFishingGame &p);
    void AnswerMakeGuild(CHARACTER *ch, const TPacketCGAnswerMakeGuild &p);

    void Fishing(CHARACTER *ch, const TPacketCGFishing &p);
    void ItemGive(CHARACTER *ch, const TPacketCGGiveItem &p);
    int MyShop(CHARACTER *ch, const CgMyShopOpen &p);
    void Hack(CHARACTER *ch, const TPacketCGHack &p);

    void Refine(CHARACTER *ch, const TPacketCGRefine &p);

#ifdef __OFFLINE_SHOP__
    void ReceiveOfflineShopPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShop &p);
    int ReceiveOfflineShopCreatePacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopCreate &p);
    void ReceiveOfflineShopNamePacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopName &p);
#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
    void ReceiveOfflineShopPositionPacket(CHARACTER *pkCharacter, TPacketCGOfflineShopPosition *pkData);
#endif
    void ReceiveOfflineShopAddItemPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopAddItem &p);
    void ReceiveOfflineShopMoveItemPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopMoveItem &p);
    void ReceiveOfflineShopRemoveItemPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopRemoveItem &p);
    void ReceiveOfflineShopBuyItemPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopBuyItem &p);
    void ReceiveOfflineShopWithdrawGoldPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopWithdrawGold &p);
    void ReceiveOfflineShopClickShopPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopClickShop &p);
    void ReceiveOfflineShopReopenPacket(CHARACTER *pkCharacter, const TPacketCGOfflineShopReopen &p);
#endif

    void OpenPlayerShop(CHARACTER *ch);
    void OpenPlayerShopSearch(CHARACTER *ch);

    int MyShopDeco(CHARACTER *ch, const TPacketCGMyShopDeco &p);
    int Switchbot(CHARACTER *ch, const CgSwitchbotPacket &p);
    int Changelook(CHARACTER *ch, const CgChangeLookPacket &p);
    int LevelPet(CHARACTER *ch, const CgLevelPetPacket &p);

    int AcceRefine(CHARACTER *ch, const TPacketCGAcce &p);
#ifdef ENABLE_MOVE_COSTUME_ATTR
    void ItemCombination(CHARACTER *ch, const TPacketCGItemCombiation &p);
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
    void ShopSearch(CHARACTER *ch, const TPacketCGShopSearch &p);
#endif
#ifdef __ADMIN_MANAGER__
    int AdminManager(CHARACTER *ch, const char *c_pData, size_t uiBytes);
#endif

    int SyncPosition(CHARACTER *ch, const char *c_pcData, size_t uiBytes);
    void ChatFilter(CHARACTER *ch, const TPacketCGChatFilter &c_pData);
    void MyShopWarp(CHARACTER *ch);
    void WikiRequest(CHARACTER *ch, const CgRecvWikiPacket &p);
};

class CInputDead : public CInputMain
{
  protected:
    bool Analyze(DESC *d, const PacketHeader &header, const boost::asio::const_buffer &data) override;
};

std::string HandleSocketCmd(DESC *d, const std::string &cmd);
bool HandleDbPacket(const DbPacketHeader &header, const char *data);

#endif /* METIN2_SERVER_GAME_INPUT_H */
