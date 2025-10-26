#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../GameLib/ItemData.h"
#include "../EterLib/GrpText.h"
#include <optional>
#include <game/Constants.hpp>
#include <game/MobConstants.hpp>
#include <game/ItemConstants.hpp>
#include <game/MotionConstants.hpp>

extern std::string g_strGuildSymbolPathName;

enum ESlotType
{
    SLOT_TYPE_NONE,
    SLOT_TYPE_INVENTORY,
    SLOT_TYPE_SKILL,
    SLOT_TYPE_EMOTION,
    SLOT_TYPE_SHOP,
    SLOT_TYPE_EXCHANGE_OWNER,
    SLOT_TYPE_EXCHANGE_TARGET,
    SLOT_TYPE_QUICK_SLOT,
    SLOT_TYPE_SAFEBOX,
    SLOT_TYPE_PRIVATE_SHOP,
    SLOT_TYPE_ACCE,
    SLOT_TYPE_MALL,
    SLOT_TYPE_DRAGON_SOUL_INVENTORY,
    SLOT_TYPE_MYSHOP,
#ifdef ENABLE_GUILD_STORAGE
	SLOT_TYPE_GUILDSTORAGE,
#endif
#ifdef __OFFLINE_SHOP__
    SLOT_TYPE_OFFLINE_SHOP,
#endif
    SLOT_TYPE_CHANGE_LOOK,
    SLOT_TYPE_MAX,
};

enum EOnTopWindows
{
    ON_TOP_WND_NONE,
    ON_TOP_WND_SHOP,
    ON_TOP_WND_EXCHANGE,
    ON_TOP_WND_SAFEBOX,
    ON_TOP_WND_PRIVATE_SHOP,
    ON_TOP_WND_ITEM_COMB,
    ON_TOP_WND_PET_FEED,
    ON_TOP_WND_SWITCHBOT,
    ON_TOP_WND_REFINE,
    ON_TOP_WND_SHOP_BUILD,
};

enum ECombSlot
{
    COMB_WND_SLOT_MEDIUM,
    COMB_WND_SLOT_BASE,
    COMB_WND_SLOT_MATERIAL,
    COMB_WND_SLOT_RESULT,
    COMB_WND_SLOT_MAX,
};

const uint32_t c_QuickBar_Line_Count = 3;
const uint32_t c_QuickBar_Slot_Count = 12;

const float c_Idle_WaitTime = 5.0f;

const int c_Monster_Race_Start_Number = 6;
const int c_Monster_Model_Start_Number = 20001;

const float c_fAttack_Delay_Time = 0.2f;
const float c_fHit_Delay_Time = 0.1f;
const float c_fCrash_Wave_Time = 0.2f;
const float c_fCrash_Wave_Distance = 3.0f;

const float c_fHeight_Step_Distance = 50.0f;

enum
{
    DISTANCE_TYPE_FOUR_WAY,
    DISTANCE_TYPE_EIGHT_WAY,
    DISTANCE_TYPE_ONE_WAY,
    DISTANCE_TYPE_MAX_NUM,
};

const float c_fMagic_Script_Version = 1.0f;
const float c_fSkill_Script_Version = 1.0f;
const float c_fMagicSoundInformation_Version = 1.0f;
const float c_fBattleCommand_Script_Version = 1.0f;
const float c_fEmotionCommand_Script_Version = 1.0f;
const float c_fActive_Script_Version = 1.0f;
const float c_fPassive_Script_Version = 1.0f;

// Used by PushMove
const float c_fWalkDistance = 175.0f;
const float c_fRunDistance = 310.0f;

#define FILE_MAX_LEN 128

inline float GetSqrtDistance(int ix1, int iy1, int ix2, int iy2) // By sqrt
{
    float dx, dy;

    dx = float(ix1 - ix2);
    dy = float(iy1 - iy2);

    return sqrtf(dx * dx + dy * dy);
}


void SetGuildSymbolPath(const char *c_szPathName);
const char *GetGuildSymbolFileName(uint32_t dwGuildID);
uint8_t SlotTypeToInvenType(uint8_t bSlotType);
uint8_t WindowTypeToSlotType(uint8_t bWindowType);

uint8_t GetLanguageIDByName(const std::string &lang);
std::optional<std::string> GetLanguageNameByID(uint8_t code);
std::optional<std::string> GetLangDisplayName(uint8_t code);

uint8_t GetApplyPoint(uint8_t applyType);
uint8_t GetPointApply(uint8_t pointType);
