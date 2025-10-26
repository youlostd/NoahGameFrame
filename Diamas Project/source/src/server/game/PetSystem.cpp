#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "PetSystem.h"
#include <game/GamePacket.hpp>
#include "item_manager.h"
#include "item.h"
#include <Eigen/Core>
#include <game/AffectConstants.hpp>
#include <game/MotionConstants.hpp>
#include "main.h"
#include <game/MotionTypes.hpp>
#include "motion.h"

#include <gsl/gsl_util>

EVENTINFO(petsystem_event_info)
{
    CPetSystem *pPetSystem;
};

EVENTFUNC(petsystem_update_event)
{
    auto *info = static_cast<petsystem_event_info *>(event->info);
    if (!info)
    {
        SPDLOG_ERROR("petsystem_update_event> <Factor> Null pointer");
        return 0;
    }

    CPetSystem *pPetSystem = info->pPetSystem;
    if (!pPetSystem)
        return 0;

    if (!pPetSystem->Update())
    {
        pPetSystem->m_pkPetSystemUpdateEvent = nullptr;
        return 0;
    }

    return THECORE_SECS_TO_PASSES(1) / 4ull;
}

///////////////////////////////////////////////////////////////////////////////////////
//  CPetActor
///////////////////////////////////////////////////////////////////////////////////////

CPetActor::CPetActor(CHARACTER &owner, uint32_t vnum, uint32_t options, uint16_t scale)
    : m_owner(owner)
      , m_dwVnum(vnum)
      , m_dwOptions(options)
      , m_dwLastActionTime(0)
      , m_scale(scale)
      , m_pkChar(nullptr)
{
    // ctor
}

CPetActor::~CPetActor() { Unsummon(); }

bool CPetActor::Mount()
{
    if (HasOption(EPetOption_Mountable))
        m_owner.MountVnum(m_dwVnum);

    return m_owner.GetMountVnum() == m_dwVnum;
}

void CPetActor::Unmount()
{
    if (m_owner.GetMountVnum() == m_dwVnum)
        m_owner.MountVnum(0);
}

unsigned CPetActor::Summon(std::string petName, bool bSpawnFar)
{
    int32_t x = m_owner.GetX();
    int32_t y = m_owner.GetY();
    int32_t z = m_owner.GetZ();

    if (bSpawnFar)
    {
        x += (Random::get(0, 1) * 2 - 1) * Random::get(2000, 2500);
        y += (Random::get(0, 1) * 2 - 1) * Random::get(2000, 2500);
    }
    else
    {
        x += Random::get(-100, 100);
        y += Random::get(-100, 100);
    }

    if (m_pkChar)
    {
        if (HasOption(EPetOption_Scaleable))
            m_pkChar->SetScale(GetScale());

        if (HasOption(EPetOption_Combatable))
            m_pkChar->SetLevel(GetLevel());

        auto ownerId = m_owner.GetPlayerID();
        m_pkChar->SetRequirementFunction([ownerId](CHARACTER *player) -> bool
        {
            const auto *owner = g_pCharManager->FindByPID(ownerId);
            if (!owner)
                return false;

            if (owner->IsObserverMode()) { return player->IsObserverMode(); }

            return true;
        });

        m_pkChar->Show(m_owner.GetMapIndex(), x, y);
        return m_pkChar->GetVID();
    }

    m_pkChar = g_pCharManager->SpawnMob(m_dwVnum,
                                        m_owner.GetMapIndex(),
                                        x, y, z,
                                        false,
                                        static_cast<int>(m_owner.GetRotation() + 180.0f),
                                        false, false, 100, true);

    if (!m_pkChar)
    {
        SPDLOG_ERROR("[CPetSystem::Summon] Failed to summon the pet. (vnum: {0})", m_dwVnum);
        return 0;
    }

    m_pkChar->SetPet(this);
    m_pkChar->MarkAsPet();

    auto ownerId = m_owner.GetPlayerID();
    m_pkChar->SetRequirementFunction([ownerId](CHARACTER *player) -> bool
    {
        const auto *owner = g_pCharManager->FindByPID(ownerId);
        if (!owner)
            return false;

        if (owner->IsObserverMode()) { return player->IsObserverMode(); }

        return true;
    });

    if (HasOption(EPetOption_Combatable))
    {
        m_pkChar->PointChange(POINT_ATT_GRADE_BONUS, GetAttackGrade());
        m_pkChar->SetAttackMob();
        m_pkChar->SetNoAttackChunjo();
        m_pkChar->SetNoAttackJinno();
        m_pkChar->SetNoAttackShinsu();
        m_pkChar->SetLevel(GetLevel());
        m_pkChar->MarkAsAttackPet();
    }

    if (HasOption(EPetOption_Scaleable))
        m_pkChar->SetScale(GetScale());

    //m_owner.DetailLog();
    //m_pkChar->DetailLog();

    m_pkChar->SetEmpire(m_owner.GetEmpire());
    m_pkChar->SetName(petName);

    m_pkChar->Show(m_owner.GetMapIndex(), x, y, z);
    return m_pkChar->GetVID();
}

void CPetActor::Unsummon()
{
    if (!m_pkChar)
        return;

    m_pkChar->SetPet(nullptr);
    M2_DESTROY_CHARACTER(m_pkChar);
    m_pkChar = nullptr;
}

bool CPetActor::Update()
{
    bool executeNextAction = true;

    if (this->GetCharacter()->GetVictim() && !m_owner.IsDead()) { executeNextAction = this->Attack(); }
    else if (m_owner.IsDead() || GetDistanceToOwner() >= START_FOLLOW_DISTANCE_ATTACKING)
    {
        m_pkChar->SetVictim(nullptr);
        executeNextAction = true;
    }

    if (get_dword_time() > m_nextIdleActionTime)
        this->UpdateIdleActionAI(START_FOLLOW_DISTANCE / 2, START_FOLLOW_DISTANCE);

    if (executeNextAction)
    {
        if (this->IsSummoned() && HasOption(EPetOption_Followable))
            executeNextAction = this->UpdateFollowAI();
    }

    return true;
}

void CPetActor::SetScale(uint16_t scale)
{
    m_scale = scale;
    if (m_pkChar)
        m_pkChar->SetScale(m_scale);
}

void CPetActor::SetLevel(uint16_t level) { m_level = level; }

bool CPetActor::Attack(CHARACTER *pkVictim)
{
    if (!HasOption(EPetOption_Combatable))
        return true;

    if (pkVictim)
    {
        if ((!pkVictim->IsMonster() && !pkVictim->IsStone()) || pkVictim->IsDead())
            return true;

        if (m_pkChar->GetVictim())
            return true;
    }
    else
    {
        if (m_pkChar->GetVictim() && GetDistanceToOwner() >= START_FOLLOW_DISTANCE_ATTACKING)
        {
            m_pkChar->SetVictim(nullptr);
            return true;
        }

        pkVictim = m_pkChar->GetVictim();

        if (!pkVictim)
            return true;
    }

    m_pkChar->SetVictim(pkVictim);

    float fDist = DISTANCE_APPROX(m_pkChar->GetX() - pkVictim->GetX(), m_pkChar->GetY() - pkVictim->GetY());
    if (fDist >= (m_pkChar->GetMobAttackRange() * 1.15))
    {
        __CHARACTER_GotoNearTarget(m_pkChar, pkVictim);
        return true;
    }

    if (get_dword_time() - m_pkChar->GetLastAttackTime() >= 3000)
    {
        if (!m_pkChar->Attack(pkVictim, 0, 0)) // If the attack fails? Why did it fail? TODO
            m_pkChar->SetStateDuration(THECORE_SECS_TO_PASSES(1) / 2);
        else
        {
            if (!pkVictim->IsInSafezone())
            {
                m_pkChar->SetRotationToXY(pkVictim->GetX(), pkVictim->GetY());
                m_pkChar->SendMovePacket(FUNC_ATTACK, 0, m_pkChar->GetX(),
                                         m_pkChar->GetY(),
                                         0);

                auto key = MakeMotionKey(MOTION_MODE_GENERAL, MOTION_NORMAL_ATTACK);
                auto motion = GetMotionManager().Get(m_pkChar->GetRaceNum(), key);

                uint32_t duration = motion ? motion->duration : 2000;
                m_pkChar->SetStateDuration(THECORE_MSECS_TO_PASSES(duration));
            }
        }
    }

    return false;
}

bool CPetActor::UpdateIdleActionAI(float fMinDist, float fMaxDist)
{
    int32_t ownerX = m_owner.GetX();
    int32_t ownerY = m_owner.GetY();

    int32_t charX = m_pkChar->GetX();
    int32_t charY = m_pkChar->GetY();

    const auto dist = DISTANCE_APPROX(charX - ownerX, charY - ownerY);

    float fDist = Random::get(fMinDist, fMaxDist);
    float r = Random::get(0.0f, 359.0f);
    float dest_x = m_owner.GetX() + fDist * std::cos(r);
    float dest_y = m_owner.GetY() + fDist * std::sin(r);

    m_pkChar->SetNowWalking(true);

    if (!m_pkChar->IsStateMove())
        m_pkChar->Goto(dest_x, dest_y);
    else
        return false;

    m_nextIdleActionTime = get_dword_time() + Random::get(5000, 12000);

    return true;
}

int32_t CPetActor::GetDistanceToOwner()
{
    int32_t ownerX = m_owner.GetX();
    int32_t ownerY = m_owner.GetY();

    int32_t charX = m_pkChar->GetX();
    int32_t charY = m_pkChar->GetY();

    return DISTANCE_APPROX(charX - ownerX, charY - ownerY);
}

bool CPetActor::UpdateFollowAI()
{
    uint32_t currentTime = get_dword_time();

    int32_t ownerX = m_owner.GetX();
    int32_t ownerY = m_owner.GetY();

    const auto dist = GetDistanceToOwner();

    if (dist >= RESPAWN_DISTANCE)
    {
        float fx, fy;
        GetDeltaByDegree(m_owner.GetRotation(), -APPROACH, &fx, &fy);

        if (m_pkChar->Show(m_owner.GetMapIndex(), ownerX + fx, ownerY + fy))
            return true;
    }

    bool bRun = false;

    if (dist >= (START_FOLLOW_DISTANCE))
    {
        if (dist >= START_RUN_DISTANCE)
            bRun = true;

        m_pkChar->SetNowWalking(!bRun);

        auto followSuccess = Follow(APPROACH);

        m_pkChar->SetLastAttacked(currentTime);
        m_dwLastActionTime = currentTime;
    }

    return true;
}

bool CPetActor::Follow(int32_t minDistance) const
{
    if (!m_pkChar)
        return false;

    int32_t ownerX = m_owner.GetX();
    int32_t ownerY = m_owner.GetY();

    int32_t petX = m_pkChar->GetX();
    int32_t petY = m_pkChar->GetY();

    auto dist = DISTANCE_APPROX(ownerX - petX, ownerY - petY);
    if (dist <= minDistance)
        return false;

    Eigen::Vector2f owner(ownerX, ownerY);
    Eigen::Vector2f pet(petX, petY);

    Eigen::Vector2f pos = pet + (owner - pet).normalized() * (dist - minDistance);

    return !m_pkChar->Goto(pos.x() + 0.5f, pos.y() + 0.5f);
}

void CPetActor::SetAttackGrade(SocketValue val) { m_attGrade = gsl::narrow_cast<uint32_t>(val); }

///////////////////////////////////////////////////////////////////////////////////////
//  CPetSystem
///////////////////////////////////////////////////////////////////////////////////////

CPetSystem::CPetSystem(CHARACTER &owner)
    : m_owner(owner)
      , m_dwUpdatePeriod(400)
      , m_dwLastUpdateTime(0)
{
    // ctor
}

CPetSystem::~CPetSystem() { event_cancel(&m_pkPetSystemUpdateEvent); }

void CPetSystem::LaunchAttack(CHARACTER *pkVictim)
{
    if (!pkVictim)
        return;

    for (auto &itemPet : m_itemPets)
    {
        const auto &petActor = itemPet.second;

        if (petActor->IsSummoned())
            petActor->Attack(pkVictim);
    }
}

bool CPetSystem::Update()
{
    auto now = get_dword_time();

    if (m_dwUpdatePeriod + m_dwLastUpdateTime > now)
        return true;

    for (auto it = m_itemPets.begin(), end = m_itemPets.end(); it != end;)
    {
        const auto &petActor = it->second;
        if (!petActor->IsSummoned() || !petActor->Update())
            it = m_itemPets.erase(it);
        else
            ++it;
    }

    m_dwLastUpdateTime = now;
    return !m_itemPets.empty();
}

void CPetSystem::ProccessItemSettingsAttackPet(CPetActor::PetSettings &setting, CItem *item)
{
    setting.isScaleAblePet = item->GetValue(ITEM_VALUE_PET_IS_SCALEABLE) == 1;
    if (setting.isScaleAblePet)
    {
        setting.options |= CPetActor::EPetOptions::EPetOption_Scaleable;
        setting.scale = item->GetValue(ITEM_VALUE_PET_SCALE);

        const auto socketScaleBonus = item->GetSocket(1);
        if (socketScaleBonus) { setting.scale += socketScaleBonus; }
    }

    setting.isAttackingPet = item->GetValue(ITEM_VALUE_PET_IS_ATTACKING);
    if (setting.isAttackingPet)
    {
        setting.options |= CPetActor::EPetOptions::EPetOption_Combatable;

        const auto socketMobVnum = item->GetSocket(0);
        if (socketMobVnum)
            setting.mobVnum = socketMobVnum;

        setting.level = item->GetValue(ITEM_VALUE_ATT_PET_LEVEL);
    }

    setting.attackGradeBonus = item->GetValue(ITEM_VALUE_ATT_PET_GRADE_BONUS);
    const auto socketAttGrade = item->GetSocket(2);
    if (socketAttGrade) { setting.attackGradeBonus += socketAttGrade; }
}

CPetActor *CPetSystem::SummonItem(uint32_t mobVnum, uint32_t itemId,
                                  bool bSpawnFar,
                                  uint32_t options)
{
    const auto it = m_itemPets.find(itemId);
    if (it != m_itemPets.end())
    {
        SPDLOG_ERROR("Item %u already has a pet", itemId);
        return nullptr;
    }

    CPetActor::PetSettings setting{};
    setting.mobVnum = mobVnum;
    setting.options = options;

    auto *item = ITEM_MANAGER::instance().Find(itemId);
    if (item && !(item->GetItemType() == ITEM_TOGGLE && item->GetSubType() == TOGGLE_LEVEL_PET))
    {
        ProccessItemSettingsAttackPet(setting, item);
    }
    else if (item && (item->GetItemType() == ITEM_TOGGLE && item->GetSubType() == TOGGLE_LEVEL_PET))
    {
        setting.options |= CPetActor::EPetOptions::EPetOption_Scaleable;
        const auto valueScale = std::max<int32_t>(item->GetValue(2), 1);
        const auto socketScale = std::clamp<uint16_t>(item->GetSocket(ITEM_SOCKET_LEVEL_PET_LEVEL) / valueScale,
                                                      item->GetValue(0), item->GetValue(1));
        if (socketScale) { setting.scale = socketScale; }
        else { setting.scale = 20; }
    }

    auto petActor = std::make_unique<CPetActor>(m_owner, setting.mobVnum, setting.options, setting.scale);
    petActor->SetScale(setting.scale);
    petActor->SetLevel(setting.level);
    petActor->SetAttackGrade(setting.attackGradeBonus);

    const auto *pkMob = CMobManager::instance().Get(setting.mobVnum);
    std::string name = m_owner.GetName();
    const std::string petName = pkMob ? pkMob->szLocaleName : "";
    name.append("'s ").
         append(petName);

    petActor->Summon(std::move(name), bSpawnFar);

    if (!m_pkPetSystemUpdateEvent)
    {
        auto *info = AllocEventInfo<petsystem_event_info>();
        info->pPetSystem = this;
        m_pkPetSystemUpdateEvent = event_create(petsystem_update_event, info, 3);
    }

    const auto &[it2, success] = m_itemPets.emplace(itemId, std::move(petActor));
    if (!success)
        return nullptr;

    return it2->second.get();
}

void CPetSystem::UnsummonItem(uint32_t itemId)
{
    const auto it = m_itemPets.find(itemId);
    if (it == m_itemPets.end())
    {
        SPDLOG_ERROR("Item %u not found", itemId);
        return;
    }

    m_itemPets.erase(it);

    if (m_itemPets.empty())
        event_cancel(&m_pkPetSystemUpdateEvent);
}

CPetActor *CPetSystem::GetPetActorByItemId(uint32_t itemId)
{
    const auto it = m_itemPets.find(itemId);
    if (it == m_itemPets.end()) { return nullptr; }

    return it->second.get();
}
