#include "PythonGuild.h"
#include "../gamelib/RaceManager.h"
#include "MarkManager.h"
#include "PythonPlayer.h"
#include "StdAfx.h"
#include "game/BuildingTypes.hpp"
#include <pak/ClientDb.hpp>
#include <pybind11/stl.h>

#include "pak/Util.hpp"

std::map<uint32_t, uint32_t> g_GuildSkillSlotToIndexMap;

void CPythonGuild::EnableGuild()
{
    m_bGuildEnable = TRUE;
}

void CPythonGuild::SetGuildMoney(Gold dwMoney)
{
    m_GuildInfo.guildMoney = dwMoney;
}

void CPythonGuild::SetGuildEXP(uint8_t byLevel, uint32_t dwEXP)
{
    m_GuildInfo.dwGuildLevel = byLevel;
    m_GuildInfo.dwCurrentExperience = dwEXP;
}

void CPythonGuild::SetGradeData(uint8_t byGradeNumber,
                                TGuildGrade rGuildGradeData)
{
    m_GradeDataMap[byGradeNumber] = rGuildGradeData;
}

void CPythonGuild::SetGradeName(uint8_t byGradeNumber, const char* c_szName)
{
    if (!__IsGradeData(byGradeNumber))
        return;

    TGuildGrade& rGradeData = m_GradeDataMap.find(byGradeNumber)->second;
    rGradeData.grade_name = c_szName;
}

void CPythonGuild::SetGradeAuthority(uint8_t byGradeNumber,
                                     uint64_t byAuthority)
{
    if (!__IsGradeData(byGradeNumber))
        return;

    TGuildGrade& rGradeData = m_GradeDataMap.find(byGradeNumber)->second;
    rGradeData.auth_flag = byAuthority;
}

void CPythonGuild::ClearComment()
{
    m_GuildBoardCommentVector.clear();
}

void CPythonGuild::RegisterComment(uint32_t dwCommentID, const char* c_szName,
                                   const char* c_szComment)
{
    if (0 == strlen(c_szComment))
        return;

    TGuildBoardCommentData CommentData;
    CommentData.dwCommentID = dwCommentID;
    CommentData.strName = c_szName;
    CommentData.strComment = c_szComment;

    m_GuildBoardCommentVector.push_back(CommentData);
}

void CPythonGuild::RegisterMember(TGuildMemberData& rGuildMemberData)
{
    TGuildMemberData* pGuildMemberData;
    if (GetMemberDataPtrByPID(rGuildMemberData.dwPID, &pGuildMemberData)) {
        pGuildMemberData->byGeneralFlag = rGuildMemberData.byGeneralFlag;
        pGuildMemberData->byGrade = rGuildMemberData.byGrade;
        pGuildMemberData->byLevel = rGuildMemberData.byLevel;
        pGuildMemberData->dwOffer = rGuildMemberData.dwOffer;
    } else {
        m_GuildMemberDataVector.push_back(rGuildMemberData);
    }

    __CalculateLevelAverage();
    __SortMember();
}

struct CPythonGuild_FFindGuildMemberByPID {
    CPythonGuild_FFindGuildMemberByPID(uint32_t dwSearchingPID_)
        : dwSearchingPID(dwSearchingPID_)
    {
    }

    int operator()(CPythonGuild::TGuildMemberData& rGuildMemberData)
    {
        return rGuildMemberData.dwPID == dwSearchingPID;
    }

    uint32_t dwSearchingPID;
};

struct CPythonGuild_FFindGuildMemberByName {
    CPythonGuild_FFindGuildMemberByName(const char* c_szSearchingName)
        : strSearchingName(c_szSearchingName)
    {
    }

    int operator()(CPythonGuild::TGuildMemberData& rGuildMemberData)
    {
        return 0 == strSearchingName.compare(rGuildMemberData.strName);
    }

    std::string strSearchingName;
};

void CPythonGuild::ChangeGuildMemberGrade(uint32_t dwPID, uint8_t byGrade)
{
    TGuildMemberData* pGuildMemberData;
    if (!GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
        return;

    pGuildMemberData->byGrade = byGrade;
}

void CPythonGuild::ChangeGuildMemberGeneralFlag(uint32_t dwPID, uint8_t byFlag)
{
    TGuildMemberData* pGuildMemberData;
    if (!GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
        return;

    pGuildMemberData->byGeneralFlag = byFlag;
}

void CPythonGuild::RemoveMember(uint32_t dwPID)
{
    auto itor = std::find_if(m_GuildMemberDataVector.begin(),
                             m_GuildMemberDataVector.end(),
                             CPythonGuild_FFindGuildMemberByPID(dwPID));

    if (m_GuildMemberDataVector.end() == itor)
        return;

    m_GuildMemberDataVector.erase(itor);
}
#ifdef ENABLE_GUILD_STORAGE
void CPythonGuild::RegisterGlyphCount(uint32_t dwID, uint8_t count)
{
    m_guildGlyphCount.insert(std::make_pair(dwID, count));
}
#endif

void CPythonGuild::RegisterGuildName(uint32_t dwID, const std::string& name)
{
    m_GuildNameMap.insert_or_assign(dwID, name);
}

bool CPythonGuild::IsMainPlayer(uint32_t dwPID)
{
    TGuildMemberData* pGuildMemberData;
    if (!GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
        return FALSE;

    CPythonPlayer& rPlayer = CPythonPlayer::Instance();
    return 0 == pGuildMemberData->strName.compare(rPlayer.GetName());
}

bool CPythonGuild::IsGuildLeader(const char* szName)
{
    TGuildMemberData* pGuildMemberData;
    if (!GetMemberDataPtrByName(szName, &pGuildMemberData))
        return FALSE;

    return 1 == pGuildMemberData->byGeneralFlag;
}

bool CPythonGuild::IsGuildEnable()
{
    return m_bGuildEnable;
}

#ifdef ENABLE_GUILD_STORAGE
unsigned char CPythonGuild::GetGlyphCount(uint32_t dwID)
{
    if (m_guildGlyphCount.end() == m_guildGlyphCount.find(dwID))
        return 0;

    return m_guildGlyphCount[dwID];
}
#endif

CPythonGuild::TGuildInfo& CPythonGuild::GetGuildInfoRef()
{
    return m_GuildInfo;
}

bool CPythonGuild::GetGradeDataPtr(uint32_t dwGradeNumber, TGuildGrade** ppData)
{
    const auto itor = m_GradeDataMap.find(dwGradeNumber);
    if (m_GradeDataMap.end() == itor)
        return FALSE;

    *ppData = &(itor->second);

    return TRUE;
}

const CPythonGuild::TGuildBoardCommentDataVector&
CPythonGuild::GetGuildBoardCommentVector()
{
    return m_GuildBoardCommentVector;
}

uint32_t CPythonGuild::GetMemberCount()
{
    return m_GuildMemberDataVector.size();
}

bool CPythonGuild::GetMemberDataPtr(uint32_t dwIndex, TGuildMemberData** ppData)
{
    if (dwIndex >= m_GuildMemberDataVector.size())
        return FALSE;

    *ppData = &m_GuildMemberDataVector[dwIndex];

    return TRUE;
}

bool CPythonGuild::GetMemberDataPtrByPID(uint32_t dwPID,
                                         TGuildMemberData** ppData)
{
    auto itor = std::find_if(m_GuildMemberDataVector.begin(),
                             m_GuildMemberDataVector.end(),
                             CPythonGuild_FFindGuildMemberByPID(dwPID));

    if (m_GuildMemberDataVector.end() == itor)
        return FALSE;

    *ppData = &(*itor);
    return TRUE;
}

bool CPythonGuild::GetMemberDataPtrByName(const char* c_szName,
                                          TGuildMemberData** ppData)
{
    auto itor = std::find_if(
        m_GuildMemberDataVector.begin(), m_GuildMemberDataVector.end(),
        [c_szName](CPythonGuild::TGuildMemberData& rGuildMemberData) {
            return !strcmpi(c_szName, rGuildMemberData.strName.c_str());
        });

    if (m_GuildMemberDataVector.end() == itor)
        return false;

    *ppData = &(*itor);
    return true;
}

uint32_t CPythonGuild::GetGuildMemberLevelSummary()
{
    return m_dwMemberLevelSummary;
}

uint32_t CPythonGuild::GetGuildMemberLevelAverage()
{
    return m_dwMemberLevelAverage;
}

uint32_t CPythonGuild::GetGuildExperienceSummary()
{
    return m_dwMemberExperienceSummary;
}

CPythonGuild::TGuildSkillData& CPythonGuild::GetGuildSkillDataRef()
{
    return m_GuildSkillData;
}

std::optional<std::string> CPythonGuild::GetGuildName(uint32_t dwID)
{
    if (m_GuildNameMap.end() == m_GuildNameMap.find(dwID))
        return std::nullopt;

    return m_GuildNameMap[dwID];
}

uint32_t CPythonGuild::GetGuildID()
{
    return m_GuildInfo.dwGuildID;
}

bool CPythonGuild::HasGuildLand()
{
    return m_GuildInfo.bHasLand;
}

void CPythonGuild::StartGuildWar(uint32_t dwEnemyGuildID)
{
    int i;

    for (i = 0; i < ENEMY_GUILD_SLOT_MAX_COUNT; ++i)
        if (dwEnemyGuildID == m_adwEnemyGuildID[i])
            return;

    for (i = 0; i < ENEMY_GUILD_SLOT_MAX_COUNT; ++i)
        if (0 == m_adwEnemyGuildID[i]) {
            m_adwEnemyGuildID[i] = dwEnemyGuildID;
            break;
        }
}

void CPythonGuild::EndGuildWar(uint32_t dwEnemyGuildID)
{
    for (int i = 0; i < ENEMY_GUILD_SLOT_MAX_COUNT; ++i)
        if (dwEnemyGuildID == m_adwEnemyGuildID[i])
            m_adwEnemyGuildID[i] = 0;
}

uint32_t CPythonGuild::GetEnemyGuildID(uint32_t dwIndex)
{
    if (dwIndex >= ENEMY_GUILD_SLOT_MAX_COUNT)
        return 0;

    return m_adwEnemyGuildID[dwIndex];
}

bool CPythonGuild::IsDoingGuildWar()
{
    for (int i = 0; i < ENEMY_GUILD_SLOT_MAX_COUNT; ++i)
        if (0 != m_adwEnemyGuildID[i]) {
            return TRUE;
        }

    return FALSE;
}

namespace gtl
{
template <typename ContainerT, typename PredicateT>
void erase_if(ContainerT& items, const PredicateT& predicate)
{
    for (auto it = items.begin(); it != items.end();) {
        if (predicate(*it))
            it = items.erase(it);
        else
            ++it;
    }
};
} // namespace gtl

void CPythonGuild::RemoveBuilding(DWORD vid)
{
    gtl::erase_if(m_guildObjects,
                  [vid](const GuildObject& v) { return v.vid == vid; });
}

void CPythonGuild::AddBuilding(uint32_t vid, GuildObject obj)
{
    auto it =
        std::find_if(m_guildObjects.begin(), m_guildObjects.end(),
                     [vid](const GuildObject& v) { return v.vid == vid; });

    if (it != m_guildObjects.end())
        return;

    m_guildObjects.emplace_back(obj);
}

GuildObject CPythonGuild::GetGuildObject(uint32_t index)
{
    if (index >= m_guildObjects.size())
        return {};

    return m_guildObjects[index];
}

GuildObject CPythonGuild::GetGuildObjectByVid(uint32_t vid)
{
    auto it =
        std::find_if(m_guildObjects.begin(), m_guildObjects.end(),
                     [vid](const GuildObject& v) { return v.vid == vid; });

    if (it == m_guildObjects.end())
        return {};

    return *it;
}

uint32_t CPythonGuild::GetGuildObjectCount()
{
    return m_guildObjects.size();
}

bool CPythonGuild::LoadObjectProto(const std::string& filename)
{
    std::vector<ObjectProto> entries;
    if (!LoadClientDb(GetVfs(), filename, entries))
        return false;

    for (const auto& entry : entries) {
        m_objectProto.emplace_back(entry);
        CRaceManager::Instance().RegisterRace(entry.vnum, entry.msmFilename);
    }

    return true;
}

bool CPythonGuild::LoadObjectNames(const std::string& filename)
{
    auto str = LoadFileToString(GetVfs(), filename.c_str());
    if (!str) {
        SPDLOG_ERROR("Failed to read {0}", filename);
        return false;
    }

    std::vector<std::string> lines;
    storm::Tokenize(str.value(), "\r\n", lines);

    std::vector<storm::String> args;

    int i = 0;
    for (const auto& line : lines) {
        args.clear();
        storm::Tokenize(line, "\t", args);

        if (args.empty())
            continue;

        if (args.size() != 2) {
            SPDLOG_ERROR("{0}:{1}: expected 2 tokens, got {2}", filename, i,
                         args.size());
            continue;
        }

        uint32_t vnum;
        if (!storm::ParseNumber(args[0], vnum)) {
            SPDLOG_ERROR("{0}:{1}: expected vnum number, got {2}", filename, i,
                         args[0]);
            continue;
        }

        m_objectNames[vnum] = args[1];
    }

    return true;
}

py::bytes CPythonGuild::GetObjectName(uint32_t vnum) const
{
    const auto it = m_objectNames.find(vnum);
    if (it != m_objectNames.end())
        return py::bytes(it->second);
    return py::bytes("");
}

void CPythonGuild::__CalculateLevelAverage()
{
    m_dwMemberLevelSummary = 0;
    m_dwMemberLevelAverage = 0;
    m_dwMemberExperienceSummary = 0;

    if (m_GuildMemberDataVector.empty())
        return;

    TGuildMemberDataVector::iterator itor;

    // Sum Level & Experience
    itor = m_GuildMemberDataVector.begin();
    for (; itor != m_GuildMemberDataVector.end(); ++itor) {
        TGuildMemberData& rGuildMemberData = *itor;
        m_dwMemberLevelSummary += rGuildMemberData.byLevel;
        m_dwMemberExperienceSummary += rGuildMemberData.dwOffer;
    }

    assert(!m_GuildMemberDataVector.empty());
    m_dwMemberLevelAverage =
        m_dwMemberLevelSummary / m_GuildMemberDataVector.size();
}

struct CPythonGuild_SLessMemberGrade {
    bool operator()(CPythonGuild::TGuildMemberData& rleft,
                    CPythonGuild::TGuildMemberData& rright)
    {
        if (rleft.byGrade < rright.byGrade)
            return true;

        return false;
    }
};

void CPythonGuild::__SortMember()
{
    std::sort(m_GuildMemberDataVector.begin(), m_GuildMemberDataVector.end(),
              CPythonGuild_SLessMemberGrade());
}

bool CPythonGuild::__IsGradeData(uint8_t byGradeNumber)
{
    return m_GradeDataMap.end() != m_GradeDataMap.find(byGradeNumber);
}

void CPythonGuild::__Initialize()
{
    ZeroMemory(&m_GuildInfo, sizeof(m_GuildInfo));
    ZeroMemory(&m_GuildSkillData, sizeof(m_GuildSkillData));
    ZeroMemory(&m_adwEnemyGuildID,
               ENEMY_GUILD_SLOT_MAX_COUNT * sizeof(uint32_t));
    m_GradeDataMap.clear();
    m_GuildMemberDataVector.clear();
    m_dwMemberLevelSummary = 0;
    m_dwMemberLevelAverage = 0;
    m_bGuildEnable = FALSE;
    m_GuildNameMap.clear();
#ifdef ENABLE_GUILD_STORAGE
    m_guildGlyphCount.clear();
#endif
    m_guildObjects.clear();
}

void CPythonGuild::Destroy()
{
    __Initialize();
}

CPythonGuild::CPythonGuild()
{
    __Initialize();
}

CPythonGuild::~CPythonGuild() {}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject* guildIsGuildEnable(PyObject* poSelf, PyObject* poArgs)
{
    return Py_BuildValue("i", CPythonGuild::Instance().IsGuildEnable());
}

PyObject* guildGetGuildID(PyObject* poSelf, PyObject* poArgs)
{
    return Py_BuildValue("i", CPythonGuild::Instance().GetGuildID());
}

PyObject* guildHasGuildLand(PyObject* poSelf, PyObject* poArgs)
{
    return Py_BuildValue("i", CPythonGuild::Instance().HasGuildLand());
}

PyObject* guildGetGuildName(PyObject* poSelf, PyObject* poArgs)
{
    int iGuildID;
    if (!PyTuple_GetInteger(poArgs, 0, &iGuildID)) {
        CPythonGuild::TGuildInfo& rGuildInfo =
            CPythonGuild::Instance().GetGuildInfoRef();
        return Py_BuildValue("s", rGuildInfo.szGuildName);
    }

    std::string strGuildName =
        CPythonGuild::Instance().GetGuildName(iGuildID).value_or("Noname");

    return Py_BuildValue("s", strGuildName.c_str());
}

PyObject* guildGetGuildMasterName(PyObject* poSelf, PyObject* poArgs)
{
    CPythonGuild::TGuildInfo& rGuildInfo =
        CPythonGuild::Instance().GetGuildInfoRef();

    CPythonGuild::TGuildMemberData* pData;
    if (!CPythonGuild::Instance().GetMemberDataPtrByPID(rGuildInfo.dwMasterPID,
                                                        &pData))
        return Py_BuildValue("s", "Noname");

    return Py_BuildValue("s", pData->strName.c_str());
}

PyObject* guildGetEnemyGuildName(PyObject* poSelf, PyObject* poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BuildValue("s", "");

    uint32_t dwEnemyGuildID = CPythonGuild::Instance().GetEnemyGuildID(iIndex);

    std::string strEnemyGuildName =
        CPythonGuild::Instance().GetGuildName(dwEnemyGuildID).value_or("");

    return Py_BuildValue("s", strEnemyGuildName.c_str());
}

PyObject* guildGetGuildMoney(PyObject* poSelf, PyObject* poArgs)
{
    CPythonGuild::TGuildInfo& rGuildInfo =
        CPythonGuild::Instance().GetGuildInfoRef();
    return Py_BuildValue("L", rGuildInfo.guildMoney);
}

PyObject* guildGetGuildBoardCommentCount(PyObject* poSelf, PyObject* poArgs)
{
    const CPythonGuild::TGuildBoardCommentDataVector& rCommentVector =
        CPythonGuild::Instance().GetGuildBoardCommentVector();
    return Py_BuildValue("i", rCommentVector.size());
}

PyObject* guildGetGuildBoardCommentData(PyObject* poSelf, PyObject* poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BuildException();

    const CPythonGuild::TGuildBoardCommentDataVector& c_rCommentVector =
        CPythonGuild::Instance().GetGuildBoardCommentVector();
    if (uint32_t(iIndex) >= c_rCommentVector.size())
        return Py_BuildValue("iss", 0, "Noname", "Noname");

    const CPythonGuild::TGuildBoardCommentData& c_rData =
        c_rCommentVector[iIndex];

    return Py_BuildValue("iss", c_rData.dwCommentID, c_rData.strName.c_str(),
                         c_rData.strComment.c_str());
}

PyObject* guildGetGuildLevel(PyObject* poSelf, PyObject* poArgs)
{
    CPythonGuild::TGuildInfo& rGuildInfo =
        CPythonGuild::Instance().GetGuildInfoRef();
    return Py_BuildValue("i", rGuildInfo.dwGuildLevel);
}

PyObject* guildGetGuildExperience(PyObject* poSelf, PyObject* poArgs)
{
    CPythonGuild::TGuildInfo& rGuildInfo =
        CPythonGuild::Instance().GetGuildInfoRef();

    int GULID_MAX_LEVEL = 22;
    if (rGuildInfo.dwGuildLevel >= GULID_MAX_LEVEL)
        return Py_BuildValue("ii", 0, 0);

    unsigned lastExp = LocaleService_GetNeededGuildExp(rGuildInfo.dwGuildLevel);

    return Py_BuildValue("ii", rGuildInfo.dwCurrentExperience,
                         lastExp - rGuildInfo.dwCurrentExperience);
}

PyObject* guildGetGuildMemberCount(PyObject* poSelf, PyObject* poArgs)
{
    CPythonGuild::TGuildInfo& rGuildInfo =
        CPythonGuild::Instance().GetGuildInfoRef();
    return Py_BuildValue("ii", rGuildInfo.dwCurrentMemberCount,
                         rGuildInfo.dwMaxMemberCount);
}

PyObject* guildGetGuildMemberLevelSummary(PyObject* poSelf, PyObject* poArgs)
{
    return Py_BuildValue("i",
                         CPythonGuild::Instance().GetGuildMemberLevelSummary());
}

PyObject* guildGetGuildMemberLevelAverage(PyObject* poSelf, PyObject* poArgs)
{
    return Py_BuildValue("i",
                         CPythonGuild::Instance().GetGuildMemberLevelAverage());
}

PyObject* guildGetGuildExperienceSummary(PyObject* poSelf, PyObject* poArgs)
{
    return Py_BuildValue("i",
                         CPythonGuild::Instance().GetGuildExperienceSummary());
}

PyObject* guildGetGuildSkillPoint(PyObject* poSelf, PyObject* poArgs)
{
    const CPythonGuild::TGuildSkillData& c_rSkillData =
        CPythonGuild::Instance().GetGuildSkillDataRef();
    return Py_BuildValue("i", c_rSkillData.bySkillPoint);
}

PyObject* guildGetDragonPowerPoint(PyObject* poSelf, PyObject* poArgs)
{
    const CPythonGuild::TGuildSkillData& c_rSkillData =
        CPythonGuild::Instance().GetGuildSkillDataRef();
    return Py_BuildValue("ii", c_rSkillData.wGuildPoint,
                         c_rSkillData.wMaxGuildPoint);
}

PyObject* guildGetGuildSkillLevel(PyObject* poSelf, PyObject* poArgs)
{
    assert(FALSE && !"guildGetGuildSkillLevel - »ç¿ëÇÏÁö ¾Ê´Â ÇÔ¼öÀÔ´Ï´Ù.");

    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
        return Py_BuildException();

    const CPythonGuild::TGuildSkillData& c_rSkillData =
        CPythonGuild::Instance().GetGuildSkillDataRef();
    return Py_BuildValue("i", c_rSkillData.bySkillLevel[iSkillIndex]);
}

PyObject* guildGetSkillLevel(PyObject* poSelf, PyObject* poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        return Py_BuildException();

    std::map<uint32_t, uint32_t>::iterator itor =
        g_GuildSkillSlotToIndexMap.find(iSlotIndex);

    if (g_GuildSkillSlotToIndexMap.end() == itor)
        return Py_BuildValue("i", 0);

    uint32_t dwSkillIndex = itor->second;
    assert(dwSkillIndex < CPythonGuild::GUILD_SKILL_MAX_NUM);

    const CPythonGuild::TGuildSkillData& c_rSkillData =
        CPythonGuild::Instance().GetGuildSkillDataRef();
    return Py_BuildValue("i", c_rSkillData.bySkillLevel[dwSkillIndex]);
}

PyObject* guildGetSkillMaxLevelNew(PyObject* poSelf, PyObject* poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        return Py_BuildException();

    std::map<uint32_t, uint32_t>::iterator itor =
        g_GuildSkillSlotToIndexMap.find(iSlotIndex);

    if (g_GuildSkillSlotToIndexMap.end() == itor)
        return Py_BuildValue("i", 0);

    uint32_t dwSkillIndex = itor->second;
    assert(dwSkillIndex < CPythonGuild::GUILD_SKILL_MAX_NUM);

    const CPythonGuild::TGuildSkillData& c_rSkillData =
        CPythonGuild::Instance().GetGuildSkillDataRef();
    return Py_BuildValue("i", c_rSkillData.bySkillLevel[dwSkillIndex]);
}

PyObject* guildSetSkillIndex(PyObject* poSelf, PyObject* poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        return Py_BuildException();
    int iSkillIndex;
    if (!PyTuple_GetInteger(poArgs, 1, &iSkillIndex))
        return Py_BuildException();

    g_GuildSkillSlotToIndexMap.insert(std::make_pair(iSlotIndex, iSkillIndex));

    Py_RETURN_NONE;
}

PyObject* guildGetSkillIndex(PyObject* poSelf, PyObject* poArgs)
{
    int iSlotIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
        return Py_BuildException();

    std::map<uint32_t, uint32_t>::iterator itor =
        g_GuildSkillSlotToIndexMap.find(iSlotIndex);

    if (g_GuildSkillSlotToIndexMap.end() == itor)
        return Py_BuildValue("i", 0);

    uint32_t dwSkillIndex = itor->second;
    return Py_BuildValue("i", dwSkillIndex);
}

PyObject* guildGetGradeData(PyObject* poSelf, PyObject* poArgs)
{
    int iGradeNumber;
    if (!PyTuple_GetInteger(poArgs, 0, &iGradeNumber))
        return Py_BuildException();

    TGuildGrade* pData;
    if (!CPythonGuild::Instance().GetGradeDataPtr(iGradeNumber, &pData))
        return Py_BuildValue("si", "?", 0);

    return Py_BuildValue("sK", pData->grade_name.c_str(), pData->auth_flag);
}

PyObject* guildGetGradeName(PyObject* poSelf, PyObject* poArgs)
{
    int iGradeNumber;
    if (!PyTuple_GetInteger(poArgs, 0, &iGradeNumber))
        return Py_BuildException();

    TGuildGrade* pData;
    if (!CPythonGuild::Instance().GetGradeDataPtr(iGradeNumber, &pData))
        return Py_BuildValue("s", "?");

    return Py_BuildValue("s", pData->grade_name.c_str());
}

PyObject* guildGetMemberCount(PyObject* poSelf, PyObject* poArgs)
{
    return Py_BuildValue("i", CPythonGuild::Instance().GetMemberCount());
}

PyObject* guildGetMemberData(PyObject* poSelf, PyObject* poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BuildException();

    CPythonGuild::TGuildMemberData* pData;
    if (!CPythonGuild::Instance().GetMemberDataPtr(iIndex, &pData))
        return Py_BuildValue("isiiiii", -1, "", 0, 0, 0, 0, 0);

    return Py_BuildValue("isiiiii", pData->dwPID, pData->strName.c_str(),
                         pData->byGrade, pData->byJob, pData->byLevel,
                         pData->dwOffer, pData->byGeneralFlag);
}

PyObject* guildMemberIndexToPID(PyObject* poSelf, PyObject* poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BuildException();

    CPythonGuild::TGuildMemberData* pData;
    if (!CPythonGuild::Instance().GetMemberDataPtr(iIndex, &pData))
        return Py_BuildValue("i", -1);

    return Py_BuildValue("i", pData->dwPID);
}

PyObject* guildIsMember(PyObject* poSelf, PyObject* poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BuildException();

    CPythonGuild::TGuildMemberData* pData;
    if (CPythonGuild::Instance().GetMemberDataPtr(iIndex, &pData))
        return Py_BuildValue("i", TRUE);

    return Py_BuildValue("i", FALSE);
}

PyObject* guildIsMemberByName(PyObject* poSelf, PyObject* poArgs)
{
    std::string szName;
    if (!PyTuple_GetString(poArgs, 0, szName))
        return Py_BuildException();

    CPythonGuild::TGuildMemberData* pData;
    if (CPythonGuild::Instance().GetMemberDataPtrByName(szName.c_str(), &pData))
        return Py_BuildValue("i", TRUE);

    return Py_BuildValue("i", FALSE);
}

PyObject* guildMainPlayerHasAuthority(PyObject* poSelf, PyObject* poArgs)
{
    uint64_t iAuthority;
    if (!PyTuple_GetUnsignedLongLong(poArgs, 0, &iAuthority))
        return Py_BuildException();

    CPythonPlayer& rPlayer = CPythonPlayer::Instance();
    const char* c_szMainPlayerName = rPlayer.GetName();

    CPythonGuild::TGuildMemberData* pMemberData;
    if (!CPythonGuild::Instance().GetMemberDataPtrByName(c_szMainPlayerName,
                                                         &pMemberData))
        return Py_BuildValue("i", FALSE);

    TGuildGrade* pGradeData;
    if (!CPythonGuild::Instance().GetGradeDataPtr(pMemberData->byGrade,
                                                  &pGradeData))
        return Py_BuildValue("i", FALSE);

    return Py_BuildValue("i", IS_SET(pGradeData->auth_flag, iAuthority));
}

PyObject* guildDestroy(PyObject* poSelf, PyObject* poArgs)
{
    //
    // g_GuildSkillSlotToIndexMap.clear();
    Py_RETURN_NONE;
}

PyObject* guildGuildIDToMarkID(PyObject* poSelf, PyObject* poArgs)
{
    int guild_id;
    if (!PyTuple_GetInteger(poArgs, 0, &guild_id))
        return Py_BuildException();

    return Py_BuildValue("i",
                         CGuildMarkManager::Instance().GetMarkID(guild_id));
}

PyObject* guildGetMarkImageFilenameByMarkID(PyObject* poSelf, PyObject* poArgs)
{
    int markID;

    if (!PyTuple_GetInteger(poArgs, 0, &markID))
        return Py_BuildException();

    std::string imagePath;
    CGuildMarkManager::Instance().GetMarkImageFilename(
        markID / CGuildMarkImage::MARK_TOTAL_COUNT, imagePath);
    return Py_BuildValue("s", imagePath.c_str());
}

PyObject* guildGetMarkIndexByMarkID(PyObject* poSelf, PyObject* poArgs)
{
    int markID;

    if (!PyTuple_GetInteger(poArgs, 0, &markID))
        return Py_BuildException();

    return Py_BuildValue("i", markID % CGuildMarkImage::MARK_TOTAL_COUNT);
}

void init_guild(py::module& m)
{
    py::module guild = m.def_submodule("guild", "Provides guild support");

    py::class_<ObjectMaterial>(guild, "ObjectMaterial")
        .def(py::init<>())
        .def_readwrite("itemVnum", &ObjectMaterial::itemVnum)
        .def_readwrite("count", &ObjectMaterial::count);

    py::class_<ObjectProto>(guild, "ObjectProto")
        .def(py::init<>())
        .def_readwrite("vnum", &ObjectProto::vnum)
        .def_readwrite("type", &ObjectProto::type)
        .def_readwrite("name", &ObjectProto::name)
        .def_readwrite("price", &ObjectProto::price)
        .def_property_readonly(
            "materials",
            [](const ObjectProto& c)
                -> const std::array<ObjectMaterial, OBJECT_MATERIAL_MAX_NUM>& {
                return reinterpret_cast<
                    const std::array<ObjectMaterial, OBJECT_MATERIAL_MAX_NUM>&>(
                    c.materials);
            })
        .def_readwrite("life", &ObjectProto::life)
        .def_property_readonly(
            "region",
            [](const ObjectProto& c) -> const std::array<int32_t, 4>& {
                return reinterpret_cast<const std::array<int32_t, 4>&>(
                    c.region);
            })
        .def_readwrite("npcVnum", &ObjectProto::npcVnum)
        .def_readwrite("groupVnum", &ObjectProto::groupVnum)
        .def_readwrite("dependOnGroupVnum", &ObjectProto::dependOnGroupVnum)
        .def_readwrite("msmFilename", &ObjectProto::msmFilename)
        .def_property_readonly(
            "rotationLimits",
            [](const ObjectProto& c) -> const std::array<float, 3>& {
                return reinterpret_cast<const std::array<float, 3>&>(
                    c.rotationLimits);
            })
        .def_readwrite("allowPlacement", &ObjectProto::allowPlacement);

    py::class_<GuildObject>(guild, "GuildObject")
        .def(py::init<>())
        .def_readwrite("vid", &GuildObject::vid)
        .def_readwrite("x", &GuildObject::x)
        .def_readwrite("y", &GuildObject::y)
        .def_readwrite("vid", &GuildObject::vid)
        .def_readwrite("vnum", &GuildObject::vnum)
        .def_readwrite("zRot", &GuildObject::zRot)
        .def_readwrite("xRot", &GuildObject::xRot)
        .def_readwrite("yRot", &GuildObject::yRot);

    py::class_<CPythonGuild, std::unique_ptr<CPythonGuild, py::nodelete>>(
        guild, "guildMgr")
        .def(py::init([]() {
                 return std::unique_ptr<CPythonGuild, py::nodelete>(
                     CPythonGuild::InstancePtr());
             }),
             pybind11::return_value_policy::reference_internal)

        .def("GetObjectProto", &CPythonGuild::GetObjectProto)
        .def("GetObjectName", &CPythonGuild::GetObjectName)
        .def("GetGuildObjectByVid", &CPythonGuild::GetGuildObjectByVid)
        .def("GetGuildObjects", &CPythonGuild::GetGuildObjects)
        .def("GetGuildObject", &CPythonGuild::GetGuildObject)
        .def("GetGuildObjectCount", &CPythonGuild::GetGuildObjectCount);
}

extern "C" void initguild()
{
    static PyMethodDef s_methods[] = {
        // Enable
        {"IsGuildEnable", guildIsGuildEnable, METH_VARARGS},
        {"GuildIDToMarkID", guildGuildIDToMarkID, METH_VARARGS},
        {"GetMarkImageFilenameByMarkID", guildGetMarkImageFilenameByMarkID,
         METH_VARARGS},
        {"GetMarkIndexByMarkID", guildGetMarkIndexByMarkID, METH_VARARGS},

        // GuildInfo
        {"GetGuildID", guildGetGuildID, METH_VARARGS},
        {"HasGuildLand", guildHasGuildLand, METH_VARARGS},
        {"GetGuildName", guildGetGuildName, METH_VARARGS},
        {"GetGuildMasterName", guildGetGuildMasterName, METH_VARARGS},
        {"GetEnemyGuildName", guildGetEnemyGuildName, METH_VARARGS},
        {"GetGuildMoney", guildGetGuildMoney, METH_VARARGS},

        // BoardPage
        {"GetGuildBoardCommentCount", guildGetGuildBoardCommentCount,
         METH_VARARGS},
        {"GetGuildBoardCommentData", guildGetGuildBoardCommentData,
         METH_VARARGS},

        // MemberPage
        {"GetGuildLevel", guildGetGuildLevel, METH_VARARGS},
        {"GetGuildExperience", guildGetGuildExperience, METH_VARARGS},
        {"GetGuildMemberCount", guildGetGuildMemberCount, METH_VARARGS},
        {"GetGuildMemberLevelSummary", guildGetGuildMemberLevelSummary,
         METH_VARARGS},
        {"GetGuildMemberLevelAverage", guildGetGuildMemberLevelAverage,
         METH_VARARGS},
        {"GetGuildExperienceSummary", guildGetGuildExperienceSummary,
         METH_VARARGS},

        // SkillPage
        {"GetGuildSkillPoint", guildGetGuildSkillPoint, METH_VARARGS},
        {"GetDragonPowerPoint", guildGetDragonPowerPoint, METH_VARARGS},
        {"GetGuildSkillLevel", guildGetGuildSkillLevel, METH_VARARGS},
        {"GetSkillLevel", guildGetSkillLevel, METH_VARARGS},
        {"GetSkillMaxLevelNew", guildGetSkillMaxLevelNew, METH_VARARGS},

        {"SetSkillIndex", guildSetSkillIndex, METH_VARARGS},
        {"GetSkillIndex", guildGetSkillIndex, METH_VARARGS},

        // GradePage
        {"GetGradeData", guildGetGradeData, METH_VARARGS},
        {"GetGradeName", guildGetGradeName, METH_VARARGS},

        // About Member
        {"GetMemberCount", guildGetMemberCount, METH_VARARGS},
        {"GetMemberData", guildGetMemberData, METH_VARARGS},
        {"MemberIndexToPID", guildMemberIndexToPID, METH_VARARGS},
        {"IsMember", guildIsMember, METH_VARARGS},
        {"IsMemberByName", guildIsMemberByName, METH_VARARGS},
        {"MainPlayerHasAuthority", guildMainPlayerHasAuthority, METH_VARARGS},

        // Guild
        {"Destroy", guildDestroy, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    PyObject* poModule = Py_InitModule("guild", s_methods);
    PyModule_AddIntConstant(poModule, "AUTH_ADD_MEMBER", GUILD_AUTH_ADD_MEMBER);
    PyModule_AddIntConstant(poModule, "AUTH_REMOVE_MEMBER",
                            GUILD_AUTH_REMOVE_MEMBER);
    PyModule_AddIntConstant(poModule, "AUTH_NOTICE", GUILD_AUTH_NOTICE);
    PyModule_AddIntConstant(poModule, "AUTH_SKILL", GUILD_AUTH_USE_SKILL);
#ifdef ENABLE_GUILD_STORAGE
    PyModule_AddIntConstant(poModule, "AUTH_PUT_GOLD", GUILD_AUTH_PUT_GOLD);
    PyModule_AddIntConstant(poModule, "AUTH_TAKE_GOLD", GUILD_AUTH_TAKE_GOLD);
    PyModule_AddIntConstant(poModule, "AUTH_USE_GS", GUILD_AUTH_USE_GS);
    PyModule_AddIntConstant(poModule, "AUTH_GS1_PUTITEM",
                            GUILD_AUTH_GS1_PUTITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS1_TAKEITEM",
                            GUILD_AUTH_GS1_TAKEITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS2_PUTITEM",
                            GUILD_AUTH_GS2_PUTITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS2_TAKEITEM",
                            GUILD_AUTH_GS2_TAKEITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS3_PUTITEM",
                            GUILD_AUTH_GS3_PUTITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS3_TAKEITEM",
                            GUILD_AUTH_GS3_TAKEITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS4_PUTITEM",
                            GUILD_AUTH_GS4_PUTITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS4_TAKEITEM",
                            GUILD_AUTH_GS4_TAKEITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS5_PUTITEM",
                            GUILD_AUTH_GS5_PUTITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS5_TAKEITEM",
                            GUILD_AUTH_GS5_TAKEITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS6_PUTITEM",
                            GUILD_AUTH_GS6_PUTITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS6_TAKEITEM",
                            GUILD_AUTH_GS6_TAKEITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS7_PUTITEM",
                            GUILD_AUTH_GS7_PUTITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS7_TAKEITEM",
                            GUILD_AUTH_GS7_TAKEITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS8_PUTITEM",
                            GUILD_AUTH_GS8_PUTITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS8_TAKEITEM",
                            GUILD_AUTH_GS8_TAKEITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS9_PUTITEM",
                            GUILD_AUTH_GS9_PUTITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS9_TAKEITEM",
                            GUILD_AUTH_GS9_TAKEITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS10_PUTITEM",
                            GUILD_AUTH_GS10_PUTITEM);
    PyModule_AddIntConstant(poModule, "AUTH_GS10_TAKEITEM",
                            GUILD_AUTH_GS10_TAKEITEM);
    PyModule_AddIntConstant(poModule, "AUTH_LOG", GUILD_AUTH_LOG);
    PyModule_AddIntConstant(poModule, "AUTH_MANAGE", GUILD_AUTH_MANAGE);
#endif
    PyModule_AddIntConstant(poModule, "ENEMY_GUILD_SLOT_MAX_COUNT",
                            CPythonGuild::ENEMY_GUILD_SLOT_MAX_COUNT);
}
