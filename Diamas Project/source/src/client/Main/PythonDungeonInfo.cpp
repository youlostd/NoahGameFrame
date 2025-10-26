#include "StdAfx.h"
#include "PythonDungeonInfo.h"

#include <pak/Vfs.hpp>
#include <pak/Util.hpp>

// For test 
#include "PythonApplication.h"

bool CPythonDungeonInfo::LoadDungeonInfo(const char * c_szFileName)
{
	auto fp = GetVfs().Open(c_szFileName, kVfsOpenFullyBuffered);
    if (!fp)
        return false;

	const auto size = fp->GetSize();

    storm::View data(storm::GetDefaultAllocator());
	fp->GetView(0, data, size);

    CMemoryTextFileLoader textFileLoader;
    CTokenVector stTokenVector;

    textFileLoader.Bind(std::string_view(reinterpret_cast<const char *>(data.GetData()), size));

	CTokenVector TokenVector;
	for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLineByTab(i, &TokenVector))
			continue;

		std::string strLine = textFileLoader.GetLineString(i);

		if (TokenVector.size() != TABLE_TOKEN_TYPE_MAX_NUM)
		{
			SPDLOG_ERROR("CPythonDungeonInfo::LoadDungeonInfo({}) - Strange Token Count [Line:{} / TokenCount:{}]", c_szFileName, i, TokenVector.size());
			continue;
		}

		int iID = atoi(TokenVector[TABLE_TOKEN_TYPE_ID].c_str());

		TDungeonInfoData dungeonInfo = {};
		
		dungeonInfo.bID = (BYTE)iID;
		
		const std::string & c_strDungeonName = TokenVector[TABLE_TOKEN_TYPE_NAME];
		dungeonInfo.strName = c_strDungeonName.empty() ? "DUNGEON_NO_NAME" : c_strDungeonName;
		
		{
			const std::string & c_strDungeonType = TokenVector[TABLE_TOKEN_TYPE_TYPE];
			if(c_strDungeonType == "PRIVATE")
				dungeonInfo.bType = DUNGEON_TYPE_PRIVATE;
			else if(c_strDungeonType == "PUBLIC")
				dungeonInfo.bType = DUNGEON_TYPE_PUBLIC;
			else
				dungeonInfo.bType = DUNGEON_TYPE_UNKNOWN;
		}
		
		{
			const std::string & c_strDungeonOrg = TokenVector[TABLE_TOKEN_TYPE_ORGANIZATION];
			if(c_strDungeonOrg == "SINGLE")
				dungeonInfo.bOrganization = DUNGEON_ORG_SINGLE;
			else if(c_strDungeonOrg == "PARTY")
				dungeonInfo.bOrganization = DUNGEON_ORG_PARTY;
			else if(c_strDungeonOrg == "HYBRID")
				dungeonInfo.bOrganization = DUNGEON_ORG_HYBRID;
			else
				dungeonInfo.bOrganization = DUNGEON_ORG_UNKNOWN;
		}
		
		{
			const std::string & c_strMinLevel = TokenVector[TABLE_TOKEN_TYPE_MIN_LEVEL];
			if (!c_strMinLevel.empty())
			{
				int minLevel = atoi(c_strMinLevel.c_str());
				dungeonInfo.bMinLevel = (BYTE)minLevel;
			}
			else
				dungeonInfo.bMinLevel = 0;
		}
		
		{
			const std::string & c_strMaxLevel = TokenVector[TABLE_TOKEN_TYPE_MAX_LEVEL];
			if (!c_strMaxLevel.empty())
			{
				int maxLevel = atoi(c_strMaxLevel.c_str());
				dungeonInfo.bMaxLevel = (BYTE)maxLevel;
			}
			else
				dungeonInfo.bMaxLevel = 0;
		}
		
		{
			const std::string & c_strMinParty = TokenVector[TABLE_TOKEN_TYPE_MIN_PARTY];
			if (!c_strMinParty.empty())
			{
				int minParty = atoi(c_strMinParty.c_str());
				dungeonInfo.bMinParty = (BYTE)minParty;
			}
			else
				dungeonInfo.bMinParty = 0;
		}
		
		{
			const std::string & c_strMaxParty = TokenVector[TABLE_TOKEN_TYPE_MAX_PARTY];
			if (!c_strMaxParty.empty())
			{
				int maxParty = atoi(c_strMaxParty.c_str());
				dungeonInfo.bMaxParty = (BYTE)maxParty;
			}
			else
				dungeonInfo.bMaxParty = 0;
		}
		
		{
			const std::string & c_strCooldown = TokenVector[TABLE_TOKEN_TYPE_COOLDOWN];
			if (!c_strCooldown.empty())
			{
				int iCooldown = atoi(c_strCooldown.c_str());
				dungeonInfo.wCooldown = (WORD)iCooldown;
			}
			else
				dungeonInfo.wCooldown = 0;
		}
		
		{
			const std::string & c_strRuntime = TokenVector[TABLE_TOKEN_TYPE_RUNTIME];
			if (!c_strRuntime.empty())
			{
				int iRunTime = atoi(c_strRuntime.c_str());
				dungeonInfo.wRunTime = (WORD)iRunTime;
			}
			else
				dungeonInfo.wRunTime = 0;
		}

		{
			const std::string & c_strEnterMapIndex = TokenVector[TABLE_TOKEN_TYPE_ENTER_MAP_INDEX];
			if (!c_strEnterMapIndex.empty())
			{
				long lEnterMapIdx = atoi(c_strEnterMapIndex.c_str());
				dungeonInfo.lEnterMapIndex = lEnterMapIdx;
			}
			else
				dungeonInfo.lEnterMapIndex = 0;
		}
	    {
			const std::string & c_strEnterX = TokenVector[TABLE_TOKEN_TYPE_ENTER_X];
			if (!c_strEnterX.empty())
			{
				long lEnterX = atoi(c_strEnterX.c_str());
				dungeonInfo.lEnterX = lEnterX;
			}
			else
				dungeonInfo.lEnterX = 0;
		}
		
		{
			const std::string & c_strEnterY = TokenVector[TABLE_TOKEN_TYPE_ENTER_Y];
			if (!c_strEnterY.empty())
			{
				long lEnterY = atoi(c_strEnterY.c_str());
				dungeonInfo.lEnterY = lEnterY;
			}
			else
				dungeonInfo.lEnterY = 0;
		}
		
		{
			const std::string & c_strTicketVnum = TokenVector[TABLE_TOKEN_TYPE_TICKET_VNUM];
			if (!c_strTicketVnum.empty())
			{
				int iTicketVnum = atoi(c_strTicketVnum.c_str());
				dungeonInfo.dwTicketVnum = (DWORD)iTicketVnum;
			}
			else
				dungeonInfo.dwTicketVnum = 0;
		}
		
		{
			const std::string & c_strTicketCount = TokenVector[TABLE_TOKEN_TYPE_TICKET_COUNT];
			if (!c_strTicketCount.empty())
			{
				int iTicketCount = atoi(c_strTicketCount.c_str());
				dungeonInfo.bTicketCount = (BYTE)iTicketCount;
			}
			else
				dungeonInfo.bTicketCount = 0;
		}

		const std::string & c_strStRaceFlag = TokenVector[TABLE_TOKEN_TYPE_ST_RACEFLAG];
		dungeonInfo.strStRaceFlag = c_strStRaceFlag.empty() ? "NO_ST_RACE_FLAG" : c_strStRaceFlag;
		
		const std::string & c_strResRaceFlag = TokenVector[TABLE_TOKEN_TYPE_RES_RACEFLAG];
		dungeonInfo.strResRaceFlag = c_strResRaceFlag.empty() ? "NO_RES_RACE_FLAG" : c_strResRaceFlag;

		m_DungeonInfoMap.insert(TDungeonInfoDataMap::value_type(iID, dungeonInfo));
	}

	return true;
}

const CPythonDungeonInfo::TDungeonInfoData * CPythonDungeonInfo::GetDungeonByID(BYTE bID) const
{
    auto it = m_DungeonInfoMap.find(bID);
	if (it != m_DungeonInfoMap.end())
		return &it->second;
	
	return NULL;
}

bool CPythonDungeonInfo::IsValidDungeon(BYTE bID)
{
	TDungeonInfoDataMap::iterator it = m_DungeonInfoMap.find(bID);
	if(it != m_DungeonInfoMap.end())
		return true;

	return false;
}

void CPythonDungeonInfo::ClearInfo()
{
	m_dungeonInfo.clear();
}

void CPythonDungeonInfo::AppendDungeonInfo(const TPlayerDungeonInfo & info)
{
	m_dungeonInfo.push_back(info);
}

const TPlayerDungeonInfo * CPythonDungeonInfo::GetDungeonInfo(BYTE bID) const
{
	if(m_dungeonInfo.empty())
		return NULL;
	
	for(int i = 0; i < m_dungeonInfo.size(); i++)
	{
		if(m_dungeonInfo.at(i).bDungeonID == bID)
			return &m_dungeonInfo.at(i);
	}
	
	return NULL;
}

void CPythonDungeonInfo::UpdateDungeonCooldown(BYTE bDungeonID, DWORD dwCooldownEnd)
{
	if(m_dungeonInfo.empty())
		return;
	
	for(int i = 0; i < m_dungeonInfo.size(); i++)
	{
		if(m_dungeonInfo.at(i).bDungeonID == bDungeonID)
		{
			m_dungeonInfo.at(i).dwCooldownEnd = dwCooldownEnd;
			break;
		}
	}
}

void CPythonDungeonInfo::AppendDungeonRankingSet(const TDungeonRankSet & set)
{
	m_dungeonRankingInfo.push_back(set);
}

const TDungeonRankSet * CPythonDungeonInfo::GetDungeonRankingSet(BYTE bDungeonID, BYTE bRankingType) const
{
	if(m_dungeonRankingInfo.empty())
		return NULL;
	
	for(int i = 0; i < m_dungeonRankingInfo.size(); i++)
	{
		if(m_dungeonRankingInfo.at(i).bDungeonID == bDungeonID && m_dungeonRankingInfo.at(i).bType == bRankingType)
			return &m_dungeonRankingInfo.at(i);
	}
	
	return NULL;
}

void CPythonDungeonInfo::Destroy(bool bIsPartial)
{
	if(!bIsPartial)
	{
		m_DungeonInfoMap.clear();
	}

	m_dungeonInfo.clear();
	m_bNeedServerRequest = true;
	
	m_dungeonRankingInfo.clear();
	for(int i = 0; i < DUNGEON_ID_MAX; i++)
		for(int j = 0; j < DUNGEON_RANK_TYPE_MAX; j++)
			m_bRankingServerRequest[i][j] = true;
}

CPythonDungeonInfo::CPythonDungeonInfo()
{
}
CPythonDungeonInfo::~CPythonDungeonInfo()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
PyObject * dungeonInfoIsValidDungeon(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();

	return Py_BuildValue("b", CPythonDungeonInfo::Instance().IsValidDungeon(bID));
}

PyObject * dungeonInfoGetName(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const CPythonDungeonInfo::TDungeonInfoData * dungeonInstance = CPythonDungeonInfo::Instance().GetDungeonByID(bID);
	if(dungeonInstance)
		return Py_BuildValue("s", dungeonInstance->strName.c_str());
	
	return Py_BuildValue("s", "DUNGEON_NO_NAME");
}

PyObject * dungeonInfoGetLevel(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const CPythonDungeonInfo::TDungeonInfoData * dungeonInstance = CPythonDungeonInfo::Instance().GetDungeonByID(bID);
	if(dungeonInstance)
		return Py_BuildValue("ii", dungeonInstance->bMinLevel, dungeonInstance->bMaxLevel);
	
	return Py_BuildValue("ii", 0, 0);
}

PyObject * dungeonInfoGetPartyMembers(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const CPythonDungeonInfo::TDungeonInfoData * dungeonInstance = CPythonDungeonInfo::Instance().GetDungeonByID(bID);
	if(dungeonInstance)
		return Py_BuildValue("ii", dungeonInstance->bMinParty, dungeonInstance->bMaxParty);
	
	return Py_BuildValue("ii", 0, 0);
}

PyObject * dungeonInfoGetCooldown(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const CPythonDungeonInfo::TDungeonInfoData * dungeonInstance = CPythonDungeonInfo::Instance().GetDungeonByID(bID);
	if(dungeonInstance)
		return Py_BuildValue("i", dungeonInstance->wCooldown);
	
	return Py_BuildValue("i", 0);
}

PyObject * dungeonInfoGetRuntime(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const CPythonDungeonInfo::TDungeonInfoData * dungeonInstance = CPythonDungeonInfo::Instance().GetDungeonByID(bID);
	if(dungeonInstance)
		return Py_BuildValue("i", dungeonInstance->wRunTime);
	
	return Py_BuildValue("i", 0);
}

PyObject * dungeonInfoGetCooldownEnd(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const TPlayerDungeonInfo * myInfo = CPythonDungeonInfo::Instance().GetDungeonInfo(bID);
	if(myInfo)
		return Py_BuildValue("i", myInfo->dwCooldownEnd);
	
	return Py_BuildValue("i", 0);
}

PyObject * dungeonInfoGetType(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const CPythonDungeonInfo::TDungeonInfoData * dungeonInstance = CPythonDungeonInfo::Instance().GetDungeonByID(bID);
	if(dungeonInstance)
		return Py_BuildValue("i", dungeonInstance->bType);
	
	return Py_BuildValue("i", 0);
}

PyObject * dungeonInfoGetOrganization(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const CPythonDungeonInfo::TDungeonInfoData * dungeonInstance = CPythonDungeonInfo::Instance().GetDungeonByID(bID);
	if(dungeonInstance)
		return Py_BuildValue("i", dungeonInstance->bOrganization);
	
	return Py_BuildValue("i", 0);
}

PyObject * dungeonInfoGetEnterCoord(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const CPythonDungeonInfo::TDungeonInfoData * dungeonInstance = CPythonDungeonInfo::Instance().GetDungeonByID(bID);
	if(dungeonInstance)
		return Py_BuildValue("iii", dungeonInstance->lEnterMapIndex, dungeonInstance->lEnterX, dungeonInstance->lEnterY);
	
	return Py_BuildValue("iii", 0, 0, 0);
}

PyObject * dungeonInfoGetStRaceFlag(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const CPythonDungeonInfo::TDungeonInfoData * dungeonInstance = CPythonDungeonInfo::Instance().GetDungeonByID(bID);
	if(dungeonInstance)
		return Py_BuildValue("s", dungeonInstance->strStRaceFlag.c_str());
	
	return Py_BuildValue("s", "NO_ST_RACE_FLAG");
}

PyObject * dungeonInfoGetResRaceFlag(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const CPythonDungeonInfo::TDungeonInfoData * dungeonInstance = CPythonDungeonInfo::Instance().GetDungeonByID(bID);
	if(dungeonInstance)
		return Py_BuildValue("s", dungeonInstance->strResRaceFlag.c_str());
	
	return Py_BuildValue("s", "NO_RES_RACE_FLAG");
}

PyObject * dungeonInfoGetTicket(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const CPythonDungeonInfo::TDungeonInfoData * dungeonInstance = CPythonDungeonInfo::Instance().GetDungeonByID(bID);
	if(dungeonInstance)
		return Py_BuildValue("ii", dungeonInstance->dwTicketVnum, dungeonInstance->bTicketCount);
	
	return Py_BuildValue("ii", 0, 0);
}

PyObject * dungeonInfoGetPersonalStats(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bID;
	if (!PyTuple_GetByte(poArgs, 0, &bID))
		return Py_BadArgument();
	
	const TPlayerDungeonInfo * myInfo = CPythonDungeonInfo::Instance().GetDungeonInfo(bID);
	if (myInfo)
	{
		return Py_BuildValue("iii", myInfo->wCompleteCount, myInfo->wFastestTime, myInfo->dwHightestDamage);
	}

	return Py_BuildValue("iii", 0, 0, 0);
}

PyObject * dungeonInfoIsNeedRequest(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("b", CPythonDungeonInfo::Instance().IsNeedRequestInfo());
}

PyObject * dungeonInfoIsRankingNeedRequest(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bDungeonID;
	if (!PyTuple_GetByte(poArgs, 0, &bDungeonID))
		return Py_BadArgument();
	
	BYTE bRankingType;
	if (!PyTuple_GetByte(poArgs, 1, &bRankingType))
		return Py_BadArgument();
	
	if (bDungeonID >= DUNGEON_ID_MAX)
		return Py_BuildValue("b", false);
	
	if (bRankingType >= DUNGEON_RANK_TYPE_MAX)
		return Py_BuildValue("b", false);
	
	return Py_BuildValue("b", CPythonDungeonInfo::Instance().IsRankingRequestInfo(bDungeonID, bRankingType));
}

PyObject * dungeonInfoGetRankingData(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bDungeonID;
	if (!PyTuple_GetByte(poArgs, 0, &bDungeonID))
		return Py_BadArgument();
	
	BYTE bRankingType;
	if (!PyTuple_GetByte(poArgs, 1, &bRankingType))
		return Py_BadArgument();

    PyObject* poList = PyList_New(0);
	const TDungeonRankSet * pData = CPythonDungeonInfo::Instance().GetDungeonRankingSet(bDungeonID, bRankingType);
	if (pData)
	{
		
		for(int i = 0; i < DUNGEON_RANKING_MAX_NUM; ++i)
		{
			PyObject* oRankItem = PyTuple_New(3);

			PyTuple_SetItem(oRankItem, 0, Py_BuildValue("i", i+1));
			PyTuple_SetItem(oRankItem, 1, Py_BuildValue("s", pData->rankItems[i].szName));
			PyTuple_SetItem(oRankItem, 2, Py_BuildValue("i", pData->rankItems[i].dwValue));

			PyList_Append(poList, oRankItem);
		}
		
		return poList;
	}

	return poList;
}

PyObject * dungeonInfoClearInfo(PyObject * poSelf, PyObject * poArgs)
{
	CPythonDungeonInfo::Instance().Destroy(true);
	return Py_BuildNone();
}

extern "C" void initdungeonInfo()
{
	static PyMethodDef s_methods[] =
	{
		{ "ClearInfo",								dungeonInfoClearInfo,						METH_VARARGS },
		{ "IsNeedRequest",							dungeonInfoIsNeedRequest,					METH_VARARGS },
		
		{ "IsValidDungeon",							dungeonInfoIsValidDungeon,					METH_VARARGS },
		{ "GetDungeonName",							dungeonInfoGetName,							METH_VARARGS },
		{ "GetDungeonLevel",						dungeonInfoGetLevel,						METH_VARARGS },
		{ "GetDungeonPartyMembers",					dungeonInfoGetPartyMembers,					METH_VARARGS },
		{ "GetDungeonCooldown",						dungeonInfoGetCooldown,						METH_VARARGS },
		{ "GetDungeonRuntime",						dungeonInfoGetRuntime,						METH_VARARGS },
		{ "GetDungeonType",							dungeonInfoGetType,							METH_VARARGS },
		{ "GetDungeonOrganization",					dungeonInfoGetOrganization,					METH_VARARGS },
		{ "GetDungeonEnterCoord",					dungeonInfoGetEnterCoord,					METH_VARARGS },
		{ "GetDungeonStRaceFlag",					dungeonInfoGetStRaceFlag,					METH_VARARGS },
		{ "GetDungeonResRaceFlag",					dungeonInfoGetResRaceFlag,					METH_VARARGS },
		{ "GetDungeonTicket",						dungeonInfoGetTicket,						METH_VARARGS },

		{ "GetDungeonCooldownEnd",					dungeonInfoGetCooldownEnd,					METH_VARARGS },
		{ "GetDungeonPersonalStats",				dungeonInfoGetPersonalStats,				METH_VARARGS },
		
		{ "IsRankingNeedRequest",					dungeonInfoIsRankingNeedRequest,			METH_VARARGS },
		{ "GetRankingData",							dungeonInfoGetRankingData,					METH_VARARGS },
		{ NULL,										NULL,										NULL },
	};

	PyObject * poModule = Py_InitModule("dungeonInfo", s_methods);


	PyModule_AddIntMacro(poModule, DUNGEON_ID_NONE);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_OWL);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_GARDEN);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_DEEP_CAVE);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_NEVERLAND);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_WOOD);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_ICESTORM);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_SHADOW_TOWER);
	PyModule_AddIntMacro(poModule, DUNDEON_ID_PYRA);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_DEMON);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_SLIME_1);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_SLIME_2);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_SLIME_3);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_SLIME_4);
	PyModule_AddIntMacro(poModule, DUNGEON_ID_RATHALOS_1);

	PyModule_AddIntMacro(poModule, DUNGEON_ID_MAX);
	
	PyModule_AddIntConstant(poModule, "DUNGEON_TYPE_PRIVATE",		CPythonDungeonInfo::DUNGEON_TYPE_PRIVATE);
	PyModule_AddIntConstant(poModule, "DUNGEON_TYPE_PUBLIC",		CPythonDungeonInfo::DUNGEON_TYPE_PUBLIC);
	
	PyModule_AddIntConstant(poModule, "DUNGEON_ORG_SINGLE",			CPythonDungeonInfo::DUNGEON_ORG_SINGLE);
	PyModule_AddIntConstant(poModule, "DUNGEON_ORG_PARTY",			CPythonDungeonInfo::DUNGEON_ORG_PARTY);
	PyModule_AddIntConstant(poModule, "DUNGEON_ORG_HYBRID",			CPythonDungeonInfo::DUNGEON_ORG_HYBRID);
	
	PyModule_AddIntConstant(poModule, "DUNGEON_RANK_TYPE_COUNT",	DUNGEON_RANK_TYPE_COUNT);
	PyModule_AddIntConstant(poModule, "DUNGEON_RANK_TYPE_TIME",		DUNGEON_RANK_TYPE_TIME);
	PyModule_AddIntConstant(poModule, "DUNGEON_RANK_TYPE_DMG",		DUNGEON_RANK_TYPE_DMG);
}
