#ifndef METIN2_CLIENT_MAIN_PYTHONEXCHANGE_H
#define METIN2_CLIENT_MAIN_PYTHONEXCHANGE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/GamePacket.hpp>

/*
 *	��ȯ â ����
 */
class CPythonExchange : public CSingleton<CPythonExchange>
{
public:
    enum
    {
        EXCHANGE_ITEM_MAX_NUM = 24,
    };

    typedef struct trade
    {
        uint32_t vid;

        ClientItemData item[EXCHANGE_ITEM_MAX_NUM];

        uint8_t accept;
        Gold elk;
    } TExchangeData;

public:
    CPythonExchange();
    virtual ~CPythonExchange();

    void Clear();

    void Start();
    void End();
    bool isTrading();

    // Interface

    void SetTargetVID(uint32_t vid)
    {
        m_victim.vid = vid;
    }

    uint32_t GetTargetVID() const
    {
        return m_victim.vid;
    }

    void SetElkToTarget(Gold elk);
    void SetElkToSelf(Gold elk);

    Gold GetElkFromTarget();
    Gold GetElkFromSelf();

    void SetItemToTarget(uint32_t pos, const ClientItemData& data);
    void SetItemToSelf(uint32_t pos, const ClientItemData& data);


    void                  DelItemOfTarget(uint8_t pos);
    void                  DelItemOfSelf(uint8_t pos);
    const ClientItemData* GetItemDataFromSelf(uint8_t pos);
    const ClientItemData* GetItemDataFromTarget(uint8_t pos);

    uint32_t GetItemVnumFromTarget(uint8_t pos);
    uint32_t GetItemVnumFromSelf(uint8_t pos);
    uint32_t GetChangeLookVnumFromTarget(unsigned char pos);
    uint32_t GetChangeLookVnumFromSelf(unsigned char pos);

    uint32_t GetItemCountFromTarget(uint8_t pos);
    uint32_t GetItemCountFromSelf(uint8_t pos);

    SocketValue GetItemMetinSocketFromTarget(uint8_t pos, int iMetinSocketPos);
    SocketValue GetItemMetinSocketFromSelf(uint8_t pos, int iMetinSocketPos);

    void GetItemAttributeFromTarget(uint8_t pos, int iAttrPos, ApplyType *pbyType, ApplyValue *psValue);
    void GetItemAttributeFromSelf(uint8_t pos, int iAttrPos, ApplyType *pbyType, ApplyValue *psValue);

    void SetAcceptToTarget(uint8_t Accept);
    void SetAcceptToSelf(uint8_t Accept);

    bool GetAcceptFromTarget();
    bool GetAcceptFromSelf();

    bool GetElkMode();
    void SetElkMode(bool value);

protected:
    bool m_isTrading;

    bool m_elk_mode; // ��ũ�� Ŭ���ؼ� ��ȯ�������� ���� ������.
    TExchangeData m_self;
    TExchangeData m_victim;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONEXCHANGE_H */
