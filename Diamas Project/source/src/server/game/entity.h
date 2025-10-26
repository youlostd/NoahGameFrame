#ifndef METIN2_SERVER_GAME_ENTITY_H
#define METIN2_SERVER_GAME_ENTITY_H

#include <Config.hpp>

#include <net/Type.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/robin_hood.h>

class SECTREE;

#include "char_manager.h"
class DESC;

class CEntity
{
public:
    typedef robin_hood::unordered_map<CEntity *, int> ENTITY_MAP;
    typedef std::function<bool(CHARACTER *)> ENTITY_REQ_FUNC;
public:
    CEntity(int type);
    virtual ~CEntity();

    virtual void EncodeInsertPacket(CEntity *entity) = 0;
    virtual void EncodeRemovePacket(CEntity *entity) = 0;

protected:
    virtual void Destroy();

public:
    void SetType(int type);
    int GetEntityType() const;
    bool IsType(int type) const;

    void ViewCleanup();
    void ViewInsert(CEntity *entity, bool recursive = true);
    void ViewRemove(CEntity *entity, bool recursive = true);
    void ViewReencode();

    int GetViewAge() const { return m_iViewAge; }

    long GetX() const { return m_pos.x; }

    long GetY() const { return m_pos.y; }

    long GetZ() const { return m_pos.z; }

    const PIXEL_POSITION &GetXYZ() const { return m_pos; }

    uint16_t GetScale() const { return m_scale; }

    void SetScale(uint16_t scale) { m_scale = scale; }

    void SetXYZ(long x, long y, long z) { m_pos.x = x, m_pos.y = y, m_pos.z = z; }

    void SetXYZ(const PIXEL_POSITION &pos) { m_pos = pos; }

    SECTREE *GetSectree() const { return m_pSectree; }

    void SetSectree(SECTREE *tree) { m_pSectree = tree; }

    void UpdateSectree();

    template <typename Function>
    void ForEachSeen(Function &f);

    void BindDesc(DESC *_d) { m_lpDesc = _d; }

    DESC *GetDesc() const { return m_lpDesc; }

    void SetMapIndex(long l) { m_lMapIndex = l; }

    long GetMapIndex() const { return m_lMapIndex; }

    void SetObserverMode(bool bFlag);

    bool IsObserverMode() const { return m_bIsObserver; }

    [[nodiscard]] ENTITY_REQ_FUNC GetRequirementFunction() const { return m_fReq; };

    void SetRequirementFunction(ENTITY_REQ_FUNC f) { m_fReq = std::move(f); }
    const ENTITY_MAP& GetViewMap()  { return m_map_view; }
protected:
    bool m_bIsObserver = false;
    bool m_isInOXField = false;
    bool m_bObserverModeChange = false;
    ENTITY_MAP m_map_view;
    long m_lMapIndex = 0;

private:
    DESC *m_lpDesc = nullptr;

    int m_iType = -1;
    bool m_bIsDestroyed = false;

    PIXEL_POSITION m_pos{};
    uint16_t m_scale = 100;

    int m_iViewAge = 0;

    SECTREE *m_pSectree = nullptr;
    ENTITY_REQ_FUNC m_fReq;
};


template <typename Function>
void CEntity::ForEachSeen(Function &f)
{
    std::vector<CEntity *> seen;
    seen.reserve(m_map_view.size());

    auto collector = [&seen](const ENTITY_MAP::value_type &p) { seen.push_back(p.first); };

    // TODO(imer): Some functions expect the old behaviour of including this in
    // their around view.
    // figure those out and manually call f on this
    seen.push_back(this);

    std::for_each(m_map_view.begin(), m_map_view.end(), collector);

    // This is very fragile. All of ForEachSeen()'s callers are only interested
    // in characters. One of these characters might however be destroyed while
    // we are iterating over our snapshot.
    CharacterSnapshotGuard guard;

    for (const auto &e : seen)
        f(e);
}


#endif /* METIN2_SERVER_GAME_ENTITY_H */
