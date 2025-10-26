#include <game/attribute.h>
#include "sectree_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "desc_manager.h"
#include <game/GamePacket.hpp>

SECTREE::SECTREE() { Initialize(); }

SECTREE::~SECTREE() { Destroy(); }

void SECTREE::Initialize()
{
    m_id.package = 0;
    m_pkAttribute = nullptr;
    m_iPCCount = 0;
    isClone = false;
}

void SECTREE::Destroy()
{
    if (!m_set_entity.empty())
    {
        auto it = m_set_entity.begin();

        for (; it != m_set_entity.end(); ++it)
        {
            CEntity *ent = *it;

            if (ent->IsType(ENTITY_CHARACTER))
            {
                CHARACTER *ch = (CHARACTER *)ent;

                //SPDLOG_ERROR("Sectree: destroying character: %s is_pc %d", ch->GetName(), ch->IsPC() ? 1 : 0);

                if (ch->IsToggleMount())
                    continue;

                if (ch->GetDesc()) { DESC_MANAGER::instance().DestroyDesc(ch->GetDesc()); }
                else
                    M2_DESTROY_CHARACTER(ch);
            }
            else if (ent->IsType(ENTITY_ITEM))
            {
                CItem *item = (CItem *)ent;

                //SPDLOG_ERROR("Sectree: destroying Item: %s", item->GetName());

                M2_DESTROY_ITEM(item);
            }
            //else
                //SPDLOG_ERROR("Sectree: unknown type: %d", ent->GetEntityType());
        }
    }
    m_set_entity.clear();

    if (!isClone && m_pkAttribute)
    {
        delete (m_pkAttribute);
        m_pkAttribute = nullptr;
    }
}

SECTREEID SECTREE::GetID() { return m_id; }

void SECTREE::IncreasePC()
{
    auto it_tree = m_neighbor_list.begin();

    while (it_tree != m_neighbor_list.end())
    {
        ++(*it_tree)->m_iPCCount;
        ++it_tree;
    }
}

void SECTREE::DecreasePC()
{
    auto itTree = m_neighbor_list.begin();

    while (itTree != m_neighbor_list.end())
    {
        auto tree = *itTree++;

        if (--tree->m_iPCCount <= 0)
        {
            if (tree->m_iPCCount < 0)
            {
                //SPDLOG_ERROR("tree pc count lower than zero (value %d coord %d %d)", tree->m_iPCCount,
                //             tree->m_id.coord.x, tree->m_id.coord.y);
                tree->m_iPCCount = 0;
            }

            auto itEntity = tree->m_set_entity.begin();

            while (itEntity != tree->m_set_entity.end())
            {
                auto pkEnt = *(itEntity++);

                if (pkEnt->IsType(ENTITY_CHARACTER))
                {
                    auto *ch = static_cast<CHARACTER *>(pkEnt);
                    ch->StopStateMachine();
                }
            }
        }
    }
}

bool SECTREE::InsertEntity(CEntity *pkEnt)
{
    SECTREE *pkCurTree;

    if ((pkCurTree = pkEnt->GetSectree()) == this)
        return false;

    if (m_set_entity.find(pkEnt) != m_set_entity.end())
    {
        SPDLOG_ERROR("SECTREE: Entity {:p} already exist in this sectree!", (void*)(pkEnt));
        return false;
    }

    if (pkCurTree)
        pkCurTree->m_set_entity.erase(pkEnt);

    pkEnt->SetSectree(this);

    m_set_entity.insert(pkEnt);

    if (pkEnt->IsType(ENTITY_CHARACTER))
    {
        CHARACTER *pkChr = (CHARACTER *)pkEnt;

        if (pkChr->IsPC())
        {
            IncreasePC();

            if (pkCurTree)
                pkCurTree->DecreasePC();
        }
        else if (m_iPCCount > 0 && !pkChr->IsWarp() && !pkChr->IsGoto()) // PC가 아니고 이 곳에 PC가 있다면 Idle event를 시작 시킨다.
        {
            pkChr->StartStateMachine();
        }
    }

    return true;
}

void SECTREE::RemoveEntity(CEntity *pkEnt)
{
    auto it = m_set_entity.find(pkEnt);

    if (it == m_set_entity.end()) { return; }
    m_set_entity.erase(it);

    pkEnt->SetSectree(nullptr); // Should we really handle this here?

    if (pkEnt->IsType(ENTITY_CHARACTER))
    {
        if (((CHARACTER *)pkEnt)->IsPC())
            DecreasePC();
    }
}

void SECTREE::BindAttribute(CAttribute *pkAttribute) { m_pkAttribute = pkAttribute; }

void SECTREE::CloneAttribute(SECTREE *tree)
{
    m_pkAttribute = tree->m_pkAttribute;
    isClone = true;
}

void SECTREE::SetAttribute(uint32_t x, uint32_t y, uint32_t dwAttr)
{
    assert(m_pkAttribute != NULL);
    m_pkAttribute->Set(x, y, dwAttr);
}

void SECTREE::RemoveAttribute(uint32_t x, uint32_t y, uint32_t dwAttr)
{
    assert(m_pkAttribute != NULL);
    m_pkAttribute->Remove(x, y, dwAttr);
}

uint32_t SECTREE::GetAttribute(long x, long y)
{
    assert(m_pkAttribute != NULL);
    return m_pkAttribute->Get((x % SECTREE_SIZE) / CELL_SIZE, (y % SECTREE_SIZE) / CELL_SIZE);
}

bool SECTREE::IsAttr(long x, long y, uint32_t dwFlag)
{
    if (IS_SET(GetAttribute(x, y), dwFlag))
        return true;

    return false;
}

int SECTREE::GetEventAttribute(long x, long y) { return GetAttribute(x, y) >> 8; }
