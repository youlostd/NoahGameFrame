#ifndef METIN2_SERVER_GAME_MASTERPACKETS_HPP
#define METIN2_SERVER_GAME_MASTERPACKETS_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/MasterPackets.hpp>

class MasterSocket;

bool HandleShutdownBroadcastPacket(MasterSocket *master,
                                   const MgShutdownBroadcastPacket &p);

bool HandleMaintenanceBroadcastPacket(MasterSocket *master,
                                      const MgMaintenanceBroadcastPacket &p);

bool HandleHostedMapsPacket(MasterSocket *master, const MgHostedMapsPacket &p);

bool HandleLoginSuccessPacket(MasterSocket *master,
                              const MgLoginSuccessPacket &p);
bool HandleLoginFailurePacket(MasterSocket *master,
                              const MgLoginFailurePacket &p);
bool HandleLoginDisconnectPacket(MasterSocket *master,
                                 const MgLoginDisconnectPacket &p);

bool HandleCharacterEnterPacket(MasterSocket *master,
                                const MgCharacterEnterPacket &p);
bool HandleCharacterLeavePacket(MasterSocket *master,
                                const MgCharacterLeavePacket &p);
bool HandleCharacterWarpPacket(MasterSocket *master,
                               const MgCharacterWarpPacket &p);
bool HandleCharacterDisconnectPacket(MasterSocket *master,
                                     const MgCharacterDisconnectPacket &p);
bool HandleCharacterTransferPacket(MasterSocket *master,
                                   const MgCharacterTransferPacket &p);
bool HandleMonarchCharacterWarpPacket(MasterSocket *master,
                                      const MgCharacterWarpPacket &p);

bool HandleMessengerAddPacket(MasterSocket *master,
                              const MgMessengerAddPacket &p);

bool HandleMessengerRequestAddPacket(MasterSocket *master,
                                     const MgMessengerRequestAddPacket &p);

bool HandleMessengerAuthAddPacket(MasterSocket *master,
                                  const MgMessengerAuthAddPacket &p);
bool HandleMessengerDelPacket(MasterSocket *master,
                              const MgMessengerDelPacket &p);

bool HandleGuildChatPacket(MasterSocket *master, const MgGuildChatPacket &p);
bool HandleGuildMemberCountBonusPacket(MasterSocket *master,
                                       const MgGuildMemberCountBonusPacket &p);
bool HandleGuildWarZoneInfoPacket(MasterSocket *master,
                                  const MgGuildWarZoneInfoPacket &p);

bool HandleChatPacket(MasterSocket *master, const MgChatPacket &p);
bool HandleWhisperPacket(MasterSocket *master, const MgWhisperPacket &p);
bool HandleWhisperErrorPacket(MasterSocket *master, const MgWhisperErrorPacket &p);
bool HandleShoutPacket(MasterSocket *master, const MgShoutPacket &p);
bool HandleNoticePacket(MasterSocket *master, const MgNoticePacket &p);
bool HandleBlockChatPacket(MasterSocket *master, const MgBlockChatPacket &p);
bool HandleOfflineShopBuyInfoPacket(MasterSocket *master, const MgOfflineShopBuyInfoPacket &p);
bool HandleOfflineShopTimePacket(MasterSocket *master, const MgOfflineShopTimePacket &p);
bool HandleWorldBossPacket(MasterSocket *master, const MgWorldBossPacket &p);
bool HandleHyperlinkItemRequest(MasterSocket *master, const GmHyperlinkRequestPacket &p);
bool HandleHyperlinkItemResult(MasterSocket *master, const GmHyperlinkResultPacket &p);
bool HandleDropStatusPacket(MasterSocket *master, const MgDropStatusPacket&p);


#endif
