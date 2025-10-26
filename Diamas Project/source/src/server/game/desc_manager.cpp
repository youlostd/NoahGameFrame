#include "desc_manager.h"
#include "char.h"
#include "desc.h"
#include "DbCacheSocket.hpp"
#include "messenger_manager.h"

namespace
{
uint32_t CreateHandshake() { return Random::get<uint32_t>(); }
} // namespace

DESC_MANAGER::DESC_MANAGER(asio::io_service &ioService)
    : m_acceptor(ioService), m_currentHandle(0), m_iLocalUserCount(0), m_aiEmpireUserCount{},
      m_masterSocket(std::make_shared<MasterSocket>(ioService))
{
}

void DESC_MANAGER::Setup(std::string_view addr, std::string_view port)
{
    boost::system::error_code ec;
    m_acceptor.Bind(addr, port, ec, [this](asio::ip::tcp::socket socket) {
        SPDLOG_INFO("Accepted connection on {0}", socket);
        Accept(std::move(socket));
    });
}

void DESC_MANAGER::Quit()
{
    // TODO(tim): This isn't very efficient..
    auto setCopy = m_set_pkDesc;
    for (const auto &p : setCopy)
    {
        p->SetDisconnectReason("Shutdown");
        p->Disconnect();
    }

    m_masterSocket->Disconnect();
    m_acceptor.Close();
}

void DESC_MANAGER::ConnectMaster(const std::string &addr, const std::string &port)
{
    m_masterSocket->Connect(addr, port);
}

void DESC_MANAGER::ConnectAccount(uint32_t aid, DESC *d)
{
    m_loggedIn.insert(std::make_pair(aid, d));
    m_loggedInHwid.emplace(d->GetHWIDHash(), d);
}

void DESC_MANAGER::DisconnectAccount(uint32_t aid) { m_loggedIn.erase(aid); }

void DESC_MANAGER::DisconnectAccountDesc(DESC *d)
{
    for (auto it = m_loggedInHwid.begin(); it != m_loggedInHwid.end();)
    {
        if (it->first == d->GetHWIDHash() && it->second == d)
            it = m_loggedInHwid.erase(it);
        else
            ++it;
    }
}

void DESC_MANAGER::DestroyDesc(DESC *d, bool bEraseFromSet)
{
    if (bEraseFromSet)
        m_set_pkDesc.erase(std::static_pointer_cast<DESC>(d->shared_from_this()));

    if (d->GetHandle() != 0)
        m_map_handle.erase(d->GetHandle());
}

DESC *DESC_MANAGER::FindByAid(uint32_t aid)
{
    const auto it = m_loggedIn.find(aid);
    if (m_loggedIn.end() == it)
        return nullptr;

    return it->second;
}

std::vector<DESC *> DESC_MANAGER::FindByHwid(const std::string &hwid)
{
    std::vector<DESC *> v;
    auto range = m_loggedInHwid.equal_range(hwid);
    for (auto it = range.first; it != range.second; ++it)
    {
        v.emplace_back(it->second);
    }
    return v;
}

DESC *DESC_MANAGER::FindByHandle(uint32_t handle)
{
    auto it = m_map_handle.find(handle);
    if (m_map_handle.end() == it)
        return nullptr;

    return it->second;
}

const DESC_MANAGER::DESC_SET &DESC_MANAGER::GetClientSet() const { return m_set_pkDesc; }

void DESC_MANAGER::UpdateLocalUserCount()
{
    m_iLocalUserCount = 0;
    std::fill(std::begin(m_aiEmpireUserCount), std::end(m_aiEmpireUserCount), 0);

    for (const auto &d : m_set_pkDesc)
    {
        if (d->GetCharacter())
        {
            ++m_iLocalUserCount;
            ++m_aiEmpireUserCount[d->GetCharacter()->GetEmpire()];
        }
    }

    m_aiEmpireUserCount[1] += m_onlinePlayers.GetEmpireUserCount(1);
    m_aiEmpireUserCount[2] += m_onlinePlayers.GetEmpireUserCount(2);
    m_aiEmpireUserCount[3] += m_onlinePlayers.GetEmpireUserCount(3);
}

void DESC_MANAGER::GetUserCount(int &iTotal, int **paiEmpireUserCount, int &iLocalCount)
{
    *paiEmpireUserCount = &m_aiEmpireUserCount[0];

    iTotal = m_iLocalUserCount + m_onlinePlayers.GetUserCount();
    iLocalCount = m_iLocalUserCount;
}

void DESC_MANAGER::Accept(asio::ip::tcp::socket socket)
{
    // Seems like the connection was already closed just ignore it
    if (!socket.is_open())
        return;

    const auto handle = ++m_currentHandle;

    auto desc = std::make_shared<DESC>(std::move(socket), handle, CreateHandshake());

    if (!desc->GetSocket().is_open()) {
        --m_currentHandle;
        return;
    }

    desc->Setup();

    if (!desc->GetSocket().is_open()) {
        --m_currentHandle;
        return;
    }

    m_set_pkDesc.insert(desc);
    m_map_handle.emplace(handle, desc.get());
}
