#ifndef METIN2_CLIENT_MAIN_PYTHONNONPLAYER_H
#define METIN2_CLIENT_MAIN_PYTHONNONPLAYER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <unordered_map>
#include <optional>

#include <game/Constants.hpp>
#include <game/Types.hpp>

#include <game/MobTypes.hpp>

/*
 *	NPC ������ ������ Ÿ���� ���� �Ѵ�.
 */
class NpcManager : public CSingleton<NpcManager>
{
public:
    typedef std::list<TMobTable *> TMobTableList;
    typedef std::map<uint32_t, float> TSpecularConfigMap;
    typedef std::unordered_map<uint32_t, std::string> NameMap;

#ifdef INGAME_WIKI

    typedef struct SWikiInfoTable
    {
        TMobTable mobTable;

        bool isSet;
        bool isFiltered;
        std::vector<TDropInfo> dropList;
    } TWikiInfoTable;

    typedef std::map<DWORD, TWikiInfoTable> TNonPlayerDataMap;

#else
		typedef std::map<uint32_t, std::unique_ptr<TMobTable>> TNonPlayerDataMap;

#endif // INGAME_WIKI

public:
    NpcManager(void);
    virtual ~NpcManager(void);

    float GetMonsterDamageMultiply(uint32_t dwVnum);
    uint32_t GetMonsterST(uint32_t dwVnum);
    uint32_t GetMonsterDX(uint32_t dwVnum);
    uint16_t GetMobRegenCycle(uint32_t dwVnum);
    unsigned char GetMobRegenPercent(uint32_t dwVnum);
    uint32_t GetMobGoldMin(uint32_t dwVnum);
    uint32_t GetMobGoldMax(uint32_t dwVnum);
    uint32_t GetMobResist(uint32_t dwVnum, uint8_t bResistNum);
    bool HasSpecularConfig(uint32_t vnum);
    float GetSpecularValue(uint32_t vnum);
    void Clear();
    void Destroy();

    bool LoadRaceHeight(const std::string &filename);
    bool LoadHugeRace(const std::string &filename);
    bool LoadNpcList(const std::string &filename);
    bool LoadSpeculaSettingFile(const char *c_szFileName);
    bool LoadNonPlayerData(const char *c_szFileName);
    bool LoadNames(const char *filename);

    const TMobTable *GetTable(uint32_t dwVnum);
    TWikiInfoTable *GetWikiTable(DWORD dwVnum);

    std::optional<std::string> GetName(uint32_t dwVnum);
    bool GetInstanceType(uint32_t dwVnum, uint8_t *pbType);
    uint8_t GetEventType(uint32_t dwVnum);
    uint32_t GetAttElementFlag(uint32_t dwVID);
    uint32_t GetMonsterColor(uint32_t dwVnum);
    const char *GetMonsterName(uint32_t dwVnum);
    uint32_t GetScale(uint32_t dwVnum);

    std::optional<std::unordered_map<uint32_t, std::string>> GetEntriesByName(const std::string &name, int32_t limit);

    uint32_t GetMobRank(uint32_t dwVnum);

    // Function for outer
    void GetMatchableMobList(int iLevel, int iInterval, TMobTableList *pMobTableList);
    uint32_t GetMonsterExp(uint32_t dwVnum);

#ifdef INGAME_WIKI
    size_t WikiLoadClassMobs(BYTE bType, WORD fromLvl, WORD toLvl);

    std::vector<DWORD> *WikiGetLastMobs()
    {
        return &m_vecTempMob;
    }

    void WikiSetBlacklisted(DWORD vnum);

    void BuildWikiSearchList();
    DWORD GetVnumByNamePart(const char *c_pszName);

#endif

protected:
    TNonPlayerDataMap m_protoMap;
    NameMap m_nameMap;
    TSpecularConfigMap m_specularConfig;
#ifdef INGAME_WIKI
    void SortMobDataName();
    std::vector<DWORD> m_vecTempMob;
    std::vector<TMobTable *> m_vecWikiNameSort;
#endif

#ifdef __WORLD_BOSS__
public:
    bool LoadWorldBossInfo(const char *szFileName);
    const std::vector<TWorldBossInfo> &GetWorldBossInfos();
private:
    std::vector<TWorldBossInfo> m_vec_kWorldBossInfo;
#endif
};
#endif /* METIN2_CLIENT_MAIN_PYTHONNONPLAYER_H */
