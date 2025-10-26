#include "StdAfx.h"
#include "PythonApplication.h"

#ifdef __OFFLINE_SHOP__
#include "PythonOfflineShop.h"
#include "PythonNetworkStream.h"
#include "PythonBackground.h"

#define NETSTREAM CPythonNetworkStream::instance()

CPythonOfflineShop::CPythonOfflineShop()
{
    this->m_poHandler = NULL;
}

CPythonOfflineShop::~CPythonOfflineShop()
{
}

void CPythonOfflineShop::SendCreateMyShopPacket()
{
    TPacketCGOfflineShop kPacket;
    kPacket.bySubHeader = EOfflineShopCGSubHeader::CREATE_MY_SHOP;

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP, kPacket);
}

void CPythonOfflineShop::SendCloseMyShopPacket()
{
    TPacketCGOfflineShop kPacket;
    kPacket.bySubHeader = EOfflineShopCGSubHeader::CLOSE_MY_SHOP;

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP, kPacket);
}

void CPythonOfflineShop::SendCloseOtherShopPacket()
{
    TPacketCGOfflineShop kPacket;
    kPacket.bySubHeader = EOfflineShopCGSubHeader::CLOSE_OTHER_SHOP;

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP, kPacket);
}

void CPythonOfflineShop::SendCreatePacket(
    const char *c_szShopName,
#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
	float fX, float fY,
#endif
    bool bIsShowEditor,
    std::vector<TOfflineShopCGItemInfo> *pvecItems
#ifdef __OFFLINE_SHOP_OPENING_TIME__
	, int iOpeningTime
#endif
    )
{
    TPacketCGOfflineShopCreate kPacket;
    kPacket.szShopName = c_szShopName;
#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
	kPacket.fX = fX;
	kPacket.fY = fY;
#endif

    kPacket.bIsShowEditor = bIsShowEditor;
    kPacket.dwItemCount = pvecItems->size();

#ifdef __OFFLINE_SHOP_OPENING_TIME__
	kPacket.iOpeningTime = iOpeningTime;
#endif

    kPacket.items = *pvecItems;
    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP_CREATE, kPacket);


}

void CPythonOfflineShop::SendShopNamePacket(const char *c_szShopName)
{
    TPacketCGOfflineShopName kPacket;
    kPacket.szShopName = c_szShopName;

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP_NAME, kPacket);
}

#ifndef __OFFLINE_SHOP_DISALLOW_MANUAL_POSITIONING__
void CPythonOfflineShop::SendShopPositionPacket(float fX, float fY)
{
	TPacketCGOfflineShopPosition kPacket;
	kPacket.byHeader = HEADER_CG_OFFLINE_SHOP_POSITION;
	kPacket.fX = fX;
	kPacket.fY = fY;

	gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP, kPacket);
}
#endif

void CPythonOfflineShop::SendAddItemPacket(TItemPos *pInventoryPosition, uint32_t dwDisplayPosition, LONGLONG llPrice)
{
    TPacketCGOfflineShopAddItem kPacket;
    memcpy(&kPacket.kInventoryPosition, pInventoryPosition, sizeof(kPacket.kInventoryPosition));
    kPacket.dwDisplayPosition = dwDisplayPosition;
    kPacket.llPrice = llPrice;

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP_ITEM_ADD, kPacket);
}

void CPythonOfflineShop::SendMoveItemPacket(uint32_t dwOldDisplayPosition, uint32_t dwNewDisplayPosition)
{
    TPacketCGOfflineShopMoveItem kPacket;
    kPacket.dwOldDisplayPosition = dwOldDisplayPosition;
    kPacket.dwNewDisplayPosition = dwNewDisplayPosition;

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP_ITEM_MOVE, kPacket);
}

void CPythonOfflineShop::SendRemoveItemPacket(uint32_t dwDisplayPosition, TItemPos *pkInventoryPosition)
{
    TPacketCGOfflineShopRemoveItem kPacket;
    kPacket.dwDisplayPosition = dwDisplayPosition;
    kPacket.kInventoryPosition = *pkInventoryPosition;

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP_ITEM_REMOVE, kPacket);
}

void CPythonOfflineShop::SendBuyItemPacket(uint32_t dwDisplayPosition, TItemPos *pkInventoryPosition)
{
    TPacketCGOfflineShopBuyItem kPacket;
    kPacket.dwDisplayPosition = dwDisplayPosition;
    kPacket.kInventoryPosition = *pkInventoryPosition;

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP_ITEM_BUY, kPacket);
}

void CPythonOfflineShop::SendWithdrawGoldPacket(LONGLONG llGold)
{
    TPacketCGOfflineShopWithdrawGold kPacket;
    kPacket.llGold = llGold;

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP_WITHDRAW_GOLD, kPacket);
}

void CPythonOfflineShop::SendClickShopPacket(uint32_t dwShopOwnerPID)
{
    TPacketCGOfflineShopClickShop kPacket;
    kPacket.dwShopOwnerPID = dwShopOwnerPID;

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP_CLICK, kPacket);
}

#ifdef __OFFLINE_SHOP_OPENING_TIME__
	void CPythonOfflineShop::SendReopenShopPacket(int iOpeningTime)
#else
void CPythonOfflineShop::SendReopenShopPacket()
#endif
{
    TPacketCGOfflineShopReopen kPacket;
#ifdef __OFFLINE_SHOP_OPENING_TIME__
	kPacket.iOpeningTime = iOpeningTime;
#endif

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP_REOPEN, kPacket);
}

#ifdef __OFFLINE_SHOP_USE_BUNDLE__
void CPythonOfflineShop::SendCancelCreateShopPacket()
{
    TPacketCGOfflineShop kPacket;
    kPacket.bySubHeader = CANCEL_CREATE_MY_SHOP;

    gPythonNetworkStream->Send(HEADER_CG_OFFLINE_SHOP, kPacket);
}
#endif

bool CPythonOfflineShop::ReceiveOfflineShopPacket(const TPacketGCOfflineShop& p)
{
    if (!this->m_poHandler)
    {
        return true;
    }

    PyCallClassMemberFunc(
        this->m_poHandler,
        "BINARY_ReceiveOfflineShopPacket",
        Py_BuildValue("(i)",
                      p.bySubHeader
            )
        );

    return true;
}

bool CPythonOfflineShop::ReceiveAdditionalInfoPacket(const TPacketGCOfflineShopAdditionalInfo& p)
{
    if (!this->m_poHandler)
    {
        return false;
    }

    PyCallClassMemberFunc(
        this->m_poHandler,
        "BINARY_ReceiveAdditionalInfoPacket",
        Py_BuildValue("(isi)",
                      p.dwVID,
                      p.szShopName.c_str(),
                      p.dwShopOwnerPID
            )
        );

    return true;
}

bool CPythonOfflineShop::ReceiveAddItemPacket(const TPacketGCOfflineShopAddItem& kPacket)
{
    if (!this->m_poHandler)
    {
        return true;
    }

    PyCallClassMemberFunc(
        this->m_poHandler,
        "BINARY_ReceiveAddItemPacket",
        Py_BuildValue(
            "(iii[LLLLLL][(id)(id)(id)(id)(id)(id)(id)]"
#ifdef __ITEM_RUNE__
	        "i"
#endif
            "iL)",
            kPacket.data.vnum,
            kPacket.data.transVnum,
            kPacket.data.count,
            kPacket.data.sockets[0], kPacket.data.sockets[1], kPacket.data.sockets[2],
            kPacket.data.sockets[3], kPacket.data.sockets[4], kPacket.data.sockets[5],
            kPacket.data.attrs[0].bType, kPacket.data.attrs[0].sValue,
            kPacket.data.attrs[1].bType, kPacket.data.attrs[1].sValue,
            kPacket.data.attrs[2].bType, kPacket.data.attrs[2].sValue,
            kPacket.data.attrs[3].bType, kPacket.data.attrs[3].sValue,
            kPacket.data.attrs[4].bType, kPacket.data.attrs[4].sValue,
            kPacket.data.attrs[5].bType, kPacket.data.attrs[5].sValue,
            kPacket.data.attrs[6].bType, kPacket.data.attrs[6].sValue,
            kPacket.dwDisplayPosition,
            kPacket.llPrice
            )
        );

    return true;
}

bool CPythonOfflineShop::ReceiveMoveItemPacket(const TPacketGCOfflineShopMoveItem& kPacket)
{
    if (!this->m_poHandler)
    {
        return true;
    }

    PyCallClassMemberFunc(
        this->m_poHandler,
        "BINARY_ReceiveMoveItemPacket",
        Py_BuildValue("(ii)",
                      kPacket.dwOldDisplayPosition,
                      kPacket.dwNewDisplayPosition
            )
        );

    return true;
}

bool CPythonOfflineShop::ReceiveRemoveItemPacket(const TPacketGCOfflineShopRemoveItem& kPacket)
{
    if (!this->m_poHandler)
    {
        return true;
    }

    PyCallClassMemberFunc(
        this->m_poHandler,
        "BINARY_ReceiveRemoveItemPacket",
        Py_BuildValue("(i)",
                      kPacket.dwDisplayPosition
            )
        );

    return true;
}

bool CPythonOfflineShop::ReceiveGoldPacket(const TPacketGCOfflineShopGold& kPacket)
{
    if (!this->m_poHandler)
    {
        return true;
    }

    PyCallClassMemberFunc(
        this->m_poHandler,
        "BINARY_ReceiveGoldPacket",
        Py_BuildValue("(K)",
                      kPacket.llGold
            )
        );

    return true;
}

bool CPythonOfflineShop::ReceiveNamePacket(const TPacketGCOfflineShopName& kPacket)
{
    if (!this->m_poHandler)
    {
        return true;
    }

    PyCallClassMemberFunc(
        this->m_poHandler,
        "BINARY_ReceiveNamePacket",
        Py_BuildValue("(s)",
                      kPacket.szName.c_str()
            )
        );

    return true;
}

#if defined(__OFFLINE_SHOP_OPENING_TIME__) || defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
bool CPythonOfflineShop::ReceiveLeftOpeningTimePacket(const TPacketGCOfflineShopLeftOpeningTime& kPacket)
{
    if (!this->m_poHandler)
    {
        return false;
    }


#ifdef __OFFLINE_SHOP_OPENING_TIME__
	PyCallClassMemberFunc(
	    this->m_poHandler,
	    "BINARY_ReceiveLeftOpeningTimePacket",
	    Py_BuildValue("(i)",
	                  kPacket.iLeftOpeningTime
	                 )
	);
#elif defined(__OFFLINE_SHOP_FARM_OPENING_TIME__)
    PyCallClassMemberFunc(
        this->m_poHandler,
        "BINARY_ReceiveLeftOpeningTimePacket",
        Py_BuildValue("(ii)",
                      kPacket.iFarmedOpeningTime,
                      kPacket.iSpecialOpeningTime
            )
        );
#endif

    return true;
}
#endif

#ifdef __OFFLINE_SHOP_FARM_OPENING_TIME__
bool CPythonOfflineShop::ReceiveOfflineShopEditorPositionInfoPacket(const TPacketGCOfflineShopEditorPositionInfo& kPacket)
{
    if (!this->m_poHandler)
    {
        return false;
    }

    uint32_t x, y;

    const auto pkMapInfo = CPythonBackground::instance().GetMapInfo(kPacket.lMapIndex);
    if (pkMapInfo)
    {
        x = (kPacket.lX) / 100;
        y = (kPacket.lY) / 100;
    }
    else
    {
        x = 0;
        y = 0;
    }

    PyCallClassMemberFunc(
        this->m_poHandler,
        "BINARY_ReceiveEditorPositionInfoPacket",
        Py_BuildValue("(iiii)",
                      kPacket.lMapIndex,
                      x,
                      y,
                      kPacket.byChannel
            )
        );

    return true;
}
#endif

void CPythonOfflineShop::SetPythonHandler(PyObject *poHandler)
{
    this->m_poHandler = poHandler;
}
#endif
