#include "char.h"
#include "char_manager.h"
#include "sectree_manager.h"
#include "DbCacheSocket.hpp"

#include "wedding.h"
#include "config.h"
#include "utils.h"
#include "questmanager.h"
#include "desc.h"
#include "DbCacheSocket.hpp"
#include "desc_manager.h"
#include "MasterUtil.hpp"

struct OnlinePlayer;
extern bool g_bShutdown;

namespace marriage
{
const int MAX_LOVE_GRADE = 4;
const int MAX_MARRIAGE_UNIQUE_ITEM = 6;

struct TMarriageItemBonusByGrade
{
    uint32_t dwVnum;
    int value[MAX_LOVE_GRADE];
} g_ItemBonus[MAX_MARRIAGE_UNIQUE_ITEM] = {
    {71069, {4, 5, 6, 8,}},     // 관통 증가
    {71070, {10, 12, 15, 20,}}, // 경험치 증가
    {71071, {4, 5, 6, 8,}},     // 크리티컬 증가
    {71072, {-4, -5, -6, -8,}}, // 상대방 공격력 감소
    {71073, {20, 25, 30, 40,}}, // 공격력 증가 (절대값)
    {71074, {12, 16, 20, 30,}}, // 방어력 증가 (절대값)

    //{ 71069,	1,	2,	3,	6,	8,  }, // 관통 증가
    //{ 71070,	5,	7,	10,	15,	20, }, // 경험치 증가
    //{ 71071,	1,	2,	3,	6,	8,  }, // 크리티컬 증가
    //{ 71072,	5,	10,	15,	20,	30, }, // 상대방이 입은 데미지를 나에게로
    //{ 71073,	10,	15,	20,	25,	40, }, // 공격력 증가 (절대값)
    //{ 71074,	5,	10,	15,	20,	30, }, // 방어력 증가 (절대값)
};

const int MARRIAGE_POINT_PER_DAY = 1;
const int MARRIAGE_POINT_PER_DAY_FAST = 2;
using namespace std;

void SendLoverInfo(CHARACTER *ch, const string &lover_name, int love_point)
{
    TPacketGCLoverInfo p;
    p.name = lover_name;
    p.love_point = love_point;
    ch->GetDesc()->Send(HEADER_GC_LOVER_INFO, p);
}

TMarriage::~TMarriage()
{
    StopNearCheckEvent();
    if (IsOnline())
    {
        ch1->ChatPacket(CHAT_TYPE_COMMAND, "lover_divorce");
        ch2->ChatPacket(CHAT_TYPE_COMMAND, "lover_divorce");
    }
    delete (pWeddingInfo);
    pWeddingInfo = nullptr;
}

int TMarriage::GetMarriageGrade()
{
    int point = std::clamp(GetMarriagePoint(), 50, 100);

    if (point < 65)
        return 0;

    if (point < 80)
        return 1;

    if (point < 100)
        return 2;

    return 3;
}

int TMarriage::GetMarriagePoint()
{
    int point_per_day = MARRIAGE_POINT_PER_DAY;
    int max_limit = 30;
    if (IsOnline())
    {
        if (ch1->GetPremiumRemainSeconds(PREMIUM_MARRIAGE_FAST) > 0 ||
            ch2->GetPremiumRemainSeconds(PREMIUM_MARRIAGE_FAST) > 0)
        {
            point_per_day = MARRIAGE_POINT_PER_DAY_FAST;
            max_limit = 40;
        }
    }

    int days = (get_global_time() - marry_time) / 86400;

    return std::min(50 + std::min(days * point_per_day, max_limit) + std::min(love_point / 1000000, max_limit),
                    100);
}

bool TMarriage::IsNear()
{
    if (!is_married)
        return false;

    if (!IsOnline())
        return false;

    return ch1->GetMapIndex() == ch2->GetMapIndex();
}

// 금슬 수치
int TMarriage::GetBonus(uint32_t dwItemVnum, bool bShare, CHARACTER *me)
{
    if (!is_married)
        return 0;

    // 주변에 없을때는 자기 기능만 적용된다.

    // 해당 아이템이 어떤 기능을 하는지 찾는다.
    int iFindedBonusIndex = 0;
    {
        for (iFindedBonusIndex = 0; iFindedBonusIndex < MAX_MARRIAGE_UNIQUE_ITEM; ++iFindedBonusIndex)
        {
            if (g_ItemBonus[iFindedBonusIndex].dwVnum == dwItemVnum)
                break;
        }

        if (iFindedBonusIndex == MAX_MARRIAGE_UNIQUE_ITEM)
            return 0;
    }

    if (bShare)
    {
        // 두명의 보너스를 합한다.
        int count = 0;
        if (nullptr != ch1 && ch1->IsEquipUniqueItem(dwItemVnum))
            count ++;
        if (nullptr != ch2 && ch2->IsEquipUniqueItem(dwItemVnum))
            count ++;

        const TMarriageItemBonusByGrade &rkBonus = g_ItemBonus[iFindedBonusIndex];

        if (count >= 1)
            return rkBonus.value[GetMarriageGrade()];
        return 0;
    }
    else
    {
        // 상대방 것만 계산
        int count = 0;
        if (me != ch1 && nullptr != ch1 && ch1->IsEquipUniqueItem(dwItemVnum))
            count ++;
        if (me != ch2 && nullptr != ch2 && ch2->IsEquipUniqueItem(dwItemVnum))
            count ++;

        const TMarriageItemBonusByGrade &rkBonus = g_ItemBonus[iFindedBonusIndex];

        if (count >= 1)
            return rkBonus.value[GetMarriageGrade()];
        return 0;
    }
}

void TMarriage::Login(CHARACTER *ch)
{
    if (ch->GetPlayerID() == m_pid1)
    {
        ch1 = ch;
        if (is_married)
            SendLoverInfo(ch1, name2, GetMarriagePoint());
    }
    else if (ch->GetPlayerID() == m_pid2)
    {
        ch2 = ch;
        if (is_married)
            SendLoverInfo(ch2, name1, GetMarriagePoint());
    }

    // 둘 다 이 프로세스에 로그인 중이면 포인터를 연결하고 이벤트 발생
    if (IsOnline())
    {
        ch1->SetMarryPartner(ch2);
        ch2->SetMarryPartner(ch1);

        StartNearCheckEvent();
    }

    // 둘 다 로그인 되어 있다면 패킷을 보낸다.
    if (is_married)
    {    	
		bool m1_online = ch1 != nullptr;
		bool m2_online = ch2 != nullptr;

		OnlinePlayer* op;

		if (!m1_online) {
			if (op = DESC_MANAGER::instance().GetOnlinePlayers().Get(m_pid1); op) {
				m1_online = true;
			}
		}

		if (!m2_online) {
			if (op = DESC_MANAGER::instance().GetOnlinePlayers().Get(m_pid2); op) {
				m2_online = true;
			}
		}

		if (ch1 && ch2) {
			ch1->ChatPacket(CHAT_TYPE_COMMAND, "lover_login");
			ch2->ChatPacket(CHAT_TYPE_COMMAND, "lover_login");
			// sys_log(0, "lover_login %u %u", m_pid1, m_pid2);
		} else {
			if (ch1 && m2_online) {
				ch1->ChatPacket(CHAT_TYPE_COMMAND, "lover_login");
				RelayChatPacket(m_pid2, CHAT_TYPE_COMMAND, "lover_login");

			} else if (ch2 && m1_online) {
				ch2->ChatPacket(CHAT_TYPE_COMMAND, "lover_login");
				RelayChatPacket(m_pid1, CHAT_TYPE_COMMAND, "lover_login");
			}
		}
    	
        SPDLOG_TRACE("lover_login {0} {1}", m_pid1, m_pid2);
    }
}

void TMarriage::Logout(uint32_t pid)
{
    if (pid == m_pid1)
        ch1 = nullptr;
    else if (pid == m_pid2)
        ch2 = nullptr;

    if (ch1 || ch2)
    {
        Save();

        if (ch1)
            ch1->SetMarryPartner(nullptr);

        if (ch2)
            ch2->SetMarryPartner(nullptr);

        StopNearCheckEvent();
    }

    if (is_married)
    {
        if (ch1)
            ch1->ChatPacket(CHAT_TYPE_COMMAND, "lover_logout");
        else
            RelayChatPacket(m_pid1, CHAT_TYPE_COMMAND, "lover_logout");

        if (ch2)
            ch2->ChatPacket(CHAT_TYPE_COMMAND, "lover_logout");
        else
            RelayChatPacket(m_pid2, CHAT_TYPE_COMMAND, "lover_logout");

        SPDLOG_TRACE("lover_logout {0} {1}", m_pid1, m_pid2);
    }
}

void TMarriage::NearCheck()
{
    if (!is_married)
        return;

    if (!IsOnline())
    {
        StopNearCheckEvent();
        return;
    }
    SPDLOG_INFO("NearCheck %u %u %d %d %d", m_pid1, m_pid2, IsNear(), isLastNear, byLastLovePoint,
                GetMarriagePoint());

    if (IsNear() && !isLastNear)
    {
        isLastNear = true;
        ch1->ChatPacket(CHAT_TYPE_COMMAND, "lover_near");
        ch2->ChatPacket(CHAT_TYPE_COMMAND, "lover_near");
    }
    else if (!IsNear() && isLastNear)
    {
        isLastNear = false;
        ch1->ChatPacket(CHAT_TYPE_COMMAND, "lover_far");
        ch2->ChatPacket(CHAT_TYPE_COMMAND, "lover_far");
    }

    if (byLastLovePoint != GetMarriagePoint())
    {
        byLastLovePoint = GetMarriagePoint();
        TPacketGCLovePointUpdate p;
        p.love_point = byLastLovePoint;
       
        ch1->GetDesc()->Send(HEADER_GC_LOVE_POINT_UPDATE, p);
        ch2->GetDesc()->Send(HEADER_GC_LOVE_POINT_UPDATE, p);
    }
}

EVENTINFO(near_check_event_info)
{
    TMarriage *pMarriage;

    near_check_event_info()
        : pMarriage(nullptr)
    {
    }
};

EVENTFUNC(near_check_event)
{
    near_check_event_info *info = static_cast<near_check_event_info *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("near_check_event> <Factor> Null pointer");
        return 0;
    }

    TMarriage *pMarriage = info->pMarriage;
    pMarriage->NearCheck();
    return THECORE_SECS_TO_PASSES(5);
}

void TMarriage::StartNearCheckEvent()
{
    StopNearCheckEvent();

    near_check_event_info *info = AllocEventInfo<near_check_event_info>();
    info->pMarriage = this;
    eventNearCheck = event_create(near_check_event, info, 1);
}

void TMarriage::StopNearCheckEvent()
{
    byLastLovePoint = 0;
    isLastNear = false;
    event_cancel(&eventNearCheck);
}

void TMarriage::Save()
{
    SPDLOG_INFO("TMarriage::Save() - RequestUpdate.bSave=%d", bSave);
    if (bSave)
    {
        CManager::instance().RequestUpdate(m_pid1, m_pid2, love_point, is_married);
        bSave = false;
    }
}

void TMarriage::SetMarried()
{
    is_married = 1;
    bSave = true;
    Save();

    if (IsOnline())
    {
        SendLoverInfo(ch1, name2, GetMarriagePoint());
        SendLoverInfo(ch2, name1, GetMarriagePoint());

        ch1->ChatPacket(CHAT_TYPE_COMMAND, "lover_login");
        ch2->ChatPacket(CHAT_TYPE_COMMAND, "lover_login");
    }
}

void TMarriage::Update(uint32_t point)
{
    if (!IsOnline())
        return;

    if (point > 0 && is_married)
    {
        bSave = true;
        love_point += point;

        love_point = std::min(love_point, 2000000000);

        if (gConfig.testServer)
        {
            auto ch = g_pCharManager->FindByPID(m_pid1);
            if (ch)
                ch->ChatPacket(CHAT_TYPE_PARTY, "lovepoint bykill %.3g total %d", love_point / 1000000.,
                               GetMarriagePoint());

            ch = g_pCharManager->FindByPID(m_pid2);
            if (ch)
                ch->ChatPacket(CHAT_TYPE_PARTY, "lovepoint bykill %.3g total %d", love_point / 1000000.,
                               GetMarriagePoint());
        }
    }
}

void TMarriage::WarpToWeddingMap(uint32_t dwPID)
{
    if (!pWeddingInfo)
        return;

    CHARACTER *ch = g_pCharManager->FindByPID(dwPID);
    if (ch)
    {
        PIXEL_POSITION pos;
        if (!SECTREE_MANAGER::instance().GetRecallPositionByEmpire(pWeddingInfo->dwMapIndex / 10000, 0, pos))
        {
            SPDLOG_ERROR("cannot get warp position");
            return;
        }
        ch->SaveExitLocation();
        ch->WarpSet(pWeddingInfo->dwMapIndex, pos.y, pos.x);
    }
}

void TMarriage::RequestEndWedding()
{
    if (!pWeddingInfo)
        return;
    CManager::instance().RequestEndWedding(m_pid1, m_pid2);
}

CManager::CManager()
{
}

CManager::~CManager()
{
}

bool CManager::IsMarriageUniqueItem(uint32_t dwItemVnum)
{
    for (int i = 0; i < MAX_MARRIAGE_UNIQUE_ITEM; i++)
    {
        if (g_ItemBonus[i].dwVnum == dwItemVnum)
            return true;
    }
    return false;
}

bool CManager::IsMarried(uint32_t dwPlayerID)
{
    TMarriage *pkMarriageFinded = Get(dwPlayerID);
    if (pkMarriageFinded && pkMarriageFinded->is_married)
        return true;

    return false;
}

bool CManager::IsEngaged(uint32_t dwPlayerID)
{
    TMarriage *pkMarriageFinded = Get(dwPlayerID);
    if (pkMarriageFinded && !pkMarriageFinded->is_married)
        return true;

    return false;
}

bool CManager::IsEngagedOrMarried(uint32_t dwPlayerID) { return Get(dwPlayerID) != nullptr; }

bool CManager::Initialize() { return true; }

void CManager::Destroy()
{
}

void Align(uint32_t &dwPID1, uint32_t &dwPID2)
{
    if (dwPID1 > dwPID2)
        std::swap(dwPID1, dwPID2);
}

TMarriage *CManager::Get(uint32_t dwPlayerID)
{
    auto it = m_MarriageByPID.find(dwPlayerID);

    if (it != m_MarriageByPID.end())
        return it->second;

    return nullptr;
}

void CManager::RequestAdd(uint32_t dwPID1, uint32_t dwPID2, const char *szName1, const char *szName2)
{
    if (dwPID1 > dwPID2)
    {
        std::swap(dwPID1, dwPID2);
        std::swap(szName1, szName2);
    }

    TPacketMarriageAdd p;

    p.dwPID1 = dwPID1;
    p.dwPID2 = dwPID2;
    strlcpy(p.szName1, szName1, sizeof(p.szName1));
    strlcpy(p.szName2, szName2, sizeof(p.szName2));
    db_clientdesc->DBPacket(HEADER_GD_MARRIAGE_ADD, 0, &p, sizeof(p));
}

void CManager::Add(uint32_t dwPID1, uint32_t dwPID2, time_t tMarryTime, const char *szName1, const char *szName2)
{
    if (IsEngagedOrMarried(dwPID1) || IsEngagedOrMarried(dwPID2))
    {
        SPDLOG_ERROR("cannot marry already married character. %d - %d", dwPID1, dwPID2);
        return;
    }

    if (dwPID1 > dwPID2)
    {
        std::swap(dwPID1, dwPID2);
        std::swap(szName1, szName2);
    }

    TMarriage *pMarriage = new TMarriage(dwPID1, dwPID2, 0, tMarryTime, szName1, szName2);
    m_Marriages.insert(pMarriage);
    m_MarriageByPID.insert(make_pair(dwPID1, pMarriage));
    m_MarriageByPID.insert(make_pair(dwPID2, pMarriage));
    {
        CHARACTER *A = g_pCharManager->FindByPID(dwPID1);
        CHARACTER *B = g_pCharManager->FindByPID(dwPID2);

        if (A && B)
        {
            // 웨딩 맵 요청을 보낸다
            TPacketWeddingRequest p;
            p.dwPID1 = dwPID1;
            p.dwPID2 = dwPID2;
            db_clientdesc->DBPacket(HEADER_GD_WEDDING_REQUEST, 0, &p, sizeof(p));
        }
    }
}

void CManager::RequestUpdate(uint32_t dwPID1, uint32_t dwPID2, int iUpdatePoint, uint8_t byMarried)
{
    Align(dwPID1, dwPID2);

    TPacketMarriageUpdate p;
    p.dwPID1 = dwPID1;
    p.dwPID2 = dwPID2;
    p.iLovePoint = iUpdatePoint;
    p.byMarried = byMarried;
    db_clientdesc->DBPacket(HEADER_GD_MARRIAGE_UPDATE, 0, &p, sizeof(p));
}

void CManager::Update(uint32_t dwPID1, uint32_t dwPID2, long lTotalPoint, uint8_t byMarried)
{
    TMarriage *pMarriage = Get(dwPID1);

    if (!pMarriage || pMarriage->GetOther(dwPID1) != dwPID2)
    {
        SPDLOG_ERROR("not under marriage : %u %u", dwPID1, dwPID2);
        return;
    }

    pMarriage->love_point = lTotalPoint;
    pMarriage->is_married = byMarried;
}

void CManager::RequestRemove(uint32_t dwPID1, uint32_t dwPID2)
{
    Align(dwPID1, dwPID2);

    TPacketMarriageRemove p;
    p.dwPID1 = dwPID1;
    p.dwPID2 = dwPID2;
    db_clientdesc->DBPacket(HEADER_GD_MARRIAGE_REMOVE, 0, &p, sizeof(p));
}

void CManager::Remove(uint32_t dwPID1, uint32_t dwPID2)
{
    TMarriage *pMarriage = Get(dwPID1);
    if (!pMarriage || pMarriage->GetOther(dwPID1) != dwPID2)
    {
        SPDLOG_ERROR("not under marriage : %u %u", dwPID1, dwPID2);
        return;
    }

    m_Marriages.erase(pMarriage);
    m_MarriageByPID.erase(dwPID1);
    m_MarriageByPID.erase(dwPID2);

    delete (pMarriage);
}

void CManager::Login(CHARACTER *ch)
{
    uint32_t pid = ch->GetPlayerID();

    TMarriage *pMarriage = Get(pid);
    if (!pMarriage)
        return;

    pMarriage->Login(ch);
}

void CManager::Logout(uint32_t pid)
{
    TMarriage *pMarriage = Get(pid);

    if (!pMarriage)
        return;

    pMarriage->Logout(pid);
}

void CManager::Logout(CHARACTER *ch) { Logout(ch->GetPlayerID()); }

void CManager::WeddingReady(uint32_t dwPID1, uint32_t dwPID2, uint32_t dwMapIndex)
{
    TMarriage *pMarriage = Get(dwPID1);
    if (!pMarriage || pMarriage->GetOther(dwPID1) != dwPID2)
    {
        SPDLOG_ERROR("wrong marriage %u, %u", dwPID1, dwPID2);
        return;
    }

    TWeddingInfo *pwi;
    if (pMarriage->pWeddingInfo)
        pwi = pMarriage->pWeddingInfo;
    else
    {
        pwi = new TWeddingInfo;
        pMarriage->pWeddingInfo = pwi;
    }

    pwi->dwMapIndex = dwMapIndex;
}

void CManager::WeddingStart(uint32_t dwPID1, uint32_t dwPID2)
{
    TMarriage *pMarriage = Get(dwPID1);
    if (!pMarriage || pMarriage->GetOther(dwPID1) != dwPID2)
    {
        SPDLOG_ERROR("wrong marriage %u, %u", dwPID1, dwPID2);
        return;
    }

    TWeddingInfo *pwi = pMarriage->pWeddingInfo;

    if (!pwi)
        return;

    // 결혼자들을 워프시켜야함
    pMarriage->WarpToWeddingMap(dwPID1);
    pMarriage->WarpToWeddingMap(dwPID2);

    // 등록해서 메뉴창에서 이름나와야함
    m_setWedding.insert(make_pair(dwPID1, dwPID2));
}

void CManager::WeddingEnd(uint32_t dwPID1, uint32_t dwPID2)
{
    TMarriage *pMarriage = Get(dwPID1);
    if (!pMarriage || pMarriage->GetOther(dwPID1) != dwPID2)
    {
        SPDLOG_ERROR("wrong marriage %u, %u", dwPID1, dwPID2);
        return;
    }

    if (!pMarriage->pWeddingInfo)
    {
        SPDLOG_ERROR("not under wedding %u, %u", dwPID1, dwPID2);
        return;
    }

    // 맵에서 빼내야합니다
    if (gConfig.IsHostingMap(WEDDING_MAP_INDEX))
        if (!WeddingManager::instance().End(pMarriage->pWeddingInfo->dwMapIndex))
        {
            SPDLOG_ERROR("wedding map error: map_index=%d", pMarriage->pWeddingInfo->dwMapIndex);
            return;
        }

    delete (pMarriage->pWeddingInfo);
    pMarriage->pWeddingInfo = nullptr;

    m_setWedding.erase(make_pair(dwPID1, dwPID2));
}

void CManager::RequestEndWedding(uint32_t dwPID1, uint32_t dwPID2)
{
    TPacketWeddingEnd p;
    p.dwPID1 = dwPID1;
    p.dwPID2 = dwPID2;

    db_clientdesc->DBPacket(HEADER_GD_WEDDING_END, 0, &p, sizeof(p));
}
}
