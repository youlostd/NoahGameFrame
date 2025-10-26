#include "StdAfx.h"
#include "PythonGuildStorage.h"
#include <game/GamePacket.hpp>
#include "PythonNetworkStream.h"
#include "../GameLib/ItemManager.h"
#ifdef ENABLE_GUILD_STORAGE

void CPythonGuildStorage::AddLog(TPacketGuildStorageLogGC pack)
{
	SPDLOG_ERROR("AddLog: {0}, {1}, {2}, {3}", pack.memberName, pack.action, pack.timestamp, pack.vnum);
	m_LogVector.push_back(pack);
}

void CPythonGuildStorage::RequestLog()
{
	gPythonNetworkStream->SendRequestLogPacket();
}

void CPythonGuildStorage::ClearLog()
{
	m_LogVector.clear();
}

std::vector<TPacketGuildStorageLogGC> CPythonGuildStorage::GetLogs()
{
	return m_LogVector;
}
void CPythonGuildStorage::SetItemData(uint32_t dwSlotIndex, const ClientItemData & rItemData)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		SPDLOG_ERROR("CPythonGuildStorage::SetItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
		return;
	}

	m_ItemInstanceVector[dwSlotIndex] = rItemData;
}

void CPythonGuildStorage::OpenGuildStorage()
{
	m_ItemInstanceVector.clear();
	m_ItemInstanceVector.resize(GUILDSTORAGE_TOTAL_SIZE);

	for (uint32_t i = 0; i < m_ItemInstanceVector.size(); ++i)
	{
		ClientItemData & rInstance = m_ItemInstanceVector[i];
		ZeroMemory(&rInstance, sizeof(rInstance));
	}
}

void CPythonGuildStorage::DelItemData(uint32_t dwSlotIndex)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		SPDLOG_ERROR("CPythonGuildStorage::DelItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
		return;
	}

	ClientItemData & rInstance = m_ItemInstanceVector[dwSlotIndex];
	ZeroMemory(&rInstance, sizeof(rInstance));
}

int CPythonGuildStorage::GetCurrentGuildStorageSize()
{
	return m_ItemInstanceVector.size();
}

bool CPythonGuildStorage::GetSlotItemID(uint32_t dwSlotIndex, uint32_t* pdwItemID)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		SPDLOG_ERROR("CPythonGuildStorage::GetSlotItemID(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
		return FALSE;
	}

	*pdwItemID = m_ItemInstanceVector[dwSlotIndex].vnum;

	return TRUE;
}

bool CPythonGuildStorage::GetSlotItemTransmutationVnum(uint32_t dwSlotIndex, uint32_t* pdwItemID)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		SPDLOG_ERROR("CPythonGuildStorage::GetSlotItemID(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
		return FALSE;
	}

	*pdwItemID = m_ItemInstanceVector[dwSlotIndex].transVnum;

	return TRUE;
}

bool CPythonGuildStorage::GetItemDataPtr(uint32_t dwSlotIndex, ClientItemData ** ppInstance)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		SPDLOG_ERROR("CPythonGuildStorage::GetItemData(dwSlotIndex={0}) - Strange slot index", dwSlotIndex);
		return FALSE;
	}

	*ppInstance = &m_ItemInstanceVector[dwSlotIndex];

	return TRUE;
}

CPythonGuildStorage::CPythonGuildStorage()
{
	m_ItemInstanceVector.clear();
	m_LogVector.clear();
	m_ItemInstanceVector.resize(GUILDSTORAGE_TOTAL_SIZE);

	for (uint32_t i = 0; i < m_ItemInstanceVector.size(); ++i)
	{
		ClientItemData & rInstance = m_ItemInstanceVector[i];
		ZeroMemory(&rInstance, sizeof(rInstance));
	}
}

CPythonGuildStorage::~CPythonGuildStorage()
{
}

PyObject * guildStorageGetCurrentGuildStorageSize(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonGuildStorage::Instance().GetCurrentGuildStorageSize());
}

PyObject * guildStorageGetItemID(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	ClientItemData * pInstance;
	if (!CPythonGuildStorage::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->vnum);
}

PyObject * guildStorageGetItemTransmutationVnum(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	ClientItemData * pInstance;
	if (!CPythonGuildStorage::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->transVnum);
}

PyObject * guildStorageGetItemCount(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	ClientItemData * pInstance;
	if (!CPythonGuildStorage::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->count);
}

PyObject * guildStorageGetItemFlags(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	ClientItemData * pInstance;
	if (!CPythonGuildStorage::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	auto proto = CItemManager::instance().GetProto(pInstance->vnum);
	if(proto) {
		return Py_BuildValue("i", proto->GetFlags());
	}

	return Py_BuildValue("i", 0);
}

PyObject * guildStorageGetItemMetinSocket(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();
	int iSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
		return Py_BadArgument();

	if (iSocketIndex >= ITEM_SOCKET_MAX_NUM)
		return Py_BuildException();

	ClientItemData * pItemData;
	if (!CPythonGuildStorage::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
		return Py_BuildException();

	return Py_BuildValue("L", pItemData->sockets[iSocketIndex]);
}

PyObject * guildStorageGetItemAttribute(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_MAX_NUM)
	{
		ClientItemData * pItemData;
		if (CPythonGuildStorage::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
			return Py_BuildValue("id", pItemData->attrs[iAttrSlotIndex].bType, pItemData->attrs[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("id", 0, 0.0);
}


PyObject * guildStorageGetLogs(PyObject * poSelf, PyObject * poArgs)
{
	auto logs = CPythonGuildStorage::Instance().GetLogs();
	PyObject *pTuple = PyTuple_New(logs.size());
	for (UINT i = 0; i < logs.size(); i++)
	{
		PyObject *pTupleSocket = PyTuple_New(ITEM_SOCKET_MAX_NUM);
		for (UINT j = 0; j < ITEM_SOCKET_MAX_NUM; j++) {
			PyTuple_SetItem(pTupleSocket, j, Py_BuildValue("i", logs[i].alSockets[j]));
		}
		
		PyObject *pTupleAttr = PyTuple_New(ITEM_ATTRIBUTE_MAX_NUM);
		for (UINT j = 0; j < ITEM_ATTRIBUTE_MAX_NUM; j++) {
			PyTuple_SetItem(pTupleAttr, j, Py_BuildValue("(ii)", logs[i].aAttr[j].bType, logs[i].aAttr[j].sValue));
		}

		PyTuple_SetItem(pTuple, i, Py_BuildValue("(sisliiOO)", logs[i].memberName, logs[i].action, logs[i].timestamp, logs[i].vnum, logs[i].page, logs[i].count, pTupleSocket, pTupleAttr));
	}
	return Py_BuildValue("O", pTuple);
}

PyObject * guildStorageRequestLogs(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGuildStorage::instance().RequestLog();

	return Py_BuildValue("i", 0);
}
extern "C" void initguildStorage()
{
	static PyMethodDef s_methods[] =
	{
		// GuildStorage
		{ "GetCurrentGuildStorageSize", guildStorageGetCurrentGuildStorageSize, METH_VARARGS },
		{ "GetItemTransmutationVnum", guildStorageGetItemTransmutationVnum, METH_VARARGS },
		{ "GetItemID", guildStorageGetItemID, METH_VARARGS },
		{ "GetItemCount", guildStorageGetItemCount, METH_VARARGS },
		{ "GetItemFlags", guildStorageGetItemFlags, METH_VARARGS },
		{ "GetItemMetinSocket", guildStorageGetItemMetinSocket, METH_VARARGS },
		{ "GetItemAttribute", guildStorageGetItemAttribute, METH_VARARGS },
		{ "GetLogs", guildStorageGetLogs, METH_VARARGS },
		{ "RequestLogs", guildStorageRequestLogs, METH_VARARGS },													   

		{ NULL, NULL, NULL },
	};

	PyObject * poModule = Py_InitModule("guildStorage", s_methods);
	PyModule_AddIntConstant(poModule, "GUILDSTORAGE_SLOT_X_COUNT", CPythonGuildStorage::GUILDSTORAGE_SLOT_X_COUNT);
	PyModule_AddIntConstant(poModule, "GUILDSTORAGE_SLOT_Y_COUNT", CPythonGuildStorage::GUILDSTORAGE_SLOT_Y_COUNT);
	PyModule_AddIntConstant(poModule, "GUILDSTORAGE_PAGE_SIZE", CPythonGuildStorage::GUILDSTORAGE_PAGE_SIZE);
	PyModule_AddIntConstant(poModule, "GUILDSTORAGE_GLYPH_COUNT", CPythonGuildStorage::GUILDSTORAGE_GLYPH_COUNT);
	PyModule_AddIntConstant(poModule, "GUILDSTORAGE_TAB_COUNT", CPythonGuildStorage::GUILDSTORAGE_TAB_COUNT);
}
#endif
