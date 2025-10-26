#include "OnlinePlayers.hpp"
#include "Server.hpp"
#include "GameSocket.hpp"

#include <game/MasterPackets.hpp>

#include <net/Util.hpp>

OnlinePlayers::OnlinePlayers(Server &server) : m_server(server)
{
    // ctor
}

void OnlinePlayers::Enter(OnlinePlayer player)
{
    // Copy the PID before we move |player|
    const auto pid = player.pid;

    const auto res = m_players.emplace(pid, std::make_unique<OnlinePlayer>(std::move(player)));

    if (!res.second)
    {
        SPDLOG_ERROR("Player {0} is already registered", pid);
        return;
    }

    auto &newPlayer = res.first->second;

    const auto res2 = m_playersByAid.emplace(newPlayer->aid, newPlayer.get());
    if (!res2.second)
    {
        SPDLOG_ERROR("Account {0} (player {1}) is already registered", newPlayer->aid, newPlayer->pid);
    }

    const auto res3 = m_playersByName.emplace(newPlayer->name, newPlayer.get());
    if (!res3.second)
    {
        SPDLOG_ERROR("Player {0} ({1}) is already registered", newPlayer->name, newPlayer->pid);
    }
    auto& it = m_playersByHwid[newPlayer->hwid];
    it.insert(newPlayer->aid);

    StopExpiringMultiFarm(newPlayer->aid, newPlayer->hwid);

    UpdateDropStatus(newPlayer->hwid);

    BroadcastEnter(newPlayer.get());
}

void OnlinePlayers::StartExpiringMultiFarm(uint32_t aid, const std::string& hwid)
{
    m_expiringMultiFarmBlock.emplace_back(std::make_pair(hwid, aid));
}

void OnlinePlayers::StopExpiringMultiFarm(uint32_t aid, const std::string& hwid)
{
	m_expiringMultiFarmBlock.erase(std::remove_if(m_expiringMultiFarmBlock.begin(), m_expiringMultiFarmBlock.end(), [aid, hwid] (const std::pair<std::string, uint32_t> aid2) {
		return aid2.first == hwid && aid2.second == aid;
	}), m_expiringMultiFarmBlock.end());


}

void OnlinePlayers::Update()
{
    auto it = m_expiringMultiFarmBlock.begin();

    while (it != m_expiringMultiFarmBlock.end())
    {
        auto wit = m_playersByHwid.find(it->first);
        if (wit != m_playersByHwid.end())
        {

            auto &it2 = wit->second;
            it2.erase(it->second);
            it = m_expiringMultiFarmBlock.erase(it);
        }
        else
        {
            ++it;
        }
    }

   /* for(const auto& v : m_playersByHwid)
         UpdateDropStatus(v.first);*/
}
           


void OnlinePlayers::UpdateDropStatus(std::string hwid)
{
    auto v = GetByHwid(hwid);
    for (int i = 0; i < v.size(); ++i)
    {

        auto d2 = v[i];

        if (i == 0 || i == 1)
        {
            MgDropStatusPacket p{};
            p.aid = d2.second;
            p.state = 1;
            BroadcastPacket(m_server.GetGameSockets(), kMgDropStatus, p);
        }
        else
        {
            MgDropStatusPacket p{};
            p.aid = d2.second;
            p.state = 0;
            BroadcastPacket(m_server.GetGameSockets(), kMgDropStatus, p);
        }
    }
}

void OnlinePlayers::Leave(uint32_t pid)
{
    const auto it = m_players.find(pid);
    if (it == m_players.end())
    {
        SPDLOG_ERROR("Player {0} isn't registered", pid);
        return;
    }

    BroadcastLeave(it->second.get());

    m_playersByAid.erase(it->second->aid);
    m_playersByName.erase(it->second->name);
    StartExpiringMultiFarm(it->second->aid, it->second->hwid);
    auto hwid = it->second->hwid;
    m_players.erase(it);

    UpdateDropStatus(hwid);
}

void OnlinePlayers::RemoveAll(GameSocket *game)
{
    for (auto it = m_players.begin(); it != m_players.end();)
    {
        if (it->second->game != game)
        {
            ++it;
            continue;
        }

        m_playersByAid.erase(it->second->aid);
        m_playersByName.erase(it->second->name);
        auto hwid = it->second->hwid;
        UpdateDropStatus(hwid);

        it = m_players.erase(it);
    }
}

const OnlinePlayer *OnlinePlayers::Get(uint32_t pid) const { return const_cast<OnlinePlayers *>(this)->Get(pid); }

const OnlinePlayer *OnlinePlayers::Get(const std::string &name) const
{
    return const_cast<OnlinePlayers *>(this)->Get(name);
}

const OnlinePlayer *OnlinePlayers::GetByAid(uint32_t aid) const
{
    return const_cast<OnlinePlayers *>(this)->GetByAid(aid);
}

OnlinePlayer *OnlinePlayers::Get(uint32_t pid)
{
    const auto it = m_players.find(pid);
    if (it != m_players.end())
        return it->second.get();

    return nullptr;
}

OnlinePlayer *OnlinePlayers::Get(const std::string &name)
{
    const auto it = m_playersByName.find(name);
    if (it != m_playersByName.end())
        return it->second;

    return nullptr;
}

OnlinePlayer *OnlinePlayers::GetByAid(uint32_t aid)
{
    const auto it = m_playersByAid.find(aid);
    if (it != m_playersByAid.end())
        return it->second;

    return nullptr;
}

std::vector<std::pair<std::string, uint32_t>> OnlinePlayers::GetByHwid(const std::string& hwid)
{

    std::vector<std::pair<std::string, uint32_t>> v;
    const auto& it = m_playersByHwid.find(hwid);
    if(it == m_playersByHwid.end())
        return v;

    for(const auto& aid : it->second)
    {
        v.emplace_back(std::make_pair(hwid, aid));
    }

    return v;
}
void OnlinePlayers::BroadcastEnter(const OnlinePlayer *player)
{
    MgCharacterEnterPacket p;
    p.aid = player->aid;
    p.pid = player->pid;
    p.name = player->name;
    p.mapIndex = player->mapIndex;
    p.empire = player->empire;
    p.channel = player->channel;
    p.hwid = player->hwid;
    BroadcastPacket(m_server.GetGameSockets(), kMgCharacterEnter, p, player->game);
}

void OnlinePlayers::BroadcastLeave(const OnlinePlayer *player)
{
    MgCharacterLeavePacket p;
    p.pid = player->pid;
    BroadcastPacket(m_server.GetGameSockets(), kMgCharacterLeave, p, player->game);
}
