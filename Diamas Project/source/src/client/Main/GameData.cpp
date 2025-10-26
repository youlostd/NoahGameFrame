#include "GameData.hpp"

#include "PythonApplication.h"
#include "PythonPlayer.h"
#include "PythonSkill.h"
#include "../EterLib/CharacterEffectRegistry.hpp"
#include "../gamelib/RaceManager.h"
#include "game/AffectConstants.hpp"
#include "PythonBindings.h"
#include "../GameLib/WikiManager.hpp"
#include "pak/Util.hpp"

class CPythonItem;

enum SkillMotionIndexes
{
    HORSE_SKILL_WILDATTACK = MOTION_SKILL + 121,
    HORSE_SKILL_CHARGE = MOTION_SKILL + 122,
    HORSE_SKILL_SPLASH = MOTION_SKILL + 123,

    GUILD_SKILL_DRAGONBLOOD = MOTION_SKILL + 101,
    GUILD_SKILL_DRAGONBLESS = MOTION_SKILL + 102,
    GUILD_SKILL_BLESSARMOR = MOTION_SKILL + 103,
    GUILD_SKILL_SPPEDUP = MOTION_SKILL + 104,
    GUILD_SKILL_DRAGONWRATH = MOTION_SKILL + 105,
    GUILD_SKILL_MAGICUP = MOTION_SKILL + 106,

    COMBO_TYPE_1 = 0,
    COMBO_TYPE_2 = 1,
    COMBO_TYPE_3 = 2,

    COMBO_INDEX_1 = 0,
    COMBO_INDEX_2 = 1,
    COMBO_INDEX_3 = 2,
    COMBO_INDEX_4 = 3,
    COMBO_INDEX_5 = 4,
    COMBO_INDEX_6 = 5,
};

const static std::unordered_map<uint32_t, std::string> aniMap = {
    {MOTION_CLAP, "clap.msa"},
    {MOTION_CHEERS_1, "cheers_1.msa"},
    {MOTION_CHEERS_2, "cheers_2.msa"},
    {MOTION_DANCE_1, "dance_1.msa"},
    {MOTION_DANCE_2, "dance_2.msa"},
    {MOTION_DANCE_3, "dance_3.msa"},
    {MOTION_DANCE_4, "dance_4.msa"},
    {MOTION_DANCE_5, "dance_5.msa"},
    {MOTION_DANCE_6, "dance_6.msa"},
    {MOTION_DANCE_7, "dance_7.msa"},
    {MOTION_EMOTION_PUSH_UP, "pushup.msa"},
    {MOTION_EMOTION_EXERCISE, "exercise.msa"},
    {MOTION_EMOTION_DOZE, "doze.msa"},
    {MOTION_EMOTION_SELFIE, "selfie.msa"},
    {MOTION_CONGRATULATION, "congratulation.msa"},
    {MOTION_FORGIVE, "forgive.msa"},
    {MOTION_ANGRY, "angry.msa"},
    {MOTION_ATTRACTIVE, "attractive.msa"},
    {MOTION_SAD, "sad.msa"},
    {MOTION_SHY, "shy.msa"},
    {MOTION_CHEERUP, "cheerup.msa"},
    {MOTION_BANTER, "banter.msa"},
    {MOTION_JOY, "joy.msa"},
    {MOTION_THROW_MONEY, "ridack_animoney.msa"},
    {MOTION_FRENCH_KISS_WITH_WARRIOR, "french_kiss_with_warrior.msa"},
    {MOTION_FRENCH_KISS_WITH_ASSASSIN, "french_kiss_with_assassin.msa"},
    {MOTION_FRENCH_KISS_WITH_SURA, "french_kiss_with_sura.msa"},
    {MOTION_FRENCH_KISS_WITH_SHAMAN, "french_kiss_with_shaman.msa"},
    {MOTION_KISS_WITH_WARRIOR, "kiss_with_warrior.msa"},
    {MOTION_KISS_WITH_ASSASSIN, "kiss_with_assassin.msa"},
    {MOTION_KISS_WITH_SURA, "kiss_with_sura.msa"},
    {MOTION_KISS_WITH_SHAMAN, "kiss_with_shaman.msa"},
    {MOTION_SLAP_HIT_WITH_WARRIOR, "slap_hit.msa"},
    {MOTION_SLAP_HIT_WITH_ASSASSIN, "slap_hit.msa"},
    {MOTION_SLAP_HIT_WITH_SURA, "slap_hit.msa"},
    {MOTION_SLAP_HIT_WITH_SHAMAN, "slap_hit.msa"},
    {MOTION_SLAP_HURT_WITH_WARRIOR, "slap_hurt.msa"},
    {MOTION_SLAP_HURT_WITH_ASSASSIN, "slap_hurt.msa"},
    {MOTION_SLAP_HURT_WITH_SURA, "slap_hurt.msa"},
    {MOTION_SLAP_HURT_WITH_SHAMAN, "slap_hurt.msa"},

};

auto RegisterSharedEmotionAnis(uint32_t mode, const std::string path) -> void
{
    auto &chrmgr = CRaceManager::instance();
    chrmgr.SetPathName(path);

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();

    if (!pRaceData)
    {
        SPDLOG_ERROR("RaceData has not been selected");
        return;
    }

    pRaceData->RegisterMotionMode(mode);

    for (const auto &elem : aniMap)
    {
        chrmgr.RegisterCacheMotionData(mode, elem.first, elem.second);
    }
}

auto RegisterEmotionAnis(std::string path) -> void
{
    auto &chrmgr = CRaceManager::instance();

    std::string actionPath = path + "action/";
    std::string weddingPath = path + "wedding/";

    RegisterSharedEmotionAnis(MOTION_MODE_GENERAL, actionPath);
    RegisterSharedEmotionAnis(MOTION_MODE_WEDDING_DRESS, actionPath);

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();

    if (!pRaceData)
    {
        SPDLOG_ERROR("RaceData has not been selected");
        return;
    }

    chrmgr.SetPathName(weddingPath);
    pRaceData->RegisterMotionMode(MOTION_MODE_WEDDING_DRESS);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_WEDDING_DRESS, MOTION_WAIT, "wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_WEDDING_DRESS, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_WEDDING_DRESS, MOTION_RUN, "walk.msa");
}

auto SetIntroMotions(uint8_t mode, const std::string folder) -> void
{
    auto &chrmgr = CRaceManager::instance();
    chrmgr.SetPathName(folder);

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
    {
        SPDLOG_ERROR("RaceData has not been selected");
        return;
    }

    pRaceData->RegisterMotionMode(mode);
    chrmgr.RegisterCacheMotionData(mode, MOTION_INTRO_WAIT, "wait.msa");
    chrmgr.RegisterCacheMotionData(mode, MOTION_INTRO_SELECTED, "selected.msa");
    chrmgr.RegisterCacheMotionData(mode, MOTION_INTRO_NOT_SELECTED, "not_selected.msa");
}

auto SetGeneralMotions(uint8_t mode, const std::string &folder) -> void
{
    auto &chrmgr = CRaceManager::instance();
    chrmgr.SetPathName(folder);

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
    {
        SPDLOG_ERROR("RaceData has not been selected");
        return;
    }

    pRaceData->RegisterMotionMode(mode);
    chrmgr.RegisterCacheMotionData(mode, MOTION_WAIT, "wait.msa");
    chrmgr.RegisterCacheMotionData(mode, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(mode, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(mode, MOTION_DAMAGE, "damage.msa", 50);
    chrmgr.RegisterCacheMotionData(mode, MOTION_DAMAGE, "damage_1.msa", 50);
    chrmgr.RegisterCacheMotionData(mode, MOTION_DAMAGE_BACK, "damage_2.msa", 50);
    chrmgr.RegisterCacheMotionData(mode, MOTION_DAMAGE_BACK, "damage_3.msa", 50);
    chrmgr.RegisterCacheMotionData(mode, MOTION_DAMAGE_FLYING, "damage_flying.msa");
    chrmgr.RegisterCacheMotionData(mode, MOTION_STAND_UP, "falling_stand.msa");
    chrmgr.RegisterCacheMotionData(mode, MOTION_DAMAGE_FLYING_BACK, "back_damage_flying.msa");
    chrmgr.RegisterCacheMotionData(mode, MOTION_STAND_UP_BACK, "back_falling_stand.msa");
    chrmgr.RegisterCacheMotionData(mode, MOTION_DEAD, "dead.msa");
    chrmgr.RegisterCacheMotionData(mode, MOTION_DIG, "dig.msa");
}

auto LoadWarrior(uint8_t race, const std::string &path) -> void
{
    auto &chrmgr = CRaceManager::instance();

    chrmgr.SelectRace(race);

    SetGeneralMotions(MOTION_MODE_GENERAL, path + "general/");

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
    {
        SPDLOG_ERROR("RaceData has not been selected");
        return;
    }

    if (!pRaceData->SetMotionRandomWeight(MOTION_MODE_GENERAL, MOTION_WAIT, 0, 70))
        SPDLOG_ERROR("Failed to SetMotionRandomWeight");

    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_WAIT, "wait_1.msa", 30);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_COMBO_ATTACK_1, "attack.msa", 30);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_COMBO_ATTACK_1, "attack_1.msa", 50);

    chrmgr.SetPathName(path + "skill/");

    for (int i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; ++i)
    {
        std::string endString = "";
        if (i != 0)
            endString = fmt::format("_{0}", i + 1);

        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 1,
                                       "samyeon" + endString + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 2,
                                       "palbang" + endString + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 3,
                                       "jeongwi" + endString + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 4,
                                       "geomgyeong" + endString + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 5,
                                       "tanhwan" + endString + ".msa");

        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 6,
                                       "gihyeol" + endString + ".msa");

        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 16,
                                       "gigongcham" + endString + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 17,
                                       "gyeoksan" + endString + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 18,
                                       "daejin" + endString + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 19,
                                       "cheongeun" + endString + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 20,
                                       "geompung" + endString + ".msa");

        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 21,
                                       "noegeom" + endString + ".msa");
    }

    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_MAGICUP, "guild_jumunsul.msa");

    pRaceData->ReserveComboAttack(MOTION_MODE_GENERAL, 0, 1);
    pRaceData->RegisterComboAttack(MOTION_MODE_GENERAL, 0, 0, MOTION_COMBO_ATTACK_1);

    // emotion.RegisterEmotionAnis(path)

    chrmgr.SetPathName(path + "onehand_sword/");
    pRaceData->RegisterMotionMode(MOTION_MODE_ONEHAND_SWORD);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_WAIT, "wait.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_WAIT, "wait_1.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE, "damage.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE, "damage_1.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE_BACK, "damage_2.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE_BACK, "damage_3.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_4, "combo_04.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_5, "combo_05.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_6, "combo_06.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_7, "combo_07.msa");

    pRaceData->ReserveComboAttack(MOTION_MODE_ONEHAND_SWORD, 0, 4);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 0, 0, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 0, 1, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 0, 2, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 0, 3, MOTION_COMBO_ATTACK_4);
    // Combo Type 2
    pRaceData->ReserveComboAttack(MOTION_MODE_ONEHAND_SWORD, 1, 5);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 1, 0, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 1, 1, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 1, 2, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 1, 3, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 1, 4, MOTION_COMBO_ATTACK_7);
    // Combo Type 3
    pRaceData->ReserveComboAttack(MOTION_MODE_ONEHAND_SWORD, 2, 6);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 2, 0, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 2, 1, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 2, 2, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 2, 3, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 2, 4, MOTION_COMBO_ATTACK_6);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, 2, 5, MOTION_COMBO_ATTACK_4);

    // TWOHAND_SWORD BATTLE
    chrmgr.SetPathName(path + "twohand_sword/");
    pRaceData->RegisterMotionMode(MOTION_MODE_TWOHAND_SWORD);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_WAIT, "wait.msa", 70);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_WAIT, "wait_1.msa", 30);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_DAMAGE, "damage.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_DAMAGE, "damage_1.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_DAMAGE_BACK, "damage_2.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_DAMAGE_BACK, "damage_3.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_COMBO_ATTACK_4, "combo_04.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_COMBO_ATTACK_5, "combo_05.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_COMBO_ATTACK_6, "combo_06.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_TWOHAND_SWORD, MOTION_COMBO_ATTACK_7, "combo_07.msa");

    // Combo Type 1
    pRaceData->ReserveComboAttack(MOTION_MODE_TWOHAND_SWORD, 0, 4);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 0, 0, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 0, 1, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 0, 2, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 0, 3, MOTION_COMBO_ATTACK_4);
    // Combo Type 2
    pRaceData->ReserveComboAttack(MOTION_MODE_TWOHAND_SWORD, 1, 5);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 1, 0, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 1, 1, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 1, 2, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 1, 3, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 1, 4, MOTION_COMBO_ATTACK_7);
    // Combo Type 3
    pRaceData->ReserveComboAttack(MOTION_MODE_TWOHAND_SWORD, 2, 6);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 2, 0, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 2, 1, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 2, 2, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 2, 3, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 2, 4, MOTION_COMBO_ATTACK_6);
    pRaceData->RegisterComboAttack(MOTION_MODE_TWOHAND_SWORD, 2, 5, MOTION_COMBO_ATTACK_4);

    // FISHING
    chrmgr.SetPathName(path + "fishing/");
    pRaceData->RegisterMotionMode(MOTION_MODE_FISHING);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_WAIT, "wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_THROW, "throw.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_WAIT, "fishing_wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_STOP, "fishing_cancel.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_REACT, "fishing_react.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_CATCH, "fishing_catch.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_FAIL, "fishing_fail.msa");

    // HORSE
    chrmgr.SetPathName(path + "horse/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait.msa", 90);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait_1.msa", 9);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait_2.msa", 1);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DAMAGE, "damage.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DAMAGE_BACK, "damage.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DEAD, "dead.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, HORSE_SKILL_CHARGE, "skill_charge.msa");

    // HORSE_ONEHAND_SWORD
    chrmgr.SetPathName(path + "horse_onehand_sword/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE_ONEHAND_SWORD);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    pRaceData->ReserveComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, 0, 3);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, 0, 0, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, 0, 1, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, 0, 2, MOTION_COMBO_ATTACK_3);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, HORSE_SKILL_WILDATTACK, "skill_wildattack.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, HORSE_SKILL_SPLASH, "skill_splash.msa");

    chrmgr.SetPathName(path + "horse_twohand_sword/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE_TWOHAND_SWORD);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_TWOHAND_SWORD, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_TWOHAND_SWORD, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_TWOHAND_SWORD, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    pRaceData->ReserveComboAttack(MOTION_MODE_HORSE_TWOHAND_SWORD, 0, 3);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_TWOHAND_SWORD, 0, 0, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_TWOHAND_SWORD, 0, 1, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_TWOHAND_SWORD, 0, 2, MOTION_COMBO_ATTACK_3);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_TWOHAND_SWORD, HORSE_SKILL_WILDATTACK, "skill_wildattack.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_TWOHAND_SWORD, HORSE_SKILL_SPLASH, "skill_splash.msa");

    // Bone
    pRaceData->RegisterAttachingBoneName(PART_WEAPON, "equip_right_hand");

    RegisterEmotionAnis(path);
}

auto LoadAssassin(uint8_t race, const std::string &path) -> void
{
    auto &chrmgr = CRaceManager::instance();

    // Assassin
    ////////////////////////////////////////////////////////////////////////////////////////#
    chrmgr.SelectRace(race);

    // GENERAL MOTION MODE
    SetGeneralMotions(MOTION_MODE_GENERAL, path + "general/");

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
    {
        SPDLOG_ERROR("RaceData has not been selected");
        return;
    }

    if (!pRaceData->SetMotionRandomWeight(MOTION_MODE_GENERAL, MOTION_WAIT, 0, 70))
        SPDLOG_ERROR("Failed to SetMotionRandomWeight");

    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_WAIT, "wait_1.msa", 30);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_COMBO_ATTACK_1, "attack.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_COMBO_ATTACK_1, "attack_1.msa", 50);

    pRaceData->ReserveComboAttack(MOTION_MODE_GENERAL, COMBO_TYPE_1, 1);
    pRaceData->RegisterComboAttack(MOTION_MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);

    // SKILL
    chrmgr.SetPathName(path + "skill/");
    for (int i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; ++i)
    {
        std::string END_STRING = "";
        if (i != 0)
            END_STRING = fmt::format("_{0}", i + 1);
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 1,
                                       "amseup" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 2,
                                       "gungsin" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 3,
                                       "charyun" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 4,
                                       "eunhyeong" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 5,
                                       "sangong" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 6,
                                       "seomjeon" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 16,
                                       "yeonsa" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 17,
                                       "gwangyeok" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 18,
                                       "hwajo" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 19,
                                       "gyeonggong" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 20,
                                       "dokgigung" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 21,
                                       "seomgwang" + END_STRING + ".msa");
    }

    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_MAGICUP, "guild_jumunsul.msa");

    // ONEHAND_SWORD BATTLE
    chrmgr.SetPathName(path + "onehand_sword/");
    pRaceData->RegisterMotionMode(MOTION_MODE_ONEHAND_SWORD);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_WAIT, "wait.msa", 70);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_WAIT, "wait_1.msa", 30);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE, "damage.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE, "damage_1.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE_BACK, "damage_2.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE_BACK, "damage_3.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_4, "combo_04.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_5, "combo_05.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_6, "combo_06.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_7, "combo_07.msa");

    // Combo Type 1
    pRaceData->ReserveComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_1, 4);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, MOTION_COMBO_ATTACK_4);
    // Combo Type 2
    pRaceData->ReserveComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, 5);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, MOTION_COMBO_ATTACK_7);
    // Combo Type 3
    pRaceData->ReserveComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, 6);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, MOTION_COMBO_ATTACK_6);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, MOTION_COMBO_ATTACK_4);

    // DUALHAND_SWORD BATTLE
    chrmgr.SetPathName(path + "dualhand_sword/");
    pRaceData->RegisterMotionMode(MOTION_MODE_DUALHAND_SWORD);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_WAIT, "wait.msa", 70);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_WAIT, "wait_1.msa", 30);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_DAMAGE, "damage.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_DAMAGE, "damage_1.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_DAMAGE_BACK, "damage_2.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_DAMAGE_BACK, "damage_3.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_COMBO_ATTACK_4, "combo_04.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_COMBO_ATTACK_5, "combo_05.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_COMBO_ATTACK_6, "combo_06.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_COMBO_ATTACK_7, "combo_07.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_DUALHAND_SWORD, MOTION_COMBO_ATTACK_8, "combo_08.msa");

    // Combo Type 1
    pRaceData->ReserveComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_1, 4);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, MOTION_COMBO_ATTACK_4);
    // Combo Type 2
    pRaceData->ReserveComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_2, 5);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, MOTION_COMBO_ATTACK_7);
    // Combo Type 3
    pRaceData->ReserveComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_3, 6);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, MOTION_COMBO_ATTACK_6);
    pRaceData->RegisterComboAttack(MOTION_MODE_DUALHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, MOTION_COMBO_ATTACK_8);

    // BOW BATTLE
    chrmgr.SetPathName(path + "bow/");
    pRaceData->RegisterMotionMode(MOTION_MODE_BOW);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BOW, MOTION_WAIT, "wait.msa", 70);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BOW, MOTION_WAIT, "wait_1.msa", 30);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BOW, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BOW, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BOW, MOTION_DAMAGE, "damage.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BOW, MOTION_DAMAGE, "damage_1.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BOW, MOTION_DAMAGE_BACK, "damage_2.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BOW, MOTION_DAMAGE_BACK, "damage_3.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BOW, MOTION_COMBO_ATTACK_1, "attack.msa");
    pRaceData->ReserveComboAttack(MOTION_MODE_BOW, COMBO_TYPE_1, 1);
    pRaceData->RegisterComboAttack(MOTION_MODE_BOW, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);

    // FISHING
    chrmgr.SetPathName(path + "fishing/");
    pRaceData->RegisterMotionMode(MOTION_MODE_FISHING);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_WAIT, "wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_THROW, "throw.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_WAIT, "fishing_wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_STOP, "fishing_cancel.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_REACT, "fishing_react.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_CATCH, "fishing_catch.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_FAIL, "fishing_fail.msa");

    // HORSE
    chrmgr.SetPathName(path + "horse/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait.msa", 90);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait_1.msa", 9);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait_2.msa", 1);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DAMAGE, "damage.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DAMAGE_BACK, "damage.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DEAD, "dead.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, HORSE_SKILL_CHARGE, "skill_charge.msa");

    // HORSE_ONEHAND_SWORD
    chrmgr.SetPathName(path + "horse_onehand_sword/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE_ONEHAND_SWORD);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, HORSE_SKILL_WILDATTACK, "skill_wildattack.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, HORSE_SKILL_SPLASH, "skill_splash.msa");
    pRaceData->ReserveComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, 3);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);

    // HORSE_DUALHAND_SWORD
    chrmgr.SetPathName(path + "horse_dualhand_sword/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE_DUALHAND_SWORD);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_DUALHAND_SWORD, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_DUALHAND_SWORD, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_DUALHAND_SWORD, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_DUALHAND_SWORD, HORSE_SKILL_WILDATTACK, "skill_wildattack.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_DUALHAND_SWORD, HORSE_SKILL_SPLASH, "skill_splash.msa");
    pRaceData->ReserveComboAttack(MOTION_MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, 3);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1,
                                   MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2,
                                   MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_DUALHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3,
                                   MOTION_COMBO_ATTACK_3);

    // HORSE_BOW
    chrmgr.SetPathName(path + "horse_bow/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE_BOW);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BOW, MOTION_WAIT, "wait.msa", 90);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BOW, MOTION_WAIT, "wait_1.msa", 9);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BOW, MOTION_WAIT, "wait_2.msa", 1);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BOW, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BOW, MOTION_DAMAGE, "damage.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BOW, MOTION_DEAD, "dead.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BOW, MOTION_COMBO_ATTACK_1, "attack.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BOW, HORSE_SKILL_WILDATTACK, "skill_wildattack.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BOW, HORSE_SKILL_SPLASH, "skill_splash.msa");
    pRaceData->ReserveComboAttack(MOTION_MODE_HORSE_BOW, COMBO_TYPE_1, 1);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_BOW, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);

    pRaceData->RegisterAttachingBoneName(PART_WEAPON, "equip_right");
    pRaceData->RegisterAttachingBoneName(PART_WEAPON_LEFT, "equip_left");

    RegisterEmotionAnis(path);
}

auto LoadSura(uint8_t race, const std::string &path) -> void
{
    auto &chrmgr = CRaceManager::instance();

    // Sura
    ////////////////////////////////////////////////////////////////////////////////////////#
    chrmgr.SelectRace(race);

    // GENERAL MOTION MODE
    SetGeneralMotions(MOTION_MODE_GENERAL, path + "general/");

    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
    {
        SPDLOG_ERROR("RaceData has not been selected");
        return;
    }

    if (!pRaceData->SetMotionRandomWeight(MOTION_MODE_GENERAL, MOTION_WAIT, 0, 70))
        SPDLOG_ERROR("Failed to SetMotionRandomWeight");

    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_COMBO_ATTACK_1, "attack.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_COMBO_ATTACK_1, "attack_1.msa", 50);

    pRaceData->ReserveComboAttack(MOTION_MODE_GENERAL, COMBO_TYPE_1, 1);
    pRaceData->RegisterComboAttack(MOTION_MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);

    // SKILL
    chrmgr.SetPathName(path + "skill/");
    // chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL+4, "geongon.msa")
    for (int i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; ++i)
    {
        std::string END_STRING = "";
        if (i != 0)
            END_STRING = fmt::format("_{0}", i + 1);
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 1,
                                       "swaeryeong" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 2,
                                       "yonggwon" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 3,
                                       "gwigeom" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 4,
                                       "gongpo" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 5,
                                       "jumagap" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 6,
                                       "pabeop" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 16,
                                       "maryeong" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 17,
                                       "hwayeom" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 18,
                                       "muyeong" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 19,
                                       "heuksin" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 20,
                                       "tusok" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 21,
                                       "mahwan" + END_STRING + ".msa");
    }

    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_MAGICUP, "guild_jumunsul.msa");

    // ONEHAND_SWORD BATTLE
    chrmgr.SetPathName(path + "onehand_sword/");
    pRaceData->RegisterMotionMode(MOTION_MODE_ONEHAND_SWORD);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_WAIT, "wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE, "damage.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE, "damage.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE, "damage_1.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE_BACK, "damage_2.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_DAMAGE_BACK, "damage_3.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_4, "combo_04.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_5, "combo_05.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_6, "combo_06.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_7, "combo_07.msa");

    // Combo Type 1
    pRaceData->ReserveComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_1, 4);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_4, MOTION_COMBO_ATTACK_4);
    // Combo Type 2
    pRaceData->ReserveComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, 5);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_4, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_2, COMBO_INDEX_5, MOTION_COMBO_ATTACK_7);
    // Combo Type 3
    pRaceData->ReserveComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, 6);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_4, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_5, MOTION_COMBO_ATTACK_6);
    pRaceData->RegisterComboAttack(MOTION_MODE_ONEHAND_SWORD, COMBO_TYPE_3, COMBO_INDEX_6, MOTION_COMBO_ATTACK_4);

    // FISHING
    chrmgr.SetPathName(path + "fishing/");
    pRaceData->RegisterMotionMode(MOTION_MODE_FISHING);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_WAIT, "wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_THROW, "throw.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_WAIT, "fishing_wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_STOP, "fishing_cancel.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_REACT, "fishing_react.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_CATCH, "fishing_catch.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_FAIL, "fishing_fail.msa");

    // HORSE
    chrmgr.SetPathName(path + "horse/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait.msa", 90);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait_1.msa", 9);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait_2.msa", 1);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DAMAGE, "damage.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DAMAGE_BACK, "damage.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DEAD, "dead.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, HORSE_SKILL_CHARGE, "skill_charge.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, HORSE_SKILL_SPLASH, "skill_splash.msa");

    // HORSE_ONEHAND_SWORD
    chrmgr.SetPathName(path + "horse_onehand_sword/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE_ONEHAND_SWORD);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    pRaceData->ReserveComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, 3);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_ONEHAND_SWORD, COMBO_TYPE_1, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_ONEHAND_SWORD, HORSE_SKILL_WILDATTACK, "skill_wildattack.msa");

    pRaceData->RegisterAttachingBoneName(PART_WEAPON, "equip_right");
    RegisterEmotionAnis(path);
}

auto LoadShaman(uint8_t race, const std::string &path) -> void
{
    auto &chrmgr = CRaceManager::instance();

    // Shaman
    ////////////////////////////////////////////////////////////////////////////////////////#
    chrmgr.SelectRace(race);
    CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
    {
        SPDLOG_ERROR("RaceData has not been selected");
        return;
    }

    // GENERAL MOTION MODE
    SetGeneralMotions(MOTION_MODE_GENERAL, path + "general/");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_COMBO_ATTACK_1, "attack.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_COMBO_ATTACK_1, "attack_1.msa", 50);

    pRaceData->ReserveComboAttack(MOTION_MODE_GENERAL, COMBO_TYPE_1, 1);
    pRaceData->RegisterComboAttack(MOTION_MODE_GENERAL, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);

    // Fan
    chrmgr.SetPathName(path + "fan/");
    pRaceData->RegisterMotionMode(MOTION_MODE_FAN);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_WAIT, "wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_DAMAGE, "damage.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_DAMAGE, "damage_1.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_DAMAGE_BACK, "damage_2.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_DAMAGE_BACK, "damage_3.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_COMBO_ATTACK_4, "combo_04.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_COMBO_ATTACK_5, "combo_05.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_COMBO_ATTACK_6, "combo_06.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FAN, MOTION_COMBO_ATTACK_7, "combo_07.msa");

    // Combo Type 1
    pRaceData->ReserveComboAttack(MOTION_MODE_FAN, COMBO_TYPE_1, 4);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_1, COMBO_INDEX_4, MOTION_COMBO_ATTACK_4);
    // Combo Type 2
    pRaceData->ReserveComboAttack(MOTION_MODE_FAN, COMBO_TYPE_2, 5);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_4, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_2, COMBO_INDEX_5, MOTION_COMBO_ATTACK_7);
    // Combo Type 3
    pRaceData->ReserveComboAttack(MOTION_MODE_FAN, COMBO_TYPE_3, 6);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_4, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_5, MOTION_COMBO_ATTACK_6);
    pRaceData->RegisterComboAttack(MOTION_MODE_FAN, COMBO_TYPE_3, COMBO_INDEX_6, MOTION_COMBO_ATTACK_4);

    // Bell
    chrmgr.SetPathName(path + "Bell/");
    pRaceData->RegisterMotionMode(MOTION_MODE_BELL);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_WAIT, "wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_DAMAGE, "damage.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_DAMAGE, "damage_1.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_DAMAGE_BACK, "damage_2.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_DAMAGE_BACK, "damage_3.msa", 50);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_COMBO_ATTACK_4, "combo_04.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_COMBO_ATTACK_5, "combo_05.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_COMBO_ATTACK_6, "combo_06.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_BELL, MOTION_COMBO_ATTACK_7, "combo_07.msa");

    // Combo Type 1
    pRaceData->ReserveComboAttack(MOTION_MODE_BELL, COMBO_TYPE_1, 4);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_1, COMBO_INDEX_4, MOTION_COMBO_ATTACK_4);
    // Combo Type 2
    pRaceData->ReserveComboAttack(MOTION_MODE_BELL, COMBO_TYPE_2, 5);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_4, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_2, COMBO_INDEX_5, MOTION_COMBO_ATTACK_7);
    // Combo Type 3
    pRaceData->ReserveComboAttack(MOTION_MODE_BELL, COMBO_TYPE_3, 6);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_4, MOTION_COMBO_ATTACK_5);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_5, MOTION_COMBO_ATTACK_6);
    pRaceData->RegisterComboAttack(MOTION_MODE_BELL, COMBO_TYPE_3, COMBO_INDEX_6, MOTION_COMBO_ATTACK_4);

    // SKILL
    chrmgr.SetPathName(path + "skill/");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 1, "bipabu.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 2, "yongpa.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 3, "paeryong.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 4, "hosin_target.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 5, "boho_target.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 6, "gicheon_target.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 16, "noejeon.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 17, "byeorak.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 18, "pokroe.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 19, "jeongeop_target.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 20, "kwaesok_target.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + 21, "jeungryeok_target.msa");
    // chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL+10,	"budong.msa")

    auto START_INDEX = 0;

    for (int i = 1; i < 4; ++i)
    {
        std::string END_STRING = "";
        if (i != 0)
            END_STRING = fmt::format("_{0}", i + 1);

        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 1,
                                       "bipabu" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 2,
                                       "yongpa" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 3,
                                       "paeryong" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 4,
                                       "hosin" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 5,
                                       "boho" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 6,
                                       "gicheon" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 16,
                                       "noejeon" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 17,
                                       "byeorak" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 18,
                                       "pokroe" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 19,
                                       "jeongeop" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 20,
                                       "kwaesok" + END_STRING + ".msa");
        chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL + (i * CPythonSkill::SKILL_GRADEGAP) + 21,
                                       "jeungryeok" + END_STRING + ".msa");
    }

    //# chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, MOTION_SKILL+(i*CPythonSkill::SKILL_GRADEGAP)+10,	"budong"
    //+ END_STRING + ".msa")

    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONBLOOD, "guild_yongsinuipi.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONBLESS, "guild_yongsinuichukbok.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_BLESSARMOR, "guild_seonghwigap.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_SPPEDUP, "guild_gasokhwa.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_DRAGONWRATH, "guild_yongsinuibunno.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_GENERAL, GUILD_SKILL_MAGICUP, "guild_jumunsul.msa");

    // FISHING
    chrmgr.SetPathName(path + "fishing/");
    pRaceData->RegisterMotionMode(MOTION_MODE_FISHING);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_WAIT, "wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_THROW, "throw.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_WAIT, "fishing_wait.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_STOP, "fishing_cancel.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_REACT, "fishing_react.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_CATCH, "fishing_catch.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_FISHING, MOTION_FISHING_FAIL, "fishing_fail.msa");

    // HORSE
    chrmgr.SetPathName(path + "horse/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait.msa", 90);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait_1.msa", 9);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WAIT, "wait_2.msa", 1);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_WALK, "walk.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_RUN, "run.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DAMAGE, "damage.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DAMAGE_BACK, "damage.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, MOTION_DEAD, "dead.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, HORSE_SKILL_CHARGE, "skill_charge.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE, HORSE_SKILL_SPLASH, "skill_splash.msa");

    // HORSE_FAN
    chrmgr.SetPathName(path + "horse_fan/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE_FAN);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_FAN, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_FAN, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_FAN, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    pRaceData->ReserveComboAttack(MOTION_MODE_HORSE_FAN, COMBO_TYPE_1, 3);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_FAN, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_FAN, COMBO_TYPE_1, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_FAN, COMBO_TYPE_1, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_FAN, HORSE_SKILL_WILDATTACK, "skill_wildattack.msa");

    // HORSE_BELL
    chrmgr.SetPathName(path + "horse_bell/");
    pRaceData->RegisterMotionMode(MOTION_MODE_HORSE_BELL);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BELL, MOTION_COMBO_ATTACK_1, "combo_01.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BELL, MOTION_COMBO_ATTACK_2, "combo_02.msa");
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BELL, MOTION_COMBO_ATTACK_3, "combo_03.msa");
    pRaceData->ReserveComboAttack(MOTION_MODE_HORSE_BELL, COMBO_TYPE_1, 3);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_BELL, COMBO_TYPE_1, COMBO_INDEX_1, MOTION_COMBO_ATTACK_1);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_BELL, COMBO_TYPE_1, COMBO_INDEX_2, MOTION_COMBO_ATTACK_2);
    pRaceData->RegisterComboAttack(MOTION_MODE_HORSE_BELL, COMBO_TYPE_1, COMBO_INDEX_3, MOTION_COMBO_ATTACK_3);
    chrmgr.RegisterCacheMotionData(MOTION_MODE_HORSE_BELL, HORSE_SKILL_WILDATTACK, "skill_wildattack.msa");

    pRaceData->RegisterAttachingBoneName(PART_WEAPON, "equip_right");
    pRaceData->RegisterAttachingBoneName(PART_WEAPON_LEFT, "equip_left");
    RegisterEmotionAnis(path);
}

auto RegisterRace(uint8_t race, const std::string msmPath) -> void
{
    auto &chrmgr = CRaceManager::instance();
    chrmgr.CreateRace(race);
    chrmgr.SelectRace(race);

    auto pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
    if (!pRaceData)
    {
        SPDLOG_ERROR("RaceData has not been selected");
        return;
    }

    pRaceData->LoadRaceData(msmPath.c_str());
}

auto RegisterGameSound() -> void
{
    CPythonItem &item = CPythonItem::Instance();
    item.SetUseSoundFileName(item.USESOUND_DEFAULT, "sound/ui/drop.wav");
    item.SetUseSoundFileName(item.USESOUND_ACCESSORY, "sound/ui/equip_ring_amulet.wav");
    item.SetUseSoundFileName(item.USESOUND_ARMOR, "sound/ui/equip_metal_armor.wav");
    item.SetUseSoundFileName(item.USESOUND_BOW, "sound/ui/equip_bow.wav");
    item.SetUseSoundFileName(item.USESOUND_WEAPON, "sound/ui/equip_metal_weapon.wav");
    item.SetUseSoundFileName(item.USESOUND_POTION, "sound/ui/eat_potion.wav");
    item.SetUseSoundFileName(item.USESOUND_PORTAL, "sound/ui/potal_scroll.wav");

    item.SetDropSoundFileName(item.DROPSOUND_DEFAULT, "sound/ui/drop.wav");
    item.SetDropSoundFileName(item.DROPSOUND_ACCESSORY, "sound/ui/equip_ring_amulet.wav");
    item.SetDropSoundFileName(item.DROPSOUND_ARMOR, "sound/ui/equip_metal_armor.wav");
    item.SetDropSoundFileName(item.DROPSOUND_BOW, "sound/ui/equip_bow.wav");
    item.SetDropSoundFileName(item.DROPSOUND_WEAPON, "sound/ui/equip_metal_weapon.wav");
}

auto RegisterGameEffects() -> void
{
    CInstanceBase::SetDustGap(250);
    CInstanceBase::SetHorseDustGap(500);

    CPythonPlayer::Instance().RegisterEffect(CPythonPlayer::EFFECT_PICK, "d:/ymir work/effect/etc/click/click.mse",
                                             true);

    gCharacterEffectRegistry->RegisterEffect(EFFECT_DUST, "", "d:/ymir work/effect/etc/dust/dust.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_HORSE_DUST, "", "d:/ymir work/effect/etc/dust/running_dust.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_HIT, "", "d:/ymir work/effect/hit/blow_1/blow_1_low.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_HPUP_RED, "",
                                             "d:/ymir work/effect/etc/recuperation/drugup_red.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SPUP_BLUE, "",
                                             "d:/ymir work/effect/etc/recuperation/drugup_blue.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SPEEDUP_GREEN, "",
                                             "d:/ymir work/effect/etc/recuperation/drugup_green.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DXUP_PURPLE, "",
                                             "d:/ymir work/effect/etc/recuperation/drugup_purple.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_AUTO_HPUP, "",
                                             "d:/ymir work/effect/etc/recuperation/autodrugup_red.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_AUTO_SPUP, "",
                                             "d:/ymir work/effect/etc/recuperation/autodrugup_blue.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_RAMADAN_RING_EQUIP, "",
                                             "d:/ymir work/effect/etc/buff/buff_item1.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_HALLOWEEN_CANDY_EQUIP, "",
                                             "d:/ymir work/effect/etc/buff/buff_item2.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_HAPPINESS_RING_EQUIP, "",
                                             "d:/ymir work/effect/etc/buff/buff_item3.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_LOVE_PENDANT_EQUIP, "",
                                             "d:/ymir work/effect/etc/buff/buff_item4.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_ACCE_SUCESS_ABSORB, "",
                                             "d:/ymir work/effect/etc/buff/buff_item6.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_ACCE_EQUIP, "", "d:/ymir work/effect/etc/buff/buff_item7.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_ACCE_BACK, "Bip01","D:/ymir work/effect/effect/armor/acc_01.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EASTER_CANDY_EQIP, "",
                                             "d:/ymir work/effect/etc/buff/buff_item8.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_CAPE_OF_COURAGE, "",
                                             "d:/ymir work/effect/etc/buff/buff_umhang.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_PENETRATE, "Bip01", "d:/ymir work/effect/hit/gwantong.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_THUNDER_AREA, "",
                                             "D:/ymir work/effect/monster/light_emissive3.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_THUNDER, "",
                                             "D:/ymir work/effect/monster/yellow_tigerman_24_1.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_HEAL, "", "D:/ymir work/pc/shaman/effect/jeongeop_2.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_CAPE_OF_COURAGE, "", "D:/ymir work/effect/etc/buff/buff_item9.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_CHOCOLATE_PENDANT, "",
                                             "D:/ymir work/effect/etc/buff/buff_item10.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_PEPSI_EVENT, "", "D:/ymir work/effect/etc/buff/buff_item11.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DRAGONLAIR_STONE_UNBEATABLE_1, "",
                                             "D:/ymir work/effect/monster2/redD_moojuk.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DRAGONLAIR_STONE_UNBEATABLE_2, "",
                                             "D:/ymir work/effect/monster2/redD_moojuk_blue.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DRAGONLAIR_STONE_UNBEATABLE_3, "",
                                             "D:/ymir work/effect/monster2/redD_moojuk_green.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_FEATHER_WALK, "", "d:/ymir work/effect/hit/gyeonggong_boom.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_BATTLE_POTION, "", "D:/ymir work/effect/etc/buff/buff_item12.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_REFLECT, "", "D:/ymir work/effect/hit/blow_4/blow_4_ref.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SKILL_DAMAGE_ZONE, "",
                                             "D:/ymir work/effect/monster2/12_shelter_in_01.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SKILL_SAFE_ZONE, "",
                                             "D:/ymir work/effect/monster2/12_shelter_in_02.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_METEOR, "", "D:/ymir work/effect/monster2/12_tiger_s3_drop.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_BEAD_RAIN, "", "D:/ymir work/effect/monster2/12_dra_s2_drop.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_FALL_ROCK, "", "D:/ymir work/effect/monster2/12_mon_s3_drop.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_ARROW_RAIN, "", "D:/ymir work/effect/monster2/12_sna_s3_drop.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_HORSE_DROP, "", "D:/ymir work/effect/monster2/12_hor_s3_drop.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EGG_DROP, "", "D:/ymir work/effect/monster2/12_chi_s3_drop.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DEAPO_BOOM, "",
                                             "D:/ymir work/effect/monster2/daepo_na_02_boom.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_FLOWER_EVENT, "",
                                             "d:/ymir work/effect/etc/buff/buff_item15_flower.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_GEM_PENDANT, "", "d:/ymir work/effect/etc/buff/buff_item16.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_FIRECRACKER, "",
                                             "d:/ymir work/effect/etc/firecracker/newyear_firecracker.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SPIN_TOP, "", "d:/ymir work/effect/etc/firecracker/paing_i.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SELECT, "", "d:/ymir work/effect/etc/click/click_select.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_TARGET, "", "d:/ymir work/effect/etc/click/click_glow_select.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_STUN, "Bip01 Head", "d:/ymir work/effect/etc/stun/stun.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_CRITICAL, "Bip01 R Hand", "d:/ymir work/effect/hit/critical.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DAMAGE_TARGET, "",
                                             "d:/ymir work/effect/affect/damagevalue/target.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DAMAGE_NOT_TARGET, "",
                                             "d:/ymir work/effect/affect/damagevalue/nontarget.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DAMAGE_SELFDAMAGE, "",
                                             "d:/ymir work/effect/affect/damagevalue/damage.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DAMAGE_SELFDAMAGE2, "",
                                             "d:/ymir work/effect/affect/damagevalue/damage_1.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DAMAGE_POISON, "",
                                             "d:/ymir work/effect/affect/damagevalue/poison.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DAMAGE_MISS, "", "d:/ymir work/effect/affect/damagevalue/miss.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DAMAGE_TARGETMISS, "",
                                             "d:/ymir work/effect/affect/damagevalue/target_miss.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_DAMAGE_CRITICAL, "",
                                             "d:/ymir work/effect/affect/damagevalue/critical.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SUCCESS, "", "d:/ymir work/effect/success.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_FAIL, "", "d:/ymir work/effect/fail.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_PERCENT_DAMAGE1, "", "d:/ymir work/effect/hit/percent_damage1.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_PERCENT_DAMAGE2, "", "d:/ymir work/effect/hit/percent_damage2.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_PERCENT_DAMAGE3, "", "d:/ymir work/effect/hit/percent_damage3.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_SPAWN_APPEAR, "Bip01",
                                             "d:/ymir work/effect/etc/appear_die/monster_appear.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SPAWN_DISAPPEAR, "Bip01",
                                             "d:/ymir work/effect/etc/appear_die/monster_die.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_FLAME_ATTACK, "equip_right_hand",
                                             "d:/ymir work/effect/hit/blow_flame/flame_3_weapon.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_FLAME_HIT, "",
                                             "d:/ymir work/effect/hit/blow_flame/flame_3_blow.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_FLAME_ATTACH, "",
                                             "d:/ymir work/effect/hit/blow_flame/flame_3_body.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_ELECTRIC_ATTACK, "equip_right",
                                             "d:/ymir work/effect/hit/blow_electric/light_1_weapon.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_ELECTRIC_HIT, "",
                                             "d:/ymir work/effect/hit/blow_electric/light_1_blow.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_ELECTRIC_ATTACH, "",
                                             "d:/ymir work/effect/hit/blow_electric/light_1_body.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_LEVELUP, "", "d:/ymir work/effect/etc/levelup_1/level_up.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SKILLUP, "", "d:/ymir work/effect/etc/skillup/skillup_1.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMPIRE + 1, "Bip01", "d:/ymir work/effect/etc/empire/empire_A.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMPIRE + 2, "Bip01", "d:/ymir work/effect/etc/empire/empire_B.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMPIRE + 3, "Bip01", "d:/ymir work/effect/etc/empire/empire_C.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_WEAPON_ONEHANDED, "equip_right_hand",
                                             "d:/ymir work/pc/warrior/effect/geom_sword_loop.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_WEAPON_TWOHANDED, "equip_right_hand",
                                             "d:/ymir work/pc/warrior/effect/geom_spear_loop.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_SWORD_REFINED7, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_7.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SWORD_REFINED8, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_8.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SWORD_REFINED9, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_9.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_BOW_REFINED7, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_7_b.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_BOW_REFINED8, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_8_b.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_BOW_REFINED9, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_9_b.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_FANBELL_REFINED7, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_7_f.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_FANBELL_REFINED8, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_8_f.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_FANBELL_REFINED9, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_9_f.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_FANBELL_REFINED7_LEFT, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_7_f.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_FANBELL_REFINED8_LEFT, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_8_f.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_FANBELL_REFINED9_LEFT, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_9_f.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_SMALLSWORD_REFINED7, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_7_s.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SMALLSWORD_REFINED8, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_8_s.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SMALLSWORD_REFINED9, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_9_s.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_SMALLSWORD_REFINED7_LEFT, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_7_s.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SMALLSWORD_REFINED8_LEFT, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_8_s.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SMALLSWORD_REFINED9_LEFT, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_9_s.mse");

    // CLAW RIGHT
    gCharacterEffectRegistry->RegisterEffect(EFFECT_CLAW_REFINED7, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_7_w.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_CLAW_REFINED8, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_8_w.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_CLAW_REFINED9, "PART_WEAPON",
                                             "D:/ymir work/effect/effect/sword/sword_9_w.mse");

    // CLAW LEFT
    gCharacterEffectRegistry->RegisterEffect(EFFECT_CLAW_REFINED7_LEFT, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_7_w.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_CLAW_REFINED8_LEFT, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_8_w.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_CLAW_REFINED9_LEFT, "PART_WEAPON_LEFT",
                                             "D:/ymir work/effect/effect/sword/sword_9_w.mse");

    // BODY
    gCharacterEffectRegistry->RegisterEffect(EFFECT_BODYARMOR_REFINED7, "Bip01",
                                             "D:/ymir work/effect/effect/armor/armor_7.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_BODYARMOR_REFINED8, "Bip01",
                                             "D:/ymir work/effect/effect/armor/armor_8.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_BODYARMOR_REFINED9, "Bip01",
                                             "D:/ymir work/effect/effect/armor/armor_9.mse");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_GM, "Bip01", "d:/ymir work/effect/gm_logos/gm.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SGM, "Bip01", "d:/ymir work/effect/gm_logos/super_game_master.mse");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_SA, "Bip01", "d:/ymir work/effect/gm_logos/server_admin.mse");
}

auto RegisterGameAffects() -> void
{
    gCharacterEffectRegistry->RegisterAffect(AFFECT_POISON, "Bip01",
                                             "d:/ymir work/effect/hit/blow_poison/poison_loop.mse");
    gCharacterEffectRegistry->RegisterAffect(AFFECT_SLOW, "", "d:/ymir work/effect/affect/slow.mse");
    gCharacterEffectRegistry->RegisterAffect(AFFECT_STUN, "Bip01 Head", "d:/ymir work/effect/etc/stun/stun_loop.mse");
    gCharacterEffectRegistry->RegisterAffect(AFFECT_FIRE, "Bip01", "d:/ymir work/effect/hit/blow_flame/flame_loop.mse");
    gCharacterEffectRegistry->RegisterAffect(AFFECT_BLEEDING, "Bip01",
                                             "d:/ymir work/effect/hit/blow_poison/bleeding_loop.mse");
    //gCharacterEffectRegistry->RegisterAffect(AFFECT_DRAGON_SOUL_DECK_0, "", "d:/ymir work/effect/affect/dss.mse");
    //gCharacterEffectRegistry->RegisterAffect(AFFECT_DRAGON_SOUL_DECK_1, "", "d:/ymir work/effect/affect/dss.mse");
    gCharacterEffectRegistry->RegisterAffect(AFFECT_PREVENT_GOOD, "Bip01 Head",
                                             "d:/ymir work/pc/sura/effect/pabeop_loop.mse"); // ����

    // RegisterAffect(AFFECT_DUNGEON_READY, "", "d:/ymir work/effect/etc/ready/ready.mse")

    // RegisterAffect(AFFECT_BUILDING_CONSTRUCTION_SMALL, "", "d:/ymir work/guild/effect/10_construction.mse")
    // RegisterAffect(AFFECT_BUILDING_CONSTRUCTION_LARGE, "", "d:/ymir work/guild/effect/20_construction.mse")
    // RegisterAffect(AFFECT_BUILDING_UPGRADE, "", "d:/ymir work/guild/effect/20_upgrade.mse")

    gCharacterEffectRegistry->RegisterAffect(AFFECT_WAR_FLAG_1, "",
                                             "d:/ymir work/effect/etc/guild_war_flag/flag_red.mse");
    gCharacterEffectRegistry->RegisterAffect(AFFECT_WAR_FLAG_2, "",
                                             "d:/ymir work/effect/etc/guild_war_flag/flag_blue.mse");
    gCharacterEffectRegistry->RegisterAffect(AFFECT_WAR_FLAG_3, "",
                                             "d:/ymir work/effect/etc/guild_war_flag/flag_yellow.mse");

    gCharacterEffectRegistry->RegisterAffect(AFFECT_STATUE_1, "", "d:/ymir work/effect/monster2/redd_moojuk.mse");
    gCharacterEffectRegistry->RegisterAffect(AFFECT_STATUE_2, "", "d:/ymir work/effect/monster2/redd_moojuk.mse");
    gCharacterEffectRegistry->RegisterAffect(AFFECT_STATUE_3, "", "d:/ymir work/effect/monster2/redd_moojuk_blue.mse");
    gCharacterEffectRegistry->RegisterAffect(AFFECT_STATUE_4, "", "d:/ymir work/effect/monster2/redd_moojuk_green.mse");

    // RegisterAffect(AFFECT_CHEONGEUN, "", "d:/ymir work/pc/warrior/effect/gyeokgongjang_loop.mse") // õ���� (�ؿ���
    // �ֵ�-_-)
    gCharacterEffectRegistry->RegisterAffect(SKILL_CHUNKEON, "",
                                             "d:/ymir work/pc/warrior/effect/gyeokgongjang_loop.mse");
    gCharacterEffectRegistry->RegisterAffect(SKILL_GYEONGGONG, "",
                                             "d:/ymir work/pc/assassin/effect/gyeonggong_loop.mse"); // �ڰ� - ���
    gCharacterEffectRegistry->RegisterAffect(SKILL_GWIGEOM, "Bip01 R Finger2",
                                             "d:/ymir work/pc/sura/effect/gwigeom_loop.mse");
    gCharacterEffectRegistry->RegisterAffect(SKILL_TERROR, "", "d:/ymir work/pc/sura/effect/fear_loop.mse");
    // ���� - ����
    gCharacterEffectRegistry->RegisterAffect(SKILL_JUMAGAP, "", "d:/ymir work/pc/sura/effect/jumagap_loop.mse");
    // ���� - �ָ���
    gCharacterEffectRegistry->RegisterAffect(SKILL_HOSIN, "", "d:/ymir work/pc/shaman/effect/3hosin_loop.mse");
    // ���� - ȣ��
    gCharacterEffectRegistry->RegisterAffect(SKILL_REFLECT, "", "d:/ymir work/pc/shaman/effect/boho_loop.mse");
    // ���� - ��ȣ
    gCharacterEffectRegistry->RegisterAffect(SKILL_KWAESOK, "", "d:/ymir work/pc/shaman/effect/10kwaesok_loop.mse");
    ;
    // ���� - ���
    gCharacterEffectRegistry->RegisterAffect(SKILL_MANASHIELD, "", "d:/ymir work/pc/sura/effect/heuksin_loop.mse");
    gCharacterEffectRegistry->RegisterAffect(SKILL_MUYEONG, "", "d:/ymir work/pc/sura/effect/muyeong_loop.mse");
    gCharacterEffectRegistry->RegisterAffect(SKILL_GICHEON, "Bip01 R Hand",
                                             "d:/ymir work/pc/shaman/effect/6gicheon_hand.mse");
    gCharacterEffectRegistry->RegisterAffect(SKILL_JEUNGRYEOK, "Bip01 L Hand",
                                             "d:/ymir work/pc/shaman/effect/jeungryeok_hand.mse");
    gCharacterEffectRegistry->RegisterAffect(SKILL_PABEOB, "Bip01 Head", "d:/ymir work/pc/sura/effect/pabeop_loop.mse");
    // RegisterAffect(AFFECT_FALLEN_CHEONGEUN, "","d:/ymir work/pc/warrior/effect/gyeokgongjang_loop.mse") // õ����
    // (Fallen)
    gCharacterEffectRegistry->RegisterAffect(SKILL_JEOKRANG, "Bip01",
                                             "d:/ymir work/effect/hit/blow_flame/flame_loop_w.mse");
    // gCharacterEffectRegistry->RegisterAffect(SKILLL_CHEONGRANG, "", "d:/ymir
    // work/pc3/common/effect/gyeokgongjang_loop_w.mse")
    gCharacterEffectRegistry->RegisterAffect(AFFECT_EPIC, "Bip01",
                                             "d:/ymir work/effect/pvm_mde/ridack_epic.mse");
    
}

auto RegisterFlyEffects() -> void
{
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_EXP, CFlyingManager::INDEX_FLY_TYPE_NORMAL,
        "d:/ymir work/effect/etc/gathering/ga_piece_yellow_small2.msf"); // 노란색 (EXP)
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_HP_MEDIUM, CFlyingManager::INDEX_FLY_TYPE_NORMAL,
        "d:/ymir work/effect/etc/gathering/ga_piece_red_small.msf"); // 빨간색 (HP) 작은거
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_HP_BIG, CFlyingManager::INDEX_FLY_TYPE_NORMAL,
        "d:/ymir work/effect/etc/gathering/ga_piece_red_big.msf"); // 빨간색 (HP) 큰거
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_SP_SMALL, CFlyingManager::INDEX_FLY_TYPE_NORMAL,
        "d:/ymir work/effect/etc/gathering/ga_piece_blue_warrior_small.msf"); // 파란색 꼬리만 있는거
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_SP_MEDIUM, CFlyingManager::INDEX_FLY_TYPE_NORMAL,
        "d:/ymir work/effect/etc/gathering/ga_piece_blue_small.msf"); // 파란색 작은거
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_SP_BIG, CFlyingManager::INDEX_FLY_TYPE_NORMAL,
        "d:/ymir work/effect/etc/gathering/ga_piece_blue_big.msf"); // 파란색 큰거
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_FIREWORK1, CFlyingManager::INDEX_FLY_TYPE_FIRE_CRACKER,
        "d:/ymir work/effect/etc/firecracker/firecracker_1.msf"); // 폭죽 1
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_FIREWORK2, CFlyingManager::INDEX_FLY_TYPE_FIRE_CRACKER,
        "d:/ymir work/effect/etc/firecracker/firecracker_2.msf"); // 폭죽 2
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_FIREWORK3, CFlyingManager::INDEX_FLY_TYPE_FIRE_CRACKER,
        "d:/ymir work/effect/etc/firecracker/firecracker_3.msf"); // 폭죽 3
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_FIREWORK4, CFlyingManager::INDEX_FLY_TYPE_FIRE_CRACKER,
        "d:/ymir work/effect/etc/firecracker/firecracker_4.msf"); // 폭죽 4
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_FIREWORK5, CFlyingManager::INDEX_FLY_TYPE_FIRE_CRACKER,
        "d:/ymir work/effect/etc/firecracker/firecracker_5.msf"); // 폭죽 5
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_FIREWORK6, CFlyingManager::INDEX_FLY_TYPE_FIRE_CRACKER,
        "d:/ymir work/effect/etc/firecracker/firecracker_6.msf"); // 폭죽 6
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_FIREWORK_CHRISTMAS, CFlyingManager::INDEX_FLY_TYPE_FIRE_CRACKER,
        "d:/ymir work/effect/etc/firecracker/firecracker_xmas.msf"); // 폭죽 X-Mas
    CFlyingManager::Instance().RegisterIndexedFlyData(FLY_CHAIN_LIGHTNING, CFlyingManager::INDEX_FLY_TYPE_NORMAL,
                                                      "d:/ymir work/pc/shaman/effect/pokroe.msf"); // 폭뢰격
    CFlyingManager::Instance().RegisterIndexedFlyData(
        FLY_HP_SMALL, CFlyingManager::INDEX_FLY_TYPE_NORMAL,
        "d:/ymir work/effect/etc/gathering/ga_piece_red_smallest.msf"); // 빨간색 매우 작은거
    CFlyingManager::Instance().RegisterIndexedFlyData(FLY_SKILL_MUYEONG, CFlyingManager::INDEX_FLY_TYPE_AUTO_FIRE,
                                                      "d:/ymir work/pc/sura/effect/muyeong_fly.msf"); // 무영진
    CFlyingManager::Instance().RegisterIndexedFlyData(FLY_QUIVER_ATTACK_NORMAL, CFlyingManager::INDEX_FLY_TYPE_NORMAL,
                                                      "d:/ymir work/pc/assassin/effect/arrow_02.msf");
}

auto RegisterEmoticons() -> void
{
    ////////////////////////////////////////////////////////////////////////////////////////
    // Emoticon
    std::string EmoticonStr = "d:/ymir work/effect/etc/emoticon/";
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 0, "", ((EmoticonStr + "sweat.mse").c_str()));
    gPythonNetworkStream->RegisterEmoticonString("(Sweat)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 1, "", (EmoticonStr + "money.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(Money)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 2, "", (EmoticonStr + "happy.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(Happy)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 3, "", (EmoticonStr + "love_s.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(Good)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 4, "", (EmoticonStr + "love_l.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(Love)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 5, "", (EmoticonStr + "angry.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(Angry)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 6, "", (EmoticonStr + "aha.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(Aha)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 7, "", (EmoticonStr + "gloom.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(Depressed)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 8, "", (EmoticonStr + "sorry.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(Sorry)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 9, "", (EmoticonStr + "!_mix_back.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(!)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 10, "", (EmoticonStr + "question.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(Question)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 11, "", (EmoticonStr + "fish.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(Fish)");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 12, "", (EmoticonStr + "alcohol.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(alcohol)");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 13, "", (EmoticonStr + "busy.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(busy)");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 14, "", (EmoticonStr + "call.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(call)");

    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 15, "", (EmoticonStr + "celebration.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(celebration)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 16, "", (EmoticonStr + "charging.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(charging)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 17, "", (EmoticonStr + "hungry.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(hungry)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 18, "", (EmoticonStr + "letter.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(letter)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 19, "", (EmoticonStr + "nosay.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(nosay)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 20, "", (EmoticonStr + "siren.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(siren)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 21, "", (EmoticonStr + "weather1.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(weather1)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 22, "", (EmoticonStr + "weather2.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(weather2)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 23, "", (EmoticonStr + "weather3.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(weather3)");
    gCharacterEffectRegistry->RegisterEffect(EFFECT_EMOTICON + 24, "", (EmoticonStr + "whirl.mse").c_str());
    gPythonNetworkStream->RegisterEmoticonString("(whirl)");
}

void LoadRaceMotions(uint32_t race)
{
    CRaceManager::instance().PreloadRace(race);

    auto pRaceData = CRaceManager::instance().GetRaceDataPointer(race);
    if (pRaceData)
        pRaceData.value()->LoadMotions();
}

bool LoadGameData()
{
    auto &itemMgr = CItemManager::Instance();
    auto &nonPlayer = NpcManager::Instance();
    auto &dungeonMan = CPythonDungeonInfo::instance();

    if (!WikiManager::instance().LoadWikiConfig("data/wiki_config.txt"))
        return false;

    if (!itemMgr.LoadItemAttrProtos())
        return false;

    if (!itemMgr.LoadItemTable("data/item_proto"))
    {
        SPDLOG_ERROR("LoadGameData - Failed to load data/item_proto");
        return false;
    }

    if (!itemMgr.ReadLevelPetData("data/togggle_level_pet.txt"))
    {
        SPDLOG_ERROR("LoadGameData - Failed to load data/togggle_level_pet.txt");
        return false;
    }

    if (!itemMgr.LoadItemList("data/item_list.txt"))
    {
        SPDLOG_ERROR("LoadGameData - LoadItemList Error");
        return false;
    }

    auto data = LoadFileToString(GetVfs(), "data/hunting_missions.txt");
    if (data)
    {
        if (!HuntingManager::instance().LoadClient(data.value()))
        {
            SPDLOG_ERROR("LoadGameData - Unable to load hunting mission data");
            return false;
        }
    }

    if (!CPythonCube::instance().Initialize())
    {
        return false;
    }

    if (!itemMgr.LoadDragonSoulTable("data/dragon_soul_table.txt"))
    {
        SPDLOG_ERROR("LoadGameData - LoadDragonSoulTable Error");
        return false;
    }

    if (!itemMgr.LoadRareItems("data/item_rare.txt"))
    {
        SPDLOG_WARN("LoadGameData - Failed to load data/item_rare.txt");
    }

    if (!itemMgr.LoadItemScale("data/item_scale.txt"))
    {
        SPDLOG_WARN("LoadGameData - Failed to load data/item_scale.txt");
    }

#ifdef ENABLE_SHINING_SYSTEM
    if (!itemMgr.LoadShiningTable("data/shiningtable.txt"))
    {
        SPDLOG_DEBUG("LoadGameData - Failed to load data/shiningtable.txt");
    }
#endif

    if (!NpcManager::instance().LoadNonPlayerData("data/mob_proto"))
    {
        SPDLOG_ERROR("LoadLocaleData - Failed to load data/mob_proto");
        return false;
    }

    if (!CPythonGuild::Instance().LoadObjectProto("data/object_proto"))
        Py_RETURN_FALSE;

    if (!nonPlayer.LoadNonPlayerData("data/mob_proto"))
    {
        return false;
    }

    if (!nonPlayer.LoadNpcList("data/npclist.txt"))
    {
        SPDLOG_DEBUG("LoadLocaleData - LoadNpcList({0}) Error", "data/npclist.txt");
    }

    if (!nonPlayer.LoadRaceHeight("data/race_height.txt"))
    {
        SPDLOG_DEBUG("LoadLocaleData - LoadRaceHeight({0}) Error", "data/race_height.txt");
    }

    if (!nonPlayer.LoadHugeRace("data/huge_race.txt"))
    {
        SPDLOG_DEBUG("LoadLocaleData - LoadHugeRace({0}) Error", "data/huge_race.txt");
    }

    if (!nonPlayer.LoadSpeculaSettingFile("data/npc_specular.txt"))
    {
        SPDLOG_DEBUG("LoadLocaleData - LoadSpeculaSettingFile({0}) Error", "data/npc_specular.txt");
    }

    if (!nonPlayer.LoadWorldBossInfo("data/world_boss.txt"))
    {
        SPDLOG_DEBUG("LoadLocaleData - LoadWorldBossInfo({0}) Error", "data/world_boss.txt");
    }

    if (!dungeonMan.LoadDungeonInfo("data/dungeon_info.txt"))
    {
        SPDLOG_DEBUG("LoadLocaleData - LoadWorldBossInfo({0}) Error", "data/world_boss.txt");
        return false;
    }
    RegisterFlyEffects();
    RegisterEmoticons();
    RegisterGameEffects();
    RegisterGameAffects();
    RegisterGameSound();

    RegisterRace(MAIN_RACE_WARRIOR_M, "data/warrior_m.msm");
    SetIntroMotions(MOTION_MODE_GENERAL, "d:/ymir work/pc/warrior/intro/");

    RegisterRace(MAIN_RACE_WARRIOR_W, "data/warrior_w.msm");
    SetIntroMotions(MOTION_MODE_GENERAL, "d:/ymir work/pc2/warrior/intro/");

    RegisterRace(MAIN_RACE_ASSASSIN_W, "data/assassin_w.msm");
    SetIntroMotions(MOTION_MODE_GENERAL, "d:/ymir work/pc/assassin/intro/");

    RegisterRace(MAIN_RACE_ASSASSIN_M, "data/assassin_m.msm");
    SetIntroMotions(MOTION_MODE_GENERAL, "d:/ymir work/pc2/assassin/intro/");

    RegisterRace(MAIN_RACE_SURA_M, "data/sura_m.msm");
    SetIntroMotions(MOTION_MODE_GENERAL, "d:/ymir work/pc/sura/intro/");

    RegisterRace(MAIN_RACE_SURA_W, "data/sura_w.msm");
    SetIntroMotions(MOTION_MODE_GENERAL, "d:/ymir work/pc2/sura/intro/");

    RegisterRace(MAIN_RACE_SHAMAN_W, "data/shaman_w.msm");
    SetIntroMotions(MOTION_MODE_GENERAL, "d:/ymir work/pc/shaman/intro/");

    RegisterRace(MAIN_RACE_SHAMAN_M, "data/shaman_m.msm");
    SetIntroMotions(MOTION_MODE_GENERAL, "d:/ymir work/pc2/shaman/intro/");

    LoadWarrior(MAIN_RACE_WARRIOR_M, "d:/ymir work/pc/warrior/");
    LoadWarrior(MAIN_RACE_WARRIOR_W, "d:/ymir work/pc2/warrior/");

    LoadAssassin(MAIN_RACE_ASSASSIN_W, "d:/ymir work/pc/assassin/");
    LoadAssassin(MAIN_RACE_ASSASSIN_M, "d:/ymir work/pc2/assassin/");

    LoadSura(MAIN_RACE_SURA_M, "d:/ymir work/pc/sura/");
    LoadSura(MAIN_RACE_SURA_W, "d:/ymir work/pc2/sura/");

    LoadShaman(MAIN_RACE_SHAMAN_M, "d:/ymir work/pc2/shaman/");
    LoadShaman(MAIN_RACE_SHAMAN_W, "d:/ymir work/pc/shaman/");

    for (uint32_t i = 0; i < MAIN_RACE_WOLFMAN_M; ++i)
    {
        LoadRaceMotions(i);
    }

    for (int i = 34000; i < 34060 + 1; ++i)
    {
        LoadRaceMotions(i);
    }

    for (int i = 35000; i < 35005 + 1; ++i)
    {
        LoadRaceMotions(i);
    }

    auto &background = CPythonBackground::Instance();
    background.RegisterDungeonMapName("metin2_map_spiderdungeon");
    background.RegisterDungeonMapName("metin2_map_monkeydungeon");
    background.RegisterDungeonMapName("metin2_map_monkeydungeon_02");
    background.RegisterDungeonMapName("metin2_map_monkeydungeon_03");
    background.RegisterDungeonMapName("metin2_map_devilscatacomb");
    background.RegisterDungeonMapName("metin2_map_Mt_Th_dungeon_01");
    background.RegisterDungeonMapName("metin2_map_orclabyrinth");
    background.RegisterDungeonMapName("metin2_map_deviltower1");
    background.RegisterDungeonMapName("metin2_map_anglar_dungeon_01");

    auto colorInfo = py::module::import("colorInfo");

    std::unordered_map<uint16_t, std::vector<int>> NAME_COLOR_DICT = {

        {CInstanceBase::NAMECOLOR_NORMAL_PC, colorInfo.attr("CHR_NAME_RGB_PC").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_NORMAL_NPC, colorInfo.attr("CHR_NAME_RGB_NPC").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_STONE, colorInfo.attr("CHR_NAME_RGB_STONE").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_NORMAL_MOB, colorInfo.attr("CHR_NAME_RGB_MOB").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_PVP, colorInfo.attr("CHR_NAME_RGB_PVP").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_PK, colorInfo.attr("CHR_NAME_RGB_PK").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_PARTY, colorInfo.attr("CHR_NAME_RGB_PARTY").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_WARP, colorInfo.attr("CHR_NAME_RGB_WARP").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_WAYPOINT, colorInfo.attr("CHR_NAME_RGB_WAYPOINT").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_BUFFBOT, colorInfo.attr("CHR_NAME_RGB_BUFFBOT").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_EMPIRE_MOB, colorInfo.attr("CHR_NAME_RGB_EMPIRE_MOB").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_EMPIRE_NPC, colorInfo.attr("CHR_NAME_RGB_EMPIRE_NPC").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_EMPIRE_PC + 1, colorInfo.attr("CHR_NAME_RGB_EMPIRE_PC_A").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_EMPIRE_PC + 2, colorInfo.attr("CHR_NAME_RGB_EMPIRE_PC_B").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_EMPIRE_PC + 3, colorInfo.attr("CHR_NAME_RGB_EMPIRE_PC_C").cast<std::vector<int>>()},
        {CInstanceBase::NAMECOLOR_DEAD, colorInfo.attr("CHR_NAME_RGB_DEAD").cast<std::vector<int>>()},
    };

    for (auto &[name, rgb] : NAME_COLOR_DICT)
        RegisterNameColor(name, rgb[0], rgb[1], rgb[2]);

    std::vector<std::vector<int>> TITLE_COLOR_DICT = {
        colorInfo.attr("TITLE_RGB_GOOD_35").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_34").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_33").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_32").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_31").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_30").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_29").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_28").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_27").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_26").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_25").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_24").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_23").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_22").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_21").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_20").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_19").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_18").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_17").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_16").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_15").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_14").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_13").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_12").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_11").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_10").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_9").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_8").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_7").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_6").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_5").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_4").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_3").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_2").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_GOOD_1").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_NORMAL").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_1").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_2").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_3").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_4").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_5").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_6").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_7").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_8").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_9").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_10").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_11").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_12").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_13").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_14").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_15").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_16").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_17").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_18").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_19").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_20").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_21").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_22").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_23").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_24").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_25").cast<std::vector<int>>(),
        colorInfo.attr("TITLE_RGB_EVIL_26").cast<std::vector<int>>(),
    };

    for (int j = 0; j < TITLE_COLOR_DICT.size(); j++)
    {
        auto &rgb = TITLE_COLOR_DICT[j];
        RegisterTitleColor(j, rgb[0], rgb[1], rgb[2]);
    }
    CInstanceBase::SetEmpireNameMode(false);
    return true;
}

using RaceKey = uint32_t;
using JobKey = uint32_t;
using SkillVnumKey = uint16_t;

std::unordered_map<RaceKey, std::unordered_map<JobKey, std::vector<SkillVnumKey>>> kMainSkillIndices = {
    {JOB_WARRIOR,
     {
         {1, {1, 2, 3, 4, 5, 0, 0, 0, 137, 0, 138, 0, 139, 0}},
         {2, {16, 17, 18, 19, 20, 0, 0, 0, 137, 0, 138, 0, 139, 0}},
         {99, {122, 123, 121, 124, 125, 129, 0, 0, 130, 131, 141, 143}},
     }},
    {JOB_ASSASSIN,
     {
         {1, {31, 32, 33, 34, 35, 0, 0, 0, 137, 0, 138, 0, 139, 0, 140}},
         {2, {46, 47, 48, 49, 50, 0, 0, 0, 137, 0, 138, 0, 139, 0, 140}},
         {99, {122, 123, 121, 124, 125, 129, 0, 0, 130, 131, 141, 143}},
     }},
    {JOB_SURA,
     {
         {1, {61, 62, 63, 64, 65, 66, 0, 0, 137, 0, 138, 0, 139, 0}},
         {2, {76, 77, 78, 79, 80, 81, 0, 0, 137, 0, 138, 0, 139, 0}},
         {99, {122, 123, 121, 124, 125, 129, 0, 0, 130, 131, 141, 143}},
     }},
    {JOB_SHAMAN,
     {
         {1, {91, 92, 93, 94, 95, 96, 0, 0, 137, 0, 138, 0, 139, 0}},
         {2, {106, 107, 108, 109, 110, 111, 0, 0, 137, 0, 138, 0, 139, 0}},
         {99, {122, 123, 121, 124, 125, 129, 0, 0, 130, 131, 141, 143}},
     }},
    {JOB_WOLFMAN,
     {
         {1, {170, 171, 172, 173, 174, 175, 0, 0, 137, 0, 138, 0, 139, 0}},
         {2, {170, 171, 172, 173, 174, 175, 0, 0, 137, 0, 138, 0, 139, 0}},
         {99, {122, 123, 121, 124, 125, 129, 0, 0, 130, 131, 141, 142}},
     }},
};

std::vector<SkillVnumKey> kActiveGuildSkillIndices = {
    152, 153, 154, 155, 156, 157,
};

std::array<size_t, 12> support_skill_list{122, 123, 121, 124, 129, 128, 0, 0, 130, 131, 141, 143};
std::array<size_t, 6> horse_skill_index_list{137, 138, 139, 0, 0, 0};

auto RegisterSkill(uint8_t race, uint8_t group, uint8_t empire) -> void
{
    auto &playerInst = CPythonPlayer::instance();
    auto job = GetJobByRace(race);

    if (kMainSkillIndices.count(job))
    {
        if (kMainSkillIndices[job].count(group))
        {
            const auto &skillList = kMainSkillIndices[job][group];

            for (size_t i = 0; i < skillList.size(); ++i)
            {
                const auto &skillIndex = skillList[i];
                if (i != 6 && i != 7)
                    playerInst.SetSkill(i + 1, skillIndex);
            }

            for (size_t i = 0; i < support_skill_list.size(); ++i)
            {
                playerInst.SetSkill(i + 100 + 1, support_skill_list.at(i));
            }
            for (size_t i = 0; i < horse_skill_index_list.size(); ++i)
            {
                playerInst.SetSkill(i + 120 + 1, horse_skill_index_list.at(i));
            }
        }
    }

    if (empire)
    {
        std::vector<SkillVnumKey> languageSkillList;
        for (int i = 0; i < 3; ++i)
        {
            if (i + 1 != empire)
                languageSkillList.push_back(SKILL_LANGUAGE1 + i);

            for (int j = 0; j < languageSkillList.size(); ++j)
            {
                playerInst.SetSkill(107 + j, languageSkillList[j]);
            }
        }
    }

    // Skilltree skills
    for (int i = 0; i < 24; ++i)
    {
        playerInst.SetSkill(i + 180 + 300, i + 180);
    }

    // Guild passive skill
    playerInst.SetSkill(200, 151);

    for (int i = 0; i < kActiveGuildSkillIndices.size(); ++i)
    {
        playerInst.SetSkill(210 + i, kActiveGuildSkillIndices[i]);
    }
}
