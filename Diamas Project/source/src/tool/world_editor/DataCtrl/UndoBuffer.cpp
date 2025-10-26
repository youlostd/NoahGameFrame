#include "StdAfx.h"
#include "UndoBuffer.h"

METIN2_BEGIN_NS

void CUndoBuffer::ClearTail(uint32_t dwIndex)
{
	if (dwIndex >= m_UndoDataDeque.size())
		return;

	for (uint32_t i = dwIndex; i < m_UndoDataDeque.size(); ++i)
	{
		delete m_UndoDataDeque[i];
	}

	m_UndoDataDeque.resize(dwIndex);
}

void CUndoBuffer::Backup(IUndoData * pData)
{
	ClearTail(m_dwCurrentStackPosition+1);

	pData->Backup();
	pData->BackupStatement();
	m_UndoDataDeque.push_back(pData);

	TUndoDataIterator aUndoDataIterator;
	while (m_UndoDataDeque.size() > m_ucMAXBufferCount)
	{
		aUndoDataIterator = m_UndoDataDeque.begin();
		IUndoData * pUndoDataTobeDeleted = *aUndoDataIterator;
		delete pUndoDataTobeDeleted;
// 		m_UndoDataDeque.pop_front();
		m_UndoDataDeque.erase(aUndoDataIterator);
	}

	m_dwCurrentStackPosition = m_UndoDataDeque.size()-1;
}
void CUndoBuffer::BackupCurrent(IUndoData * pData)
{
	if (m_pTopData)
		delete m_pTopData;

	m_pTopData = pData;
	pData->Backup();
	pData->BackupStatement();
}
void CUndoBuffer::Undo()
{
	IUndoData * pUndoData;
	if (!GetUndoData(m_dwCurrentStackPosition, &pUndoData))
		return;

	pUndoData->Restore();
	m_dwCurrentStackPosition--;

	////////////////////////////////////////////////////////////////////////

	IUndoData * pStatementUndoData;
	if (!GetUndoData(m_dwCurrentStackPosition, &pStatementUndoData))
		return;
	pStatementUndoData->RestoreStatement();
}
void CUndoBuffer::Redo()
{
	m_dwCurrentStackPosition = std::min<uint32_t>(m_UndoDataDeque.size() - 1,
	                                           m_dwCurrentStackPosition + 1);

	IUndoData * pUndoData;
	if (!GetUndoData(m_dwCurrentStackPosition+1, &pUndoData))
		return;

	pUndoData->Restore();

	////////////////////////////////////////////////////////////////////////

	IUndoData * pStatementUndoData;
	if (!GetUndoData(m_dwCurrentStackPosition, &pStatementUndoData))
		return;
	pStatementUndoData->RestoreStatement();
}

bool CUndoBuffer::GetUndoData(uint32_t dwIndex, IUndoData ** ppUndoData)
{
	if (dwIndex > m_UndoDataDeque.size())
		return false;

	if (m_UndoDataDeque.size() == dwIndex)
	{
		if (m_pTopData)
		{
			*ppUndoData = m_pTopData;
			return true;
		}

		return false;
	}

	*ppUndoData = m_UndoDataDeque[dwIndex];

	return true;
}

CUndoBuffer::CUndoBuffer():m_ucMAXBufferCount(30)
{
	m_pTopData = NULL;
	m_dwCurrentStackPosition = -1;
}
CUndoBuffer::~CUndoBuffer()
{
	if (m_pTopData)
	{
		delete m_pTopData;
		m_pTopData = NULL;
	}

	for (const auto& qe : m_UndoDataDeque) {
		// TODO(tim): Use smart-pointer instead
		delete qe;
	}
}

METIN2_END_NS
