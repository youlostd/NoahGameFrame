#ifndef METIN2_SERVER_GAME_EXCHANGE_H
#define METIN2_SERVER_GAME_EXCHANGE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <boost/signals2/connection.hpp>

class CGrid;
class CHARACTER;

#include "ItemGrid.hpp"

enum EExchangeValues
{
    EXCHANGE_MAX_DISTANCE = 1000
};

class CExchange
{
public:
    CExchange(CHARACTER *pOwner);
    ~CExchange();

    bool Accept(bool bIsAccept = true);
    void Cancel();

    bool AddGold(Gold lGold);
    bool AddItem(TItemPos item_pos, uint8_t display_pos);
    int32_t FindEmpty(uint8_t size) const;
    bool RemoveItem(uint8_t pos);

    CHARACTER *GetOwner() { return m_pOwner; }

    CExchange *GetCompany() { return m_pCompany; }

    bool GetAcceptStatus() const { return m_bAccept; }

    void SetCompany(CExchange *pExchange) { m_pCompany = pExchange; }

private:
    bool Done(uint32_t tradeID, bool firstPlayer);
    bool Check(int *piItemCount);
    bool CheckSpace();

private:
    CExchange *m_pCompany; // 상대방의 CExchange 포인터
    CHARACTER *m_pOwner;

    bool m_bAccept;
    Gold m_gold;
    int m_lLastCriticalUpdatePulse;

    ItemGrid m_grid;

    boost::signals2::scoped_connection m_onDestroy[EXCHANGE_ITEM_MAX_NUM];
};

#endif /* METIN2_SERVER_GAME_EXCHANGE_H */
