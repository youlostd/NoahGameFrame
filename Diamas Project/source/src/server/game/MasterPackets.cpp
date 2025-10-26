#include "MasterPackets.hpp"
#include "desc_manager.h"
#include "map_location.h"
#include "char_manager.h"
#include "char.h"
#include "ChatUtil.hpp"
#include "questmanager.h"
#include "messenger_manager.h"
#include "guild_manager.h"
#include "guild.h"
#include "config.h"
#include "MasterSocket.hpp"
#include "MasterUtil.hpp"
#include "DbCacheSocket.hpp"
#include "input.h"

#include <game/MasterPackets.hpp>

#include "TextTagUtil.hpp"
#include "OfflineShopManager.h"
#include "party.h"
#include "marriage.h"
#include "gm.h"
#include <game/AffectConstants.hpp>


#include "item.h"
#include "item_manager.h"
#include "wedding.h"

bool HandleShutdownBroadcastPacket(MasterSocket *master,
                                   const MgShutdownBroadcastPacket &p)
{
    SPDLOG_ERROR("Received shutdown: {0}", p.reason);
    Shutdown(p.countdown);
    return true;
}

bool HandleMaintenanceBroadcastPacket(MasterSocket *master,
                                      const MgMaintenanceBroadcastPacket &p)
{
    const DESC_MANAGER::DESC_SET &c_set_desc = DESC_MANAGER::instance().GetClientSet();
    for (const auto d : c_set_desc)
    {
        if (d->GetCharacter())
        {
            TPacketGCMaintenance p2;
            p2.seconds = p.seconds;
            p2.duration = p.duration;
            d->Send(HEADER_GC_MAINTENANCE, p2);
        }
    }

    return true;
}

bool HandleHostedMapsPacket(MasterSocket *master, const MgHostedMapsPacket &p)
{
    SPDLOG_INFO("MAPS for {0}:{1} count {2}",
                p.publicAddr, p.publicPort, p.hostedMaps.size());

    for (auto mapIndex : p.hostedMaps)
        CMapLocation::instance().Insert(p.channel, mapIndex,
                                        std::string(p.publicAddr).c_str(),
                                        p.publicPort);

    return true;
}

bool HandleLoginSuccessPacket(MasterSocket *master,
                              const MgLoginSuccessPacket &p)
{
    SPDLOG_TRACE("MASTER: LoginSuccess for {0}", p.handle);

    const auto d = DESC_MANAGER::instance().FindByHandle(p.handle);
    if (!d)
    {
        SendLogoutPacket(p.aid);
        return true;
    }

    d->SetAid(p.aid);
    d->SetSocialId(p.socialId);
    d->SetPremiumTimes(p.premiumTimes);
    d->SetHWIDHash(p.hwid);

    DESC_MANAGER::instance().ConnectAccount(p.aid, d);


    GdPlayerListQueryPacket p2;
    p2.aid = p.aid;
    db_clientdesc->DBPacket(HEADER_GD_PLAYER_LIST_QUERY, d->GetHandle(),
                            &p2, sizeof(p2));
    return true;
}

bool HandleLoginFailurePacket(MasterSocket *master,
                              const MgLoginFailurePacket &p)
{
    SPDLOG_INFO("MASTER: LoginFailure for {0}", p.handle);

    const auto d = DESC_MANAGER::instance().FindByHandle(p.handle);
    if (!d)
        return true;

    LoginFailure(d, p.status.c_str());
    return true;
}

bool HandleLoginDisconnectPacket(MasterSocket *master,
                                 const MgLoginDisconnectPacket &p)
{
    SPDLOG_TRACE("MASTER: Login DC for {0}", p.aid);

    const auto d = DESC_MANAGER::instance().FindByAid(p.aid);
    if (!d)
        return true;

    d->DisconnectOfSameLogin();
    return true;
}

bool HandleCharacterEnterPacket(MasterSocket *master,
                                const MgCharacterEnterPacket &p)
{
    SPDLOG_TRACE("Enter: {0} {1} ch {2} map {3}",
                 p.pid, p.name, p.channel, p.mapIndex);

    OnlinePlayer op;
    op.aid = p.aid;
    op.pid = p.pid;
    op.name = p.name;
    op.mapIndex = p.mapIndex;
    op.empire = p.empire;
    op.channel = p.channel;
    op.hwid = p.hwid;
    DESC_MANAGER::instance().GetOnlinePlayers().Enter(std::move(op));

    COfflineShopManager::instance().LoginOwner(p.pid, true);
    CGuildManager::instance().P2PLoginMember(p.pid);
    CPartyManager::instance().P2PLogin(p.pid, p.name.c_str(), p.mapIndex, p.channel);
    MessengerManager::instance().P2PLogin(p.name);

    return true;
}

bool HandleCharacterLeavePacket(MasterSocket *master,
                                const MgCharacterLeavePacket &p)
{
    SPDLOG_INFO("Leave: {0}", p.pid);

    auto op = DESC_MANAGER::instance().GetOnlinePlayers().Get(p.pid);

    if (op)
    {
        CGuildManager::instance().P2PLogoutMember(p.pid);
        CPartyManager::instance().P2PLogout(p.pid, op->mapIndex, op->channel);
        MessengerManager::instance().P2PLogout(op->name);
        marriage::CManager::instance().Logout(p.pid);
    }

    DESC_MANAGER::instance().GetOnlinePlayers().Leave(p.pid);

    return true;
}

bool HandleCharacterDisconnectPacket(MasterSocket *master,
                                     const MgCharacterDisconnectPacket &p)
{
    const auto ch = g_pCharManager->FindByPID(p.pid);
    if (!ch)
        return true;

    ch->Disconnect("MASTER: manual");
    return true;
}

bool HandleCharacterWarpPacket(MasterSocket *master,
                               const MgCharacterWarpPacket &p)
{
    const auto ch = g_pCharManager->FindByPID(p.pid);
    if (!ch)
        return true;

    ch->WarpSet(p.mapIndex, p.x, p.y, p.channel);
    return true;
}

bool HandleCharacterTransferPacket(MasterSocket *master,
                                   const MgCharacterTransferPacket &p)
{
    const auto ch = g_pCharManager->FindByPID(p.targetPid);
    if (!ch)
        return true;

    if (marriage::WeddingManager::instance().IsWeddingMap(ch->GetMapIndex()))
        return true;

    GmCharacterWarpPacket p2;
    p2.pid = p.sourcePid;
    p2.mapIndex = ch->GetMapIndex();
    p2.x = ch->GetX();
    p2.y = ch->GetY();
    p2.channel = gConfig.channel;
    master->Send(kGmCharacterWarp, p2);
    return true;
}

bool HandleMonarchCharacterWarpPacket(MasterSocket *master,
                                      const MgCharacterWarpPacket &p)
{
    const auto ch = g_pCharManager->FindByPID(p.pid);
    if (!ch)
        return true;

    auto &qm = quest::CQuestManager::instance();

    const auto qIndex = qm.GetQuestIndexByName("monarch_transfer");
    if (qIndex != 0)
    {
        ch->SetQuestFlag("monarch_transfer.x", p.x);
        ch->SetQuestFlag("monarch_transfer.y", p.y);
        qm.Letter(p.pid, qIndex, 0);
    }

    return true;
}

bool HandleMessengerAddPacket(MasterSocket *master,
                              const MgMessengerAddPacket &p)
{
    MessengerManager::instance().__AddToList(p.name,
                                             p.companion);
    return true;
}

bool HandleMessengerRequestAddPacket(MasterSocket *master,
                                     const MgMessengerRequestAddPacket &p)
{
    const auto ch = g_pCharManager->FindPC(p.companion);
    if (!ch)
        return true;

    if (quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID())->IsRunning())
        return true;

    if (GM::get_level(p.account.c_str()) > GM_PLAYER && !ch->IsGM() && !GM::check_allow(
            ch->GetGMLevel(), GM_ALLOW_ADD_PLAYER_AS_FRIEND)) { return true; }

    if (!(GM::get_level(p.account.c_str()) > GM_PLAYER) && ch->IsGM() && !GM::check_allow(
            ch->GetGMLevel(), GM_ALLOW_ADD_GM_AS_FRIEND)) { return true; }

    if (ch->IsBlockMode(BLOCK_MESSENGER_INVITE)) { return true; }

    MessengerManager::instance().RequestToAdd(p.account,
                                              p.companion);

    ch->ChatPacket(CHAT_TYPE_COMMAND, "messenger_auth %s", p.account.c_str());

    return true;
}

bool HandleMessengerAuthAddPacket(MasterSocket *master,
                                  const MgMessengerAuthAddPacket &p)
{
    MessengerManager::instance().AuthToAdd(p.account,
                                           p.companion, p.deny);
    return true;
}

bool HandleMessengerDelPacket(MasterSocket *master,
                              const MgMessengerDelPacket &p)
{
    MessengerManager::instance().__RemoveFromList(p.name,
                                                  p.companion);
    return true;
}

bool HandleGuildChatPacket(MasterSocket *master, const MgGuildChatPacket &p)
{
    const auto g = CGuildManager::instance().FindGuild(p.gid);
    if (!g)
        return true;

    std::string message = p.message;

    switch (p.type)
    {
    case MgGuildChatPacket::kGuildChat:
        g->MasterChat(message.c_str(), "");
        break;

    case MgGuildChatPacket::kGuildChatLocalized:
        g->MasterChatLocalized(message.c_str());
        break;

    default:
        break;
    }

    return true;
}

bool HandleGuildMemberCountBonusPacket(MasterSocket *master,
                                       const MgGuildMemberCountBonusPacket &p)
{
    const auto g = CGuildManager::instance().FindGuild(p.gid);
    if (!g)
        return true;

    g->SetMemberCountBonus(p.bonus);
    return true;
}

bool HandleGuildWarZoneInfoPacket(MasterSocket *master,
                                  const MgGuildWarZoneInfoPacket &p)
{
    auto &gm = CGuildManager::instance();

    const auto g1 = gm.FindGuild(p.gid1);
    const auto g2 = gm.FindGuild(p.gid2);
    if (g1 && g2)
    {
        g1->SetGuildWarMapIndex(p.gid2, p.mapIndex);
        g2->SetGuildWarMapIndex(p.gid1, p.mapIndex);
    }

    return true;
}

bool HandleChatPacket(MasterSocket *master, const MgChatPacket &p)
{
    const auto ch = g_pCharManager->FindByPID(p.pid);
    if (!ch)
        return true;

    TPacketGCChat pack;
    pack.type = p.type;
    pack.id = 0;
    pack.bEmpire = ch->GetEmpire();
    pack.message = p.message;
    ch->GetDesc()->Send(HEADER_GC_CHAT, pack);

    return true;
}

bool HandleWhisperErrorPacket(MasterSocket *master, const MgWhisperErrorPacket &p)
{
    const auto ch = g_pCharManager->FindByPID(p.pid);
    if (!ch)
        return true;

    TPacketGCWhisper pack{};
    pack.bType = p.type;
    pack.szNameFrom = p.to;
    ch->GetDesc()->Send(HEADER_GC_WHISPER, pack);
    return true;
}

bool HandleWhisperPacket(MasterSocket *master, const MgWhisperPacket &p)
{
    auto* target = g_pCharManager->FindByPID(p.targetPid);
    if (!target)
        return true;

    if (p.type == WHISPER_TYPE_NORMAL && target->IsBlockMode(BLOCK_WHISPER, p.sourcePid))
    {
        GmWhisperErrorPacket p2;
        p2.pid = p.sourcePid;
        p2.type = WHISPER_TYPE_TARGET_BLOCKED;
        p2.to = target->GetName();
        master->Send(kGmWhisperError, p2);
        return true;
    }

	SendWhisperPacket(target, p.type, p.message, p.localeCode, p.sourcePid, p.from);   
    return true;
}

bool HandleShoutPacket(MasterSocket *master, const MgShoutPacket &p)
{
    std::string message = p.message;
    SendShout(message.c_str(), p.empire, p.lang);
    return true;
}

bool HandleNoticePacket(MasterSocket *master, const MgNoticePacket &p)
{
    std::string message = p.message;

    switch (p.type)
    {
    case NOTICE_TYPE_DEFAULT:
        SendNotice(message.c_str());
        break;
    case NOTICE_TYPE_BIG:
        SendBigNotice(p.message.c_str(), -1);
        break;
    case NOTICE_TYPE_FORMATTED:
        SendNoticeSpecial(p.message.c_str());
        break;
    case NOTICE_TYPE_WHISPER:
        SendWhisperAll(p.message.c_str());
        break;
    default:
        break;
    }

    return true;
}

bool HandleBlockChatPacket(MasterSocket *master, const MgBlockChatPacket &p)
{
    const auto ch = g_pCharManager->FindByPID(p.pid);
    if (!ch)
        return true;

    ch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, p.duration, 0, true);
    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "Chat has been banned on the operator form.");
    return true;
}

bool HandleOfflineShopBuyInfoPacket(MasterSocket *master, const MgOfflineShopBuyInfoPacket &p)
{
    const auto ch = g_pCharManager->FindByPID(p.ownerPid);
    if (!ch)
        return true;

    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You sold %dx %s for %lld Yang.", p.count, TextTag::itemname(p.vnum).c_str(),
                       p.price);
    return true;
}

bool HandleOfflineShopTimePacket(MasterSocket *master, const MgOfflineShopTimePacket &p)
{
    COfflineShopManager::instance().ReceiveP2PFarmOpeningTimePacket(p);
    return true;
}

bool HandleWorldBossPacket(MasterSocket *master, const MgWorldBossPacket &p)
{
    g_pCharManager->SpawnWorldBoss(p.bossIndex, p.mapIndex, p.x, p.y);
    return true;
}

bool HandleHyperlinkItemResult(MasterSocket *master, const GmHyperlinkResultPacket &p) {

    for(const auto& d : p.data) {
        ITEM_MANAGER::instance().RegisterHyperlink(d.id, d);
        CgHyperlinkItemPacket p;
        p.d = d;
        BroadcastPacket(DESC_MANAGER::instance().GetClientSet(), HEADER_GC_HYPERLINK_ITEM, p);
    }

    return true;
}

bool HandleDropStatusPacket(MasterSocket *master, const MgDropStatusPacket&p) {

    auto d = DESC_MANAGER::instance().FindByAid(p.aid);
    if(d) {
        d->SetDropsAllowed(p.state);
    }

    return true;
}


bool HandleHyperlinkItemRequest(MasterSocket *master, const GmHyperlinkRequestPacket &p)
{
    std::vector<ClientItemData> v;
    for(const auto& id : p.ids) {
        auto item = ITEM_MANAGER::instance().Find(id);
        if(item) {
            ClientItemData d;
            d.id = id;
            d.vnum = item->GetVnum();
            d.count = item->GetCount();
            d.highlighted = false;
            d.nSealDate = item->GetSealDate();
            d.transVnum = item->GetTransmutationVnum();
            std::memcpy(d.attrs, item->GetAttributes(), sizeof(d.attrs));
            std::memcpy(d.sockets, item->GetSockets(), sizeof(d.sockets));

            ITEM_MANAGER::instance().RegisterHyperlink(id, d);
            CgHyperlinkItemPacket p;
            p.d = d;
            BroadcastPacket(DESC_MANAGER::instance().GetClientSet(), HEADER_GC_HYPERLINK_ITEM, p);
            v.emplace_back(d);
        }
    }

    GmHyperlinkResultPacket p2 = {};
    p2.data = v;
    master->Send(kGmHyperlinkResult, p2);
    
    return true;
}