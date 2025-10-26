#include "char.h"

#ifdef __OFFLINE_SHOP__

#include "OfflineShopManager.h"
#include "OfflineShop.h"
#include "desc.h"
#include "OfflineShopConfig.h"

void CHARACTER::SetViewingOfflineShop(COfflineShop *pkOfflineShop) { this->m_pkViewingOfflineShop = pkOfflineShop; }

void CHARACTER::SetMyOfflineShop(COfflineShop *pkOfflineShop) { this->m_pkMyOfflineShop = pkOfflineShop; }

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
void CHARACTER::SetOfflineShopFarmedOpeningTime(int iTime)
{
    this->m_iOfflineShopFarmedOpeningTime = std::clamp(iTime, 0, OFFLINE_SHOP_MAX_FARM_TIME);
}

void CHARACTER::SetOfflineShopSpecialOpeningTime(int iTime)
{
    this->m_iOfflineShopSpecialOpeningTime = std::clamp(iTime, 0, OFFLINE_SHOP_MAX_SPECIAL_TIME);
}

int CHARACTER::GetOfflineShopFarmedOpeningTime() const { return this->m_iOfflineShopFarmedOpeningTime; }

int CHARACTER::GetOfflineShopSpecialOpeningTime() const { return this->m_iOfflineShopSpecialOpeningTime; }

void CHARACTER::SendLeftOpeningTimePacket()
{
    if (!this->GetDesc() || !this->GetDesc()->IsPhase(PHASE_GAME)) { return; }

    TPacketGCOfflineShopLeftOpeningTime kPacket;
    kPacket.iFarmedOpeningTime = this->GetOfflineShopFarmedOpeningTime();
    kPacket.iSpecialOpeningTime = this->GetOfflineShopSpecialOpeningTime();
    this->GetDesc()->Send(HEADER_GC_OFFLINE_SHOP_LEFT_OPENING_TIME, kPacket);
}
#endif

#ifdef __OFFLINE_SHOP_USE_BUNDLE__
void CHARACTER::OfflineShopStartCreate()
{
    if (this->GetMyOfflineShop() && this->GetMyOfflineShop()->GetItemCount() > 0)
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "You have to close your shop before you can create a new one.");
        return;
    }

    if (this->IsBuildingOfflineShop())
    {
        SendI18nChatPacket(this, CHAT_TYPE_INFO, "You cannot use this item because you opened a private shop.");
        return;
    }

    this->SetBuildingOfflineShop(true);
    this->SendOfflineShopStartCreatePacket();
}

void CHARACTER::SetBuildingOfflineShop(bool val) { this->m_buildingOfflineShop = val; }

bool CHARACTER::IsBuildingOfflineShop() const { return this->m_buildingOfflineShop; }

void CHARACTER::SendOfflineShopStartCreatePacket()
{
    if (this->GetMyOfflineShop() && this->GetMyOfflineShop()->GetItemCount() > 0)
        return;

    TPacketGCOfflineShop kPacket;
    kPacket.bySubHeader = CREATE_SHOP;
    this->GetDesc()->Send(HEADER_GC_OFFLINE_SHOP, kPacket);
}
#endif
#endif
