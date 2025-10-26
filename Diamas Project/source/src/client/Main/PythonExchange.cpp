#include "stdafx.h"
#include "PythonExchange.h"

void CPythonExchange::SetElkToTarget(Gold elk)
{
    m_victim.elk = elk;
}

void CPythonExchange::SetElkToSelf(Gold elk)
{
    m_self.elk = elk;
}

Gold CPythonExchange::GetElkFromTarget()
{
    return m_victim.elk;
}

Gold CPythonExchange::GetElkFromSelf()
{
    return m_self.elk;
}

void CPythonExchange::SetItemToTarget(uint32_t pos, const ClientItemData& data)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return;

    m_victim.item[pos] = data;
}

void CPythonExchange::SetItemToSelf(uint32_t pos, const ClientItemData& data)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return;

    m_self.item[pos] = data;
}


void CPythonExchange::DelItemOfTarget(uint8_t pos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return;

    m_victim.item[pos] = {};
}

void CPythonExchange::DelItemOfSelf(uint8_t pos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return;

    m_self.item[pos] = {};
}

const ClientItemData* CPythonExchange::GetItemDataFromSelf(uint8_t pos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return nullptr;

    return &m_self.item[pos];
}

const ClientItemData* CPythonExchange::GetItemDataFromTarget(uint8_t pos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return nullptr;

    return &m_victim.item[pos];
}

uint32_t CPythonExchange::GetItemVnumFromTarget(uint8_t pos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return 0;

    return m_victim.item[pos].vnum;
}

uint32_t CPythonExchange::GetItemVnumFromSelf(uint8_t pos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return 0;

    return m_self.item[pos].vnum;
}

uint32_t CPythonExchange::GetChangeLookVnumFromTarget(uint8_t pos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return 0;

    return m_victim.item[pos].transVnum;
}

uint32_t CPythonExchange::GetChangeLookVnumFromSelf(uint8_t pos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return 0;

    return m_self.item[pos].transVnum;
}

uint32_t CPythonExchange::GetItemCountFromTarget(uint8_t pos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return 0;

    return m_victim.item[pos].count;
}

uint32_t CPythonExchange::GetItemCountFromSelf(uint8_t pos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return 0;

    return m_self.item[pos].count;
}

SocketValue CPythonExchange::GetItemMetinSocketFromTarget(uint8_t pos, int iMetinSocketPos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return 0;

    return m_victim.item[pos].sockets[iMetinSocketPos];
}

SocketValue CPythonExchange::GetItemMetinSocketFromSelf(uint8_t pos, int iMetinSocketPos)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return 0;

    return m_self.item[pos].sockets[iMetinSocketPos];
}

void CPythonExchange::GetItemAttributeFromTarget(uint8_t pos, int iAttrPos, ApplyType *pbyType, ApplyValue *psValue)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return;

    *pbyType = m_victim.item[pos].attrs[iAttrPos].bType;
    *psValue = m_victim.item[pos].attrs[iAttrPos].sValue;
}

void CPythonExchange::GetItemAttributeFromSelf(uint8_t pos, int iAttrPos, ApplyType *pbyType, ApplyValue *psValue)
{
    if (pos >= EXCHANGE_ITEM_MAX_NUM)
        return;

    *pbyType = m_self.item[pos].attrs[iAttrPos].bType;
    *psValue = m_self.item[pos].attrs[iAttrPos].sValue;
}

void CPythonExchange::SetAcceptToTarget(uint8_t Accept)
{
    m_victim.accept = Accept ? true : false;
}

void CPythonExchange::SetAcceptToSelf(uint8_t Accept)
{
    m_self.accept = Accept ? true : false;
}

bool CPythonExchange::GetAcceptFromTarget()
{
    return m_victim.accept ? true : false;
}

bool CPythonExchange::GetAcceptFromSelf()
{
    return m_self.accept ? true : false;
}

bool CPythonExchange::GetElkMode()
{
    return m_elk_mode;
}

void CPythonExchange::SetElkMode(bool value)
{
    m_elk_mode = value;
}

void CPythonExchange::Start()
{
    m_isTrading = true;

    m_self.vid = 0;
    m_victim.vid = 0;
}

void CPythonExchange::End()
{
    m_isTrading = false;
}

bool CPythonExchange::isTrading()
{
    return m_isTrading;
}

void CPythonExchange::Clear()
{
    memset(&m_self, 0, sizeof(m_self));
    memset(&m_victim, 0, sizeof(m_victim));
    /*
        m_self.item_vnum[0] = 30;
        m_victim.item_vnum[0] = 30;
        m_victim.item_vnum[1] = 40;
        m_victim.item_vnum[2] = 50;
    */
}

CPythonExchange::CPythonExchange()
{
    Clear();
    m_isTrading = false;
    m_elk_mode = false;
    // Clear로 옴겨놓으면 안됨. 
    // trade_start 페킷이 오면 Clear를 실행하는데
    // m_elk_mode는 클리어 되선 안됨.;  
}

CPythonExchange::~CPythonExchange()
{
}
