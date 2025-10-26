#ifndef METIN2_GAME_DBPACKETS_HPP
#define METIN2_GAME_DBPACKETS_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <boost/fusion/include/adapt_struct.hpp>
#include <game/Constants.hpp>
#include <game/ItemConstants.hpp>
#include <game/ItemTypes.hpp>
#include <game/MobConstants.hpp>
#include <game/Types.hpp>

#include <cstring>

METIN2_BEGIN_NS

typedef uint32_t IDENT;

/**
 * @version 05/06/10    Bang2ni - Myshop Pricelist 관련 패킷
 * HEADER_XX_MYSHOP_PRICELIST_XXX 추가
 */
enum {
    HEADER_GD_LOGIN,
    HEADER_GD_LOGOUT,

    HEADER_GD_PLAYER_LOAD,
    HEADER_GD_PLAYER_SAVE,
    HEADER_GD_PLAYER_CREATE,
    HEADER_GD_PLAYER_DELETE,

    HEADER_GD_LOGIN_KEY,
    HEADER_GD_PLAYER_DISCONNECT,
    HEADER_GD_BOOT,
    HEADER_GD_QUEST_SAVE,
    HEADER_GD_SAFEBOX_LOAD,
    HEADER_GD_SAFEBOX_SAVE,
    HEADER_GD_SAFEBOX_CHANGE_SIZE,

    HEADER_GD_SAFEBOX_CHANGE_PASSWORD,
    HEADER_GD_SAFEBOX_CHANGE_PASSWORD_SECOND, // Not really a packet, used internal
    HEADER_GD_DIRECT_ENTER,

    HEADER_GD_GUILD_SKILL_UPDATE,
    HEADER_GD_GUILD_EXP_UPDATE,
    HEADER_GD_GUILD_ADD_MEMBER,
    HEADER_GD_GUILD_REMOVE_MEMBER,
    HEADER_GD_GUILD_CHANGE_GRADE,
    HEADER_GD_GUILD_CHANGE_MEMBER_DATA,
    HEADER_GD_GUILD_DISBAND,
    HEADER_GD_GUILD_WAR,
    HEADER_GD_GUILD_WAR_SCORE,
    HEADER_GD_GUILD_CREATE,

    HEADER_GD_ITEM_SAVE,
    HEADER_GD_ITEM_DESTROY,

    HEADER_GD_ADD_AFFECT,
    HEADER_GD_REMOVE_AFFECT,

    HEADER_GD_HIGHSCORE_REGISTER,

    HEADER_GD_PARTY_CREATE,
    HEADER_GD_PARTY_DELETE,
    HEADER_GD_PARTY_ADD,
    HEADER_GD_PARTY_REMOVE,
    HEADER_GD_PARTY_STATE_CHANGE,
    HEADER_GD_PARTY_HEAL_USE,

    HEADER_GD_FLUSH_CACHE,
    HEADER_GD_RELOAD_PROTO,

    HEADER_GD_CHANGE_NAME,
    HEADER_GD_CHANGE_EMPIRE,

    HEADER_GD_GUILD_CHANGE_LADDER_POINT,
    HEADER_GD_GUILD_USE_SKILL,

    HEADER_GD_REQUEST_EMPIRE_PRIV,
    HEADER_GD_REQUEST_GUILD_PRIV,

    HEADER_GD_MONEY_LOG,

    HEADER_GD_GUILD_DEPOSIT_MONEY,
    HEADER_GD_GUILD_WITHDRAW_MONEY,
    HEADER_GD_GUILD_WITHDRAW_MONEY_GIVE_REPLY,

    HEADER_GD_REQUEST_CHARACTER_PRIV,

    HEADER_GD_SET_EVENT_FLAG,

    HEADER_GD_PARTY_SET_MEMBER_LEVEL,

    HEADER_GD_UPDATE_OBJECT,

    HEADER_GD_CREATE_OBJECT,
    HEADER_GD_DELETE_OBJECT,
    HEADER_GD_UPDATE_LAND,

    HEADER_GD_MARRIAGE_ADD,
    HEADER_GD_MARRIAGE_UPDATE,
    HEADER_GD_MARRIAGE_REMOVE,

    HEADER_GD_WEDDING_REQUEST,
    HEADER_GD_WEDDING_READY,
    HEADER_GD_WEDDING_END,

    HEADER_GD_SHOP,

    HEADER_GD_ADD_SWITCHBOT_DATA,
    HEADER_GD_REMOVE_SWITCHBOT_DATA,

#ifdef __OFFLINE_SHOP__
    HEADER_GD_OFFLINE_SHOP_CREATE,
    HEADER_GD_OFFLINE_SHOP_NAME,
    HEADER_GD_OFFLINE_SHOP_POSITION,
    HEADER_GD_OFFLINE_SHOP_ITEM_ADD,
    HEADER_GD_OFFLINE_SHOP_ITEM_MOVE,
    HEADER_GD_OFFLINE_SHOP_ITEM_REMOVE,
    HEADER_GD_OFFLINE_SHOP_GOLD,
    HEADER_GD_OFFLINE_SHOP_DESTROY,
    HEADER_GD_OFFLINE_SHOP_REGISTER_LISTENER,
    HEADER_GD_OFFLINE_SHOP_UNREGISTER_LISTENER,
    HEADER_GD_OFFLINE_SHOP_CLOSE,
    HEADER_GD_OFFLINE_SHOP_OPEN,
#if defined(__OFFLINE_SHOP_OPENING_TIME__) ||                                  \
    defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
    HEADER_GD_OFFLINE_SHOP_FLUSH_LEFT_OPENING_TIME,
    HEADER_GD_OFFLINE_SHOP_ITEM_BUY_BY_ID,
    HEADER_GD_OFFLINE_SHOP_ITEM_BUY_REQUEST_BY_ID,
    HEADER_GD_OFFLINE_SHOP_SEARCH_REQUEST,

#endif
#endif

    HEADER_GD_AUTH_LOGIN,
    HEADER_GD_AUTH_REGISTER,
    HEADER_GD_LOGIN_BY_KEY,
    HEADER_GD_AUTH_SET_PIN,
    HEADER_GD_HGUARD_INFO,
    HEADER_GD_HGUARD_VERIFY,
    HEADER_GD_HWID_INFO,

    HEADER_GD_MALL_LOAD,

    HEADER_GD_MYSHOP_PRICELIST_UPDATE, ///< °¡°ÝÁ¤º¸ °»½Å ¿äÃ»
    HEADER_GD_MYSHOP_PRICELIST_REQ, ///< °¡°ÝÁ¤º¸ ¸®½ºÆ® ¿äÃ»

    HEADER_GD_BLOCK_CHAT,
    HEADER_GD_SET_HWID_FLAG,

    HEADER_GD_HAMMER_OF_TOR,
    HEADER_GD_RELOAD_ADMIN,   ///<¿î¿µÀÚ Á¤º¸ ¿äÃ»
    HEADER_GD_BREAK_MARRIAGE, ///< °áÈ¥ ÆÄ±â

    HEADER_GD_REQ_CHANGE_GUILD_MASTER,

    HEADER_GD_REQ_SPARE_ITEM_ID_RANGE,

    HEADER_GD_UPDATE_HORSE_NAME,
    HEADER_GD_REQ_HORSE_NAME,

    HEADER_GD_REQUEST_CHARGE_CASH,

    HEADER_GD_DELETE_AWARDID, // delete gift notify icon

    HEADER_GD_UPDATE_EMPIRE,

    HEADER_GD_MESSENGER_SET_BLOCK,

#ifdef ENABLE_GUILD_STORAGE
    // Guildstorage sync packets
    HEADER_GD_REQUEST_GUILD_ITEMS,
    HEADER_GD_GUILDSTORAGE_MOVE,
    HEADER_GD_REQUEST_LOCK_STATE,
    HEADER_GD_GUILDSTORAGE_ADD_ITEM,
    HEADER_GD_GUILDSTORAGE_ADD_ITEM_D,
    HEADER_GD_GUILDSTORAGE_REMOVE_ITEM,
// Guildstorage sync packets
#endif
#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
    HEADER_GD_AUTO_EVENT,
    HEADER_GD_RELOAD_EVENTS,
#endif

#ifdef __DUNGEON_FOR_GUILD__
    HEADER_GD_GUILD_DUNGEON,
    HEADER_GD_GUILD_DUNGEON_CD,
#endif

#ifdef ENABLE_GEM_SYSTEM
    HEADER_GD_GEM_LOG,
#endif

    HEADER_GD_UPDATE_ITEM_NAME,
    HEADER_GD_REQ_ITEM_NAME,

#ifdef ENABLE_BATTLE_PASS
    HEADER_GD_SAVE_BATTLE_PASS,
    HEADER_GD_REGISTER_BP_RANKING,
    HEADER_GD_BATTLE_PASS_RANKING,
#endif

    HEADER_GD_SAVE_HUNTING_MISSION,
    HEADER_GD_DUNGEON_INFO_SAVE,

    HEADER_GD_SETUP,

    ///////////////////////////////////////////////
    HEADER_DG_NOTICE,

    HEADER_DG_PLAYER_LIST_RESULT,

    HEADER_DG_LOGIN_SUCCESS,
    // HEADER_DG_LOGIN_NOT_EXIST,
    // HEADER_DG_LOGIN_WRONG_PASSWD,
    // HEADER_DG_LOGIN_ALREADY,

    HEADER_DG_PLAYER_LOAD_SUCCESS,
    HEADER_DG_PLAYER_LOAD_FAILED,
    HEADER_DG_PLAYER_CREATE_SUCCESS,
    HEADER_DG_PLAYER_CREATE_ALREADY,
    HEADER_DG_PLAYER_CREATE_FAILED,
    HEADER_DG_PLAYER_DELETE_SUCCESS,
    HEADER_DG_PLAYER_DELETE_FAILED,

    HEADER_DG_BOOT,
    HEADER_DG_QUEST_LOAD,

    HEADER_DG_SAFEBOX_LOAD,
    HEADER_DG_SAFEBOX_CHANGE_SIZE,
    HEADER_DG_SAFEBOX_WRONG_PASSWORD,
    HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER,

    HEADER_DG_AFFECT_LOAD,
    HEADER_DG_MALL_LOAD,
    HEADER_DG_PET_LOAD,
    // Empty 53
    HEADER_DG_ACTIVITY_LOAD,

    HEADER_DG_DIRECT_ENTER,

    HEADER_DG_GUILD_SKILL_UPDATE,
    HEADER_DG_GUILD_SKILL_RECHARGE,
    HEADER_DG_GUILD_EXP_UPDATE,

    HEADER_DG_PARTY_CREATE,
    HEADER_DG_PARTY_DELETE,
    HEADER_DG_PARTY_ADD,
    HEADER_DG_PARTY_REMOVE,
    HEADER_DG_PARTY_STATE_CHANGE,
    HEADER_DG_PARTY_HEAL_USE,
    HEADER_DG_PARTY_SET_MEMBER_LEVEL,

    HEADER_DG_TIME,
    HEADER_DG_ITEM_ID_RANGE,

    HEADER_DG_GUILD_ADD_MEMBER,
    HEADER_DG_GUILD_REMOVE_MEMBER,
    HEADER_DG_GUILD_CHANGE_GRADE,
    HEADER_DG_GUILD_CHANGE_MEMBER_DATA,
    HEADER_DG_GUILD_DISBAND,
    HEADER_DG_GUILD_WAR,
    HEADER_DG_GUILD_WAR_SCORE,
    HEADER_DG_GUILD_TIME_UPDATE,
    HEADER_DG_GUILD_LOAD,
    HEADER_DG_GUILD_LADDER,
    HEADER_DG_GUILD_SKILL_USABLE_CHANGE,
    HEADER_DG_GUILD_MONEY_CHANGE,
    HEADER_DG_GUILD_WITHDRAW_MONEY_GIVE,

    HEADER_DG_SET_EVENT_FLAG,

    HEADER_DG_RELOAD_PROTO,
    HEADER_DG_CHANGE_NAME,

    HEADER_DA_AUTH_LOGIN,
    HEADER_DA_AUTH_REGISTER,

    HEADER_DG_HGUARD_RESULT,

    HEADER_DG_CHANGE_EMPIRE,

    HEADER_DG_CHANGE_EMPIRE_PRIV,
    HEADER_DG_CHANGE_GUILD_PRIV,

    HEADER_DG_MONEY_LOG,

    HEADER_DG_CHANGE_CHARACTER_PRIV,
    HEADER_DG_SET_HWID_FLAG,

#ifdef ENABLE_GEM_SYSTEM
    HEADER_DG_GEM_LOG,
#endif

    HEADER_DG_UPDATE_OBJECT,

    HEADER_DG_CREATE_OBJECT,
    HEADER_DG_DELETE_OBJECT,
    HEADER_DG_UPDATE_LAND,

    HEADER_DG_MARRIAGE_ADD,
    HEADER_DG_MARRIAGE_UPDATE,
    HEADER_DG_MARRIAGE_REMOVE,

    HEADER_DG_WEDDING_REQUEST,
    HEADER_DG_WEDDING_READY,
    HEADER_DG_WEDDING_START,
    HEADER_DG_WEDDING_END,

    HEADER_DG_MYSHOP_PRICELIST_RES, ///< °¡°ÝÁ¤º¸ ¸®½ºÆ® ÀÀ´ä
    HEADER_DG_RELOAD_ADMIN,         ///< ¿î¿µÀÚ Á¤º¸ ¸®·Îµå
    HEADER_DG_BREAK_MARRIAGE,       ///< °áÈ¥ ÆÄ±â

    HEADER_DG_ACK_CHANGE_GUILD_MASTER,

    HEADER_DG_ACK_SPARE_ITEM_ID_RANGE,

    HEADER_DG_UPDATE_HORSE_NAME,
    HEADER_DG_ACK_HORSE_NAME,

    HEADER_DG_RESULT_CHARGE_CASH,

#ifdef __ADMIN_MANAGER__
    HEADER_DG_ADMIN_MANAGER_CHAT_SEARCH_PLAYER,
#endif

#ifdef ENABLE_GUILD_STORAGE
    HEADER_DG_GUILDSTORAGE_MOVE,
    HEADER_DG_GUILDSTORAGE_ADD,
    HEADER_DG_GUILDSTORAGE_REMOVE,
    HEADER_DG_GUILDSTORAGE_ADD_ITEM_TO_CHARACTER,
    HEADER_DG_GUILDSTORAGE_ITEM,
    HEADER_DG_GUILDSTORAGE_ADD_ITEM_RESPONE_1,
    HEADER_DG_GUILDSTORAGE_DESTROY_QUE,
    HEADER_DG_GUILDSTORAGE_REMOVE_ITEM,
    HEADER_DG_GUILDSTORAGE_REMOVE_SYNC_ITEM,
#endif
#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER
    HEADER_DG_AUTO_EVENT,
    HEADER_DG_RELOAD_EVENTS,
#endif

#ifdef __DUNGEON_FOR_GUILD__
    HEADER_DG_GUILD_DUNGEON,
    HEADER_DG_GUILD_DUNGEON_CD,
#endif

    HEADER_DG_GUILD_WAR_RESERVE_ADD,
    HEADER_DG_GUILD_WAR_RESERVE_DEL,
    HEADER_DG_GUILD_WAR_BET,
    HEADER_GD_PLAYER_LIST_QUERY,
#ifdef __OFFLINE_SHOP__
    HEADER_DG_OFFLINE_SHOP_LOAD,
    HEADER_DG_OFFLINE_SHOP_NAME,
    HEADER_DG_OFFLINE_SHOP_ITEM_ADD,
    HEADER_DG_OFFLINE_SHOP_ITEM_MOVE,
    HEADER_DG_OFFLINE_SHOP_ITEM_REMOVE,
    HEADER_DG_OFFLINE_SHOP_GOLD,
    HEADER_DG_OFFLINE_SHOP_DESTROY,
    HEADER_DG_OFFLINE_SHOP_CLOSE,
    HEADER_DG_OFFLINE_SHOP_OPEN,
    HEADER_DG_OFFLINE_SHOP_ITEM_INFO,
    HEADER_DG_OFFLINE_SHOP_BUY_DONE,
    HEADER_DG_OFFLINE_SHOP_SEARCH_RESULT,
#endif

    HEADER_DG_UPDATE_ITEM_NAME,
    HEADER_DG_ACK_ITEM_NAME,
#ifdef ENABLE_BATTLE_PASS
    HEADER_DG_BATTLE_PASS_LOAD_RANKING,
#endif
    HEADER_DG_UPDATE_CASH,
    HEADER_DG_DUNGEON_RANKING_LOAD,
    HEADER_DG_P2P,

};

/* ----------------------------------------------
 * table
 * ----------------------------------------------
 */

#pragma pack(push, 1)

/* game Server -> DB Server */
struct DbPacketHeader {
    uint8_t id;
    uint32_t handle;
    uint32_t size;
};

struct GdPlayerListQueryPacket {
    uint32_t aid;
};

struct DgPlayerListResultPacket {
    bool failed;
    TAccountTable tab;
};
enum ERequestChargeType {
    ERequestCharge_Cash = 0,
    ERequestCharge_Mileage,
};

typedef struct SRequestChargeCash {
    uint32_t dwAID; // id(primary key) - Account Table
    uint32_t iAmount;
    ERequestChargeType eChargeType;
    bool bAdd = false;

} TRequestChargeCash;

typedef struct SPacketDGCreateSuccess {
    uint8_t slot;
    TSimplePlayer player;
} TPacketDGCreateSuccess;

typedef struct SConnectTable {
    char login[LOGIN_MAX_LEN + 1];
    IDENT ident;
} TConnectTable;

typedef struct SLoginPacket {
    char login[LOGIN_MAX_LEN + 1];
    char passwd[PASSWD_MAX_LEN + 1];
} TLoginPacket;

typedef struct SPlayerLoadPacket {
    uint32_t account_id;
    uint32_t player_id;
    uint8_t account_index; /* account ¿¡¼­ÀÇ À§Ä¡ */
} TPlayerLoadPacket;

typedef struct SPlayerCreatePacket {

    uint32_t account_id;
    uint8_t slot;
    TPlayerTable player_table;
} TPlayerCreatePacket;

struct GdPlayerDeletePacket {
    uint32_t pid;
    char name[CHARACTER_NAME_MAX_LEN + 1];
};

struct GdPlayerDisconnectPacket {
    uint32_t pid;
    uint32_t aid;
};

typedef struct SLogoutPacket {
    char login[LOGIN_MAX_LEN + 1];
    char passwd[PASSWD_MAX_LEN + 1];
} TLogoutPacket;

typedef struct SPlayerCountPacket {
    uint32_t dwCount;
} TPlayerCountPacket;

typedef struct SSafeboxTable {
    uint32_t dwID;
    uint8_t bSize;
    uint32_t dwGold;
    uint16_t wItemCount;
} TSafeboxTable;

typedef struct SSafeboxChangeSizePacket {
    uint32_t dwID;
    uint8_t bSize;
} TSafeboxChangeSizePacket;

typedef struct SSafeboxLoadPacket {
    uint32_t dwID;
    char szLogin[LOGIN_MAX_LEN + 1];
    char szPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
} TSafeboxLoadPacket;

typedef struct SSafeboxChangePasswordPacket {
    uint32_t dwID;
    char szOldPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
    char szNewPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
} TSafeboxChangePasswordPacket;

typedef struct SSafeboxChangePasswordPacketAnswer {
    uint8_t flag;
} TSafeboxChangePasswordPacketAnswer;

typedef struct SEmpireSelectPacket {
    uint32_t dwAccountID;
    uint8_t bEmpire;
} TEmpireSelectPacket;

typedef struct SPacketGDSetup {
    char gameIp[16];   // Public IP which listen to users
    char p2pIp[16];    // IP for other servers
    uint16_t gamePort; // 클라이언트가 접속하는 포트 번호
    uint16_t p2pPort;  // 서버끼리 연결 시키는 P2P 포트 번호
    uint8_t bChannel;  // 채널
    uint8_t bAuthServer;
    int32_t alMaps[32];
    uint32_t dwLoginCount;
} TPacketGDSetup;

typedef struct SPacketDGMapLocations {
    uint8_t bCount;
} TPacketDGMapLocations;

typedef struct SMapLocation {
    char szHost[MAX_HOST_LENGTH + 1];
    uint16_t wPort;
    uint8_t channel;
    int32_t alMaps[MAX_MAPS_PER_GAME];
} TMapLocation;

typedef struct SPacketDGP2P {
    char szHost[MAX_HOST_LENGTH + 1];
    uint16_t wPort;
    uint8_t bChannel;
    uint16_t wListenPort;
} TPacketDGP2P;

typedef struct SPacketGDDirectEnter {
    char login[LOGIN_MAX_LEN + 1];
    char passwd[PASSWD_MAX_LEN + 1];
    uint8_t index;
} TPacketGDDirectEnter;

typedef struct SPacketDGDirectEnter {
    TAccountTable accountTable;
    TPlayerTable playerTable;
} TPacketDGDirectEnter;

typedef struct SPacketGuildSkillUpdate {
    uint32_t guild_id;
    PointValue amount;
    uint8_t skill_levels[12];
    uint8_t skill_point;
    uint8_t save;
} TPacketGuildSkillUpdate;

typedef struct SPacketGuildExpUpdate {
    uint32_t guild_id;
    PointValue amount;
} TPacketGuildExpUpdate;

typedef struct SPacketGuildChangeMemberData {
    uint32_t guild_id;
    uint32_t pid;
    uint32_t offer;
    Level level;
    uint8_t grade;
} TPacketGuildChangeMemberData;

typedef struct SPacketDGLoginAlready {
    char szLogin[LOGIN_MAX_LEN + 1];
} TPacketDGLoginAlready;

typedef struct SPacketGDAddSwitchbotData {
    uint32_t pid;
    SwitchBotSlotData elem;
} TPacketGDAddSwitchbotData;

typedef struct SPacketGDRemoveSwitchbotData {
    uint32_t pid;
    uint8_t slot;
} TPacketGDRemoveSwitchbotData;

typedef struct SPacketGDAddAffect {
    uint32_t dwPID;
    AffectData elem;
} TPacketGDAddAffect;

typedef struct SPacketGDRemoveAffect {
    uint32_t dwPID;
    uint32_t dwType;
    uint8_t bApplyOn;
} TPacketGDRemoveAffect;

typedef struct SPacketGDHighscore {
    uint32_t dwPID;
    int32_t lValue;
    char cDir;
    char szBoard[21];
} TPacketGDHighscore;

typedef struct SPacketPartyCreate {
    uint32_t dwLeaderPID;
} TPacketPartyCreate;

typedef struct SPacketPartyDelete {
    uint32_t dwLeaderPID;
} TPacketPartyDelete;

typedef struct SPacketPartyAdd {
    uint32_t dwLeaderPID;
    uint32_t dwPID;
    uint8_t bState;
} TPacketPartyAdd;

typedef struct SPacketPartyRemove {
    uint32_t dwLeaderPID;
    uint32_t dwPID;
} TPacketPartyRemove;

typedef struct SPacketPartyStateChange {
    uint32_t dwLeaderPID;
    uint32_t dwPID;
    uint8_t bRole;
    uint8_t bFlag;
} TPacketPartyStateChange;

typedef struct SPacketPartySetMemberLevel {
    uint32_t dwLeaderPID;
    uint32_t dwPID;
    uint8_t bLevel;
} TPacketPartySetMemberLevel;

typedef struct SPacketGDBoot {
    uint32_t dwItemIDRange[2];
    char szIP[16];
} TPacketGDBoot;

typedef struct SPacketGuild {
    uint32_t dwGuild;
    uint32_t dwInfo;
} TPacketGuild;

typedef struct SPacketGDGuildAddMember {
    uint32_t dwPID;
    uint32_t dwGuild;
    uint8_t bGrade;
} TPacketGDGuildAddMember;

typedef struct SPacketDGGuildMember {
    uint32_t dwPID;
    uint32_t dwGuild;
    uint8_t bGrade;
    uint8_t isGeneral;
    uint8_t bJob;
    Level bLevel;
    uint32_t dwOffer;
    char szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketDGGuildMember;

typedef struct SPacketGuildWar {
    uint8_t bType;
    uint8_t bWar;
    uint32_t dwGuildFrom;
    uint32_t dwGuildTo;
    bool isRanked;
    int32_t lInitialScore;
} TPacketGuildWar;

// Game -> DB : 상대적 변화값
// DB -> Game : 토탈된 최종값
typedef struct SPacketGuildWarScore {
    uint32_t dwGuildGainPoint;
    uint32_t dwGuildOpponent;
    int32_t lScore;
    int32_t lBetScore;
} TPacketGuildWarScore;

typedef struct SBanwordTable {
    char szWord[BANWORD_MAX_LEN + 1];
} TBanwordTable;

struct GdChangeNamePacket {
    uint32_t pid;
    char name[CHARACTER_NAME_MAX_LEN + 1];
};

struct DgChangeNamePacket {
    uint32_t pid;
    bool failed;
    char name[CHARACTER_NAME_MAX_LEN + 1];
};

struct GdChangeEmpirePacket {
    uint32_t pid;
    uint8_t empire;
};

struct DgChangeEmpirePacket {
    uint32_t pid;
    bool failed;
    uint8_t empire;
};

typedef struct SPacketGuildLadder {
    uint32_t dwGuild;
    int32_t lLadderPoint;
    int32_t lWin;
    int32_t lDraw;
    int32_t lLoss;
} TPacketGuildLadder;

typedef struct SPacketGuildLadderPoint {
    uint32_t dwGuild;
    int32_t lChange;
} TPacketGuildLadderPoint;

typedef struct SPacketGuildUseSkill {
    uint32_t dwGuild;
    uint32_t dwSkillVnum;
    uint32_t dwCooltime;
} TPacketGuildUseSkill;

typedef struct SPacketGuildSkillUsableChange {
    uint32_t dwGuild;
    uint32_t dwSkillVnum;
    uint8_t bUsable;
} TPacketGuildSkillUsableChange;

typedef struct SPacketGDLoginKey {
    uint32_t dwAccountID;
    uint32_t dwLoginKey;
} TPacketGDLoginKey;

struct GdAuthSetPinQueryPacket {
    uint32_t aid;
    uint32_t code;
};

struct GdHwidInfoQueryPacket {
    uint32_t aid;
    char hwid[HWID_MAX_LEN + 1];
};

struct GdHGuardInfoQueryPacket {
    uint32_t aid;
    char hwid[HWID_MAX_LEN + 1];
    char macAddr[MAX_HGUARD_DATA_LENGTH + 1];
    char guid[MAX_HGUARD_DATA_LENGTH + 1];
    char cpuId[MAX_HGUARD_DATA_LENGTH + 1];
};

struct GdHGuardValidationQueryPacket {
    uint32_t aid;
    char hgHash[HWID_MAX_LEN + 1];
    char code[8 + 1];
};

struct GdAuthLoginQueryPacket {
    char login[LOGIN_MAX_LEN + 1];
    char hwid[HWID_MAX_LEN + 1];
};

struct GdAuthRegisterQueryPacket {
    char login[LOGIN_MAX_LEN + 1];
    char password[PASSWD_MAX_LEN + 1];
    char hwid[HWID_MAX_LEN + 1];
    char deleteCode[SOCIAL_ID_MAX_LEN + 1];
    char mailAdress[HWID_MAX_LEN + 1];
};

struct DgAuthLoginResultPacket {
    uint32_t id;
    uint32_t securityCode;
    uint8_t empire;
    uint8_t useHwidProt;
    char password[45 + 1];
    char socialId[SOCIAL_ID_MAX_LEN + 1];
    char status[ACCOUNT_STATUS_MAX_LEN + 1];
    char hwidHash[HGUARD_MAX_ID_LEN + 1];
    uint32_t premiumTimes[PREMIUM_MAX_NUM];
    uint32_t createTime;
};

typedef struct SPacketGDAuthLogin {
    uint32_t dwID;
    uint32_t dwLoginKey;
    char szLogin[LOGIN_MAX_LEN + 1];
    char szSocialID[SOCIAL_ID_MAX_LEN + 1];
    char szSecurityCode[SOCIAL_ID_MAX_LEN + 1];
    char lang[ACCOUNT_LANG_MAX_LEN + 1];
    uint32_t iPremiumTimes[PREMIUM_MAX_NUM];
    char szHWIDHash[HGUARD_MAX_ID_LEN + 1];
    uint8_t bEmpire;
#ifdef __ADMIN_MANAGER__
    uint32_t accban_count;
#endif
} TPacketGDAuthLogin;

typedef struct SPacketGDLoginByKey {
    char szLogin[LOGIN_MAX_LEN + 1];
    uint32_t dwLoginKey;
    char szIP[MAX_HOST_LENGTH + 1];
} TPacketGDLoginByKey;

/**
 * @version 05/06/08    Bang2ni - 지속시간 추가
 */
typedef struct SPacketGiveGuildPriv {
    uint8_t type;
    int value;
    uint32_t guild_id;
    uint32_t duration_sec; ///< 지속시간
} TPacketGiveGuildPriv;

typedef struct SPacketGiveEmpirePriv {
    uint8_t type;
    int value;
    uint8_t empire;
    uint32_t duration_sec;
} TPacketGiveEmpirePriv;

typedef struct SPacketGiveCharacterPriv {
    uint8_t type;
    int value;
    uint32_t pid;
} TPacketGiveCharacterPriv;

typedef struct SPacketRemoveGuildPriv {
    uint8_t type;
    uint32_t guild_id;
} TPacketRemoveGuildPriv;

typedef struct SPacketRemoveEmpirePriv {
    uint8_t type;
    uint8_t empire;
} TPacketRemoveEmpirePriv;

typedef struct SPacketDGChangeCharacterPriv {
    uint8_t type;
    int value;
    uint32_t pid;
    uint8_t bLog;
} TPacketDGChangeCharacterPriv;

/**
 * @version 05/06/08    Bang2ni - 지속시간 추가
 */
typedef struct SPacketDGChangeGuildPriv {
    uint8_t type;
    int value;
    uint32_t guild_id;
    uint8_t bLog;
    uint32_t end_time_sec; ///< 지속시간
} TPacketDGChangeGuildPriv;

typedef struct SPacketDGChangeEmpirePriv {
    uint8_t type;
    int value;
    uint8_t empire;
    uint8_t bLog;
    uint32_t end_time_sec;
} TPacketDGChangeEmpirePriv;

typedef struct SPacketMoneyLog {
    uint8_t type;
    uint32_t vnum;
    Gold gold;
} TPacketMoneyLog;

typedef struct SPacketGDGuildMoney {
    uint32_t dwGuild;
    Gold iGold;
} TPacketGDGuildMoney;

typedef struct SPacketDGGuildMoneyChange {
    uint32_t dwGuild;
    Gold iTotalGold;
} TPacketDGGuildMoneyChange;

typedef struct SPacketDGGuildMoneyWithdraw {
    uint32_t dwGuild;
    Gold iChangeGold;
} TPacketDGGuildMoneyWithdraw;

typedef struct SPacketGDGuildMoneyWithdrawGiveReply {
    uint32_t dwGuild;
    Gold iChangeGold;
    uint8_t bGiveSuccess;
} TPacketGDGuildMoneyWithdrawGiveReply;

typedef struct SPacketSetEventFlag {
    char szFlagName[EVENT_FLAG_NAME_MAX_LEN + 1];
    int32_t lValue;
} TPacketSetEventFlag;

typedef struct SPacketSetHwidFlag {
    char szHwid[HWID_MAX_LEN + 1];
    char szFlagName[EVENT_FLAG_NAME_MAX_LEN + 1];
    long lValue;
} TPacketSetHwidFlag;

typedef struct SPacketLoginOnSetup {
    uint32_t dwID;
    char szLogin[LOGIN_MAX_LEN + 1];
    char szSocialID[SOCIAL_ID_MAX_LEN + 1];
    char szSecurityCode[SOCIAL_ID_MAX_LEN + 1];

    char szHost[MAX_HOST_LENGTH + 1];
    uint32_t dwLoginKey;
    char lang[ACCOUNT_LANG_MAX_LEN + 1];
    bool isRestrictedAccount;
    bool bEmpire;
} TPacketLoginOnSetup;

typedef struct SPacketGDCreateObject {
    uint32_t dwVnum;
    uint32_t dwLandID;
    int32_t lMapIndex;
    int32_t x, y;
    float xRot;
    float yRot;
    float zRot;
} TPacketGDCreateObject;

typedef struct SPacketGDUpdateObject {
    uint32_t dwID;
    uint32_t dwLandID;
    int32_t lMapIndex;
    int32_t x, y;
    float xRot;
    float yRot;
    float zRot;
} TPacketGDUpdateObject;

typedef struct SGuildReserve {
    uint32_t dwID;
    uint32_t dwGuildFrom;
    uint32_t dwGuildTo;
    uint32_t dwTime;
    uint8_t bType;
    int32_t lWarPrice;
    int32_t lInitialScore;
    uint8_t bStarted;
    uint32_t dwBetFrom;
    uint32_t dwBetTo;
    int32_t lPowerFrom;
    int32_t lPowerTo;
    int32_t lHandicap;
} TGuildWarReserve;

typedef struct {
    uint32_t dwWarID;
    char szLogin[LOGIN_MAX_LEN + 1];
    uint32_t dwGold;
    uint32_t dwGuild;
} TPacketGDGuildWarBet;

// Marriage

typedef struct {
    uint32_t dwPID1;
    uint32_t dwPID2;
    uint32_t tMarryTime;
    char szName1[CHARACTER_NAME_MAX_LEN + 1];
    char szName2[CHARACTER_NAME_MAX_LEN + 1];
} TPacketMarriageAdd;

typedef struct {
    uint32_t dwPID1;
    uint32_t dwPID2;
    int32_t iLovePoint;
    uint8_t byMarried;
} TPacketMarriageUpdate;

typedef struct {
    uint32_t dwPID1;
    uint32_t dwPID2;
} TPacketMarriageRemove;

typedef struct {
    uint32_t dwPID1;
    uint32_t dwPID2;
} TPacketWeddingRequest;

typedef struct {
    uint32_t dwPID1;
    uint32_t dwPID2;
    uint32_t dwMapIndex;
} TPacketWeddingReady;

typedef struct {
    uint32_t dwPID1;
    uint32_t dwPID2;
} TPacketWeddingStart;

typedef struct {
    uint32_t dwPID1;
    uint32_t dwPID2;
} TPacketWeddingEnd;

struct MyShopPriceListRequest {
    uint32_t pid;
    TItemPos bundleItem;
};

/// 개인상점 가격정보의 헤더. 가변 패킷으로 이 뒤에 byCount 만큼의
/// TItemPriceInfo 가 온다.
struct MyShopPriceListHeader {
    uint32_t pid; ///< 가격정보를 가진 플레이어 ID
    TItemPos bundleItem;
    uint8_t count; ///< 가격정보 갯수
};

/// 개인상점의 단일 아이템에 대한 가격정보
struct MyShopPriceInfo {
    uint32_t vnum;  ///< 아이템 vnum
    uint32_t price; ///< 가격
};

/// 개인상점 아이템 가격정보 리스트 테이블
// TODO(tim): This is not a packet. It doesn't belong here.
struct MyShopItemPriceListTable {
    uint32_t pid;  ///< 가격정보를 가진 플레이어 ID
    uint8_t count; ///< 가격정보 리스트의 갯수

    MyShopPriceInfo info[SHOP_PRICELIST_MAX_NUM]; ///< 가격정보 리스트
};

#ifdef ENABLE_GUILD_STORAGE
typedef struct SQueueItem {
    TPlayerItem item;   // Item
    uint8_t bQueueType; // Queue type
    uint32_t dwHandle;  // Character handle
                        // TItemPos ItemPos,	// Return position
    uint16_t cell;
    uint8_t window_type;
    // LPITEM pkItem;
} TQueueItem;

typedef struct {
    uint32_t guildID;
    int dwSrcPos;
    int dwDestPos;
    TPlayerItem item;
    int iTick;
    uint32_t playerId;
} TPacketGuildStorageMoveItem;

typedef struct {
    TQueueItem queItem;
    uint32_t guildID;
    uint32_t queID;
    uint16_t wPos;
} TPacketGuildStorageRemoveItemRequest;

typedef struct {
    TQueueItem queItem;
    uint32_t guildID;
    uint32_t queID;
    uint16_t wPos;
} TPacketGuildStorageAddItem;

typedef struct {
    TQueueItem queItem;
    uint32_t guildID;
    uint32_t queID;
    uint16_t wPos;
} TPacketGuildStorageRemoveItemReturn;

typedef struct {
    uint16_t pos;
    uint32_t guildID;
} TPacketGuildStorageRemoveSyncItem;

// Not needed
typedef struct {
    uint32_t guildID;
    uint32_t queID;
} TPacketGuildStorageEraseQue;

typedef struct {
    uint32_t guildID;
    uint16_t wPos;
    int iTick;
} TPacketGuildStorageRemoveItem;

typedef struct {
    uint32_t guildID;
    uint16_t wPos;
    TPlayerItem item;
    int iTick;
    uint32_t dwVID;
    uint32_t queID;
} TPacketGuildStorageAddItemToChar;

typedef struct {
    uint32_t guildID;
    uint16_t wPos;
    TPlayerItem item;
    int iTick;
    uint32_t loadCount; // Just for boot
} TPacketGuildStorageItem;

typedef struct SPacketRequestGuildItems {
    uint32_t guildID;
} TPacketRequestGuildItems;

typedef struct SPacketGuildItem {
    uint32_t guildID;
    uint32_t pos;
    TPlayerItem item;
} TPacketGuildItem;

#endif

typedef struct {
    uint32_t dwPID;
    char szName[CHARACTER_NAME_MAX_LEN + 1];
    long lDuration;
    bool bIncreaseBanCounter;
} TPacketBlockChat;

// RELOAD_ADMIN
typedef struct SPacketReloadAdmin {
    char szIP[16];
} TPacketReloadAdmin;
// END_RELOAD_ADMIN

typedef struct TMonarchInfo {
    uint32_t pid[EMPIRE_MAX_NUM];  // 군주의 PID
    int64_t money[EMPIRE_MAX_NUM]; // 군주의 별개 돈
    char name[EMPIRE_MAX_NUM][32]; // 군주의 이름
    char date[EMPIRE_MAX_NUM][32]; // 군주 등록 날짜
} MonarchInfo;

typedef struct TMonarchElectionInfo {
    uint32_t pid;         // 투표 한사람 PID
    uint32_t selectedpid; // 투표 당한 PID ( 군주 참가자 )
    char date[32];        // 투표 날짜
} MonarchElectionInfo;

// 군주 출마자
typedef struct tMonarchCandidacy {
    uint32_t pid;
    char name[32];
    char date[32];
} MonarchCandidacy;

typedef struct tChangeMonarchLord {
    uint8_t bEmpire;
    uint32_t dwPID;
} TPacketChangeMonarchLord;

typedef struct tChangeMonarchLordACK {
    uint8_t bEmpire;
    uint32_t dwPID;
    char szName[32];
    char szDate[32];
} TPacketChangeMonarchLordACK;

typedef struct tChangeGuildMaster {
    // RELOAD_ADMIN
    typedef struct SPacketReloadAdmin {
        char szIP[16];
    } TPacketReloadAdmin;
    // END_RELOAD_ADMIN

    uint32_t dwGuildID;
    uint32_t idFrom;
    uint32_t idTo;
} TPacketChangeGuildMaster;

typedef struct tUpdateHorseName {
    uint32_t dwPlayerID;
    char szHorseName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketUpdateHorseName;

typedef struct tUpdateItemName {
    uint32_t dwItemID;
    char szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketUpdateItemName;

typedef struct tDC {
    char login[LOGIN_MAX_LEN + 1];
} TPacketDC;

//독일 선물 알림 기능 테스트용 패킷 정보
typedef struct tItemAwardInformer {
    char login[LOGIN_MAX_LEN + 1];
    char command[20];  //명령어
    unsigned int vnum; //아이템
} TPacketItemAwardInfromer;
// 선물 알림 기능 삭제용 패킷 정보
typedef struct tDeleteAwardID {
    uint32_t dwID;
} TPacketDeleteAwardID;

typedef struct {
    long lMapIndex;
    int channel;
} TPacketChangeChannel;

typedef struct {
    long lAddr;
    uint16_t port;
} TPacketReturnChannel;

#ifdef ENABLE_AUTOMATIC_EVENT_MANAGER

typedef struct SEventTime {
    uint8_t bMinute;
    uint8_t bHour;
    uint8_t bWeekDay;
    uint8_t bWeek;
    uint8_t bMonthDay;
    uint8_t bMonth;
} TimeTable;
enum EventPeriods {
    EVENT_NONE,
    EVENT_DAILY,
    EVENT_WEEKLY,
    EVENT_MONTHLY,
    EVENT_ANNUALLY,
};
enum EventTypes {
    EVENT_TYPE_NONE,
    EVENT_FLAG,
    EVENT_OXEVENT,
    EVENT_PVP_TOURNAMENT,
    EVENT_GUILD_TOURNAMENT,
    EVENT_MONARCH,
    EVENT_GOLD_EVENT,
    EVENT_EMPIRE_WAR,
    EVENT_MAX_TYPE_NUM
};
typedef struct SEventFlags {
    uint32_t dwID;
    char szFlag[EVENT_FLAG_NAME_MAX_LEN + 1];
    long lValue;
    bool bStart;
} EventFlag;

typedef struct SEventNotice {
    uint32_t dwID;
    char szNotice[CHAT_MAX_LEN + 1];
    bool bStart;
} EventNotice;

typedef struct SEventRegen {
    uint32_t dwID;
    char szFileName[EVENT_FLAG_NAME_MAX_LEN * 2 + 1];
    bool bStart;
} EventRegen;
typedef struct SEventLuaFile {
    uint32_t dwID;
    char szFileName[EVENT_FLAG_NAME_MAX_LEN * 2 + 1];
    bool bStart;
} EventLua;
typedef struct SEventTable {
    int GetStartRemainingTime(uint32_t globalTime)
    {
        return (int)(start_t - globalTime);
    }
    int GetEndRemainingTime(uint32_t globalTime)
    {
        return (int)(end_t - globalTime);
    }
    bool GetStarted() { return bStarted; }
    void SetStarted(bool bstarted) { bStarted = bstarted; }
    bool IsNotice() { return bIsNotice; }
    uint32_t dwID;
    char szName[EVENT_MAX_NAME_LEN + 1];
    uint8_t bPeriod;
    uint8_t bType;
    long lMapIndex;
    uint8_t bChannel;
    TimeTable start_time;
    uint32_t start_t;
    tm start_tm;
    uint32_t dwEndTime;
    uint32_t end_t;
    tm end_tm;
    uint32_t dwRemainTime;
    bool bStarted;
    bool bIsNotice;
} EventTable;

typedef struct SEventInfo {
    uint32_t dwID;
    char szName[EVENT_MAX_NAME_LEN + 1];
    uint8_t bPeriod;
    uint8_t bType;
    char szDetails[QUERY_MAX_LEN + 1];
    long lMapIndex;
    uint8_t bChannel;
    char szStartTime[EVENT_MAX_NAME_LEN + 1];
    uint32_t dwEndTime;
    bool bIsNotice;
} TEventInfo;

typedef struct SPacketAutoEvents {
    uint8_t header;
    uint32_t dwID;
    bool bValue;
    bool bEnd;
} TPacketAutoEvents;
#endif

#ifdef __DUNGEON_FOR_GUILD__
typedef struct SPacketGDGuildDungeon {
    uint32_t dwGuildID;
    uint8_t bChannel;
    long lMapIndex;
} TPacketGDGuildDungeon;

typedef struct SPacketDGGuildDungeon {
    uint32_t dwGuildID;
    uint8_t bChannel;
    long lMapIndex;
} TPacketDGGuildDungeon;

typedef struct SPacketGDGuildDungeonCD {
    uint32_t dwGuildID;
    uint32_t dwTime;
} TPacketGDGuildDungeonCD;

typedef struct SPacketDGGuildDungeonCD {
    uint32_t dwGuildID;
    uint32_t dwTime;
} TPacketDGGuildDungeonCD;
#endif

#ifdef ENABLE_GEM_SYSTEM
typedef struct SPacketGemLog {
    uint8_t type;
    int gem;
} TPacketGemLog;
#endif

#ifdef __OFFLINE_SHOP__
typedef struct SOfflineShopData {
    uint32_t dwOwnerPID;
    char szOwnerName[CHARACTER_NAME_MAX_LEN + 1];
    char szName[SHOP_SIGN_MAX_LEN + 1];

    Gold llGold;

    long lMapIndex;
    long lX, lY;

    uint8_t byChannel;

#if defined(__OFFLINE_SHOP_OPENING_TIME__) ||                                  \
    defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
    int iLeftOpeningTime;
#endif
#if !defined(__OFFLINE_SHOP_OPENING_TIME__)
    bool bOpen;
#endif
} TOfflineShopData;

typedef struct SPacketDGOfflineShopCreate {
    TOfflineShopData kShopData;
} TPacketDGOfflineShopCreate;

typedef struct SPacketGDOfflineShopCreate {
    uint32_t dwOwnerPID;
    char szOwnerName[CHARACTER_NAME_MAX_LEN + 1];
    char szName[SHOP_SIGN_MAX_LEN + 1];

    long lMapIndex;
    long lX, lY;
} TPacketGDOfflineShopCreate;

typedef struct SPacketOfflineShopUpdateName {
    uint32_t dwOwnerPID;
    char szName[SHOP_SIGN_MAX_LEN + 1];
} TPacketOfflineShopUpdateName;

typedef struct SPacketOfflineShopUpdateGold {
    uint32_t dwOwnerPID;
    Gold llGold;
} TPacketOfflineShopUpdateGold;

typedef struct SPacketGDOfflineShopUpdatePosition {
    uint32_t dwOwnerPID;
    long lMapIndex;
    long lX, lY;
} TPacketGDOfflineShopUpdatePosition;

typedef struct SPacketOfflineShopAddItem {
    uint32_t dwOwnerPID;
    TOfflineShopItemData kItem;
} TPacketOfflineShopAddItem;

typedef struct SPacketOfflineShopSendItemInfo {
    uint32_t dwOwnerPID;
    uint32_t itemId;
    TOfflineShopItemData kItem;
} TPacketOfflineShopSendItemInfo;

typedef struct SPacketOfflineShopRemoveItem {
    uint32_t dwOwnerPID;
    uint32_t dwPosition;
} TPacketOfflineShopRemoveItem;

typedef struct SPacketOfflineShopRemoveItemById {
    uint32_t dwOwnerPID;
    uint32_t itemId;
} TPacketOfflineShopRemoveItemById;

using TPacketOfflineShopRequestItem = TPacketOfflineShopRemoveItemById;

typedef struct SPacketOfflineShopMoveItem {
    uint32_t dwOwnerPID;
    uint32_t dwOldPosition;
    uint32_t dwNewPosition;
} TPacketOfflineShopMoveItem;

typedef struct SPacketOfflineShopDestroy {
    uint32_t dwOwnerPID;
} TPacketOfflineShopDestroy;

typedef struct SPacketGDOfflineShopRegisterListener {
    uint32_t dwOwnerPID;
    bool bNeedOfflineShopInfos;
} TPacketGDOfflineShopRegisterListener;

typedef struct SPacketGDOfflineShopUnregisterListener {
    uint32_t dwOwnerPID;
} TPacketGDOfflineShopUnregisterListener;

typedef struct SPacketOfflineShopClose {
    uint32_t dwOwnerPID;
} TPacketOfflineShopClose;

typedef struct SPacketOfflineShopOpen {
    uint32_t dwOwnerPID;
#ifdef __OFFLINE_SHOP_OPENING_TIME__
    int iOpeningTime;
#endif
} TPacketOfflineShopOpen;

typedef struct SPacketOfflineShopSearch {
    int32_t filterType = -1;
    int32_t itemType = -1;
    int32_t itemSubType = -1;
    int64_t vnumFilter = -1;
    int64_t vnumExtraFilter = -1;
} TPacketOfflineShopSearch;


#if defined(__OFFLINE_SHOP_OPENING_TIME__) ||                                  \
    defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
typedef struct SPacketGDOfflineShopFlushLeftOpeningTime {
    uint32_t dwOwnerPID;
    int iLeftOpeningTime;
} TPacketGDOfflineShopFlushLeftOpeningTime;
#endif
#endif

struct PacketGDMessengerSetBlock {
    uint32_t pid;
    uint32_t other_pid;
    uint16_t mode;
};

struct DgHGuardResult {
    enum { kHGuardOk, kHGuardCheck, kHGuardIncorrect };
    uint8_t code;
};

#pragma pack(pop)

METIN2_END_NS

#endif
