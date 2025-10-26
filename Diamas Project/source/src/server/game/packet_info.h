#ifndef METIN2_SERVER_GAME_PACKET_INFO_H
#define METIN2_SERVER_GAME_PACKET_INFO_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/GamePacket.hpp>
#include <unordered_map>

typedef struct SPacketElement
{
    int iSize;
    std::string stName;
    int iCalled;
    uint32_t dwLoad;
    bool isSequence;
    bool dynamic;
} TPacketElement;

class CPacketInfo
{
public:
    CPacketInfo();

    void Set(uint8_t header, int size, const char *c_pszName,
             bool isSequence = false, bool dynamic = false);

    TPacketElement *GetElement(uint8_t header);

    void Start(TPacketElement *info);
    void End();

    void Log(const char *c_pszFileName);

    void SetSequence(uint8_t header, bool bSeq);

protected:
    std::unordered_map<uint8_t, TPacketElement> m_pPacketMap;
    TPacketElement *m_pCurrentPacket;
    uint32_t m_dwStartTime;
};

class CPacketInfoCG : public CPacketInfo
{
public:
    CPacketInfoCG();
    ~CPacketInfoCG();
};

// PacketInfo P2P
class CPacketInfoGG : public CPacketInfo
{
public:
    CPacketInfoGG();
    ~CPacketInfoGG();
};

#endif /* METIN2_SERVER_GAME_PACKET_INFO_H */
