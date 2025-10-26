#include "utils.h"
#include "char.h"
#include "sectree_manager.h"
#include "config.h"

void CEntity::ViewCleanup()
{
    for (const auto &p : m_map_view)
        p.first->ViewRemove(this, false);

    m_map_view.clear();
}

void CEntity::ViewReencode()
{
    if (m_bIsObserver)
        return;

    EncodeRemovePacket(this);
    EncodeInsertPacket(this);

    for (const auto &p : m_map_view)
    {
        auto entity = p.first;

        if (!m_bIsObserver)
        {
            EncodeRemovePacket(entity);
            EncodeInsertPacket(entity);
        }

        if (!entity->m_bIsObserver)
        {
            entity->EncodeRemovePacket(this);
            entity->EncodeInsertPacket(this);
        }
    }
}

void CEntity::ViewInsert(CEntity *entity, bool recursive)
{
    if (this == entity)
        return;

    // Code 1
    if (auto it = m_map_view.find(entity); it != m_map_view.end())
    {
		it->second = m_iViewAge;
        return;
    }

    m_map_view.emplace(entity, m_iViewAge);

    if (!entity->m_bIsObserver)
        entity->EncodeInsertPacket(this);

    if (recursive)
        entity->ViewInsert(this, false);

    // Code 2

    //auto& [it, inserted] = m_map_view.insert_or_assign(entity, m_iViewAge);
    //if(inserted)
    //{
    //    if (!entity->m_bIsObserver)
    //        entity->EncodeInsertPacket(this);

    //    if (recursive)
    //        entity->ViewInsert(this, false);
    //}


}

void CEntity::ViewRemove(CEntity *entity, bool recursive)
{
    auto it = m_map_view.find(entity);
    if (it == m_map_view.end())
        return;

    m_map_view.erase(it);

    if (!entity->m_bIsObserver)
        entity->EncodeRemovePacket(this);

    if (recursive)
        entity->ViewRemove(this, false);
}

struct FuncViewInsert
{
    FuncViewInsert(CEntity *ent)
        : dwViewRange(gConfig.viewRange)
          , m_me(ent)
    {
    }

    void operator ()(CEntity *ent)
    {
        if (!ent->IsType(ENTITY_OBJECT))
            if (DISTANCE_APPROX(ent->GetX() - m_me->GetX(), ent->GetY() - m_me->GetY()) > dwViewRange)
                return;

        m_me->ViewInsert(ent);

        if (ent->IsType(ENTITY_CHARACTER) && m_me->IsType(ENTITY_CHARACTER))
        {
            CHARACTER *chMe = (CHARACTER *)m_me;
            CHARACTER *chEnt = (CHARACTER *)ent;

            if (chMe->IsPC() && !chEnt->IsPC() && !chEnt->IsWarp() && !chEnt->IsGoto())
                chEnt->StartStateMachine();
        }
    }

    int dwViewRange;
    CEntity *m_me;
};

void CEntity::UpdateSectree()
{
    if (!m_pSectree)
    {
        if (IsType(ENTITY_CHARACTER))
        {
            CHARACTER *tch = (CHARACTER *)this;
            SPDLOG_ERROR("{0}: has no valid sectree [{1},{2},{3}]", tch->GetName(), GetMapIndex(),
                         GetX(), GetY());
        }

        return;
    }

    ++m_iViewAge;

    FuncViewInsert f(this);
    m_pSectree->ForEachAround(f);

    auto it = m_map_view.begin();
    auto end = m_map_view.end();

    while (it != end)
    {
        if (it->second < m_iViewAge)
        {
            auto entity = it->first;

            if (!entity->m_bIsObserver)
                entity->EncodeRemovePacket(this);

            entity->ViewRemove(this, false);

            it = m_map_view.erase(it);
        }
        else { ++it; }
    }
}
