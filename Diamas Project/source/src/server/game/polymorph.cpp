#include "char.h"
#include "mob_manager.h"
#include <game/AffectConstants.hpp>
#include "item.h"
#include "polymorph.h"

bool CPolymorphUtils::IsOnPolymorphMapBlacklist(uint32_t mapIndex)
{
    switch (mapIndex)
    {
    case 26: // PvP Arena
        return true;
    }

    return false;
}

CPolymorphUtils::CPolymorphUtils()
{
    m_mapSPDType.insert(std::make_pair(101, 101));
    m_mapSPDType.insert(std::make_pair(1901, 1901));
}

POLYMORPH_BONUS_TYPE CPolymorphUtils::GetBonusType(uint32_t dwVnum)
{
    std::unordered_map<uint32_t, uint32_t>::iterator iter;

    iter = m_mapSPDType.find(dwVnum);

    if (iter != m_mapSPDType.end())
        return POLYMORPH_SPD_BONUS;

    iter = m_mapATKType.find(dwVnum);

    if (iter != m_mapATKType.end())
        return POLYMORPH_ATK_BONUS;

    iter = m_mapDEFType.find(dwVnum);

    if (iter != m_mapDEFType.end())
        return POLYMORPH_DEF_BONUS;

    return POLYMORPH_NO_BONUS;
}

bool CPolymorphUtils::PolymorphCharacter(CHARACTER *pChar, CItem *pItem, const TMobTable *pMob)
{
    uint8_t bySkillLevel = pChar->GetSkillLevel(POLYMORPH_SKILL_ID);
    uint32_t dwDuration = 0;
    uint32_t dwBonusPercent = 0;
    int iPolyPercent = 0;

    switch (pChar->GetSkillMasterType(POLYMORPH_SKILL_ID))
    {
    case SKILL_NORMAL:
        dwDuration = 10;
        break;

    case SKILL_MASTER:
        dwDuration = 15;
        break;

    case SKILL_GRAND_MASTER:
        dwDuration = 20;
        break;

    case SKILL_PERFECT_MASTER:
        dwDuration = 25;
        break;

    default:
        return false;
    }

    // dwDuration *= 60;

    // 변신 확률 = 캐릭터 레벨 - 몹 레벨 + 둔갑서 레벨 + 29 + 둔갑 스킬 레벨
    iPolyPercent = pChar->GetLevel() - pMob->bLevel + pItem->GetSocket(2) + (29 + bySkillLevel);

    if (iPolyPercent <= 0)
    {
        SendI18nChatPacket(pChar, CHAT_TYPE_INFO, "둔갑에 실패 하였습니다");
        return false;
    }
    else
    {
        if (Random::get(1, 100) > iPolyPercent)
        {
            SendI18nChatPacket(pChar, CHAT_TYPE_INFO, "둔갑에 실패 하였습니다");
            return false;
        }
    }

    pChar->AddAffect(AFFECT_POLYMORPH, POINT_POLYMORPH, pMob->dwVnum, dwDuration, 0, true);

    // 변신 보너스 = 둔갑 스킬 레벨 + 둔갑서 레벨
    dwBonusPercent = bySkillLevel + pItem->GetSocket(2);

    switch (GetBonusType(pMob->dwVnum))
    {
    case POLYMORPH_ATK_BONUS:
        pChar->AddAffect(AFFECT_POLYMORPH, POINT_ATT_BONUS, dwBonusPercent, dwDuration - 1, 0, false);
        break;

    case POLYMORPH_DEF_BONUS:
        pChar->AddAffect(AFFECT_POLYMORPH, POINT_DEF_BONUS, dwBonusPercent, dwDuration - 1, 0, false);
        break;

    case POLYMORPH_SPD_BONUS:
        pChar->AddAffect(AFFECT_POLYMORPH, POINT_MOV_SPEED, dwBonusPercent, dwDuration - 1, 0, false);
        break;

    default:
    case POLYMORPH_NO_BONUS:
        break;
    }

    return true;
}

bool CPolymorphUtils::UpdateBookPracticeGrade(CHARACTER *pChar, CItem *pItem)
{
    if (pChar == nullptr || pItem == nullptr)
        return false;

    if (pItem->GetSocket(1) > 0)
        pItem->SetSocket(1, pItem->GetSocket(1) - 1);
    else
        SendI18nChatPacket(pChar, CHAT_TYPE_INFO, "둔갑서 수련을 마첬습니다. 신선에게 찾아가세요.");

    return true;
}

bool CPolymorphUtils::GiveBook(CHARACTER *pChar, uint32_t dwMobVnum, uint32_t dwPracticeCount, uint8_t BookLevel,
                               uint8_t LevelLimit)
{
    // 소켓0                소켓1       소켓2
    // 둔갑할 몬스터 번호   수련정도    둔갑서 레벨
    if (pChar == nullptr)
        return false;

    CItem *pItem = pChar->AutoGiveItem(POLYMORPH_BOOK_ID, 1);

    if (pItem == nullptr)
        return false;

    if (CMobManager::instance().Get(dwMobVnum) == nullptr)
    {
        SPDLOG_ERROR("Wrong Polymorph vnum passed: CPolymorphUtils::GiveBook(PID(%d), %d %d %d %d)",
                     pChar->GetPlayerID(), dwMobVnum, dwPracticeCount, BookLevel, LevelLimit);
        return false;
    }

    pItem->SetSocket(0, dwMobVnum);       // 둔갑할 몬스터 번호
    pItem->SetSocket(1, dwPracticeCount); // 수련해야할 횟수
    pItem->SetSocket(2, BookLevel);       // 수련레벨
    return true;
}

bool CPolymorphUtils::BookUpgrade(CHARACTER *pChar, CItem *pItem)
{
    if (pChar == nullptr || pItem == nullptr)
        return false;

    pItem->SetSocket(1, pItem->GetSocket(2) * 50);
    pItem->SetSocket(2, pItem->GetSocket(2) + 1);
    return true;
}
