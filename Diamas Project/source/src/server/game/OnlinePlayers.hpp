#ifndef METIN2_SERVER_GAME_ONLINEPLAYERS_HPP
#define METIN2_SERVER_GAME_ONLINEPLAYERS_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/Constants.hpp>
#include <unordered_map>
#include <memory>

// Struct describing a player that currently plays on another game.
struct OnlinePlayer
{
    uint32_t aid;
    uint32_t pid;
    std::string name;
    uint32_t mapIndex;
    uint8_t empire;
    uint8_t channel;
    std::string hwid;
};

class OnlinePlayers
{
public:
    OnlinePlayers();

    void Enter(OnlinePlayer player);
    void Leave(uint32_t pid);

    const OnlinePlayer *Get(uint32_t pid) const;
    const OnlinePlayer *Get(const std::string &name) const;
    const OnlinePlayer *GetByAid(uint32_t aid) const;

    OnlinePlayer *Get(uint32_t pid);
    OnlinePlayer *Get(const std::string &name);
    OnlinePlayer *GetByAid(uint32_t aid);

    uint32_t GetUserCount() const;
    uint32_t GetEmpireUserCount(uint8_t empire) const;

private:
    std::unordered_map<uint32_t, std::unique_ptr<OnlinePlayer>> m_players;

    std::unordered_map<uint32_t, OnlinePlayer *> m_playersByAid;

    // key is owned by OnlinePlayer*, which is owned by |m_players|
    std::unordered_map<std::string, OnlinePlayer *> m_playersByName;

    uint32_t m_userCountByEmpire[EMPIRE_MAX_NUM];
};

#endif
