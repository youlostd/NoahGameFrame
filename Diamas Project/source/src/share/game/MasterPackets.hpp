#ifndef METIN2_SHARE_GAME_MASTERPACKETS_HPP
#define METIN2_SHARE_GAME_MASTERPACKETS_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <string>
#include <vector>
#include <boost/fusion/include/adapt_struct.hpp>

#include <game/Constants.hpp>
#include <game/Types.hpp>
#include <game/ItemTypes.hpp>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// You need to update the BOOST_FUSION_ADAPT_STRUCT calls at the end of the
// file if you modify any of the packets. Otherwise everything will break!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//
// Auth -> Master
//

enum AuthToMasterPackets
{
	kAmNone,
	kAmLogin,
};

struct AmLoginPacket
{
	uint32_t handle;
	uint32_t aid;
	std::string login;
	std::string socialId;
	uint32_t premiumTimes[PREMIUM_MAX_NUM];
	std::string hwid;
};

//
// Master -> Auth
//

enum MasterToAuthPackets
{
	kMaNone,
	kMaLoginSuccess,
	kMaLoginFailure,
};

struct MaLoginSuccessPacket
{
	uint32_t handle;
	SessionId sessionId;
};

struct MaLoginFailurePacket
{
	uint32_t handle;
	std::string status;
};

//
// Game -> Master
//

enum GameToMasterPackets
{
	kGmNone,
	kGmSetup,
	kGmShutdownBroadcast,
	kGmLogin,
	kGmLogout,
	kGmCharacterEnter,
	kGmCharacterLeave,
	kGmCharacterDisconnect,
	kGmCharacterWarp,
	kGmCharacterTransfer,
	kGmMonarchCharacterWarp,
	kGmMessengerAdd,
	kGmMessengerRequestAdd,
	kGmMessengerAuthAdd,
	kGmMessengerDel,
	kGmGuildChat,
	kGmGuildMemberCountBonus,
	kGmGuildWarZoneInfo,
	kGmChat,
	kGmWhisper,
	kGmWhisperError,
	kGmShout,
	kGmNotice,
	kGmBlockChat,
	kGmWhisperAll,
	kGmOfflineShopBuyInfo,
	kGmOfflineShopTime,
	kGmMaintenanceBroadcast,
	kGmWorldBoss,
	kGmHyperlinkItemRequest,
	kGmHyperlinkResult,
};

struct GmSetupPacket
{
	enum { kVersion = 1 };

	uint8_t version;

	uint8_t channel;

	// Address clients should attempt to connect to.
	std::string publicAddr;
	uint16_t publicPort;

	// Maps hosted on this server instance.
	std::vector<uint32_t> hostedMaps;
};

struct GmShutdownBroadcastPacket
{
	uint32_t countdown;
	std::string reason;
};

struct GmMaintenanceBroadcastPacket
{
	uint32_t seconds;
	uint32_t duration;
};

struct GmLoginPacket
{
	uint32_t handle;
	std::string login;
	SessionId sessionId;
};

struct GmLogoutPacket
{
	uint32_t aid;
};

struct GmCharacterEnterPacket
{
	uint32_t pid;
	uint32_t aid;
	std::string name;
	uint8_t empire;
	uint32_t mapIndex;
	uint8_t channel;
	std::string hwid;
};

struct GmCharacterLeavePacket
{
	uint32_t pid;
};

struct GmHyperlinkRequestPacket
{
    std::vector<uint32_t> ids;
};

struct GmHyperlinkResultPacket
{
    std::vector<ClientItemData> data;
};

struct GmCharacterDisconnectPacket
{
	uint32_t pid;
};

struct GmCharacterWarpPacket
{
	uint32_t pid;
	uint32_t mapIndex;
	uint32_t x, y;
	uint8_t channel;
};

struct GmCharacterTransferPacket
{
	uint32_t sourcePid;
	uint32_t targetPid;
};

struct GmMessengerAddPacket
{
	std::string name;
	std::string companion;
};

using GmMessengerDelPacket = GmMessengerAddPacket;

struct GmMessengerRequestAddPacket
{
	std::string account;
	std::string companion;
};

struct GmMessengerAuthAddPacket
{
	std::string account;
	std::string companion;
	uint8_t deny;
};


struct GmGuildChatPacket
{
	enum GuildChatType
	{
		kGuildChat,
		kGuildChatLocalized
	};

	uint8_t type;
	uint32_t gid;
	std::string message;
};

struct GmGuildMemberCountBonusPacket
{
	uint32_t gid;
	uint32_t bonus;
};

struct GmGuildWarZoneInfoPacket
{
	uint32_t gid1, gid2;
	uint32_t mapIndex;
};

struct GmChatPacket
{
	uint32_t pid;
	uint8_t type;
	std::string message;
};

struct GmWhisperPacket
{
	uint32_t sourcePid;
	uint32_t targetPid;
	uint32_t localeCode;
	uint8_t type;
	std::string from;
	std::string message;
};

struct GmWhisperErrorPacket
{
	uint32_t pid;
	uint8_t type;
	std::string to;
};

struct GmWhisperAllPacket
{
	std::string message;
};

struct GmShoutPacket
{
	uint8_t empire;
	uint8_t lang;
	std::string message;
};

struct GmNoticePacket
{
	uint8_t empire;
	uint8_t type;
	std::string message;
};

struct GmBlockChatPacket
{
	uint32_t pid;
	uint32_t duration;
};

struct GmOfflineShopBuyInfoPacket
{
	uint32_t ownerPid;
	uint32_t vnum;
	CountType count;
	Gold price;
};

struct GmOfflineShopTimePacket
{
	uint8_t type;
	uint32_t ownerPid;
	int32_t openingTime;
};

struct GmWorldBossPacket
{
	uint32_t bossIndex;
	uint32_t mapIndex;
	uint32_t x;
	uint32_t y;
};

//
// Master -> Game
//

enum MasterToGamePackets
{
	kMgNone,
	kMgShutdownBroadcast,
	kMgHostedMaps,
	kMgLoginSuccess,
	kMgLoginFailure,
	kMgLoginDisconnect,
	kMgCharacterEnter,
	kMgCharacterLeave,
	kMgCharacterDisconnect,
	kMgCharacterWarp,
	kMgCharacterTransfer,
	kMgMonarchCharacterWarp,
	kMgMessengerAdd,
	kMgMessengerRequestAdd,
	kMgMessengerAuthAdd,
	kMgMessengerDel,
	kMgGuildChat,
	kMgGuildMemberCountBonus,
	kMgGuildWarZoneInfo,
	kMgChat,
	kMgWhisper,
	kMgWhisperError,
	kMgShout,
	kMgNotice,
	kMgBlockChat,
	kMgOfflineShopBuyInfo,
	kMgOfflineShopTime,
	kMgMaintenanceBroadcast,
	kMgWorldBoss,
	kMgHyperlinkItemRequest,
	kMgHyperlinkResult,
	kMgDropStatus,
};

using MgShutdownBroadcastPacket = GmShutdownBroadcastPacket;
using MgMaintenanceBroadcastPacket = GmMaintenanceBroadcastPacket;

using MgWorldBossPacket = GmWorldBossPacket;


struct MgHostedMapsPacket
{
	uint8_t channel;

	// Address clients should attempt to connect to.
	std::string publicAddr;
	uint16_t publicPort;

	// Maps hosted on the server instance.
	std::vector<uint32_t> hostedMaps;
};

struct MgLoginSuccessPacket
{
	uint32_t handle;
	uint32_t aid;
	std::string socialId;
	uint32_t premiumTimes[PREMIUM_MAX_NUM];
	std::string hwid;
};

struct MgLoginFailurePacket
{
	uint32_t handle;
	std::string status;
};

struct MgLoginDisconnectPacket
{
	uint32_t aid;
};

struct MgDropStatusPacket {
    uint32_t aid;
	uint8_t state;
};


using MgCharacterEnterPacket = GmCharacterEnterPacket;
using MgCharacterLeavePacket = GmCharacterLeavePacket;
using MgCharacterDisconnectPacket = GmCharacterDisconnectPacket;
using MgCharacterWarpPacket = GmCharacterWarpPacket;
using MgCharacterTransferPacket = GmCharacterTransferPacket;

using MgMessengerAddPacket = GmMessengerAddPacket;
using MgMessengerRequestAddPacket = GmMessengerRequestAddPacket;
using MgMessengerAuthAddPacket = GmMessengerAuthAddPacket;
using MgMessengerDelPacket = GmMessengerDelPacket;

using MgGuildChatPacket = GmGuildChatPacket;
using MgGuildMemberCountBonusPacket = GmGuildMemberCountBonusPacket;
using MgGuildWarZoneInfoPacket = GmGuildWarZoneInfoPacket;

using MgChatPacket = GmChatPacket;
using MgWhisperPacket = GmWhisperPacket;
using MgWhisperErrorPacket = GmWhisperErrorPacket;
using MgWhisperAllPacket = GmWhisperAllPacket;
using MgShoutPacket = GmShoutPacket;
using MgNoticePacket = GmNoticePacket;
using MgBlockChatPacket = GmBlockChatPacket;
using MgOfflineShopBuyInfoPacket = GmOfflineShopBuyInfoPacket;
using MgOfflineShopTimePacket = GmOfflineShopTimePacket;

BOOST_FUSION_ADAPT_STRUCT(
	MgDropStatusPacket,
	aid,
	state
);


BOOST_FUSION_ADAPT_STRUCT(
	AmLoginPacket,
	handle,
	aid,
	login,
	socialId,
	premiumTimes,
	hwid
);

BOOST_FUSION_ADAPT_STRUCT(
	MaLoginSuccessPacket,
	handle,
	sessionId
);

BOOST_FUSION_ADAPT_STRUCT(
	MaLoginFailurePacket,
	handle,
	status
);

BOOST_FUSION_ADAPT_STRUCT(
	GmSetupPacket,
	version,
	channel,
	publicAddr,
	publicPort,
	hostedMaps
);

BOOST_FUSION_ADAPT_STRUCT(
	GmShutdownBroadcastPacket,
	countdown,
	reason
);

BOOST_FUSION_ADAPT_STRUCT(
	GmMaintenanceBroadcastPacket,
	seconds,
	duration
);

BOOST_FUSION_ADAPT_STRUCT(
	GmLoginPacket,
	handle,
	login,
	sessionId
);

BOOST_FUSION_ADAPT_STRUCT(
	GmLogoutPacket,
	aid
);

BOOST_FUSION_ADAPT_STRUCT(
	GmCharacterEnterPacket,
	pid,
	aid,
    name,
	empire,
	mapIndex,
	channel,
	hwid
);

BOOST_FUSION_ADAPT_STRUCT(
	GmCharacterLeavePacket,
	pid
);

BOOST_FUSION_ADAPT_STRUCT(
	GmCharacterDisconnectPacket,
	pid
);

BOOST_FUSION_ADAPT_STRUCT(
	GmCharacterWarpPacket,
	pid,
	mapIndex,
	x,
	y,
	channel
);

BOOST_FUSION_ADAPT_STRUCT(
	GmCharacterTransferPacket,
	sourcePid,
	targetPid
);

BOOST_FUSION_ADAPT_STRUCT(
	GmMessengerAddPacket,
	name,
	companion
);

BOOST_FUSION_ADAPT_STRUCT(
	GmMessengerRequestAddPacket,
	account,
	companion
);
BOOST_FUSION_ADAPT_STRUCT(
	GmMessengerAuthAddPacket,
	account,
	companion,
	deny
);

BOOST_FUSION_ADAPT_STRUCT(
	GmGuildChatPacket,
	type,
	gid,
	message
);

BOOST_FUSION_ADAPT_STRUCT(
	GmGuildMemberCountBonusPacket,
	gid,
	bonus
);

BOOST_FUSION_ADAPT_STRUCT(
	GmGuildWarZoneInfoPacket,
	gid1,
	gid2,
	mapIndex
);

BOOST_FUSION_ADAPT_STRUCT(
	GmChatPacket,
	pid,
	type,
	message
);

BOOST_FUSION_ADAPT_STRUCT(
	GmWhisperAllPacket,
	message
);

BOOST_FUSION_ADAPT_STRUCT(
	GmWhisperPacket,
	sourcePid,
	targetPid,
	localeCode,
	type,
	from,
	message
);

BOOST_FUSION_ADAPT_STRUCT(
	GmWhisperErrorPacket,
	pid,
	type,
	to
);

BOOST_FUSION_ADAPT_STRUCT(
	GmShoutPacket,
	empire,
	message
);

BOOST_FUSION_ADAPT_STRUCT(
	GmNoticePacket,
	empire,
	type,
	message
);

BOOST_FUSION_ADAPT_STRUCT(
	GmBlockChatPacket,
	pid,
	duration
);

BOOST_FUSION_ADAPT_STRUCT(
	GmOfflineShopBuyInfoPacket,
	ownerPid,
	vnum,
	count,
	price
);

BOOST_FUSION_ADAPT_STRUCT(
	MgHostedMapsPacket,
	channel,
	publicAddr,
	publicPort,
	hostedMaps
);

BOOST_FUSION_ADAPT_STRUCT(
	MgLoginSuccessPacket,
	handle,
	aid,
	socialId,
	premiumTimes,
	hwid
);

BOOST_FUSION_ADAPT_STRUCT(
	MgLoginFailurePacket,
	handle,
	status
);

BOOST_FUSION_ADAPT_STRUCT(
	MgLoginDisconnectPacket,
	aid
);

BOOST_FUSION_ADAPT_STRUCT(
	MgOfflineShopTimePacket,
	type,
	ownerPid,
	openingTime
);

BOOST_FUSION_ADAPT_STRUCT(
	GmWorldBossPacket,
	bossIndex,
	mapIndex,
	x,
	y
);

BOOST_FUSION_ADAPT_STRUCT(
	GmHyperlinkResultPacket,
	data
);

BOOST_FUSION_ADAPT_STRUCT(
	GmHyperlinkRequestPacket,
	ids
);



#endif