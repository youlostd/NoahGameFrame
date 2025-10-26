#ifndef __INC_PACKET_H__
#define __INC_PACKET_H__

#include <boost/fusion/include/adapt_struct.hpp>
#include <Services.hpp>
#include <game/Types.hpp>
#include <game/ItemTypes.hpp>
#include <game/SkillConstants.hpp>
#include <game/DungeonInfoTypes.hpp>
#include "base/Serialization.hpp"

enum EMoveFuncType {
    FUNC_WAIT,
    FUNC_MOVE,
    FUNC_ATTACK,
    FUNC_COMBO,
    FUNC_MOB_SKILL,
    _FUNC_SKILL,
    FUNC_MAX_NUM,
    FUNC_SKILL = 0x80,
};

enum
{
    /************************************************************************/
    /* Packets from Client to Server                    */
    /************************************************************************/

    HEADER_CG_PIN,
    HEADER_CG_MARK_UPLOAD,
    HEADER_CG_OFFLINE_SHOP_POSITION,
    HEADER_CG_KEY_LOGIN,
    HEADER_CG_CHARACTER_MOVE,
    HEADER_CG_OFFLINE_SHOP_REOPEN,
    HEADER_CG_OFFLINE_SHOP_ITEM_ADD,
    HEADER_CG_QUICKSLOT_SWAP,
    HEADER_CG_MYSHOP_OPEN,
    HEADER_CG_FLY_TARGETING,
    HEADER_CG_OFFLINE_SHOP_CREATE,
    HEADER_CG_OFFLINE_SHOP_NAME,
    HEADER_CG_MARK_IDXLIST,
    HEADER_CG_CHOOSE_SKILL_GROUP,
    HEADER_CG_MARK_LOGIN,
    HEADER_CG_REQ_TIMESYNC,
    HEADER_CG_ITEM_USE_TO_ITEM,
    HEADER_CG_SCRIPT_SELECT_ITEM,
    HEADER_CG_HGUARD_PARAMETERS,
    HEADER_CG_ITEM_DROP,
    HEADER_CG_OFFLINE_SHOP_CLICK,
    HEADER_CG_OFFLINE_SHOP_ITEM_MOVE,
    HEADER_CG_FISHING,
    HEADER_CG_MYSHOP_DECO,
    HEADER_CG_ITEM_PICKUP,
    HEADER_CG_QUEST_RECEIVE,
    HEADER_CG_PARTY_REMOVE,
    HEADER_CG_PARTY_INVITE,
    HEADER_CG_CHANGE_SKILL_COLOR,
    HEADER_CG_PARTY_PARAMETER,
    HEADER_CG_ITEM_DROP2,
    HEADER_CG_PARTY_INVITE_ANSWER,
    HEADER_CG_ACCE,
    HEADER_CG_CHARACTER_CREATE,
    HEADER_CG_SHOP_SEARCH,
    HEADER_CG_ITEM_DESTROY,
    HEADER_CG_SCRIPT_ANSWER,
    HEADER_CG_SHOP_SEARCH_SUB,
    HEADER_CG_SHOOT,
    HEADER_CG_CHANGE_NAME,
    HEADER_CG_SHOP,
    HEADER_CG_TARGET,
    HEADER_CG_BLOCK_MODE,
    HEADER_CG_SYMBOL_CRC,
    HEADER_CG_GUILD,
    HEADER_CG_SAFEBOX_CHECKOUT,
    HEADER_CG_OFFLINE_SHOP_ITEM_REMOVE,
    HEADER_CG_GUILD_SYMBOL_UPLOAD,
    HEADER_CG_ITEM_USE,
    HEADER_CG_SAFEBOX_CHECKIN,
    HEADER_CG_ITEM_COMBINATION,
    HEADER_CG_ATTACK,
    HEADER_CG_MALL_CHECKOUT,
    HEADER_CG_QUICKSLOT_DEL,
    HEADER_CG_REFINE,
    HEADER_CG_DUNGEON,
    HEADER_CG_MARK_CRCLIST,
    HEADER_CG_OFFLINE_SHOP_ITEM_BUY,
    HEADER_CG_MYSHOP,
    HEADER_CG_QUEST_INPUT_STRING,
    HEADER_CG_QUEST_CONFIRM,
    HEADER_CG_PARTY_SET_STATE,
    HEADER_CG_QUICKSLOT_ADD,
    HEADER_CG_MESSENGER,
    HEADER_CG_TEXT = '@',
    HEADER_CG_ENTERGAME,
    HEADER_CG_HACK,
    HEADER_CG_ITEM_MOVE,
    HEADER_CG_ITEM_SPLIT,
    HEADER_CG_TARGET_LOAD,
    HEADER_CG_ON_CLICK,
    HEADER_CG_WHISPER,
    HEADER_CG_SAFEBOX_ITEM_MOVE,
    HEADER_CG_CHARACTER_DELETE,
    HEADER_CG_ADD_FLY_TARGETING,
    HEADER_CG_ANSWER_MAKE_GUILD,
    HEADER_CG_CHAT,
    HEADER_CG_QUEST_INPUT_STRING_LONG,
    HEADER_CG_MYSHOP_OPEN_SEARCH,
    HEADER_CG_SCRIPT_BUTTON,
    HEADER_CG_EXCHANGE,
    HEADER_CG_SHOP_SEARCH_BUY,
    HEADER_CG_ITEM_GIVE,
    HEADER_CG_CHECK_COUNTRY_WHISPER,
    HEADER_CG_PARTY_USE_SKILL,
    HEADER_CG_OFFLINE_SHOP_WITHDRAW_GOLD,
    HEADER_CG_CHANGE_EMPIRE,
    HEADER_CG_DRAGON_SOUL_REFINE,
    HEADER_CG_OFFLINE_SHOP,
    HEADER_CG_HGUARD_CODE,
    HEADER_CG_MYSHOP_WARP,
    HEADER_CG_SYNC_POSITION,
    HEADER_CG_CHARACTER_SELECT,
    HEADER_CG_ITEM_USE_MULTIPLE,
    HEADER_CG_USE_SKILL,
    HEADER_CG_SWITCHBOT,
    HEADER_CG_WIKI_REQUEST,
    HEADER_CG_CHANGELOOK,
    HEADER_CG_SET_TITLE,

#ifdef ENABLE_BATTLE_PASS
    HEADER_CG_BATTLE_PASS,
#endif
    HEADER_CG_CHAT_FILTER,
#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
    HEADER_CG_CUBE_RENEWAL,
#endif
    HEADER_CG_LEVEL_PET,
    HEADER_CG_FISHING_GAME,
    HEADER_CG_DUNGEON_INFO,
    HEADER_CG_DUNGEON_WARP,
    HEADER_CG_DUNGEON_RANKING,
    HEADER_CG_HANDSHAKE = 255,
    HEADER_CG_PONG = 251,
    HEADER_CG_TIME_SYNC = 252,
    HEADER_CG_PHASE_ACK = 253,


    /************************************************************************/
    /* Packets from Server to Client                    */
    /************************************************************************/

    /********************************************************/
    HEADER_GC_TIME_SYNC = 252,
    HEADER_GC_PHASE = 253,
    HEADER_GC_HANDSHAKE = 255,
    HEADER_GC_TEXT = 254,


    SEP = 0,
    HEADER_GC_SHOPSEARCH,
    HEADER_GC_MALL_DEL,
    HEADER_GC_CHARACTER_CREATE_FAILURE,
    HEADER_GC_NPC_SKILL,
    HEADER_GC_SKILL_MOTION,
    HEADER_GC_PARTY_ADD,
    HEADER_GC_UPDATE_LAST_PLAY,
    HEADER_GC_CHARACTER_GOLD_CHANGE,
    HEADER_GC_REQUEST_MAKE_GUILD,    
    HEADER_GC_SAFEBOX_DEL,
    HEADER_GC_SCRIPT,
    HEADER_GC_MAIN_CHARACTER,
    HEADER_GC_GUILD_LIST,
    HEADER_GC_CHAR_ADDITIONAL_INFO,
    HEADER_GC_QUICKSLOT_DEL,
    HEADER_GC_TARGET_INFO,
    HEADER_GC_WHISPER,
    HEADER_GC_ITEM_OWNERSHIP,
    HEADER_GC_FISHING,
    HEADER_GC_OFFLINE_SHOP_ADDITIONAL_INFO,
    HEADER_GC_CODE_RESULT,
    HEADER_GC_ACCE,
    HEADER_GC_SYMBOL_DATA,
    HEADER_GC_SHOP_SIGN,
    HEADER_GC_SYNC_POSITION,
    HEADER_GC_CHARACTER_DELETE_SUCCESS,
    HEADER_GC_MOUNT,
    HEADER_GC_SAFEBOX_WRONG_PASSWORD,
    HEADER_GC_PIN,
    HEADER_GC_OFFLINE_SHOP_EDITOR_POSITION_INFO,
    HEADER_GC_DIG_MOTION,
    HEADER_GC_CHARACTER_UPDATE,
    HEADER_GC_UPDATE_LAND,
    HEADER_GC_OFFLINE_SHOP_LEFT_OPENING_TIME,
    HEADER_GC_VIEW_EQUIP,
    HEADER_GC_ITEM_SET,
    HEADER_GC_PARTY_REMOVE,
    HEADER_GC_SAFEBOX_SET,
    HEADER_GC_SPECIFIC_EFFECT,
    HEADER_GC_TARGET_CREATE,
    HEADER_GC_QUEST_INFO,
    HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID,
    HEADER_GC_ITEM_DROP,
    HEADER_GC_AFFECT_REMOVE,
    HEADER_GC_FLY_TARGETING,
    HEADER_GC_SET_PIN,
    HEADER_GC_PARTY_PARAMETER,
    HEADER_GC_DUNGEON,
    HEADER_GC_SEAL,
    HEADER_GC_LOGIN_SUCCESS,
    HEADER_GC_CHANNEL,
    HEADER_GC_LOGIN_FAILURE,
    HEADER_GC_MESSENGER,
    HEADER_GC_MAINTENANCE,
    HEADER_GC_BOT_REPORT,
    HEADER_GC_QUICKSLOT_SWAP,
    HEADER_GC_MARK_IDXLIST,
    HEADER_GC_PARTY_LINK,
    HEADER_GC_SAFEBOX_SIZE,
    HEADER_GC_REFINE_INFORMATION,
    HEADER_GC_CHARACTER_GOLD,
    HEADER_GC_PARTY_INVITE,
    HEADER_GC_STUN,
    HEADER_GC_ADD_FLY_TARGETING,
    HEADER_GC_DUEL_START,
    HEADER_GC_TARGET_UPDATE,
    HEADER_GC_ATTACK,
    HEADER_GC_ITEM_UPDATE,
    HEADER_GC_AFFECT_ADD,
    HEADER_GC_GUILD,
    HEADER_GC_CHANGE_EMPIRE,
    HEADER_GC_AUTH_SUCCESS,
    HEADER_GC_ITEM_GROUND_ADD,
    HEADER_GC_OFFLINE_SHOP_ITEM_ADD,
    HEADER_GC_LOVE_POINT_UPDATE,
    HEADER_GC_CHANGE_NAME,
    HEADER_GC_REFINE_INFORMATION_OLD,
    HEADER_GC_DAMAGE_INFO,
    HEADER_GC_SKILL_LEVEL,
    HEADER_GC_CHARACTER_POINTS,
    HEADER_GC_CHAT,
    HEADER_GC_AUTH_SUCCESS_OPENID,
    HEADER_GC_MAIN_CHARACTER4_BGM_VOL,
    HEADER_GC_ITEM_GROUND_DEL,
    HEADER_GC_PET_UPDATE,
    HEADER_GC_SKILL_GROUP,
    HEADER_GC_MARK_BLOCK,
    HEADER_GC_SAME_LOGIN,
    HEADER_GC_OPEN_COSTUME,
    HEADER_GC_ITEM_DEL,
    HEADER_GC_CHARACTER_CREATE_SUCCESS,
    HEADER_GC_OFFLINE_SHOP,
    HEADER_GC_CHARACTER_DEL,
    HEADER_GC_CHARACTER_POINT_CHANGE,
    HEADER_GC_MAIN_CHARACTER_OLD,
    HEADER_GC_SEPCIAL_EFFECT,
    HEADER_GC_MALL_SET,
    HEADER_GC_SHOP,
    HEADER_GC_DEAD,
    HEADER_GC_TARGET,
    HEADER_GC_EXCHANGE_INFO,
    HEADER_GC_PVP,
    HEADER_GC_THREEWAY_LIVES,
    HEADER_GC_LAND_LIST,
    HEADER_GC_CREATE_FLY,
    HEADER_GC_PARTY_UPDATE,
    HEADER_GC_EXCHANGE,
    HEADER_GC_DRAGON_SOUL_REFINE,
    HEADER_GC_CHARACTER_MOVE,
    HEADER_GC_OFFLINE_SHOP_ITEM_REMOVE,
    HEADER_GC_PANAMA_PACK,
    HEADER_GC_HYBRIDCRYPT_KEYS,
    HEADER_GC_THREEWAY_STATUS,
    HEADER_GC_OFFLINE_SHOP_NAME,
    HEADER_GC_CHARACTER_ADD,
    HEADER_GC_NPC_POSITION,
    HEADER_GC_MALL_OPEN,
    HEADER_GC_PING,
    HEADER_GC_SHOP_POSITION,
    HEADER_GC_QUEST_CONFIRM,
    HEADER_GC_TIME,
    HEADER_GC_OFFLINE_SHOP_GOLD,
    HEADER_GC_SKILL_LEVEL_OLD,
    HEADER_GC_LOVER_INFO,
    HEADER_GC_OFFLINE_SHOP_ITEM_MOVE,
    HEADER_GC_PARTY_POSITION_INFO,
    HEADER_GC_LOGIN_KEY,
    HEADER_GC_WARP,
    HEADER_GC_PARTY_UNLINK,
    HEADER_GC_WALK_MODE,
    HEADER_GC_OWNERSHIP,
    HEADER_GC_TARGET_DELETE,
    HEADER_GC_INPUT_STATE,
    HEADER_GC_QUICKSLOT_ADD,
    HEADER_GC_RESPOND_CHANNELSTATUS,
    HEADER_GC_REMOVE_METIN,
    HEADER_GC_SWITCHBOT_DATA,
    HEADER_GC_WIKI,
    HEADER_GC_WIKI_MOB,
    HEADER_GC_CHANGELOOK,
    HEADER_GC_PICKUP_INFO,
#ifdef ENABLE_BATTLE_PASS
    HEADER_GC_BATTLE_PASS_OPEN,
    HEADER_GC_BATTLE_PASS_UPDATE,
    HEADER_GC_BATTLE_PASS_RANKING,
#endif
#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
    HEADER_GC_CUBE_RENEWAL,
#endif
    HEADER_GC_UPDATE_CHAT_FILTER,
    HEADER_GC_HUNTING_MISSIONS,
    HEADER_GC_HUNTING_MISSION_UPDATE,
    HEADER_GC_LEVEL_PET,
    HEADER_GC_HYPERLINK_ITEM,
    HEADER_GC_MULTI_STATUS,
    HEADER_GC_DUNGEON_INFO,
    HEADER_GC_DUNGEON_RANKING,
    HEADER_GC_DUNGEON_UPDATE,
};

enum SwitchbotCgPacketHeaders
{
    SWITCHBOT_SUBHEADER_CG_SET_ATTRIBUTE,
    SWITCHBOT_SUBHEADER_CG_START_SLOT,
    SWITCHBOT_SUBHEADER_CG_STOP_SLOT,
};

#pragma pack(1)


struct BlankPacket
{
    uint8_t dummydata;
};

BOOST_FUSION_ADAPT_STRUCT(
    BlankPacket,
    dummydata
)

struct BlankDynamicPacket
{
        uint8_t dummydata;
};

BOOST_FUSION_ADAPT_STRUCT(
    BlankDynamicPacket,
    dummydata
)

/* Å¬¶óÀÌ¾ðÆ® Ãø¿¡¼­ º¸³»´Â ÆÐÅ¶ */

using CgPhaseAckPacket = BlankPacket ;

struct TPacketCGHandshake
{
    uint32_t dwHandshake;
    uint32_t dwTime;
    int32_t lDelta;
} ;

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGHandshake,
    dwHandshake,
    dwTime,
    lDelta
)

struct CgKeyLoginPacket
{
    std::string login;
    uint64_t sessionId;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgKeyLoginPacket,
    login,
    sessionId
)

struct TPacketCGPlayerSelect
{
    uint8_t index;
    std::string localeName;
} ;

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGPlayerSelect,
    index,
    localeName
)

struct TPacketCGPlayerDelete
{
    uint8_t index;
    std::string private_code;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGPlayerDelete,
    index,
    private_code
)

struct GcCharacterDeleteSuccessPacket
{
    uint8_t slot;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcCharacterDeleteSuccessPacket,
    slot
)

struct TPacketCGPlayerCreate
{
    uint8_t index;
    std::string name;
    uint16_t job;
    uint8_t shape;
    uint8_t Con;
    uint8_t Int;
    uint8_t Str;
    uint8_t Dex;
    uint8_t empire;
} ;

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGPlayerCreate,
    index,
    name,
    job,
    shape,
    Con,
    Int,
    Str,
    Dex,
    empire
)

struct TPacketGCPlayerCreateSuccess
{
    uint8_t slot;
    SimplePlayer player;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCPlayerCreateSuccess,
    slot,
    player
)
// °ø°Ý
struct TPacketCGAttack
{
    uint8_t    bType;            // 공격 유형
    uint32_t    dwVictimVID;    // 적 VID
    uint32_t    attackTime;
    uint32_t    motionKey;
    uint32_t    x, y;
    uint32_t    pushX, pushY;
    uint32_t    victimX, victimY;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGAttack,
    bType,
    dwVictimVID,
    attackTime,
    motionKey,
    x,
    y,
    pushX,
    pushY,
    victimX,
    victimY
)

// ÀÌµ¿

struct GcSyncOwnerPacket
{
    uint32_t ownerVid;
    uint32_t victimVid;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcSyncOwnerPacket,
    ownerVid,
    victimVid
)

struct TPacketCGSyncPositionElement
{
    uint32_t dwVID;
    int32_t lX;
    int32_t lY;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGSyncPositionElement,
    dwVID,
    lX,
    lY
)


// À§Ä¡ µ¿±âÈ­
struct TPacketCGSyncPosition // °¡º¯ ÆÐÅ¶
{
    std::vector<TPacketCGSyncPositionElement> elems;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGSyncPosition,
    elems
)


/* Ã¤ÆÃ (3) */
struct TPacketCGChat // °¡º¯ ÆÐÅ¶
{
    uint8_t type;
    uint8_t lang;
    std::string message;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGChat,
    type,
    lang,
    message
)

/* ±Ó¼Ó¸» */
struct TPacketCGWhisper
{
  std::string szNameTo;
  std::string message;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGWhisper,
    szNameTo,
    message
)

struct TPacketCGEnterGame
{
    uint8_t dummy;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGEnterGame,
    dummy
)

struct TPacketCGItemUse
{
    TItemPos Cell;
};
using TPacketGCItemUse = TPacketCGItemUse;

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGItemUse,
    Cell
)

struct CgSetTitlePacket
{
    uint32_t color;
    std::string szTitle;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgSetTitlePacket,
    color,
    szTitle
)

struct TPacketCGItemUseToItem
{
    TItemPos Cell;
    TItemPos TargetCell;
};


BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGItemUseToItem,
    Cell,
    TargetCell
)


struct TPacketCGItemDrop
{
    TItemPos Cell;
    Gold gold;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGItemDrop,
    Cell,
    gold
)

struct TPacketCGItemDrop2
{
    TItemPos Cell;
    Gold gold;
    CountType count;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGItemDrop2,
    Cell,
    gold,
    count
)

struct TPacketCGItemMove
{
    TItemPos Cell;
    TItemPos CellTo;
    CountType count;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGItemMove,
    Cell,
    CellTo,
    count
)

struct TPacketCGItemSplit
{
    TItemPos Cell;
    CountType count;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGItemSplit,
    Cell,
    count
)

struct TPacketCGItemPickup
{
    uint32_t vid;
    uint32_t time;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGItemPickup,
    vid,
    time
)

struct TPacketCGQuickslotAdd
{
    uint8_t pos;
    TQuickslot slot;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGQuickslotAdd,
    pos,
    slot
)

struct TPacketCGQuickslotDel
{
    uint8_t pos;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGQuickslotDel,
    pos
)

struct TPacketCGQuickslotSwap
{
    uint8_t pos;
    uint8_t change_pos;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGQuickslotSwap,
    pos,
    change_pos
)

struct TPacketPlayerShopSet
{
    uint8_t pos;
    uint32_t vnum;
    uint32_t transVnum;
    CountType count;
    Gold price;
    SocketValue alSockets[ITEM_SOCKET_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketPlayerShopSet,
    pos,
    vnum,
    transVnum,
    count,
    price,
    alSockets,
    aAttr
)


struct TPacketPlayerShopSign
{
    uint8_t type;
    std::string sign;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketPlayerShopSign,
    type,
    sign
)

enum
{
    SHOP_SUBHEADER_CG_END,
    SHOP_SUBHEADER_CG_BUY,
    SHOP_SUBHEADER_CG_SELL,
    SHOP_SUBHEADER_CG_SELL2
};

struct TPacketCGShopBuy
{
    CountType count;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGShopBuy,
    count
)

struct TPacketCGShopSell
{
    uint8_t pos;
    CountType count;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGShopSell,
    pos,
    count
)

struct CgShopActionBuy
{
    uint16_t pos;
    uint8_t amount;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgShopActionBuy,
    pos,
    amount
)

struct CgShopActionSell
{
    TItemPos pos;
    CountType amount;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgShopActionSell,
    pos,
    amount
)

struct TPacketCGShop
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<CgShopActionBuy, 0> buyAction;
    OptionalField<CgShopActionSell, 1> sellAction;

};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGShop,
    subheader,
    opts,
    buyAction,
    sellAction
)


struct TPacketCGOnClick
{
    uint32_t vid;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGOnClick,
    vid
)

enum
{
    EXCHANGE_SUBHEADER_CG_START,
    /* arg1 == vid of target character */
    EXCHANGE_SUBHEADER_CG_ITEM_ADD,
    /* arg1 == position of item */
    EXCHANGE_SUBHEADER_CG_ITEM_DEL,
    /* arg1 == position of item */
    EXCHANGE_SUBHEADER_CG_ELK_SET,
    /* arg1 == amount of gold */
    EXCHANGE_SUBHEADER_CG_ACCEPT,
    /* arg1 == not used */
    EXCHANGE_SUBHEADER_CG_CANCEL,
    /* arg1 == not used */
};

struct TPacketCGExchange
{
    uint8_t sub_header;
    ExchangeArg1 arg1;
    uint8_t arg2;
    TItemPos Pos;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGExchange,
    sub_header,
    arg1,
    arg2,
    Pos
)

struct TPacketCGPosition
{
    uint8_t position;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGPosition,
    position
)

struct TPacketCGScriptAnswer
{
    uint8_t answer;
    int32_t qIndex;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGScriptAnswer,
    answer,
    qIndex
)

struct TPacketCGScriptButton
{
    uint32_t idx;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGScriptButton,
    idx
)

struct TPacketCGQuestInputString
{
    std::string msg;
    int32_t qIndex;
} ;

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGQuestInputString,
    msg,
    qIndex
)

/* New packet: Communicate with quests from client */
struct TPacketCGQuestRcv
{
    uint32_t questID;
    std::string msg;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGQuestRcv,
    questID,
    msg
)
/* End of QUEST_RECEIVE packet*/

struct TPacketCGQuestConfirm
{
    uint8_t answer;
    uint32_t requestPID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGQuestConfirm,
    answer,
    requestPID
)

/*
 * ¼­¹ö Ãø¿¡¼­ º¸³»´Â ÆÐÅ¶ 
 */
struct TPacketGCQuestConfirm
{
    std::string msg;
    int32_t timeout;
    uint32_t requestPID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCQuestConfirm,
    msg,
    timeout,
    requestPID
)

using TPacketGCSyncPositionElement = TPacketCGSyncPositionElement;
using TPacketGCSyncPosition = TPacketCGSyncPosition;


struct TPacketGCOwnership
{
    uint32_t           dwOwnerVID;
    uint32_t           dwVictimVID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCOwnership,
    dwOwnerVID,
    dwVictimVID
)

struct TPacketGCHandshake
{
    uint32_t dwHandshake;
    uint32_t dwTime;
    int32_t lDelta;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCHandshake,
    dwHandshake,
    dwTime,
    lDelta
)

enum EPhase
{
    PHASE_CLOSE,
    PHASE_HANDSHAKE,
    PHASE_LOGIN,
    PHASE_SELECT,
    PHASE_LOADING,
    PHASE_GAME,
    PHASE_DEAD,

    PHASE_CLIENT_CONNECTING,
    PHASE_DBCLIENT,
    PHASE_P2P,
    PHASE_AUTH,

    PHASE_HGUARD,
    PHASE_SAME_LOGIN,
};

struct TPacketGCPhase
{
    uint8_t phase;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCPhase,
    phase
)

enum
{
    LOGIN_FAILURE_ALREADY = 1,
    LOGIN_FAILURE_ID_NOT_EXIST = 2,
    LOGIN_FAILURE_WRONG_PASS = 3,
    LOGIN_FAILURE_FALSE = 4,
    LOGIN_FAILURE_NOT_TESTOR = 5,
    LOGIN_FAILURE_NOT_TEST_TIME = 6,
    LOGIN_FAILURE_FULL = 7
};

struct TPacketGCLoginSuccess
{
    SimplePlayer players[PLAYER_PER_ACCOUNT];
    uint32_t guild_id[PLAYER_PER_ACCOUNT];
    char guild_name[PLAYER_PER_ACCOUNT][GUILD_NAME_MAX_LEN + 1];

    uint32_t handle;
    uint32_t random_key;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCLoginSuccess,
    players,
    guild_id,
    guild_name,
    handle,
    random_key
)

struct TPacketGCAuthSuccess
{
    uint32_t dwLoginKey;
    uint8_t bResult;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCAuthSuccess,
    dwLoginKey,
    bResult
)


struct TPacketGCLoginFailure
{
    std::string szStatus;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCLoginFailure,
    szStatus
)

struct GcCharacterPhaseFailurePacket
{
    enum CreateType
    {
        kEinval,
        kBlocked,
        kLoginName,
        kLevel,
        kAlready,
        kAlreadySlot,
        kGmBlocked,
        kNameLength,
        kRaceBlocked,
        kTimeLimit,
        kFullAccount,
        kInvalidRace,
        kInvalidShape,
        kInvalidName,
        kInvalidEmpire,
    };

    enum SelectType
    {
        kInvalidNameSelect,
        kUsedName,
        kStrangeIndex = 100,
    };

    uint8_t type;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcCharacterPhaseFailurePacket,
    type
)

using GcCharacterRenameFailurePacket = GcCharacterPhaseFailurePacket;
using GcCharacterCreateFailurePacket = GcCharacterPhaseFailurePacket;
enum
{
    ADD_CHARACTER_STATE_DEAD = (1 << 0),
    ADD_CHARACTER_STATE_SPAWN = (1 << 1),
    ADD_CHARACTER_STATE_GUNGON = (1 << 2),
    ADD_CHARACTER_STATE_KILLER = (1 << 3),
    ADD_CHARACTER_STATE_PARTY = (1 << 4),
    ADD_CHARACTER_STATE_GM = (1 << 5),
    ADD_CHARACTER_STATE_SGM = (1 << 6),
    ADD_CHARACTER_STATE_SA = (1 << 7),

};


struct TPacketGCCharacterAdd
{
    uint32_t dwVID;
    uint32_t dwLevel = 0;
    uint32_t dwAIFlag = 0;

    float angle;
    int32_t x;
    int32_t y;
    int32_t z;

    uint8_t bType;
    uint16_t wRaceNum;
    uint16_t bMovingSpeed;
    uint8_t bAttackSpeed;

    uint8_t bStateFlag;
    uint32_t xRot, yRot;

    uint32_t guildID = 0;
    uint16_t scale = 100;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCCharacterAdd,
    dwVID,
    dwLevel,
    dwAIFlag,
    angle,
    x,
    y,
    z,
    bType,
    wRaceNum,
    bMovingSpeed,
    bAttackSpeed,
    bStateFlag,
    xRot,
    yRot,
    guildID,
    scale
)


struct TPacketGCCharacterAdditionalInfo
{
    std::string name;
    Part adwPart[PART_MAX_NUM];
    TPlayerTitle pt;
    uint32_t dwVID = 0;
    uint32_t dwGuildID = 0;
    uint32_t dwLevel = 0;
    uint32_t dwMountVnum = 0;
    uint32_t ownerVid = 0;
    uint16_t scale = 100;
    int16_t pvpTeam = 0;
    PlayerAlignment sAlignment = 0;
    uint8_t bPKMode = 0;
    uint8_t bEmpire = 0;
    uint8_t lang = 0;
    uint8_t isGuildLeader = 0;
    uint8_t isGuildGeneral = 0;
#ifdef ENABLE_PLAYTIME_ICON
    DWORD dwPlayTime = 0;
#endif
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCCharacterAdditionalInfo,
    name,
    adwPart,
    pt,
    dwVID,
    dwGuildID,
    dwLevel,
    dwMountVnum,
    ownerVid,
    scale,
    pvpTeam,
    sAlignment,
    bPKMode,
    bEmpire,
    lang,
    isGuildLeader,
    isGuildGeneral,
    dwPlayTime
)
struct TPacketGCCharacterUpdate
{
    uint8_t bAttackSpeed;
    uint8_t bStateFlag;
    uint8_t bPKMode;
    uint8_t comboLevel;
    uint16_t scale;
    uint16_t bMovingSpeed;
    uint16_t pvpTeam;
    uint32_t dwVID;
    uint32_t dwGuildID;
    uint32_t dwLevel;
    PlayerAlignment sAlignment;
    TPlayerTitle pt;
#ifdef ENABLE_PLAYTIME_ICON
    DWORD dwPlayTime;
#endif
    Part adwPart[PART_MAX_NUM];
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCCharacterUpdate,
    bAttackSpeed,
    bStateFlag,
    bPKMode,
    comboLevel,
    scale,
    bMovingSpeed,
    pvpTeam,
    dwVID,
    dwGuildID,
    dwLevel,
    sAlignment,
    pt,
    dwPlayTime,
    adwPart
)

struct TPacketGCCharacterDelete
{
    uint32_t id;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCCharacterDelete,
    id
)

struct TPacketGCChat
{
    uint8_t type;
    uint32_t id;
    uint8_t bEmpire;
    std::string message;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCChat,
    type,
    id,
    bEmpire,
    message
)

struct TPacketGCWhisper // °¡º¯ ÆÐÅ¶
{
    uint8_t bType;
    uint32_t pid;
    uint32_t localeCode;
    std::string szNameFrom;
    std::string message;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCWhisper,
    bType,
    pid,
    localeCode,
    szNameFrom,
    message
)

struct TPacketGCExchageInfo // °¡º¯ ÆÐÅ¶
{
    uint8_t bError;
    int32_t iUnixTime;
    std::string message;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCExchageInfo,
    bError,
    iUnixTime,
    message
)

struct GcMainCharacterPacket
{
    uint32_t dwVID;
    uint32_t blockMode;
    uint16_t wRaceNum;
    std::string szName;
    int32_t mapIndex;
    int32_t lx, ly, lz;
    uint8_t empire;
    uint8_t skill_group;
    uint8_t comboLevel;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcMainCharacterPacket,
    dwVID,
    blockMode,
    wRaceNum,
    szName,
    mapIndex,
    lx,
    ly,
    lz,
    empire,
    skill_group,
    comboLevel
)


struct TPacketGCPoints
{
    PointValue points[POINT_MAX_NUM];
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCPoints,
    points
)


struct TPacketGCGold
{
    Gold gold;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCGold,
    gold
)

struct TPacketGCSkillLevel
{
    TPlayerSkill skills[SKILL_MAX_NUM];
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCSkillLevel,
    skills
)

struct TPacketGCPointChange
{
    uint32_t dwVID;
    uint8_t type;
    PointValue amount;
    PointValue value;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCPointChange,
    dwVID,
    type,
    amount,
    value
)

struct TPacketGCGoldChange
{
    uint32_t dwVID;
    Gold amount;
    Gold value;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCGoldChange,
    dwVID,
    amount,
    value
)

struct TPacketGCStun
{
    uint32_t vid;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCStun,
    vid
)

struct TPacketGCDead
{
    uint32_t vid;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCDead,
    vid
)

struct TPacketGCItemDelDeprecated
{
    TItemPos Cell;
    uint32_t vnum;
    CountType count;
    SocketValue alSockets[ITEM_SOCKET_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
};


struct GcItemSetPacket
{
    TItemPos pos;
    ClientItemData data;
    uint8_t highlight;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcItemSetPacket,
    pos,
    data,
    highlight
)

struct TPacketGCItemDel
{
    TItemPos pos;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCItemDel,
    pos
)

struct packet_item_move
{
    TItemPos Cell;
    TItemPos CellTo;
};

BOOST_FUSION_ADAPT_STRUCT(
    packet_item_move,
    Cell,
    CellTo
)

struct TPacketGCItemUpdate
{
    TItemPos Cell;
    CountType count;
    SocketValue alSockets[ITEM_SOCKET_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCItemUpdate,
    Cell,
    count,
    alSockets,
    aAttr
)

struct GcPacketItemGroundAdd
{
    int32_t x, y, z;
    uint32_t dwVID;
    uint32_t dwVnum;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcPacketItemGroundAdd,
    x,
    y,
    z,
    dwVID,
    dwVnum
)

struct TPacketGCItemOwnership
{
    uint32_t dwVID;
    std::string szName;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCItemOwnership,
    dwVID,
    szName
)

struct TPacketGCItemGroundDel
{
    uint32_t dwVID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCItemGroundDel,
    dwVID
)

struct packet_quickslot_add
{
    uint8_t pos;
    TQuickslot slot;
};

BOOST_FUSION_ADAPT_STRUCT(
    packet_quickslot_add,
    pos,
    slot
)

struct packet_quickslot_del
{
    uint8_t pos;
};

BOOST_FUSION_ADAPT_STRUCT(
    packet_quickslot_del,
    pos
)

struct packet_quickslot_swap
{
    uint8_t pos;
    uint8_t pos_to;
};

BOOST_FUSION_ADAPT_STRUCT(
    packet_quickslot_swap,
    pos,
    pos_to
)

struct packet_motion
{
    uint32_t vid;
    uint32_t victim_vid;
    uint16_t motion;
};

BOOST_FUSION_ADAPT_STRUCT(
    packet_motion,
    vid,
    victim_vid,
    motion
)
enum EPacketShopSubHeaders
{
    SHOP_SUBHEADER_GC_START,
    SHOP_SUBHEADER_GC_END,
    SHOP_SUBHEADER_GC_UPDATE_ITEM,
    SHOP_SUBHEADER_GC_UPDATE_PRICE,
    SHOP_SUBHEADER_GC_OK,
    SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY,
    SHOP_SUBHEADER_GC_SOLDOUT,
    SHOP_SUBHEADER_GC_INVENTORY_FULL,
    SHOP_SUBHEADER_GC_INVALID_POS,
    SHOP_SUBHEADER_GC_SOLD_OUT,
    SHOP_SUBHEADER_GC_START_EX,
    SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX,
    SHOP_SUBHEADER_GC_NOT_ENOUGH_CASH,

#if defined(WJ_COMBAT_ZONE)
    SHOP_SUBHEADER_GC_NOT_ENOUGH_POINTS,
    SHOP_SUBHEADER_GC_MAX_LIMIT_POINTS,
    SHOP_SUBHEADER_GC_OVERFLOW_LIMIT_POINTS,
#endif

};

struct packet_shop_item : ClientItemData
{
    Gold price;
    uint8_t display_pos;
};

BOOST_FUSION_ADAPT_STRUCT(
    packet_shop_item,
    highlighted,
    vnum,
    transVnum,
    nSealDate,
    count,
    sockets,
    attrs,
    price,
    display_pos

)

struct TPacketGCShopStart
{
    uint32_t owner_vid;
    packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShopStart,
    owner_vid,
    items
)

struct TPacketGCShopStartEx // 다음에 TSubPacketShopTab* shop_tabs 이 따라옴.
{
    struct TSubPacketShopTab
    {
        std::string name;
        uint8_t coin_type;
        uint32_t coin_vnum;
        packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
    };

    uint32_t owner_vid;
    uint8_t shop_tab_count;
    std::vector<TSubPacketShopTab> tabs;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShopStartEx::TSubPacketShopTab,
    name,
    coin_type,
    coin_vnum,
    items
)
BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShopStartEx,
    owner_vid,
    shop_tab_count,
    tabs
)

struct TPacketGCShopUpdateItem
{
    uint8_t pos;
    packet_shop_item item;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShopUpdateItem,
    pos,
    item
)

struct TPacketGCShopUpdatePrice
{
    int32_t iPrice;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShopUpdatePrice,
    iPrice
)

struct TPacketGCShop  // 가변 패킷
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<TPacketGCShopStart, 0> actionStart;
    OptionalField<TPacketGCShopStartEx, 1> actionStartEx;
    OptionalField<TPacketGCShopUpdateItem, 2> actionUpdate;
    OptionalField<int32_t, 3> actionPriceUpdate;
    OptionalField<int32_t, 4> actionNothEnough;

};


BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShop,
    subheader,
    opts,
    actionStart,
    actionStartEx,
    actionUpdate,
    actionPriceUpdate,
    actionNothEnough
)


struct GcMyShopCreatorOpen
{
    TItemPos bundleItem;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcMyShopCreatorOpen,
    bundleItem
)


struct GcMyShopPriceList
{
    struct Price
    {
        uint32_t vnum;
        Gold price;
    };

    uint16_t size;
    uint32_t count;
    // Price info[count] follows
};


BOOST_FUSION_ADAPT_STRUCT(
    GcMyShopPriceList::Price,
    vnum,
    price
)

BOOST_FUSION_ADAPT_STRUCT(
    GcMyShopPriceList,
    size,
    count
)

struct CgMyShopOpen
{
    std::string sign;
    TItemPos bundleItem;
    uint8_t count;
    std::vector<TShopItemTable> table;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgMyShopOpen,
    sign,
    bundleItem,
    count,
    table
)


struct packet_exchange
{
    uint8_t sub_header;
    OptionalFields opts;
    OptionalField<uint8_t, 0> is_me;
    OptionalField<uint64_t, 1> arg;
    OptionalField<ClientItemData, 2> itemData;
    OptionalField<TItemPos, 3> itemPos;
};

BOOST_FUSION_ADAPT_STRUCT(
    packet_exchange,
    sub_header,
    opts,
    is_me,
    arg,
    itemData,
    itemPos
)

enum EPacketTradeSubHeaders
{
    EXCHANGE_SUBHEADER_GC_START,
    /* arg1 == vid */
    EXCHANGE_SUBHEADER_GC_ITEM_ADD,
    /* arg1 == vnum  arg2 == pos  arg3 == count */
    EXCHANGE_SUBHEADER_GC_ITEM_DEL,
    EXCHANGE_SUBHEADER_GC_GOLD_ADD,
    /* arg1 == gold */
    EXCHANGE_SUBHEADER_GC_ACCEPT,
    /* arg1 == accept */
    EXCHANGE_SUBHEADER_GC_END,
    /* arg1 == not used */
    EXCHANGE_SUBHEADER_GC_ALREADY,
    /* arg1 == not used */
    EXCHANGE_SUBHEADER_GC_LESS_GOLD,
    /* arg1 == not used */
};

using TPacketGCPing = BlankPacket;

enum EPacketGiftExchangeSubHeaders
{
    GIFT_EXCHANGING_GC_ACCEPT,
};

enum EPAcketCGGiftExchangeSubHeaders
{
    GIFT_EXCHANGING_CG_REQUEST,
    GIFT_EXCHANGING_CG_ACCEPT,
};

struct packet_position
{
    uint32_t vid;
    uint8_t position;
};

BOOST_FUSION_ADAPT_STRUCT(
    packet_position,
    vid,
    position
)


struct packet_script
{
    uint8_t skin;
    uint8_t quest_flag;
    std::string script;
};


BOOST_FUSION_ADAPT_STRUCT(
    packet_script,
    skin,
    quest_flag,
    script
)

using GcScriptPacket = packet_script;

struct GcMountPacket
{
    uint32_t vid;
    uint32_t vnum;
    int32_t x, y;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcMountPacket,
    vid,
    vnum,
    x,
    y
)

// ¼ÒÀ¯±Ç


struct TPacketCGMove
{
    uint8_t  bFunc;
    uint8_t bArg;
    uint8_t  bRot;
    int32_t  lX;
    int32_t  lY;
    uint32_t dwTime;
    uint32_t color;
    uint32_t motionKey;
    uint8_t loopCount;
    uint8_t isMovingSkill;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGMove,
    bFunc,
    bArg,
    bRot,
    lX,
    lY,
    dwTime,
    color,
    motionKey
)

struct TPacketGCCreateFly
{
    uint8_t bType;
    uint32_t dwStartVID;
    uint32_t dwEndVID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCCreateFly,
    bType,
    dwStartVID,
    dwEndVID
)

struct TPacketCGFlyTargeting
{
    uint32_t dwTargetVID;
    int32_t x, y;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGFlyTargeting,
    dwTargetVID,
    x,
    y
)

struct TPacketGCFlyTargeting
{
    uint32_t dwShooterVID;
    uint32_t dwTargetVID;
    int32_t x, y;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCFlyTargeting,
    dwShooterVID,
    dwTargetVID,
    x,
    y
)

struct TPacketCGShoot
{
    uint8_t type;
    uint32_t motionKey;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGShoot,
    type,
    motionKey
)

struct TPacketGCDuelStart
{
  std::vector<uint32_t> participants;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCDuelStart,
    participants
)

enum EPVPModes
{
    PVP_MODE_NONE,
    PVP_MODE_AGREE,
    PVP_MODE_FIGHT,
    PVP_MODE_REVENGE
};

struct TPacketGCPVP
{
    uint32_t dwVIDSrc;
    uint32_t dwVIDDst;
    uint8_t bMode; // 0 ÀÌ¸é ²û, 1ÀÌ¸é ÄÔ
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCPVP,
    dwVIDSrc,
    dwVIDDst,
    bMode
)

struct TPacketCGUseSkill
{
    uint32_t dwVnum;
    uint32_t dwVID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGUseSkill,
    dwVnum,
    dwVID
)

struct TPacketCGTarget
{
    uint32_t dwVID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGTarget,
    dwVID
)

struct TPacketGCTarget
{
    uint32_t dwVID;
    int64_t prevHp, lHP, lMaxHP;
} ;

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCTarget,
    dwVID,
    prevHp,
    lHP,
    lMaxHP
)

struct TPacketGCWarp
{
    std::string lAddr;
    uint16_t wPort;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCWarp,
    lAddr,
    wPort
)

struct TPacketCGWarp
{
    uint8_t dummy;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGWarp,
    dummy
)

struct packet_quest_info
{
    uint16_t index;
    uint16_t c_index;
    uint8_t flag;
    OptionalFields opts;
    OptionalField<uint8_t, 0> isBegin;
    OptionalField<std::string, 1> title;
    OptionalField<std::string, 2> clockName;
    OptionalField<int32_t, 3> clockValue;
    OptionalField<std::string, 4> counterName;
    OptionalField<int32_t, 5> counterValue;
    OptionalField<std::string, 6> iconFile;

};

BOOST_FUSION_ADAPT_STRUCT(
    packet_quest_info,
    index,
    c_index,
    flag,
    opts,
    isBegin,
    title,
    clockName,
    clockValue,
    counterName,
    counterValue,
    iconFile
)



struct TPacketGCMove
{
    uint8_t        bFunc = 0; 
    uint8_t        bArg = 0;
    uint8_t        bRot = 0;
    uint32_t        dwVID = 0;
    int32_t        lX = 0;
    int32_t        lY = 0;
    uint32_t        dwTime = 0;
    uint32_t        dwDuration = 0;
    uint32_t    color = 0;
    uint8_t    isMovingSkill = 0;
    uint8_t    loopCount = 0;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCMove,
    bFunc,
    bArg,
    bRot,
    dwVID,
    lX,
    lY,
    dwTime,
    dwDuration,
    color
)


enum
{
    MESSENGER_SUBHEADER_GC_LIST,
    MESSENGER_SUBHEADER_GC_LOGIN,
    MESSENGER_SUBHEADER_GC_LOGOUT,
    MESSENGER_SUBHEADER_GC_INVITE,
    MESSENGER_SUBHEADER_GC_MOBILE,
    MESSENGER_SUBHEADER_GC_GM_LIST,
    MESSENGER_SUBHEADER_GC_GM_LOGIN,
    MESSENGER_SUBHEADER_GC_GM_LOGOUT,
    MESSENGER_SUBHEADER_GC_BLOCK_LIST,
    MESSENGER_SUBHEADER_GC_REMOVE_FRIEND,
};

struct TPacketGCMessengerGuildList
{
    uint8_t connected;
    uint8_t length;
    //char login[LOGIN_MAX_LEN+1];
};

struct TPacketGCMessengerGuildLogin
{
    uint8_t length;
    //char login[LOGIN_MAX_LEN+1];
} ;

struct TPacketGCMessengerGuildLogout
{
    uint8_t length;

    //char login[LOGIN_MAX_LEN+1];
} ;

struct TPacketGCMessengerList
{
    uint8_t connected; // always 0
    uint32_t lang;
    std::string name;
} ;

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCMessengerList,
    connected,
    lang,
    name
)

struct TPacketGCMessenger
{
    uint8_t subheader;
    OptionalFields opts;
    
    OptionalField<std::vector<TPacketGCMessengerList>, 0> list;
    OptionalField<std::vector<BlockedPC>, 1> listBlocked;
    OptionalField<std::string, 2> name;
    OptionalField<uint8_t, 3> state;
    OptionalField<uint8_t, 4> lang;


};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCMessenger,
    subheader,
    opts,
    list,
    listBlocked,
    name,
    state,
    lang
)


enum
{
    MESSENGER_SUBHEADER_CG_ADD_BY_VID,
    MESSENGER_SUBHEADER_CG_ADD_BY_NAME,
    MESSENGER_SUBHEADER_CG_REMOVE,
    MESSENGER_SUBHEADER_CG_INVITE_ANSWER,
    MESSENGER_SUBHEADER_CG_SET_BLOCK,

};

struct PacketCGMessengerSetBlock
{
    std::string name;
    uint16_t mode;
};

BOOST_FUSION_ADAPT_STRUCT(
    PacketCGMessengerSetBlock,
    name,
    mode
)

struct TPacketCGMessenger
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<uint32_t, 0> vid;
    OptionalField<std::string, 1> name;
    OptionalField<PacketCGMessengerSetBlock, 2> setBlock;
};


BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGMessenger,
    subheader,
    opts,
    vid,
    name,
    setBlock
)

struct TPacketCGMessengerAddByVID
{
    uint32_t vid;
} ;


BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGMessengerAddByVID,
    vid
)


struct TPacketCGMessengerRemove
{
    std::string login;
    //uint32_t account;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGMessengerRemove,
    login
)


struct TPacketCGSafeboxCheckout
{
    uint16_t bSafePos;
    TItemPos ItemPos;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGSafeboxCheckout,
    bSafePos,
    ItemPos
)

struct TPacketCGSafeboxCheckin
{
    uint16_t bSafePos;
    TItemPos ItemPos;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGSafeboxCheckin,
    bSafePos,
    ItemPos
)

///////////////////////////////////////////////////////////////////////////////////
// Party

struct TPacketCGPartyParameter
{
    uint8_t bDistributeMode;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGPartyParameter,
    bDistributeMode
)

using TPacketGCPartyParameter = TPacketCGPartyParameter;

struct TPacketGCPartyAdd
{
    uint32_t pid;
    std::string name;
    uint32_t mapIndex;
    uint32_t channel;
    uint32_t race;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCPartyAdd,
    pid,
    name,
    mapIndex,
    channel,
    race
)

struct TPacketGCPartyPositionInfo
{
    uint32_t    pid;
    uint32_t    x;
    uint32_t    y;
    float    rot;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCPartyPositionInfo,
    pid,
    x,
    y,
    rot
)

struct TPacketCGPartyInvite
{
    uint32_t vid;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGPartyInvite,
    vid
)

using TPacketGCPartyInvite = TPacketCGPartyInvite;


struct TPacketCGPartyInviteAnswer
{
    uint32_t leader_vid;
    uint8_t accept;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGPartyInviteAnswer,
    leader_vid,
    accept
)

struct TPacketGCPartyUpdate
{
    uint32_t pid;
    uint8_t role;
    uint8_t percent_hp;
    int16_t affects[7]; // TODO enum ?
    uint8_t is_leader;
    uint8_t race;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCPartyUpdate,
    pid,
    role,
    percent_hp,
    affects,
    is_leader,
    race
)


struct TPacketGCPartyRemove
{
    uint32_t pid;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCPartyRemove,
    pid
)

struct TPacketGCPartyLink
{
    uint32_t pid;
    uint32_t vid;
    uint32_t mapIndex;
    uint32_t channel;
    uint32_t race;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCPartyLink,
    pid,
    vid,
    mapIndex,
    channel,
    race
)


struct TPacketGCPartyUnlink
{
    uint32_t pid;
    uint32_t vid;
};


BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCPartyUnlink,
    pid,
    vid
)


struct TPacketCGPartyRemove
{
    uint32_t pid;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGPartyRemove,
    pid
)

struct TPacketCGPartySetState
{
    uint32_t pid;
    uint8_t byRole;
    uint8_t flag;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGPartySetState,
    pid,
    byRole,
    flag
)

enum
{
    PARTY_SKILL_HEAL = 1,
    PARTY_SKILL_WARP = 2
};

struct TPacketCGPartyUseSkill
{
    uint8_t bySkillIndex;
    uint32_t vid;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGPartyUseSkill,
    bySkillIndex,
    vid
)

struct TPacketCGSafeboxSize
{
    uint16_t bSize;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGSafeboxSize,
    bSize
)

using TPacketCGSafeboxWrongPassword = BlankPacket;

enum
{
    SAFEBOX_MONEY_STATE_SAVE,
    SAFEBOX_MONEY_STATE_WITHDRAW,
};

struct TPacketCGSafeboxMoney
{
    uint8_t bState;
    int32_t lMoney;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGSafeboxMoney,
    bState,
    lMoney
)

struct TPacketGCSafeboxMoneyChange
{
    int32_t lMoney;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCSafeboxMoneyChange,
    lMoney
)

// Guild

enum
{
    GUILD_SUBHEADER_GC_LOGIN,
    GUILD_SUBHEADER_GC_LOGOUT,
    GUILD_SUBHEADER_GC_LIST,
    GUILD_SUBHEADER_GC_GRADE,
    GUILD_SUBHEADER_GC_ADD,
    GUILD_SUBHEADER_GC_REMOVE,
    GUILD_SUBHEADER_GC_GRADE_NAME,
    GUILD_SUBHEADER_GC_GRADE_AUTH,
    GUILD_SUBHEADER_GC_INFO,
    GUILD_SUBHEADER_GC_COMMENTS,
    GUILD_SUBHEADER_GC_CHANGE_EXP,
    GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE,
    GUILD_SUBHEADER_GC_SKILL_INFO,
    GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL,
    GUILD_SUBHEADER_GC_GUILD_INVITE,
    GUILD_SUBHEADER_GC_WAR,
    GUILD_SUBHEADER_GC_GUILD_NAME,
    GUILD_SUBHEADER_GC_GUILD_WAR_LIST,
    GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST,
    GUILD_SUBHEADER_GC_WAR_SCORE,
    GUILD_SUBHEADER_GC_MONEY_CHANGE,
};

enum GUILD_SUBHEADER_CG
{
    GUILD_SUBHEADER_CG_ADD_MEMBER,
    GUILD_SUBHEADER_CG_REMOVE_MEMBER,
    GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME,
    GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY,
    GUILD_SUBHEADER_CG_OFFER,
    GUILD_SUBHEADER_CG_POST_COMMENT,
    GUILD_SUBHEADER_CG_DELETE_COMMENT,
    GUILD_SUBHEADER_CG_REFRESH_COMMENT,
    GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE,
    GUILD_SUBHEADER_CG_USE_SKILL,
    GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL,
    GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER,
    GUILD_SUBHEADER_CG_CHARGE_GSP,
    GUILD_SUBHEADER_CG_DEPOSIT_MONEY,
    GUILD_SUBHEADER_CG_WITHDRAW_MONEY,
};

struct GcGuildSkillInfo
{
    uint8_t skillPoints;
    uint8_t skillLevels[GUILD_SKILL_COUNT];

    uint16_t power;
    uint16_t maxPower;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcGuildSkillInfo,
    skillPoints,
    skillLevels,
    power,
    maxPower
)

struct TPacketGCGuildSubMember
{
    uint32_t pid;
    uint8_t byGrade;
    uint8_t byIsGeneral;
    uint8_t byJob;
    uint32_t byLevel;
    uint32_t dwOffer;
    std::string name;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCGuildSubMember,
    pid,
    byGrade,
    byIsGeneral,
    byJob,
    byLevel,
    dwOffer,
    name
)

struct TPacketGCGuildInfo
{
    uint16_t member_count;
    uint16_t max_member_count;
    uint32_t guild_id;
    uint32_t master_pid;
    uint32_t exp;
    uint8_t level;
    std::string name;
    Gold gold;
    uint8_t hasLand;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCGuildInfo,
    member_count,
    max_member_count,
    guild_id,
    master_pid,
    exp,
    level,
    name,
    gold,
    hasLand
)

struct TGuildMemberPacketData
{
    uint32_t pid;
    uint8_t grade;
    uint8_t is_general;
    uint8_t job;
    uint8_t level;
    uint32_t offer;
    uint8_t name_flag;
    std::string name;
};

BOOST_FUSION_ADAPT_STRUCT(
    TGuildMemberPacketData,
    pid,
    grade,
    is_general,
    job,
    level,
    offer,
    name_flag,
    name
)

struct TGuildGrade
{
    std::string grade_name; // 8+1 길드장, 길드원 등의 이름
    uint64_t auth_flag;
};

BOOST_FUSION_ADAPT_STRUCT(
    TGuildGrade,
    grade_name,
    auth_flag
)

struct TOneGradeNamePacket
{
    uint8_t grade;
    std::string grade_name;
};

BOOST_FUSION_ADAPT_STRUCT(
    TOneGradeNamePacket,
    grade,
    grade_name
)

struct TOneGradeAuthPacket
{
    uint8_t grade;
    uint64_t auth;
};

BOOST_FUSION_ADAPT_STRUCT(
    TOneGradeAuthPacket,
    grade,
    auth
)

struct GuildIdAndName
{
    uint32_t id;
    std::string name;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildIdAndName,
    id,
    name
)

struct GuildComment
{
    uint32_t id;
    std::string player;
    std::string comment;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildComment,
    id,
    player,
    comment
)

struct GuildStatusUpdate
{
    Level lv;
    uint32_t exp;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildStatusUpdate,
    lv,
    exp
)

struct GuildGeneralUpdate
{
    uint32_t pid;
    uint8_t isGeneral;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildGeneralUpdate,
    pid,
    isGeneral
)

struct GuildUpdateMemberGrade
{
    uint32_t pid;
    uint8_t grade;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildUpdateMemberGrade,
    pid,
    grade
)

struct GuildInvite
{
    uint32_t gid;
    std::string name;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildInvite,
    gid,
    name
)

using GuildGradeMap = std::unordered_map<uint8_t, TGuildGrade>;

struct GuildWarData
{
    uint32_t gid1;
    uint32_t gid2;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildWarData,
    gid1,
    gid2
)

struct TPacketGCGuildName
{
    uint8_t subheader;
    uint32_t guildID;
    std::string guildName;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCGuildName,
    subheader,
    guildID,
    guildName
)

struct TPacketGCGuildWar
{
    uint32_t dwGuildSelf;
    uint32_t dwGuildOpp;
    uint8_t bType;
#ifdef ENABLE_NEW_GUILD_WAR
    int bScore;
    BYTE bMinLevel;
    BYTE bMaxPlayer;
    BYTE bBinekkullanimi;
    BYTE savasci, ninja, sura, shaman;
#endif
    uint8_t bWarState;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCGuildWar,
    dwGuildSelf,
    dwGuildOpp,
    bType,
    bScore,
	bMinLevel,
	bMaxPlayer,
	bBinekkullanimi,
	savasci, ninja, sura, shaman,
    bWarState
)

struct TPacketGuildWarPoint
{
    uint32_t dwGainGuildID;
    uint32_t dwOpponentGuildID;
    int32_t lPoint;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGuildWarPoint,
    dwGainGuildID,
    dwOpponentGuildID,
    lPoint
)

struct TPacketGCGuild
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<uint64_t, 0> pid;
    OptionalField<std::vector<TPacketGCGuildSubMember>, 1> submembers;
    OptionalField<TPacketGCGuildInfo, 2> info;
    OptionalField<GuildGradeMap, 3> grades;
    OptionalField<GuildIdAndName, 4> name;
    OptionalField<TOneGradeNamePacket, 5> oneGrade;
    OptionalField<TOneGradeAuthPacket, 6> oneGradeAuth;
    OptionalField<std::vector<GuildComment>, 7> comments;
    OptionalField<GcGuildSkillInfo, 8> skillInfo;
    OptionalField<GuildStatusUpdate, 9> status;
    OptionalField<GuildGeneralUpdate, 10> updateGeneral;
    OptionalField<GuildUpdateMemberGrade, 11> updateMemberGrade;
    OptionalField<GuildInvite, 12> guildInvite;
    OptionalField<std::vector<GuildWarData>, 13> guildWars;
    OptionalField<TPacketGCGuildWar, 14> guildWar;
    OptionalField<TPacketGuildWarPoint, 15> guildWarPoint;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCGuild,
    subheader,
    opts,
    pid,
    submembers,
    info,
    grades,
    name,
    oneGrade,
    oneGradeAuth,
    comments,
    skillInfo,
    status,
    updateGeneral,
    updateMemberGrade,
    guildInvite,
    guildWars,
    guildWar,
    guildWarPoint
)


struct GuildRenameGrade
{
    uint8_t index;
    std::string name;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildRenameGrade,
    index,
    name
)

struct GuildChangeAuthority
{
    uint8_t index;
    uint64_t auth;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildChangeAuthority,
    index,
    auth
)

struct GuildUseSkill
{
    uint32_t skillIndex;
    uint32_t targetVid;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildUseSkill,
    skillIndex,
    targetVid
)

struct GuildChangeMemberGeneral
{
    uint32_t pid;
    uint8_t flag;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildChangeMemberGeneral,
    pid,
    flag
)

struct GuildChangeMemberGrade
{
    uint32_t pid;
    uint8_t grade;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildChangeMemberGrade,
    pid,
    grade
)

struct GuildInviteAnswer
{
    uint32_t gid;
    uint8_t flag;
};

BOOST_FUSION_ADAPT_STRUCT(
    GuildInviteAnswer,
    gid,
    flag
)

struct TPacketCGGuild
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<uint32_t, 0> vid;
    OptionalField<uint32_t, 1> pid;
    OptionalField<Gold, 2> money;
    OptionalField<GuildRenameGrade, 3> changeGradeName;
    OptionalField<GuildChangeAuthority, 4> changeGradeAuth;
    OptionalField<PointValue, 5> expOffer;
    OptionalField<std::string, 6> comment;
    OptionalField<uint32_t, 7> commentIndex;
    OptionalField<GuildUseSkill, 8> useSkill;
    OptionalField<GuildChangeMemberGeneral, 9> changeGeneral;
    OptionalField<GuildInviteAnswer, 10> inviteAnswer;
    OptionalField<GuildChangeMemberGrade, 11> changeMemberGrade;    
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGGuild,
    subheader,
    opts,
    vid,
    pid,
    money,
    changeGradeName,
    changeGradeAuth,
    expOffer,
    comment,
    commentIndex,
    useSkill,
    changeGeneral,
    inviteAnswer,
    changeMemberGrade
)

struct TPacketCGAnswerMakeGuild
{
    std::string guild_name;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGAnswerMakeGuild,
    guild_name
)

struct TPacketCGGuildUseSkill
{
    uint32_t dwVnum;
    uint32_t dwPID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGGuildUseSkill,
    dwVnum,
    dwPID
)

// Guild Mark
struct TPacketCGMarkLogin
{
    uint32_t handle;
    uint32_t random_key;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGMarkLogin,
    handle,
    random_key
)

struct TPacketCGMarkUpload
{
    uint32_t gid;
    uint8_t image[16 * 12 * 4];
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGMarkUpload,
    gid,
    image
)

using TPacketCGMarkIDXList = BlankPacket;

struct TPacketCGMarkCRCList
{
    uint8_t imgIdx;
    uint32_t crclist[80];
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGMarkCRCList,
    imgIdx,
    crclist
)


struct MarkIndex
{
    uint32_t guildId;
    uint32_t markIdx;
};

BOOST_FUSION_ADAPT_STRUCT(
    MarkIndex,
    guildId,
    markIdx
)


struct TPacketGCMarkIDXList
{
    std::vector<MarkIndex> indices;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCMarkIDXList,
    indices
)

struct TPacketGCMarkBlock
{
    uint8_t imgIdx;
    // µÚ¿¡ 64 x 48 x ÇÈ¼¿Å©±â(4¹ÙÀÌÆ®) = 12288¸¸Å­ µ¥ÀÌÅÍ ºÙÀ½
    std::unordered_map<uint8_t, std::vector<uint8_t>> blocks;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCMarkBlock,
    imgIdx,
    blocks
)

struct TPacketCGGuildSymbolUpload
{
    uint32_t guild_id;
    std::vector<uint8_t> data;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGGuildSymbolUpload,
    guild_id,
    data
)

struct TPacketCGSymbolCRC
{
    uint32_t guild_id;
    uint32_t crc;
    uint32_t size;
    uint8_t lastRequest;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGSymbolCRC,
    guild_id,
    crc,
    size,
    lastRequest
)

struct TPacketGCGuildSymbolData
{
    uint16_t size;
    uint32_t guild_id;
    std::vector<uint8_t> data;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCGuildSymbolData,
    size,
    guild_id,
    data
)

// Fishing

struct TPacketCGFishing
{
    uint8_t dir;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGFishing,
    dir
)

struct TPacketCGFishingGame
{
    uint8_t hitCount;
    float time;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGFishingGame,
    hitCount,
    time
)

struct FishingReactData
{
  uint32_t vid;
  uint32_t maxHit;
  uint32_t x;
  uint32_t y;
};

BOOST_FUSION_ADAPT_STRUCT(
    FishingReactData,
    vid,
    maxHit,
    x,
    y
)

struct TPacketGCFishing
{
    uint8_t subheader;
    uint32_t info;
    uint8_t dir;
    OptionalFields opts;
    OptionalField<FishingReactData, 0> fi;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCFishing,
    subheader,
    info,
    dir,
    opts,
    fi
)

enum
{
    FISHING_SUBHEADER_GC_START,
    FISHING_SUBHEADER_GC_STOP,
    FISHING_SUBHEADER_GC_REACT,
    FISHING_SUBHEADER_GC_SUCCESS,
    FISHING_SUBHEADER_GC_FAIL,
    FISHING_SUBHEADER_GC_FISH,
};

struct TPacketCGGiveItem
{
    uint32_t dwTargetVID;
    TItemPos ItemPos;
    CountType byItemCount;
};


BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGGiveItem,
    dwTargetVID,
    ItemPos,
    byItemCount
)


struct TPacketCGHack
{
    std::string szBuf;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGHack,
    szBuf
)
// SubHeader - Dungeon
enum
{
    DUNGEON_SUBHEADER_GC_TIME_ATTACK_START = 0,
    DUNGEON_SUBHEADER_GC_DESTINATION_POSITION = 1,
};

using TPacketGCDungeon = BlankPacket;

struct TPacketGCDungeonDestPosition
{
    int32_t x;
    int32_t y;
} ;

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCDungeonDestPosition,
    x,
    y
)

struct TPacketGCShopSign
{
    uint32_t dwVID;
    uint8_t type;
    std::string szSign;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShopSign,
    dwVID,
    type,
    szSign
)

struct TPacketCGMyShop
{
    std::string szSign;
    uint8_t bCount;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGMyShop,
    szSign,
    bCount
)


struct TPacketGCTime
{
    int32_t time;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCTime,
    time
)

enum
{
    WALKMODE_RUN,
    WALKMODE_WALK,
};

struct TPacketGCWalkMode
{
    uint32_t vid;
    uint8_t mode;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCWalkMode,
    vid,
    mode
)

struct TPacketGCChangeSkillGroup
{
    uint8_t skill_group;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCChangeSkillGroup,
    skill_group
)

struct TPacketCGRefine
{
    uint16_t pos;
    uint8_t type;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGRefine,
    pos,
    type
)

struct TPacketCGRequestRefineInfo
{
    uint8_t pos;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGRequestRefineInfo,
    pos
)
struct TPacketGCRefineInformation
{
    uint8_t type;
    uint16_t pos;
    uint32_t src_vnum;
    uint32_t result_vnum;
    Gold cost; // ¼Ò¿ä ºñ¿ë
    int32_t prob; // È®·ü
    std::vector<RefineMaterial> materials;
    std::vector<RefineEnhanceMaterial> enhance_mat;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCRefineInformation,
    type,
    pos,
    src_vnum,
    result_vnum,
    cost, 
    prob, 
    materials,
    enhance_mat
)

struct TNPCPosition
{
    uint8_t bType;
    uint32_t vnum;
    std::string name;
    int32_t x;
    int32_t y;
};

using TShopPosition = TNPCPosition;

BOOST_FUSION_ADAPT_STRUCT(
    TNPCPosition,
    bType,
    vnum,
    name,
    x,
    y
)

struct TPacketGCNPCPosition
{
    std::vector<TNPCPosition> positions;
};

using TPacketGCShopPosition = TPacketGCNPCPosition;

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCNPCPosition,
    positions
)

struct TPacketGCSpecialEffect
{
  uint8_t type;
  uint8_t effectType;
  uint32_t vid;
  uint32_t x;
  uint32_t y;
  float scale;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCSpecialEffect,
    type,
    effectType,
    vid,
    x,
    y,
    scale
)


struct TPacketGCChannel
{
    uint8_t channel;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCChannel,
    channel
)

struct TPacketViewEquip
{
    uint32_t vid;
    ClientItemData items[WEAR_MAX_NUM];
};


BOOST_FUSION_ADAPT_STRUCT(
    TPacketViewEquip,
    vid,
    items
)

struct TLandPacketElement
{
    uint32_t dwID;
    int32_t x, y;
    int32_t width, height;
    uint32_t dwGuildID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TLandPacketElement,
    dwID,
    x,
    y,
    width,
    height,
    dwGuildID
)


struct TPacketGCLandList
{
    std::vector<TLandPacketElement> lands;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCLandList,
    lands
)

struct TPacketGCGuildLandUpdate
{
    uint32_t landID;
    uint32_t guildID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCGuildLandUpdate,
    landID,
    guildID
)

 struct TPacketGCTargetCreate
{
    int32_t lID;
    std::string szName;
    uint32_t dwVID;
    uint8_t bType;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCTargetCreate,
    lID,
    szName,
    dwVID,
    bType
)

struct TPacketGCTargetUpdate
{
    int32_t lID;
    int32_t lX, lY;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCTargetUpdate,
    lID,
    lX,
    lY
)

struct TPacketGCTargetDelete
{
    int32_t lID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCTargetDelete,
    lID
)

struct GcAffectAddPacket
{
    uint32_t vid;
    AffectData data;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcAffectAddPacket,
    vid,
    data
)

struct GcAffectDelPacket
{
    uint32_t vid;
    uint32_t type;
    uint8_t pointType;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcAffectDelPacket,
    vid,
    type,
    pointType
)

struct TPacketGCLoverInfo
{
    std::string name;
    uint8_t love_point;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCLoverInfo,
    name,
    love_point
)

struct TPacketGCLovePointUpdate
{
    uint8_t love_point;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCLovePointUpdate,
    love_point
)
// MINING
struct TPacketGCDigMotion
{
    uint32_t vid;
    uint32_t target_vid;
    uint8_t count;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCDigMotion,
    vid,
    target_vid,
    count
)
// END_OF_MINING

// SCRIPT_SELECT_ITEM
struct TPacketCGScriptSelectItem
{
    uint32_t selection;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGScriptSelectItem,
    selection
)
// END_OF_SCRIPT_SELECT_ITEM

struct TPacketGCDamageInfo
{
    uint32_t dwVID;
    uint8_t flag;
    int32_t damage;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCDamageInfo,
    dwVID,
    flag,
    damage
)

struct TPacketGCThreeWayStatus
{
    int16_t score[3];
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCThreeWayStatus,
    score
)

struct TPacketGCThreeWayLives
{
    int32_t lives;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCThreeWayLives,
    lives
)

struct TPacketGCSpecificEffect
{
    uint32_t vid;
    std::string effect_file;
} ;

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCSpecificEffect,
    vid,
    effect_file
)
// ¿ëÈ¥¼®
enum EDragonSoulRefineWindowRefineType
{
    DragonSoulRefineWindow_UPGRADE,
    DragonSoulRefineWindow_IMPROVEMENT,
    DragonSoulRefineWindow_REFINE,
};

enum EPacketCGDragonSoulSubHeaderType
{
    DS_SUB_HEADER_OPEN,
    DS_SUB_HEADER_CLOSE,
    DS_SUB_HEADER_DO_REFINE_GRADE,
    DS_SUB_HEADER_DO_REFINE_STEP,
    DS_SUB_HEADER_DO_REFINE_STRENGTH,
    DS_SUB_HEADER_REFINE_FAIL,
    DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE,
    DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
    DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY,
    DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL,
    DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL,
    DS_SUB_HEADER_REFINE_SUCCEED,
};

struct TPacketCGDragonSoulRefine
{
    uint8_t bSubType;
    TItemPos ItemGrid[DRAGON_SOUL_REFINE_GRID_SIZE];
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGDragonSoulRefine,
    bSubType,
    ItemGrid
)

struct TPacketGCDragonSoulRefine
{
    uint8_t bSubType;
    TItemPos Pos;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCDragonSoulRefine,
    bSubType,
    Pos
)

#ifdef __OFFLINE_SHOP__
struct TPacketCGOfflineShop
{
    uint8_t    bySubHeader;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGOfflineShop,
    bySubHeader
)

enum EOfflineShopCGSubHeader
{
    CREATE_MY_SHOP,
    CLOSE_MY_SHOP,
    CLOSE_OTHER_SHOP,
#ifdef __OFFLINE_SHOP_USE_BUNDLE__
    CANCEL_CREATE_MY_SHOP,
#endif
};

struct TOfflineShopCGItemInfo
{
    TItemPos    kInventoryPosition;
    uint32_t        dwDisplayPosition;
    Gold    llPrice;
};

BOOST_FUSION_ADAPT_STRUCT(
    TOfflineShopCGItemInfo,
    kInventoryPosition,
    dwDisplayPosition,
    llPrice
)

struct TPacketCGOfflineShopCreate
{
    std::string    szShopName;
#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
    float    fX, fY;
#endif

    uint8_t    bIsShowEditor;
    uint32_t    dwItemCount;

#ifdef __OFFLINE_SHOP_OPENING_TIME__
    int32_t        iOpeningTime;
#endif

    std::vector<TOfflineShopCGItemInfo> items;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGOfflineShopCreate,
    szShopName,
    bIsShowEditor,
    dwItemCount,
    items
)


struct TPacketCGOfflineShopName
{
    std::string    szShopName;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGOfflineShopName,
    szShopName
)

#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
}; struct SPacketCGOfflineShopPosition
{
    uint8_t    byHeader;
    float    fX, fY;
} TPacketCGOfflineShopPosition;
#endif

struct TPacketCGOfflineShopAddItem
{
    TItemPos    kInventoryPosition;
    uint32_t        dwDisplayPosition;
    Gold    llPrice;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGOfflineShopAddItem,
    kInventoryPosition,
    dwDisplayPosition,
    llPrice
)

struct TPacketCGOfflineShopMoveItem
{
    uint32_t    dwOldDisplayPosition;
    uint32_t    dwNewDisplayPosition;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGOfflineShopMoveItem,
    dwOldDisplayPosition,
    dwNewDisplayPosition
)

struct TPacketCGOfflineShopRemoveItem
{
    uint32_t        dwDisplayPosition;
    TItemPos    kInventoryPosition;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGOfflineShopRemoveItem,
    dwDisplayPosition,
    dwDisplayPosition
)

struct TPacketCGOfflineShopBuyItem
{
    uint32_t        dwDisplayPosition;
    TItemPos    kInventoryPosition;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGOfflineShopBuyItem,
    dwDisplayPosition,
    kInventoryPosition
)

struct TPacketCGOfflineShopWithdrawGold
{
    Gold    llGold;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGOfflineShopWithdrawGold,
    llGold
)

struct TPacketCGOfflineShopClickShop
{
    uint32_t    dwShopOwnerPID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGOfflineShopClickShop,
    dwShopOwnerPID
)

struct TPacketCGOfflineShopReopen
{
#ifdef __OFFLINE_SHOP_OPENING_TIME__
    int32_t        iOpeningTime;
#endif
    uint8_t dummy;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGOfflineShopReopen,
    dummy
)


struct TPacketGCOfflineShop
{
    uint8_t    bySubHeader;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCOfflineShop,
    bySubHeader
)

enum EOfflineShopGCSubHeader
{
    CLEAR_SHOP,
    CREATE_SHOP,
    OPEN_SHOP,
    OPEN_MY_SHOP,
    CLOSE_SHOP,
#ifndef __OFFLINE_SHOP_OPENING_TIME__
    OPEN_MY_SHOP_FOR_OTHERS,
    CLOSE_MY_SHOP_FOR_OTHERS,
#endif
};

struct TPacketGCOfflineShopAdditionalInfo
{
    uint32_t    dwVID;
    std::string    szShopName;
    uint32_t    dwShopOwnerPID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCOfflineShopAdditionalInfo,
    dwVID,
    szShopName,
    dwShopOwnerPID
)

struct TPacketGCOfflineShopAddItem
{
    ClientItemData data;
    uint32_t        dwDisplayPosition;
    Gold    llPrice;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCOfflineShopAddItem,
    data,
    dwDisplayPosition,
    llPrice
)

struct TPacketGCOfflineShopMoveItem
{
    uint32_t    dwOldDisplayPosition;
    uint32_t    dwNewDisplayPosition;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCOfflineShopMoveItem,
    dwOldDisplayPosition,
    dwNewDisplayPosition
)

struct TPacketGCOfflineShopRemoveItem
{
    uint32_t    dwDisplayPosition;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCOfflineShopRemoveItem,
    dwDisplayPosition
)

struct TPacketGCOfflineShopGold
{
    Gold    llGold;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCOfflineShopGold,
    llGold
)

struct TPacketGCOfflineShopName
{
    std::string    szName;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCOfflineShopName,
    szName
)

#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
struct TPacketGCOfflineShopLeftOpeningTime
{
#ifdef __OFFLINE_SHOP_OPENING_TIME__
    int32_t        iLeftOpeningTime;
#elif defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
    int32_t        iFarmedOpeningTime;
    int32_t        iSpecialOpeningTime;
#endif
};
#endif

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCOfflineShopLeftOpeningTime,
    iFarmedOpeningTime,
    iSpecialOpeningTime
)

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
struct TPacketGCOfflineShopEditorPositionInfo
{
    int32_t    lMapIndex;
    int32_t    lX, lY;
    uint8_t    byChannel;
};
BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCOfflineShopEditorPositionInfo,
    lMapIndex,
    lX,
    lY,
    byChannel
)

enum EPacketGGOfflineShopFarmOpeningTimeInformation
{
    OFFLINE_SHOP_FARM_OPENING_TIME_INFORMATION_LOGIN,
    OFFLINE_SHOP_FARM_OPENING_TIME_INFORMATION_LOGOUT
};

struct TPacketGGOfflineShopFarmOpeningTime
{
    uint8_t    byType;
    uint32_t    dwOwnerPID;
    int32_t        iOpeningTime;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGGOfflineShopFarmOpeningTime,
    byType,
    dwOwnerPID,
    iOpeningTime
)


#endif

struct TPacketGGOfflineShopBuyItemInfo
{
    uint32_t        dwOwnerPID;
    uint32_t        dwVNum;
    uint8_t        byCount;
    Gold    llPrice;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGGOfflineShopBuyItemInfo,
    dwOwnerPID,
    dwVNum,
    byCount,
    llPrice
)

#endif

using TPacketCGPong = BlankPacket;

struct TPacketChannelStatus
{
    int16_t nPort;
    uint8_t bStatus;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketChannelStatus,
    nPort,
    bStatus
)

struct TPacketCGHGuardSend
{
    std::string macAddr;
    std::string guid;
    std::string hddSer;
    std::string cpuId;
    std::string hddModel;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGHGuardSend,
    macAddr,
    guid,
    hddSer,
    cpuId,
    hddModel
)

enum
{
    HGUARD_SUBHEADER_REQUIRE_CODE,
    HGUARD_SUBHEADER_INCORRECT_CODE,
};

struct TPacketGCHGuardCodeResult
{
    uint8_t subHeader;
    uint8_t status;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCHGuardCodeResult,
    subHeader,
    status
)

struct TPacketCGHGuardCodeSend
{
    std::string code;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGHGuardCodeSend,
    code
)

struct TPacketMyShopRemoveItem
{
    int32_t slot;
    TItemPos target;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketMyShopRemoveItem,
    slot,
    target
)

struct TPacketMyShopAddItem
{
    TItemPos from;
    int32_t targetPos;
    Gold price;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketMyShopAddItem,
    from,
    targetPos,
    price
)

struct TPacketGCShopStashSync
{
    Gold value;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShopStashSync,
    value
)

struct TPacketGCShopOffTimeSync
{
    uint8_t bHeader;
    uint32_t value;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShopOffTimeSync,
    bHeader,
    value
)

struct TPacketGCShopPremiumTimeSync
{
    int32_t value;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShopPremiumTimeSync,
    value
)
struct TPacketGCShopSyncPos
{
    int32_t channel;
    int32_t mapIndex;
    int32_t x;
    int32_t y;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShopSyncPos,
    channel,
    mapIndex,
    x,
    y
)
struct TPacketGCShopRename
{
    std::string sign;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCShopRename,
    sign
)

struct TPacketGCSeal
{
    TItemPos pos;
    uint8_t action;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCSeal,
    pos,
    action
)

struct TPacketGCInputState
{
    uint8_t isEnable;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCInputState,
    isEnable
)

enum ACCE_SUBHEADER_GC
{
    ACCE_SUBHEADER_GC_SET_ITEM,
    ACCE_SUBHEADER_GC_CLEAR_SLOT,
    ACCE_SUBHEADER_GC_CLEAR_ALL,
    ACCE_SUBHEADER_GC_CLEAR_ONE,
};

enum ACCE_SUBHEADER_CG
{
    ACCE_SUBHEADER_CG_REFINE_CHECKIN,
    ACCE_SUBHEADER_CG_REFINE_CHECKOUT,
    ACCE_SUBHEADER_CG_REFINE_ACCEPT,
    ACCE_SUBHEADER_CG_REFINE_CANCEL,
};


struct TPacketGCAcce
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<GcItemSetPacket, 0> itemSet;
    OptionalField<uint16_t, 1> cell;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCAcce,
    opts,
    itemSet,
    cell
)

struct TPacketCGAcceCheckout
{
    uint8_t bAccePos;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGAcceCheckout,
    bAccePos
)

struct TPacketCGAcceCheckin
{
    uint8_t bAccePos;
    TItemPos ItemPos;
    uint8_t windowType;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGAcceCheckin,
    bAccePos,
    ItemPos,
    windowType
)

struct TPacketCGAcceRefineAccept
{
    uint8_t windowType;
};


BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGAcceRefineAccept,
    windowType
)

struct TPacketCGAcce
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<TPacketCGAcceCheckout, 0> checkout;
    OptionalField<TPacketCGAcceCheckin, 1> checkin;
    OptionalField<TPacketCGAcceRefineAccept, 2> accept;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGAcce,
    subheader,
    opts,
    checkout,
    checkin,
    accept
)

struct GcChangeLookPacket
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<GcItemSetPacket, 0> itemSet;
    OptionalField<TItemPos, 1> pos;

};

BOOST_FUSION_ADAPT_STRUCT(
    GcChangeLookPacket,
    subheader,
    opts,
    itemSet,
    pos
)

enum SubHeaderChangelook {
    CHANGELOOK_SUBHEADER_CG_REFINE_CHECKIN,
    CHANGELOOK_SUBHEADER_CG_REFINE_CHECKOUT,
    CHANGELOOK_SUBHEADER_CG_REFINE_ACCEPT,
    CHANGELOOK_SUBHEADER_CG_REFINE_CANCEL,

    CHANGELOOK_SUBHEADER_GC_SET_ITEM = 0,
    CHANGELOOK_SUBHEADER_GC_CLEAR_SLOT,
    CHANGELOOK_SUBHEADER_GC_CLEAR_ALL,
};

enum SubHeaderLevelPet {
    LEVELPET_SUBHEADER_CG_OPEN,
    LEVELPET_SUBHEADER_CG_CLOSE,
    LEVELPET_SUBHEADER_CG_PLUS_ATTR,
};

enum
{
    SKILL_BOOK_COMBINATION_SUBHEADER_CG_START,
    SKILL_BOOK_COMBINATION_SUBHEADER_CG_ITEM_ADD,
    SKILL_BOOK_COMBINATION_SUBHEADER_CG_ITEM_DEL,
    SKILL_BOOK_COMBINATION_SUBHEADER_CG_ACCEPT,
    SKILL_BOOK_COMBINATION_SUBHEADER_CG_CANCEL,
};

struct TPacketCGSkillBookCombiation
{
    uint8_t sub_header;
    uint8_t DPos;
    TItemPos Pos;
};


BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGSkillBookCombiation,
    sub_header,
    DPos,
    Pos
)

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM

struct ShopSearchBuy
{
    uint32_t itemId;
};

BOOST_FUSION_ADAPT_STRUCT(
    ShopSearchBuy,
    itemId
)

struct ShopSearchItemSearch {
    int32_t vnum;
    int32_t extra;
};

BOOST_FUSION_ADAPT_STRUCT(
    ShopSearchItemSearch,
    vnum,
    extra
)

struct ShopSearchFilter {
    int8_t minAvgDmg;
    int8_t maxAvgDmg;
    int8_t minSkillDmg;
    int8_t maxSkillDmg;
    uint8_t minLevel;
    uint8_t maxLevel;
    uint8_t sex;
};

BOOST_FUSION_ADAPT_STRUCT(
    ShopSearchFilter,
    minAvgDmg,
    maxAvgDmg,
    minSkillDmg,
    maxSkillDmg,
    minLevel,
    maxLevel,
    sex
)

struct ShopSearchOpenCategory {
    int32_t itemType;
    int32_t itemSubType;
};

BOOST_FUSION_ADAPT_STRUCT(
    ShopSearchOpenCategory,
    itemType,
    itemSubType
)

struct ShopSearchSetPage {
    uint16_t page;
};

BOOST_FUSION_ADAPT_STRUCT(
    ShopSearchSetPage,
    page
)

struct TPacketCGShopSearch
{
    OptionalFields opts;
    OptionalField<ShopSearchItemSearch, 0> itemSearch;
    OptionalField<ShopSearchOpenCategory, 1> openCategory;
    OptionalField<int32_t, 2> openFilter;
    OptionalField<ShopSearchSetPage, 3> setPage;
    OptionalField<ShopSearchFilter, 4> setFilters;
    OptionalField<uint32_t, 5> buyItem;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGShopSearch,
    opts,
    itemSearch,
    openCategory,
    openFilter,
    setPage,
    setFilters,
    buyItem
)

struct ShopSearchPageInfo
{
    uint16_t page;
    uint16_t pageCount;
    uint8_t perPage;
};

BOOST_FUSION_ADAPT_STRUCT(
    ShopSearchPageInfo,
    page,
    perPage,
    pageCount
)

struct GcShopSearchPacket
{
    OptionalFields opts;
    OptionalField<std::vector<ShopSearchItemData>, 0> setItems;
    OptionalField<ShopSearchPageInfo, 1> pageInfo;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcShopSearchPacket,
    opts,
    setItems,
    pageInfo
)

#endif


#ifdef ENABLE_DESTROY_ITEM_SYSTTEM
struct TPacketCGItemDestroy
{
    TItemPos Cell;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGItemDestroy,
    Cell
)
#endif

struct TPacketGCGuildSubGrade
{
    std::string grade_name; // 8+1 길드장, 길드원 등의 이름
    uint64_t auth_flag;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCGuildSubGrade,
    grade_name,
    auth_flag
)

struct TPacketGCDestroyCharacterSuccess
{
    uint8_t account_index;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCDestroyCharacterSuccess,
    account_index
)

struct TChannelStatus
{
    uint16_t nPort;
    uint8_t bStatus;
};

BOOST_FUSION_ADAPT_STRUCT(
    TChannelStatus,
    nPort,
    bStatus
)

struct TGcUpdateLastPlay
{
    int32_t lastPlay;
};

BOOST_FUSION_ADAPT_STRUCT(
    TGcUpdateLastPlay,
    lastPlay
)

struct TPacketCGTargetLoad
{
    uint32_t dwVID;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGTargetLoad,
    dwVID
)

struct TPacketGCTargetInfo
{
    uint32_t dwVNum;
    std::vector<TDropInfo> drops;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCTargetInfo,
    dwVNum,
    drops
)

struct TPacketGCMaintenance
{
    int32_t seconds;
    int32_t duration;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCMaintenance,
    seconds,
    duration
)

#ifdef ENABLE_MOVE_COSTUME_ATTR
struct TPacketCGItemCombiation
{
    TItemPos mediumPos;
    TItemPos basePos;
    TItemPos materialPos;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGItemCombiation,
    mediumPos,
    basePos,
    materialPos
)

#endif


enum MYSHOP_SUBHEADER_CG
{
    MYSHOP_DECO_SUBHEADER_CG_STATE = 1,
    MYSHOP_DECO_SUBHEADER_CG_SET = 2,
};

using TPacketCGMyShopDecoState = uint8_t;



struct TPacketCGMyShopDecoSet
{
    uint32_t titleType;
    uint32_t polyVnum;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGMyShopDecoSet,
    titleType,
    polyVnum
)

struct TPacketCGMyShopDeco
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<TPacketCGMyShopDecoState, 0> state;
    OptionalField<TPacketCGMyShopDecoSet, 1> set;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGMyShopDeco,
    subheader,
    opts,
    state,
    set
)

struct TPacketCGCheckCountryFlagWhisper
{
    std::string szName;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGCheckCountryFlagWhisper,
    szName
)


#if defined(WJ_COMBAT_ZONE)
}; struct SPacketCGCombatZoneAction
{
    uint8_t    header;
    int32_t        action;
    int32_t        value;
} TPacketCGCombatZoneRequestAction;

}; struct SPacketGCCombatZone
{
    int32_t        rank;
    char    name[12 + 1];
    int32_t        empire;
    int32_t        points;
} TPacketGCCombatZoneRanking;

}; struct SPacketGCCombatZoneData
{
    uint8_t header;
    TPacketGCCombatZoneRanking rankingData[11];
} TPacketGCCombatZoneRankingData;

}; struct SPacketGCSendCombatZone
{
    uint8_t    header;
    uint32_t    sub_header;
    uint32_t    m_pInfoData[4];
    uint32_t    m_pDataDays[7][8];
    uint8_t    isRunning;
} TPacketGCSendCombatZone;
#endif

struct TPacketCGChangeSkillColorPacket
{
    uint32_t vnum;
    uint32_t color;
};


BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGChangeSkillColorPacket,
    vnum,
    color
)

struct TPacketCGChooseSkillGroup
{
    uint8_t skillGroup;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGChooseSkillGroup,
    skillGroup
)

struct TPacketGCNPCUseSkill
{
    uint32_t    dwVnum;
    uint32_t    dwVid;
    uint32_t    dwLevel;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCNPCUseSkill,
    dwVnum,
    dwVid,
    dwLevel
)

struct GcSkillMotionPacket
{
    uint32_t vid;
    uint32_t x;
    uint32_t y;
    uint32_t time;
    float rotation;
    uint32_t skillVnum;
    uint32_t skillLevel;
    uint32_t skillGrade;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcSkillMotionPacket,
    vid,
    x,
    y,
    time,
    rotation,
    skillVnum,
    skillLevel,
    skillGrade
)


struct CgPinResponsePacket
{
    uint32_t code;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgPinResponsePacket,
    code
)

using GcPinRequestPacket = BlankPacket ;
using GcSetPinRequestPacket = BlankPacket ;

struct CgBlockModePacket
{
    uint32_t blockMode;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgBlockModePacket,
    blockMode
)

struct CgChangeNamePacket
{
    uint8_t index;
    std::string name;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgChangeNamePacket,
    index,
    name
)

using GcChangeNamePacket = CgChangeNamePacket;

struct CgChangeEmpirePacket
{
    uint8_t slot;
    uint8_t empire;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgChangeEmpirePacket,
    slot,
    empire
)

using GcChangeEmpirePacket = CgChangeEmpirePacket;

struct CgRemoveMetinPacket
{
    TItemPos targetItem;
    uint8_t slot;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgRemoveMetinPacket,
    targetItem,
    slot
)

struct GcSwitchbotSlotDataPacket
{
    SwitchBotSlotData data;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcSwitchbotSlotDataPacket,
    data
)

struct CgSwitchbotAttributeUpdatePacket
{
    uint8_t slotIndex;
    uint8_t altIndex;
    uint8_t attrIndex;
    TItemApply attribute;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgSwitchbotAttributeUpdatePacket,
    slotIndex,
    altIndex,
    attrIndex,
    attribute
)

struct CgSwitchbotStatusPacket
{
    uint8_t slotIndex;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgSwitchbotStatusPacket,
    slotIndex
)

struct CgSwitchbotPacket
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<CgSwitchbotAttributeUpdatePacket, 0> updateAttr;
    OptionalField<CgSwitchbotStatusPacket, 1> status;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgSwitchbotPacket,
    subheader,
    opts,
    updateAttr,
    status
)

struct BlankCgPacketWithSub
{
    uint8_t subheader;
};

BOOST_FUSION_ADAPT_STRUCT(
    BlankCgPacketWithSub,
    subheader
)

struct BlankDynamicPacketWithSub
{
    uint8_t subheader;
};

BOOST_FUSION_ADAPT_STRUCT(
    BlankDynamicPacketWithSub,
    subheader
)

#ifdef INGAME_WIKI

struct CgRecvWikiPacket
{
    uint32_t ret_id;
    uint32_t vnum;
    uint8_t is_mob;
};


BOOST_FUSION_ADAPT_STRUCT(
    CgRecvWikiPacket,
    ret_id,
    vnum,
    is_mob
)


struct GcWikiItemInfo {
    uint32_t dwOrigin;
    uint8_t isSet;
    uint8_t bIsCommon;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcWikiItemInfo,
    dwOrigin,
    isSet,
    bIsCommon
)

struct GcWikiPacket
{
    uint32_t ret_id;
    uint32_t vnum;
    GcWikiItemInfo itemInfo;
    std::vector<TWikiRefineInfo> refineInfo;
    std::vector<uint32_t> chestInfos;
    std::vector<TWikiItemOriginInfo> originInfos;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcWikiPacket,
    ret_id,
    vnum,
    itemInfo,
    refineInfo,
    chestInfos,
    originInfos
)


struct GcWikiMobPacket
{
    uint32_t ret_id;
    uint32_t vnum;
    std::vector<TDropInfo> vnums;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcWikiMobPacket,
    ret_id,
    vnum,
    vnums
)

struct GcHuntingMissionPacket
{
    OptionalFields opts;
    OptionalField<std::vector<TPlayerHuntingMission>, 0> load;
    OptionalField<TPlayerHuntingMission, 1> update;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcHuntingMissionPacket,
    opts,
    load,
    update
)

struct GcItemPickupInfoPacket {
    ItemVnum vnum;
    CountType count;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcItemPickupInfoPacket,
    vnum,
    count
)
#endif

#ifdef ENABLE_BATTLE_PASS

struct GcBattlePassPacket
{
    std::vector<TBattlePassMissionInfo> missions;
    std::vector<TBattlePassRewardItem> rewards;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcBattlePassPacket,
    missions,
    rewards
)

struct GcBattlePassUpdatePacket
{
    uint8_t    bMissionType;
    uint64_t    dwNewProgress;
};

BOOST_FUSION_ADAPT_STRUCT(
    GcBattlePassUpdatePacket,
    bMissionType,
    dwNewProgress
)

struct TPacketCGBattlePassAction
{
    uint8_t    bAction;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGBattlePassAction,
    bAction
)

struct TPacketGCBattlePassRanking
{
    uint8_t    bIsGlobal;
    std::vector<TBattlePassRanking> elems;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCBattlePassRanking,
    bIsGlobal,
    elems
)

#endif

struct TPacketCGChatFilter
{
    std::vector<uint8_t> bFilterList;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketCGChatFilter,
    bFilterList
)

struct TPacketGCUpdateChatFilter
{
    std::vector<uint8_t> bFilterList;
};

BOOST_FUSION_ADAPT_STRUCT(
    TPacketGCUpdateChatFilter,
    bFilterList
)
struct CgChangeLookCheckinPacket
{
    TItemPos invenPos;
    uint8_t targetPos;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgChangeLookCheckinPacket,
    invenPos,
    targetPos
)

struct CgChangeLookPacket
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<CgChangeLookCheckinPacket, 0> checkin;
    OptionalField<uint8_t, 1> checkout;

};

BOOST_FUSION_ADAPT_STRUCT(
    CgChangeLookPacket,
    subheader,
    opts,
    checkin,
    checkout
)
struct CgLevelPetPacket
{
    uint8_t subheader;
    OptionalFields opts;
    OptionalField<uint32_t, 0> index;
    OptionalField<TItemPos, 1> itemPos;
};

BOOST_FUSION_ADAPT_STRUCT(
    CgLevelPetPacket,
    subheader,
    opts,
    index,
    itemPos
)

struct CgHyperlinkItemPacket
{
    ClientItemData d;
};

BOOST_FUSION_ADAPT_STRUCT(
  CgHyperlinkItemPacket,
  d
)

typedef struct SPacketGCDungeonInfo
{
  std::vector<TPlayerDungeonInfo> infos;
} TPacketGCDungeonInfo;
 
BOOST_FUSION_ADAPT_STRUCT(
  TPacketGCDungeonInfo,
  infos
)

typedef struct SPacketCGDungeonWarp
{
  uint8_t bDungeonID;
} TPacketCGDungeonWarp;

BOOST_FUSION_ADAPT_STRUCT(
  TPacketCGDungeonWarp,
  bDungeonID
)

typedef struct SPacketGCDungeonRanking
{
  TDungeonRankSet rankingSet;
} TPacketGCDungeonRanking;

BOOST_FUSION_ADAPT_STRUCT(
  TPacketGCDungeonRanking,
  rankingSet
)

typedef struct SPacketGCDungeonUpdate
{
  uint8_t bDungeonID;
  uint32_t dwCooldownEnd;
} TPacketGCDungeonUpdate;

BOOST_FUSION_ADAPT_STRUCT(
  TPacketGCDungeonUpdate,
  bDungeonID,
  dwCooldownEnd
)

typedef struct SPacketCGDungeonRanking
{
  uint8_t bDungeonID;
  uint8_t bRankingType;
} TPacketCGDungeonRanking;

BOOST_FUSION_ADAPT_STRUCT(
  TPacketCGDungeonRanking,
  bDungeonID,
  bRankingType
)

#pragma pack()
#endif
