#include "Marriage.h"
#include "Main.h"
#include "DBManager.h"
#include "ClientManager.h"

namespace marriage
{
	const uint32_t WEDDING_LENGTH = 60 * 60; // sec
	bool operator < (const TWedding& lhs, const TWedding& rhs)
	{
		return lhs.dwTime < rhs.dwTime;
	}

	bool operator > (const TWedding& lhs, const TWedding& rhs)
	{
		return lhs.dwTime > rhs.dwTime;
	}

	bool operator > (const TWeddingInfo &lhs, const TWeddingInfo& rhs)
	{
		return lhs.dwTime > rhs.dwTime;
	}

	CManager::CManager()
	{
	}

	CManager::~CManager()
	{
	}

	bool CManager::Initialize()
	{
		std::unique_ptr<SQLMsg> pmsg_delete(CDBManager::instance().DirectQuery("DELETE FROM marriage WHERE is_married = 0"));
		std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery("SELECT pid1, pid2, love_point, time, is_married, p1.name, p2.name FROM marriage, player as p1, player as p2 WHERE p1.id = pid1 AND p2.id = pid2"));

		SPDLOG_INFO( "MarriageList(size=%lu)", pmsg->Get()->uiNumRows);

		if (pmsg->Get()->uiNumRows > 0)
		{
			for (uint uiRow = 0; uiRow != pmsg->Get()->uiNumRows; ++uiRow)
			{
				MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

				uint32_t pid1 = 0; str_to_number(pid1, row[0]);
				uint32_t pid2 = 0; str_to_number(pid2, row[1]);
				int love_point = 0; str_to_number(love_point, row[2]);
				uint32_t time = 0; str_to_number(time, row[3]);
				uint8_t is_married = 0; str_to_number(is_married, row[4]);
				const char* name1 = row[5];
				const char* name2 = row[6];

				TMarriage* pMarriage = new TMarriage(pid1, pid2, love_point, time, is_married, name1, name2);
				m_Marriages.insert(pMarriage);
				m_MarriageByPID.insert(std::make_pair(pid1, pMarriage));
				m_MarriageByPID.insert(std::make_pair(pid2, pMarriage));

				SPDLOG_INFO( "Marriage {}: LP:{} TM:{} ST:{} {}:{} {}:{} ", uiRow, love_point, time, is_married, pid1, name1, pid2, name2);
			}
		}
		return true;
	}

	TMarriage* CManager::Get(uint32_t dwPlayerID)
	{
		auto it = m_MarriageByPID.find(dwPlayerID);

		if (it != m_MarriageByPID.end())
			return it->second;

		return nullptr;
	}

	void Align(uint32_t& rPID1, uint32_t& rPID2)
	{
		if (rPID1 > rPID2)
			std::swap(rPID1, rPID2);
	}

	void CManager::Add(uint32_t dwPID1, uint32_t dwPID2, const char* szName1, const char* szName2)
	{
		uint32_t now = CClientManager::instance().GetCurrentTime();
		if (IsMarried(dwPID1) || IsMarried(dwPID2))
		{
			spdlog::error("cannot marry already married character. %d - %d", dwPID1, dwPID2);
			return;
		}

		Align(dwPID1, dwPID2);

		std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(fmt::format("INSERT INTO marriage(pid1, pid2, love_point, time) VALUES ({}, {}, 0, {})", dwPID1, dwPID2, now)));

		if (pmsg->Get()->uiAffectedRows == 0 || pmsg->Get()->uiAffectedRows == (my_ulonglong)-1)
		{
			spdlog::error("cannot insert marriage");
			return;
		}

		SPDLOG_INFO( "MARRIAGE ADD %u %u", dwPID1, dwPID2);

		TMarriage* pMarriage = new TMarriage(dwPID1, dwPID2, 0, now, 0, szName1, szName2);
		m_Marriages.insert(pMarriage);
		m_MarriageByPID.insert(std::make_pair(dwPID1, pMarriage));
		m_MarriageByPID.insert(std::make_pair(dwPID2, pMarriage));

		TPacketMarriageAdd p;
		p.dwPID1 = dwPID1;
		p.dwPID2 = dwPID2;
		p.tMarryTime = now;
		strlcpy(p.szName1, szName1, sizeof(p.szName1));
		strlcpy(p.szName2, szName2, sizeof(p.szName2));
		CClientManager::instance().ForwardPacket(HEADER_DG_MARRIAGE_ADD, &p, sizeof(p));
	}

	void CManager::Update(uint32_t dwPID1, uint32_t dwPID2, int32_t iLovePoint, uint8_t byMarried)
	{
		TMarriage* pMarriage = Get(dwPID1);
		if (!pMarriage || pMarriage->GetOther(dwPID1) != dwPID2)
		{
			spdlog::error("not under marriage : %u %u", dwPID1, dwPID2);
			return;
		}

		if (pMarriage->love_point == iLovePoint && pMarriage->is_married == byMarried) // T#87 Just in case something weird happens. <MartPwnS>
			return;

		if (iLovePoint < 0) // T#170 Just in case we ever get a value below 0 <MartPwnS>
			return;

		Align(dwPID1, dwPID2);

		std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(fmt::format("UPDATE marriage SET love_point = {}, is_married = {} WHERE pid1 = {} AND pid2 = {}", 
				iLovePoint, byMarried, pMarriage->pid1, pMarriage->pid2)));

		if (pmsg->Get() == nullptr)
		{
			spdlog::error("pmsg->Get() == NULL! PID1: %d PID2: %d", dwPID1, dwPID2);
			return;
		}

		if (pmsg->Get()->uiAffectedRows == 0 || pmsg->Get()->uiAffectedRows == (my_ulonglong)-1)
		{
			spdlog::error("cannot update marriage : PID:%u %u", dwPID1, dwPID2);
			return;
		}

		SPDLOG_INFO( "MARRIAGE UPDATE PID:%u %u LP:%u ST:%d", dwPID1, dwPID2, iLovePoint, byMarried);
		pMarriage->love_point = iLovePoint;
		pMarriage->is_married = byMarried;

		TPacketMarriageUpdate p;
		p.dwPID1 = dwPID1;
		p.dwPID2 = dwPID2;
		p.iLovePoint = pMarriage->love_point;
		p.byMarried = pMarriage->is_married;
		CClientManager::instance().ForwardPacket(HEADER_DG_MARRIAGE_UPDATE, &p, sizeof(p));
	}

	void CManager::Remove(uint32_t dwPID1, uint32_t dwPID2)
	{
		TMarriage* pMarriage = Get(dwPID1);

		if (pMarriage)
		{
			SPDLOG_INFO( "Break Marriage pid1 %d pid2 %d Other %d", dwPID1, dwPID2, pMarriage->GetOther(dwPID1));
		}
		if (!pMarriage || pMarriage->GetOther(dwPID1) != dwPID2)
		{
			auto it = m_MarriageByPID.begin();

			for (; it != m_MarriageByPID.end(); ++it)
			{
				SPDLOG_INFO( "Marriage List pid1 %d pid2 %d", it->second->pid1, it->second->pid2);
			}
			spdlog::error("not under marriage : PID:%u %u", dwPID1, dwPID2);
			return;
		}

		Align(dwPID1, dwPID2);

		std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(fmt::format("DELETE FROM marriage WHERE pid1 = {} AND pid2 = {}", dwPID1, dwPID2)));

		if (pmsg->Get()->uiAffectedRows == 0 || pmsg->Get()->uiAffectedRows == (my_ulonglong)-1)
		{
			spdlog::error("cannot delete marriage : PID:%u %u", dwPID1, dwPID2);
			return;
		}

		SPDLOG_INFO( "MARRIAGE REMOVE PID:%u %u", dwPID1, dwPID2);

		m_Marriages.erase(pMarriage);
		m_MarriageByPID.erase(dwPID1);
		m_MarriageByPID.erase(dwPID2);

		TPacketMarriageRemove p;
		p.dwPID1 = dwPID1;
		p.dwPID2 = dwPID2;
		CClientManager::instance().ForwardPacket(HEADER_DG_MARRIAGE_REMOVE, &p, sizeof(p));

		delete pMarriage;
	}

	void CManager::EngageToMarriage(uint32_t dwPID1, uint32_t dwPID2)
	{
		TMarriage* pMarriage = Get(dwPID1);
		if (!pMarriage || pMarriage->GetOther(dwPID1) != dwPID2)
		{
			spdlog::error("not under marriage : PID:%u %u", dwPID1, dwPID2);
			return;
		}

		if (pMarriage->is_married)
		{
			spdlog::error("already married, cannot change engage to marry : PID:%u %u", dwPID1, dwPID2);
			return;
		}

		Align(dwPID1, dwPID2);

		std::unique_ptr<SQLMsg> pmsg(CDBManager::instance().DirectQuery(fmt::format("UPDATE marriage SET is_married = 1 WHERE pid1 = {} AND pid2 = {}", 
				pMarriage->pid1, pMarriage->pid2)));

		if (pmsg->Get()->uiAffectedRows == 0 || pmsg->Get()->uiAffectedRows == (my_ulonglong)-1)
		{
			spdlog::error("cannot change engage to marriage : PID:%u %u affectedRows = (%u)", dwPID1, dwPID2, pmsg->Get()->uiAffectedRows);
			return;
		}

		SPDLOG_INFO( "MARRIAGE ENGAGE->MARRIAGE PID:%u %u", dwPID1, dwPID2);
		pMarriage->is_married = 1;

		TPacketMarriageUpdate p;
		p.dwPID1 = dwPID1;
		p.dwPID2 = dwPID2;
		p.iLovePoint = pMarriage->love_point;
		p.byMarried = pMarriage->is_married;
		CClientManager::instance().ForwardPacket(HEADER_DG_MARRIAGE_UPDATE, &p, sizeof(p));
	}

	void CManager::OnSetup(CPeer* peer)
	{
		// ��ȥ�� ����� ������
		for (auto it = m_Marriages.begin(); it != m_Marriages.end(); ++it)
		{
			TMarriage* pMarriage = *it;

			{
				TPacketMarriageAdd p;
				p.dwPID1 = pMarriage->pid1;
				p.dwPID2 = pMarriage->pid2;
				p.tMarryTime = pMarriage->time;
				strlcpy(p.szName1, pMarriage->name1.c_str(), sizeof(p.szName1));
				strlcpy(p.szName2, pMarriage->name2.c_str(), sizeof(p.szName2));
				peer->EncodeHeader(HEADER_DG_MARRIAGE_ADD, 0, sizeof(p));
				peer->Encode(&p, sizeof(p));
			}

			{
				TPacketMarriageUpdate p;
				p.dwPID1 = pMarriage->pid1;
				p.dwPID2 = pMarriage->pid2;
				p.iLovePoint = pMarriage->love_point;
				p.byMarried	= pMarriage->is_married;
				peer->EncodeHeader(HEADER_DG_MARRIAGE_UPDATE, 0, sizeof(p));
				peer->Encode(&p, sizeof(p));
			}
		}

		// ��ȥ�� ������
		for (auto it = m_mapRunningWedding.begin(); it != m_mapRunningWedding.end(); ++it)
		{
			const TWedding& t = it->second;

			TPacketWeddingReady p;
			p.dwPID1 = t.dwPID1;
			p.dwPID2 = t.dwPID2;
			p.dwMapIndex = t.dwMapIndex;

			peer->EncodeHeader(HEADER_DG_WEDDING_READY, 0, sizeof(p));
			peer->Encode(&p, sizeof(p));

			TPacketWeddingStart p2;
			p2.dwPID1 = t.dwPID1;
			p2.dwPID2 = t.dwPID2;

			peer->EncodeHeader(HEADER_DG_WEDDING_START, 0, sizeof(p2));
			peer->Encode(&p2, sizeof(p2));
		}
	}

	void CManager::ReadyWedding(uint32_t dwMapIndex, uint32_t dwPID1, uint32_t dwPID2)
	{
		uint32_t dwStartTime = CClientManager::instance().GetCurrentTime();
		m_pqWeddingStart.push(TWedding(dwStartTime + 5, dwMapIndex, dwPID1, dwPID2));
	}

	void CManager::EndWedding(uint32_t dwPID1, uint32_t dwPID2)
	{
		auto it = m_mapRunningWedding.find(std::make_pair(dwPID1, dwPID2));
		if (it == m_mapRunningWedding.end())
		{
			spdlog::error("try to end wedding %u %u", dwPID1, dwPID2);
			return;
		}

		TWedding& w = it->second;

		TPacketWeddingEnd p;
		p.dwPID1 = w.dwPID1;
		p.dwPID2 = w.dwPID2;
		CClientManager::instance().ForwardPacket(HEADER_DG_WEDDING_END, &p, sizeof(p));
		m_mapRunningWedding.erase(it);
	}

	void CManager::Update()
	{
		uint32_t now = CClientManager::instance().GetCurrentTime();

		if (!m_pqWeddingEnd.empty())
		{
			while (!m_pqWeddingEnd.empty() && m_pqWeddingEnd.top().dwTime <= now)
			{
				TWeddingInfo wi = m_pqWeddingEnd.top();
				m_pqWeddingEnd.pop();

				auto it = m_mapRunningWedding.find(std::make_pair(wi.dwPID1, wi.dwPID2));
				if (it == m_mapRunningWedding.end())
					continue;

				TWedding& w = it->second;

				TPacketWeddingEnd p;
				p.dwPID1 = w.dwPID1;
				p.dwPID2 = w.dwPID2;
				CClientManager::instance().ForwardPacket(HEADER_DG_WEDDING_END, &p, sizeof(p));
				m_mapRunningWedding.erase(it);

				auto it_marriage = m_MarriageByPID.find(w.dwPID1);

				if (it_marriage != m_MarriageByPID.end())
				{
					TMarriage* pMarriage = it_marriage->second;
					if (!pMarriage->is_married)
					{
						Remove(pMarriage->pid1, pMarriage->pid2);
					}
				}
			}
		}
		if (!m_pqWeddingStart.empty())
		{
			while (!m_pqWeddingStart.empty() && m_pqWeddingStart.top().dwTime <= now)
			{
				TWedding w = m_pqWeddingStart.top();
				m_pqWeddingStart.pop();

				TPacketWeddingStart p;
				p.dwPID1 = w.dwPID1;
				p.dwPID2 = w.dwPID2;
				CClientManager::instance().ForwardPacket(HEADER_DG_WEDDING_START, &p, sizeof(p));

				w.dwTime += WEDDING_LENGTH;
				m_pqWeddingEnd.push(TWeddingInfo(w.dwTime, w.dwPID1, w.dwPID2));
				m_mapRunningWedding.insert(std::make_pair(std::make_pair(w.dwPID1, w.dwPID2), w));
			}
		}
	}
}
