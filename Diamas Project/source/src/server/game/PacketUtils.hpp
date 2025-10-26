#ifndef METIN2_SERVER_GAME_PACKETUTILS_HPP
#define METIN2_SERVER_GAME_PACKETUTILS_HPP

#include <Config.hpp>

#include <net/Type.hpp>

#include "char.h"
#include "desc.h"

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

template <class T>
struct FuncPacketAround
{
    PacketId m_id;
     T m_data;
    CEntity *m_except;

    FuncPacketAround(PacketId id, const T& data, CEntity *except = nullptr)
        : m_id(id), m_data(data), m_except(except)
    {
    }

    void operator ()(CEntity* ent);
};

template <class T>
struct FuncPacketView : public FuncPacketAround<T>
{
    FuncPacketView(PacketId id, const T& data, CEntity *except = nullptr)
        : FuncPacketAround<T>(id, data, except)
    {
    }

    void operator()(const CEntity::ENTITY_MAP::value_type &v) { FuncPacketAround<T>::operator()(v.first); }
};

template <class T>
void FuncPacketAround<T>::operator()(CEntity* ent) {
    if (ent == m_except)
        return;

    if (ent->GetDesc())
        ent->GetDesc()->Send(m_id, m_data);
}

template <class T>
void PacketAround(const CEntity::ENTITY_MAP& map, CEntity* entity, PacketId id, const T& data, CEntity* except=nullptr) {
    PacketView(map, entity, id, data, except);
}

template <class T>
void PacketView(const CEntity::ENTITY_MAP& map, CEntity* entity, PacketId id, const T& data, CEntity* except=nullptr) {
        if (!entity->GetSectree())
        return;

    FuncPacketView f(id, data, except);

    if (!entity->IsObserverMode())
        std::for_each(map.begin(), map.end(), f);

    f(CEntity::ENTITY_MAP::value_type(entity, 0));
}

template<typename  T>
struct FSendPacket
{
    PacketId id;
    T p;

    void operator()(CEntity *ent);
};

template <typename T>
void FSendPacket<T>::operator()(CEntity* ent) {
    if (ent->IsType(ENTITY_CHARACTER))
    {
        CHARACTER *ch = (CHARACTER *)ent;

        if (ch->GetDesc()) { ch->GetDesc()->Send(id, p); }
    }
}


#endif