#include "OnlinePlayers.hpp"

#include <net/Util.hpp>

#include <numeric>
#include <algorithm>
#include <game/length.h>

OnlinePlayers::OnlinePlayers()
{
    std::fill(std::begin(m_userCountByEmpire),
              std::end(m_userCountByEmpire), 0);
}

void OnlinePlayers::Enter(OnlinePlayer player)
{
    // Copy the PID before we move |player|
    const auto pid = player.pid;

    const auto res = m_players.emplace(
        pid,
        std::make_unique<OnlinePlayer>(std::move(player)));

    if (!res.second)
    {
        SPDLOG_ERROR("Player {0} is already registered", pid);
        return;
    }

    auto &newPlayer = res.first->second;

    const auto res2 = m_playersByAid.emplace(newPlayer->aid, newPlayer.get());
    if (!res2.second)
    {
        SPDLOG_ERROR("Account {0} (player {1}) is already registered",
                     newPlayer->aid, newPlayer->pid);
    }

    const auto res3 = m_playersByName.emplace(newPlayer->name, newPlayer.get());
    if (!res3.second)
    {
        SPDLOG_ERROR("Player {0} ({1}) is already registered",
                     newPlayer->name, newPlayer->pid);
    }

    if (newPlayer->empire < EMPIRE_MAX_NUM)
        ++m_userCountByEmpire[newPlayer->empire];
}

void OnlinePlayers::Leave(uint32_t pid)
{
    const auto it = m_players.find(pid);
    if (it == m_players.end())
    {
        SPDLOG_ERROR("Player {0} isn't registered", pid);
        return;
    }

    if (it->second->empire < EMPIRE_MAX_NUM)
        --m_userCountByEmpire[it->second->empire];

    m_playersByAid.erase(it->second->aid);
    m_playersByName.erase(it->second->name);

    m_players.erase(it);
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

uint32_t OnlinePlayers::GetUserCount() const
{
    return std::accumulate(std::begin(m_userCountByEmpire),
                           std::end(m_userCountByEmpire),
                           static_cast<uint32_t>(0));
}

uint32_t OnlinePlayers::GetEmpireUserCount(uint8_t empire) const { return m_userCountByEmpire[empire]; }
