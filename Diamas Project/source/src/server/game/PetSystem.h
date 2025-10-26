#ifndef METIN2_SERVER_GAME_PETSYSTEM_H
#define METIN2_SERVER_GAME_PETSYSTEM_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <unordered_map>
#include <memory>
#include "event.h"

class CHARACTER;
class CItem;

class CPetActor //: public CHARACTER
{
    friend class CHARACTER;

public:
    struct PetSettings
    {
        uint32_t mobVnum = 0;
        SocketValue attackGradeBonus = 0;
        int32_t options = 0;
        uint16_t scale = 100;
        uint16_t level = 1;
        bool isAttackingPet = false;
        bool isScaleAblePet = false;
    };

    enum EPetOptions
    {
        EPetOption_Followable = 1 << 0,
        EPetOption_Mountable = 1 << 1,
        EPetOption_Summonable = 1 << 2,
        EPetOption_Combatable = 1 << 3,
        EPetOption_Scaleable = 1 << 4,
    };

    CPetActor(CHARACTER &owner, uint32_t vnum, uint32_t options = EPetOption_Followable | EPetOption_Summonable,
              uint16_t scale = 100);
    virtual ~CPetActor();

    CHARACTER &GetOwner() const { return m_owner; }

    CHARACTER *GetOwnerPtr() const { return &m_owner; }

    uint32_t GetVnum() const { return m_dwVnum; }

    CHARACTER *GetCharacter() const { return m_pkChar; }

    bool HasOption(EPetOptions option) const { return m_dwOptions & option; }

    bool Mount();
    void Unmount();
    bool Attack(CHARACTER *pkVictim = nullptr);
    unsigned Summon(std::string petName, bool bSpawnFar = false);
    void Unsummon();

    bool IsSummoned() const { return m_pkChar; }

    virtual bool Update();
    void SetScale(uint16_t scale);
    void SetLevel(uint16_t level);

    uint16_t GetScale() const { return m_scale; }

    uint16_t GetLevel() const { return m_level; }

    uint16_t GetScalePercent() const { return m_scale / 100.0f; }

    SocketValue GetAttackGrade() const { return m_attGrade; };

    void SetAttackGrade(SocketValue val);
    bool Follow(int32_t minDistance = 50) const;

protected:
    void OnDestroy() { m_pkChar = nullptr; }

    virtual bool UpdateFollowAI();                                   ///< ??? ????? AI ??
    virtual bool UpdateIdleActionAI(float fMinDist, float fMaxDist); ///< ?? ???? ?? ?? AI ??
    int32_t GetDistanceToOwner();

    const int START_FOLLOW_DISTANCE = 200;
    const int START_FOLLOW_DISTANCE_ATTACKING = 400;
    const int START_RUN_DISTANCE = 250;
    const int RESPAWN_DISTANCE = 3500;
    const int APPROACH = 150;

    CHARACTER &m_owner;

    uint32_t m_dwVnum;
    uint32_t m_dwOptions;
    uint32_t m_dwLastActionTime;
    uint32_t m_nextIdleActionTime = 0;
    uint16_t m_scale = 100;
    uint32_t m_attGrade = 0;
    uint16_t m_level = 1;

    CHARACTER *m_pkChar; // Instance of pet(CHARACTER)
};

class CPetSystem
{
    friend EVENTFUNC(petsystem_update_event);

public:
    CPetSystem(CHARACTER &owner);
    ~CPetSystem();

    bool Update();
    void ProccessItemSettingsAttackPet(CPetActor::PetSettings &setting, CItem *item);

    CPetActor *SummonItem(uint32_t mobVnum, uint32_t itemId,
                          bool bSpawnFar,
                          uint32_t options = CPetActor::EPetOption_Followable | CPetActor::EPetOption_Summonable);

    void UnsummonItem(uint32_t itemId);
    void LaunchAttack(CHARACTER *pkVictim = nullptr);

    CPetActor *GetPetActorByItemId(uint32_t itemId);

private:
    CHARACTER &m_owner; ///< ? ???? Owner

    // Pets owned by items
    std::unordered_map<uint32_t, std::unique_ptr<CPetActor>> m_itemPets;

    // Pet update
    uint32_t m_dwUpdatePeriod; ///< ???? ?? (ms??)
    uint32_t m_dwLastUpdateTime;
    LPEVENT m_pkPetSystemUpdateEvent;
};

#endif /* METIN2_SERVER_GAME_PETSYSTEM_H */
