#include "constants.h"
#include "priv_manager.h"
#include "char.h"
#include "DbCacheSocket.hpp"
#include "guild.h"
#include "guild_manager.h"
#include "utils.h"
#include "log.h"
#include "ChatUtil.hpp"

static const char *GetEmpireName(int priv) { return LC_TEXT(c_apszEmpireNames[priv]); }

static const char *GetPrivName(int priv) { return LC_TEXT(c_apszPrivNames[priv]); }

CPrivManager::CPrivManager() : m_aakPrivEmpireData{} {  }

void CPrivManager::RequestGiveGuildPriv(uint32_t guild_id, uint8_t type, int value, uint32_t duration_sec)
{
    if (MAX_PRIV_NUM <= type)
    {
        SPDLOG_ERROR("PRIV_MANAGER: RequestGiveGuildPriv: wrong guild priv type(%u)", type);
        return;
    }

    value = std::clamp(value, 0, 50);
    duration_sec = std::clamp<uint32_t>(duration_sec, 0, 60 * 60 * 24 * 7);

    TPacketGiveGuildPriv p;
    p.type = type;
    p.value = value;
    p.guild_id = guild_id;
    p.duration_sec = duration_sec;

    db_clientdesc->DBPacket(HEADER_GD_REQUEST_GUILD_PRIV, 0, &p, sizeof(p));
}

void CPrivManager::RequestGiveEmpirePriv(uint8_t empire, uint8_t type, int value, uint32_t duration_sec)
{
    if (MAX_PRIV_NUM <= type)
    {
        SPDLOG_ERROR("PRIV_MANAGER: RequestGiveEmpirePriv: wrong empire priv type(%u)", type);
        return;
    }

    value = std::clamp(value, 0, 500);
    duration_sec = std::clamp<int>(duration_sec, 0, 60 * 60 * 24 * 7);

    TPacketGiveEmpirePriv p;
    p.type = type;
    p.value = value;
    p.empire = empire;
    p.duration_sec = duration_sec;

    db_clientdesc->DBPacket(HEADER_GD_REQUEST_EMPIRE_PRIV, 0, &p, sizeof(p));
}

void CPrivManager::RequestGiveCharacterPriv(uint32_t pid, uint8_t type, int value)
{
    if (MAX_PRIV_NUM <= type)
    {
        SPDLOG_ERROR("PRIV_MANAGER: RequestGiveCharacterPriv: wrong char priv type(%u)", type);
        return;
    }

    value = std::clamp(value, 0, 100);

    TPacketGiveCharacterPriv p;
    p.type = type;
    p.value = value;
    p.pid = pid;

    db_clientdesc->DBPacket(HEADER_GD_REQUEST_CHARACTER_PRIV, 0, &p, sizeof(p));
}

void CPrivManager::GiveGuildPriv(uint32_t guild_id, uint8_t type, int value, uint8_t bLog, uint32_t end_time_sec)
{
    if (MAX_PRIV_NUM <= type)
    {
        SPDLOG_ERROR("PRIV_MANAGER: GiveGuildPriv: wrong guild priv type(%u)", type);
        return;
    }

    SPDLOG_INFO("Set Guild Priv: guild_id(%u) type(%d) value(%d) duration_sec(%d)", guild_id, type, value,
                end_time_sec - get_global_time());

    value = std::clamp(value, 0, 50);
    end_time_sec = std::clamp<time_t>(end_time_sec, 0, get_global_time() + 60 * 60 * 24 * 7);

    m_aPrivGuild[type][guild_id].value = value;
    m_aPrivGuild[type][guild_id].end_time_sec = end_time_sec;

    CGuild *g = CGuildManager::instance().FindGuild(guild_id);

    if (g)
    {
        if (value) { SendI18nNotice("%s 길드의 %s이 %d%% 증가했습니다!", g->GetName(), GetPrivName(type), value); }
        else { SendI18nNotice("%s 길드의 %s이 정상으로 돌아왔습니다.", g->GetName(), GetPrivName(type)); }

        if (bLog) { LogManager::instance().CharLog(0, guild_id, type, value, "GUILD_PRIV", "", ""); }
    }
}

void CPrivManager::GiveCharacterPriv(uint32_t pid, uint8_t type, int value, uint8_t bLog)
{
    if (MAX_PRIV_NUM <= type)
    {
        SPDLOG_ERROR("PRIV_MANAGER: GiveCharacterPriv: wrong char priv type(%u)", type);
        return;
    }

    SPDLOG_INFO("Set Character Priv {} {} {}", pid, type, value);

    value = std::clamp(value, 0, 100);

    m_aPrivChar[type][pid] = value;

    if (bLog)
        LogManager::instance().CharLog(pid, 0, type, value, "CHARACTER_PRIV", "", "");
}

void CPrivManager::GiveEmpirePriv(uint8_t empire, uint8_t type, int value, uint8_t bLog, uint32_t end_time_sec)
{
	if (MAX_PRIV_NUM <= type) {
		SPDLOG_ERROR("PRIV_MANAGER: GiveEmpirePriv: wrong empire priv type(%u)",
		             type);
		return;
	}

	SPDLOG_INFO("Set Empire Priv: empire(%d) type(%d) value(%d) "
	            "duration_sec(%d)",
	            empire, type, value, end_time_sec - get_global_time());

	value = std::clamp(value, 0, 500);
	end_time_sec = std::clamp<time_t>(end_time_sec, 0,
	                                  get_global_time() + 60 * 60 * 24 * 7);

	SPrivEmpireData& rkPrivEmpireData = m_aakPrivEmpireData[type][empire];
	rkPrivEmpireData.m_value = value;
	rkPrivEmpireData.m_end_time_sec = end_time_sec;

    if(type != PRIV_DOUBLE_DROP_COUNT) {
	    if (value) {
		    char buf[100];
		    std::snprintf(buf, sizeof(buf), LC_TEXT("%s의 %s이 %d%% 증가했습니다!"),
		                  GetEmpireName(empire), GetPrivName(type), value);

		    if (empire)
			    SendI18nNotice("%s의 %s이 %d%% 증가했습니다!",
			                   GetEmpireName(empire), GetPrivName(type), value);
		    else
			    SendLog(buf);
	    } else {
		    char buf[100];
		    std::snprintf(buf, sizeof(buf),
		                  LC_TEXT("%s의 %s이 정상으로 돌아왔습니다."),
		                  GetEmpireName(empire), GetPrivName(type));

		    if (empire)
			    SendI18nNotice("%s의 %s이 정상으로 돌아왔습니다.",
			                   GetEmpireName(empire), GetPrivName(type));
		    else
			    SendLog(buf);
	    }
    } else {
	    if (value) {
			SendI18nNotice("%s: Double-Loot Event is active",
			                   GetEmpireName(empire), value);
	
	    } else {
            SendI18nNotice("%s: Double-Loot Event ended",
			                   GetEmpireName(empire));

	    }
    }


	if (!empire && type == PRIV_EXP_PCT) {
		if (value < 50)
			SendCommandChat("exp_bonus_event_start 0 %d %d", value, end_time_sec);
		else
			SendCommandChat("exp_bonus_event_start 1 %d %d", value, end_time_sec);
	}

    if (bLog) { LogManager::instance().CharLog(0, empire, type, value, "EMPIRE_PRIV", "", ""); }
}

void CPrivManager::RemoveGuildPriv(uint32_t guild_id, uint8_t type)
{
    if (MAX_PRIV_NUM <= type)
    {
        SPDLOG_ERROR("PRIV_MANAGER: RemoveGuildPriv: wrong guild priv type(%u)", type);
        return;
    }

    m_aPrivGuild[type][guild_id].value = 0;
    m_aPrivGuild[type][guild_id].end_time_sec = 0;
}

void CPrivManager::RemoveEmpirePriv(uint8_t empire, uint8_t type)
{
    if (MAX_PRIV_NUM <= type)
    {
        SPDLOG_ERROR("PRIV_MANAGER: RemoveEmpirePriv: wrong empire priv type(%u)", type);
        return;
    }

    SPrivEmpireData &rkPrivEmpireData = m_aakPrivEmpireData[type][empire];
    rkPrivEmpireData.m_value = 0;
    rkPrivEmpireData.m_end_time_sec = 0;
}

void CPrivManager::RemoveCharacterPriv(uint32_t pid, uint8_t type)
{
    if (MAX_PRIV_NUM <= type)
    {
        SPDLOG_ERROR("PRIV_MANAGER: RemoveCharacterPriv: wrong char priv type(%u)", type);
        return;
    }

    auto it = m_aPrivChar[type].find(pid);

    if (it != m_aPrivChar[type].end())
        m_aPrivChar[type].erase(it);
}

int CPrivManager::GetPriv(CHARACTER *ch, uint8_t type)
{
    // 캐릭터의 변경 수치가 -라면 무조건 -만 적용되게
    int val_ch = GetPrivByCharacter(ch->GetPlayerID(), type);

    if (val_ch < 0 && !ch->IsEquipUniqueItem(UNIQUE_ITEM_NO_BAD_LUCK_EFFECT))
        return val_ch;
    else
    {
        int val;

        // 개인, 제국, 길드, 전체 중 큰 값을 취한다.
        val = std::max<int>(val_ch, GetPrivByEmpire(0, type));
        val = std::max<int>(val, GetPrivByEmpire(ch->GetEmpire(), type));

        if (ch->GetGuild())
            val = std::max<int>(val, GetPrivByGuild(ch->GetGuild()->GetID(), type));

        return val;
    }
}

int CPrivManager::GetPrivByEmpire(uint8_t bEmpire, uint8_t type)
{
    SPrivEmpireData *pkPrivEmpireData = GetPrivByEmpireEx(bEmpire, type);

    if (pkPrivEmpireData)
        return pkPrivEmpireData->m_value;

    return 0;
}

int CPrivManager::GetPrivEndByEmpire(uint8_t bEmpire, uint8_t type)
{
    SPrivEmpireData *pkPrivEmpireData = GetPrivByEmpireEx(bEmpire, type);

    if (pkPrivEmpireData)
        return pkPrivEmpireData->m_end_time_sec;

    return 0;
}

CPrivManager::SPrivEmpireData *CPrivManager::GetPrivByEmpireEx(uint8_t bEmpire, uint8_t type)
{
    if (type >= MAX_PRIV_NUM)
        return nullptr;

    if (bEmpire >= EMPIRE_MAX_NUM)
        return nullptr;

    return &m_aakPrivEmpireData[type][bEmpire];
}

int CPrivManager::GetPrivByGuild(uint32_t guild_id, uint8_t type)
{
    if (type >= MAX_PRIV_NUM)
        return 0;

    auto itFind = m_aPrivGuild[type].find(guild_id);

    if (itFind == m_aPrivGuild[type].end())
        return 0;

    return itFind->second.value;
}

const CPrivManager::SPrivGuildData *CPrivManager::GetPrivByGuildEx(uint32_t dwGuildID, uint8_t byType) const
{
    if (byType >= MAX_PRIV_NUM)
        return nullptr;

    auto itFind = m_aPrivGuild[byType].find(dwGuildID);

    if (itFind == m_aPrivGuild[byType].end())
        return nullptr;

    return &itFind->second;
}

int CPrivManager::GetPrivByCharacter(uint32_t pid, uint8_t type)
{
    if (type >= MAX_PRIV_NUM)
        return 0;

    auto it = m_aPrivChar[type].find(pid);

    if (it != m_aPrivChar[type].end())
        return it->second;

    return 0;
}
