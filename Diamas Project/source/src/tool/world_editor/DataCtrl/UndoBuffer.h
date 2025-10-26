#ifndef METIN2_TOOL_WORLDEDITOR_DATACTRL_UNDOBUFFER_HPP
#define METIN2_TOOL_WORLDEDITOR_DATACTRL_UNDOBUFFER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CUndoBuffer
{
	public:
		class IUndoData
		{
			public:
				IUndoData(){}
				virtual ~IUndoData(){}

				virtual void Backup() = 0;
				virtual void Restore() = 0;

				virtual void BackupStatement() = 0;
				virtual void RestoreStatement() = 0;
		};
		typedef std::deque<IUndoData*>		TUndoDataDeque;
		typedef TUndoDataDeque::iterator	TUndoDataIterator;

	public:
		CUndoBuffer();
		~CUndoBuffer();

		void ClearTail(uint32_t dwIndex);

		void Backup(IUndoData * pData);
		void BackupCurrent(IUndoData * pData);

		void Undo();
		void Redo();

	protected:
		bool GetUndoData(uint32_t dwIndex, IUndoData ** ppUndoData);

	protected:
		uint32_t m_dwCurrentStackPosition;
		TUndoDataDeque m_UndoDataDeque;

		IUndoData * m_pTopData;

		const unsigned char m_ucMAXBufferCount;
};

METIN2_END_NS

#endif
