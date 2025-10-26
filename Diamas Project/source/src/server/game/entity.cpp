#include "entity.h"
#include "char.h"
#include "desc.h"
#include "item.h"
#include "ItemUtils.h"
#include "sectree_manager.h"

CEntity::CEntity(int type)
    : m_iType(type)
{
}

CEntity::~CEntity() {
    if (m_bIsDestroyed)
        return;

    ViewCleanup();
    m_bIsDestroyed = true;
}


void CEntity::Destroy()
{
    if (m_bIsDestroyed)
        return;

    ViewCleanup();
    m_bIsDestroyed = true;
}

void CEntity::SetType(int type) { m_iType = type; }

int CEntity::GetEntityType() const { return m_iType; }

bool CEntity::IsType(int type) const { return (m_iType == type ? true : false); }


void CEntity::SetObserverMode(bool bFlag)
{
    if (m_bIsObserver == bFlag)
        return;

    m_bIsObserver = bFlag;

    if (m_bIsObserver)
    {
        std::for_each(m_map_view.begin(), m_map_view.end(), [this](const auto &p) { EncodeRemovePacket(p.first); });
    }
    else
    {
        std::for_each(m_map_view.begin(), m_map_view.end(), [this](const auto &p) { EncodeInsertPacket(p.first); });
    }

    if (IsType(ENTITY_CHARACTER))
    {
        CHARACTER *ch = (CHARACTER *)this;
        ch->ChatPacket(CHAT_TYPE_COMMAND, "ObserverMode %d", m_bIsObserver ? 1 : 0);

        if (ch->IsPC())
        {
            if (auto *toggleMount = ch->GetToggleMount(); toggleMount)
                toggleMount->ViewReencode();

            if (const auto *item = FindToggleItem(ch, true, TOGGLE_PET);item)
            {
                if (auto *pet = ch->GetPetSystem()->GetPetActorByItemId(item->GetID()); pet)
                {
                    if (pet->IsSummoned())
                        pet->GetCharacter()->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
                }
            }
        }
    }
}
