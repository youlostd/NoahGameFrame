#ifndef METIN2_CLIENT_MAIN_PYTHONGUILD_H
#define METIN2_CLIENT_MAIN_PYTHONGUILD_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/BuildingTypes.hpp>
#include <game/GamePacket.hpp>

class CPythonGuild : public CSingleton<CPythonGuild>
{
    public:
    enum {
        GUILD_SKILL_MAX_NUM = 12,
        ENEMY_GUILD_SLOT_MAX_COUNT = 6,
    };

    typedef struct SGulidInfo {
        uint32_t dwGuildID;
        char szGuildName[GUILD_NAME_MAX_LEN];
        uint32_t dwMasterPID;
        uint32_t dwGuildLevel;
        uint32_t dwCurrentExperience;
        uint32_t dwCurrentMemberCount;
        uint32_t dwMaxMemberCount;
        Gold guildMoney;
        bool bHasLand;
    } TGuildInfo;

    typedef std::map<uint8_t, TGuildGrade> TGradeDataMap;

    typedef struct SGuildMemberData {
        uint32_t dwPID;

        std::string strName;
        uint8_t byGrade;
        uint8_t byJob;
        uint8_t byLevel;
        uint8_t byGeneralFlag;
        uint32_t dwOffer;
    } TGuildMemberData;

    typedef std::vector<TGuildMemberData> TGuildMemberDataVector;

    typedef struct SGuildBoardCommentData {
        uint32_t dwCommentID;
        std::string strName;
        std::string strComment;
    } TGuildBoardCommentData;

    typedef std::vector<TGuildBoardCommentData> TGuildBoardCommentDataVector;

    typedef struct SGuildSkillData {
        uint8_t bySkillPoint;
        uint8_t bySkillLevel[GUILD_SKILL_MAX_NUM];
        uint16_t wGuildPoint;
        uint16_t wMaxGuildPoint;
    } TGuildSkillData;

    typedef std::map<uint32_t, std::string> TGuildNameMap;

    public:
    CPythonGuild();
    virtual ~CPythonGuild();

    void Destroy();

    void EnableGuild();
    void SetGuildMoney(Gold dwMoney);
    void SetGuildEXP(uint8_t byLevel, uint32_t dwEXP);
    void SetGradeData(uint8_t byGradeNumber, TGuildGrade rGuildGradeData);
    void SetGradeName(uint8_t byGradeNumber, const char* c_szName);
    void SetGradeAuthority(uint8_t byGradeNumber, uint64_t byAuthority);
    void ClearComment();
    void RegisterComment(uint32_t dwCommentID, const char* c_szName,
                         const char* c_szComment);
    void RegisterMember(TGuildMemberData& rGuildMemberData);
    void ChangeGuildMemberGrade(uint32_t dwPID, uint8_t byGrade);
    void ChangeGuildMemberGeneralFlag(uint32_t dwPID, uint8_t byFlag);
    void RemoveMember(uint32_t dwPID);
#ifdef ENABLE_GUILD_STORAGE
    void RegisterGlyphCount(uint32_t dwID, unsigned char count);

#endif // ENABLE_GUILD_STORAGE
    void RegisterGuildName(uint32_t dwID, const std::string& name);

    bool IsMainPlayer(uint32_t dwPID);
    bool IsGuildLeader(const char* szName);
    bool IsGuildEnable();
#ifdef ENABLE_GUILD_STORAGE
    unsigned char GetGlyphCount(uint32_t dwID);

#endif // ENABLE_GUILD_STORAGE
    TGuildInfo& GetGuildInfoRef();
    bool GetGradeDataPtr(uint32_t dwGradeNumber, TGuildGrade** ppData);
    const TGuildBoardCommentDataVector& GetGuildBoardCommentVector();
    uint32_t GetMemberCount();
    bool GetMemberDataPtr(uint32_t dwIndex, TGuildMemberData** ppData);
    bool GetMemberDataPtrByPID(uint32_t dwPID, TGuildMemberData** ppData);
    bool GetMemberDataPtrByName(const char* c_szName,
                                TGuildMemberData** ppData);
    uint32_t GetGuildMemberLevelSummary();
    uint32_t GetGuildMemberLevelAverage();
    uint32_t GetGuildExperienceSummary();
    TGuildSkillData& GetGuildSkillDataRef();
    std::optional<std::string> GetGuildName(uint32_t dwID);
    uint32_t GetGuildID();
    bool HasGuildLand();

    void StartGuildWar(uint32_t dwEnemyGuildID);
    void EndGuildWar(uint32_t dwEnemyGuildID);
    uint32_t GetEnemyGuildID(uint32_t dwIndex);
    bool IsDoingGuildWar();

    void RemoveBuilding(DWORD vid);
    void AddBuilding(uint32_t vid, GuildObject obj);
    GuildObject GetGuildObject(uint32_t index);
    GuildObject GetGuildObjectByVid(uint32_t vid);
    uint32_t GetGuildObjectCount();

    bool LoadObjectProto(const std::string& filename);
    bool LoadObjectNames(const std::string& filename);

    const std::vector<ObjectProto>& GetObjectProto() const
    {
        return m_objectProto;
    }

    const std::vector<GuildObject>& GetGuildObjects() const
    {
        return m_guildObjects;
    }

    bool HasObject(uint32_t vnum)
    {
        return std::find_if(m_objectProto.begin(), m_objectProto.end(),
                            [&vnum](const ObjectProto& v) {
                                return v.vnum == vnum;
                            }) != m_objectProto.end();
    }

    py::bytes GetObjectName(uint32_t vnum) const;

    protected:
    void __CalculateLevelAverage();
    void __SortMember();
    bool __IsGradeData(uint8_t byGradeNumber);

    void __Initialize();

    protected:
    TGuildInfo m_GuildInfo{};
    TGradeDataMap m_GradeDataMap{};
    TGuildMemberDataVector m_GuildMemberDataVector{};
    TGuildBoardCommentDataVector m_GuildBoardCommentVector{};
    TGuildSkillData m_GuildSkillData{};
    TGuildNameMap m_GuildNameMap{};
    uint32_t m_adwEnemyGuildID[ENEMY_GUILD_SLOT_MAX_COUNT]{};

    std::vector<ObjectProto> m_objectProto;
    std::unordered_map<uint32_t, std::string> m_objectNames;
    std::vector<GuildObject> m_guildObjects;

#ifdef ENABLE_GUILD_STORAGE
    typedef std::map<uint32_t, uint8_t> TGuildGlyphCount;
    TGuildGlyphCount m_guildGlyphCount;

#endif // ENABLE_GUILD_STORAGE

    uint32_t m_dwMemberLevelSummary;
    uint32_t m_dwMemberLevelAverage;
    uint32_t m_dwMemberExperienceSummary;

    bool m_bGuildEnable;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONGUILD_H */
