#ifndef METIN2_SERVER_MASTER_GAMEPACKETS_HPP
#define METIN2_SERVER_MASTER_GAMEPACKETS_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/MasterPackets.hpp>


class GameSocket;

bool HandleSetupPacket(GameSocket* game, const GmSetupPacket& p);
bool HandleShutdownBroadcastPacket(GameSocket* game,
                                   const GmShutdownBroadcastPacket& p);
bool HandleMaintenanceBroadcastPacket(GameSocket* game,
                                   const GmMaintenanceBroadcastPacket& p);

bool HandleLoginPacket(GameSocket* game, const GmLoginPacket& p);
bool HandleLogoutPacket(GameSocket* game, const GmLogoutPacket& p);

bool HandleCharacterEnterPacket(GameSocket* game,
                                const GmCharacterEnterPacket& p);
bool HandleCharacterLeavePacket(GameSocket* game,
                                const GmCharacterLeavePacket& p);
bool HandleCharacterDisconnectPacket(GameSocket* game,
                                     const GmCharacterDisconnectPacket& p);
bool HandleCharacterWarpPacket(GameSocket* game,
                               const GmCharacterWarpPacket& p);
bool HandleCharacterTransferPacket(GameSocket* game,
                                   const GmCharacterTransferPacket& p);
bool HandleMonarchCharacterWarpPacket(GameSocket* game,
                                      const GmCharacterWarpPacket& p);

bool HandleMessengerAddPacket(GameSocket* game,
                              const GmMessengerAddPacket& p);
bool HandleMessengerRequestAddPacket(GameSocket* game,
                              const GmMessengerRequestAddPacket& p);
bool HandleMessengerAuthAddPacket(GameSocket* game,
                              const GmMessengerAuthAddPacket& p);
bool HandleMessengerDelPacket(GameSocket* game,
                              const GmMessengerDelPacket& p);

bool HandleGuildChatPacket(GameSocket* game, const GmGuildChatPacket& p);
bool HandleGuildMemberCountBonusPacket(GameSocket* game,
                                       const GmGuildMemberCountBonusPacket& p);
bool HandleGuildWarZoneInfoPacket(GameSocket* game,
                                  const GmGuildWarZoneInfoPacket& p);

bool HandleChatPacket(GameSocket* game, const GmChatPacket& p);
bool HandleWhisperPacket(GameSocket* game, const GmWhisperPacket& p);
bool HandleShoutPacket(GameSocket* game, const GmShoutPacket& p);
bool HandleNoticePacket(GameSocket* game, const GmNoticePacket& p);
bool HandleBlockChatPacket(GameSocket* game, const GmBlockChatPacket& p);
bool HandleOfflineShopBuyInfoPacket(GameSocket* game, const GmOfflineShopBuyInfoPacket& p);
bool HandleOfflineTimePacket(GameSocket* game, const GmOfflineShopTimePacket& p);
bool HandleWhisperErrorPacket(GameSocket* game, const GmWhisperErrorPacket& p);
bool HandleWorldBossPacket(GameSocket* game, const GmWorldBossPacket& p);
bool HandleHyperlinkRequestPacket(GameSocket* game, const GmHyperlinkRequestPacket& p);
bool HandleHyperlinkResultPacket(GameSocket* game, const GmHyperlinkResultPacket& p);



#endif
