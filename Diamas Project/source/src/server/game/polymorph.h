#ifndef METIN2_SERVER_GAME_POLYMORPH_H
#define METIN2_SERVER_GAME_POLYMORPH_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <unordered_map>

#define POLYMORPH_SKILL_ID	129
#define POLYMORPH_BOOK_ID	50322

enum POLYMORPH_BONUS_TYPE
{
    POLYMORPH_NO_BONUS,
    POLYMORPH_ATK_BONUS,
    POLYMORPH_DEF_BONUS,
    POLYMORPH_SPD_BONUS,
};

class CPolymorphUtils : public singleton<CPolymorphUtils>
{
private :
    std::unordered_map<uint32_t, uint32_t> m_mapSPDType;
    std::unordered_map<uint32_t, uint32_t> m_mapATKType;
    std::unordered_map<uint32_t, uint32_t> m_mapDEFType;

public :
    CPolymorphUtils();

    POLYMORPH_BONUS_TYPE GetBonusType(uint32_t dwVnum);

    bool PolymorphCharacter(CHARACTER *pChar, CItem *pItem, const TMobTable *pMob);
    bool UpdateBookPracticeGrade(CHARACTER *pChar, CItem *pItem);
    bool GiveBook(CHARACTER *pChar, uint32_t dwMobVnum, uint32_t dwPracticeCount, uint8_t BookLevel,
                  uint8_t LevelLimit);
    bool BookUpgrade(CHARACTER *pChar, CItem *pItem);
    bool IsOnPolymorphMapBlacklist(uint32_t mapIndex);
};

#endif /* METIN2_SERVER_GAME_POLYMORPH_H */
