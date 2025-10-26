#ifndef METIN2_SERVER_GAME_DESC_MANAGER_H
#define METIN2_SERVER_GAME_DESC_MANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <unordered_map>
#include <unordered_set>

#include <base/Singleton.hpp>
#include <net/Acceptor.hpp>
#include "MasterSocket.hpp"
#include "OnlinePlayers.hpp"

#include <base/Singleton.hpp>
#include <base/robin_hood.h>

class DESC;
class CLIENT_DESC;
class MasterSocket;

class DESC_MANAGER : public singleton<DESC_MANAGER>
{
public:
    typedef robin_hood::unordered_set<std::shared_ptr<DESC>> DESC_SET;
    typedef robin_hood::unordered_map<uint32_t, DESC *> DESC_HANDLE_MAP;

    DESC_MANAGER(asio::io_service &ioService);

    void Setup(std::string_view addr, std::string_view port);
    void Quit();

    void ConnectMaster(const std::string &addr,
                       const std::string &port);

    void DestroyDesc(DESC *d, bool erase_from_set = true);

    DESC *FindByHandle(uint32_t handle);
    const DESC_SET &GetClientSet() const;

    DESC *             FindByAid(uint32_t aid);
    std::vector<DESC*> FindByHwid(const std::string& hwid);
    void               ConnectAccount(uint32_t aid, DESC * d);
    void               DisconnectAccount(uint32_t aid);
    void               DisconnectAccountDesc(DESC* d);

    void UpdateLocalUserCount();

    uint32_t GetLocalUserCount() { return m_iLocalUserCount; }

    void GetUserCount(int &iTotal, int **paiEmpireUserCount, int &iLocalCount);

    MasterSocket *GetMasterSocket() { return m_masterSocket.get(); }

    OnlinePlayers &GetOnlinePlayers() { return m_onlinePlayers; }



private:
    void Accept(asio::ip::tcp::socket socket);

    Acceptor m_acceptor;

    DESC_SET m_set_pkDesc;

    DESC_HANDLE_MAP m_map_handle;
    uint32_t m_currentHandle;

    std::unordered_map<uint32_t, DESC *> m_loggedIn;
    std::unordered_multimap<std::string, DESC *> m_loggedInHwid;

    int m_iLocalUserCount;
    int m_aiEmpireUserCount[EMPIRE_MAX_NUM];

    std::shared_ptr<MasterSocket> m_masterSocket;

    OnlinePlayers m_onlinePlayers;
};

#endif /* METIN2_SERVER_GAME_DESC_MANAGER_H */
