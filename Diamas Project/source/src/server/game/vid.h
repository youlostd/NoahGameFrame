#ifndef METIN2_SERVER_GAME_VID_H
#define METIN2_SERVER_GAME_VID_H

#include <Config.hpp>
#include <ostream>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class VID
{
public:
    VID()
        : m_id(0), m_crc(0)
    {
    }

    VID(uint32_t id, uint32_t crc)
        : m_id{id},
          m_crc{crc} {
    }

    VID(const VID& other)
        : m_id{other.m_id},
          m_crc{other.m_crc} {
    }

    VID(VID&& other) noexcept
        : m_id{other.m_id},
          m_crc{other.m_crc} {
    }

    VID& operator=(const VID& other) {
        if (this == &other)
            return *this;
        m_id  = other.m_id;
        m_crc = other.m_crc;
        return *this;
    }

    VID& operator=(VID&& other) noexcept {
        if (this == &other)
            return *this;
        m_id  = other.m_id;
        m_crc = other.m_crc;
        return *this;
    }

    friend bool operator==(const VID& lhs, const VID& rhs) {
        return lhs.m_id == rhs.m_id
            && lhs.m_crc == rhs.m_crc;
    }

    friend bool operator!=(const VID& lhs, const VID& rhs) { return !(lhs == rhs); }

    friend bool operator<(const VID& lhs, const VID& rhs) {
        if (lhs.m_id < rhs.m_id)
            return true;
        if (rhs.m_id < lhs.m_id)
            return false;
        return lhs.m_crc < rhs.m_crc;
    }

    friend bool operator<=(const VID& lhs, const VID& rhs) { return !(rhs < lhs); }

    friend bool operator>(const VID& lhs, const VID& rhs) { return rhs < lhs; }

    friend bool operator>=(const VID& lhs, const VID& rhs) { return !(lhs < rhs); }

    friend std::ostream& operator<<(std::ostream& os, const VID& obj) {
        return os
            << "VID[ " << obj.m_id
            << " : " << obj.m_crc << "]";
    }

    friend std::size_t hash_value(const VID& obj) {
        return obj.m_crc;
    }

    operator uint32_t() const { return m_id; }

    void Reset() { m_id = 0; m_crc = 0; }

    uint32_t getCRC() const { return m_crc; }

private:
    uint32_t m_id;
    uint32_t m_crc;
};

#endif /* METIN2_SERVER_GAME_VID_H */
