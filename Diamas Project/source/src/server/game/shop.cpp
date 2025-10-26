#include "constants.h"
#include "utils.h"
#include "config.h"
#include "shop.h"
#include "desc.h"
#include "char.h"
#include "locale.h"

#include <game/GamePacket.hpp>

#include <algorithm>

Shop::Shop(uint32_t type)
    : m_type(type)
{
    // ctor
}

/*virtual*/
Shop::~Shop()
{
    TPacketGCShop pack;
    pack.subheader = SHOP_SUBHEADER_GC_END;

    Broadcast(HEADER_GC_SHOP, pack);

    std::for_each(m_guestMap.begin(), m_guestMap.end(),
                  [](const GuestMap::value_type &v) { v.first->SetShop(nullptr); });
}

void Shop::RemoveGuest(CHARACTER *ch)
{
    if (ch->GetShop() != this)
        return;

    m_guestMap.erase(ch);
    ch->SetShop(nullptr);

    TPacketGCShop pack;
    pack.subheader = SHOP_SUBHEADER_GC_END;

    ch->GetDesc()->Send(HEADER_GC_SHOP, pack);
}

void Shop::RemoveAllGuests()
{
    if (m_guestMap.empty())
        return;

    for(const auto& it : m_guestMap)
    {
        auto *ch = it.first;
        if (ch)
        {
            if (ch->GetDesc() && ch->GetShop() == this)
            {
                ch->SetShop(NULL);

                TPacketGCShop pack;
                pack.subheader = SHOP_SUBHEADER_GC_END;
                ch->GetDesc()->Send(HEADER_GC_SHOP, pack);
            }
        }
    }
    m_guestMap.clear();
}

uint32_t Shop::GetType() const { return m_type; }
