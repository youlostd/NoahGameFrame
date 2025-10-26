#ifndef METIN2_SERVER_GAME_PRIV_MANAGER_H
#define METIN2_SERVER_GAME_PRIV_MANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/Singleton.hpp>

class CHARACTER;

/**
 * @version 05/06/08	Bang2ni - Guild privilege 관련 함수 지속 시간 추가
 * 			          RequestGiveGuildPriv, GiveGuildPriv 함수 프로토타잎 수정
 * 			          m_aPrivGuild 타잎 수정
 * 			          구조체 SPrivGuildData, 멤버 함수 GetPrivByGuildEx 추가
 */
class CPrivManager : public singleton<CPrivManager>
{
public:
    CPrivManager();

    void RequestGiveGuildPriv(uint32_t guild_id, uint8_t type, int value, uint32_t dur_time_sec);
    void RequestGiveEmpirePriv(uint8_t empire, uint8_t type, int value, uint32_t dur_time_sec);
    void RequestGiveCharacterPriv(uint32_t pid, uint8_t type, int value);

    void GiveGuildPriv(uint32_t guild_id, uint8_t type, int value, uint8_t bLog, uint32_t end_time_sec);
    void GiveEmpirePriv(uint8_t empire, uint8_t type, int value, uint8_t bLog, uint32_t end_time_sec);
    void GiveCharacterPriv(uint32_t pid, uint8_t type, int value, uint8_t bLog);

    void RemoveGuildPriv(uint32_t guild_id, uint8_t type);
    void RemoveEmpirePriv(uint8_t empire, uint8_t type);
    void RemoveCharacterPriv(uint32_t pid, uint8_t type);

    int GetPriv(CHARACTER *ch, uint8_t type);
    int GetPrivByEmpire(uint8_t bEmpire, uint8_t type);
int GetPrivEndByEmpire(uint8_t bEmpire, uint8_t type);
int GetPrivByGuild(uint32_t guild_id, uint8_t type);
    int GetPrivByCharacter(uint32_t pid, uint8_t type);

public:
    struct SPrivEmpireData
    {
        int m_value;
        time_t m_end_time_sec;
    };

    SPrivEmpireData *GetPrivByEmpireEx(uint8_t bEmpire, uint8_t type);

    /// 길드 보너스 데이터
    struct SPrivGuildData
    {
        int value;           ///< 보너스 수치
        time_t end_time_sec; ///< 지속 시간
    };

    /// 길드 보너스 데이터를 얻어온다.
    /**
     * @param [in]	dwGuildID 얻어올 길드의 ID
     * @param [in]	byType 보너스 타잎
     * @return	대상 길드의 길드 보너스 데이터의 포인터, 해당하는 보너스 타잎과 길드의 ID 에 대해 보너스 데이터가 없을 경우 NULL
     */
    const SPrivGuildData *GetPrivByGuildEx(uint32_t dwGuildID, uint8_t byType) const;

private:
    SPrivEmpireData m_aakPrivEmpireData[MAX_PRIV_NUM][EMPIRE_MAX_NUM];
    std::map<uint32_t, SPrivGuildData> m_aPrivGuild[MAX_PRIV_NUM];
    std::map<uint32_t, int> m_aPrivChar[MAX_PRIV_NUM];
};

#endif /* METIN2_SERVER_GAME_PRIV_MANAGER_H */
