#ifndef METIN2_CLIENT_GAMELIB_GAMETYPE_H
#define METIN2_CLIENT_GAMELIB_GAMETYPE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <string>

#include "../eterLib/CollisionData.h"
#include "../eterLib/TextFileLoader.h"
#include <base/GroupText.hpp>
#include <storm/io/File.hpp>

class GroupTextGroup;
extern float g_fGameFPS;

extern std::string g_strResourcePath;
extern std::string g_strImagePath;

// Race Data
namespace NRaceData
{
enum EJobs
{
    JOB_WARRIOR,
    JOB_ASSASSIN,
    JOB_SURA,
    JOB_SHAMAN,
    JOB_WOLFMAN,
    JOB_MAX_NUM
};

// Character Attacking Data
enum EAttackType
{
    ATTACK_TYPE_SPLASH,
    ATTACK_TYPE_SNIPE,
};
enum EHitType
{
    HIT_TYPE_NONE,
    HIT_TYPE_GREAT,
    HIT_TYPE_GOOD,
};
enum EMotionType
{
    MOTION_TYPE_NONE,
    MOTION_TYPE_NORMAL,
    MOTION_TYPE_COMBO,
    MOTION_TYPE_SKILL,
};

typedef CDynamicSphereInstance THitTimePosition;
typedef std::map<float, THitTimePosition> THitTimePositionMap;

typedef struct SAttackData
{
    int iAttackType = 0;
    int iHittingType = 0;
    float fInvisibleTime = 0.0f;
    float fExternalForce = 0.0f;
    float fStiffenTime = 0.0f;
    int iHitLimitCount = 0;
} TAttackData;

typedef struct SHitData
{
    float fAttackStartTime;
    float fAttackEndTime;

    float fWeaponLength;
    std::string strBoneName;

    THitTimePositionMap mapHitPosition;

    bool Load(CTextFileLoader &rTextFileLoader);
    bool Load(const GroupTextGroup *group);
} THitData;

typedef std::vector<THitData> THitDataContainer;

typedef struct SMotionAttackData : public SAttackData
{
    int iMotionType;

    THitDataContainer HitDataContainer;
} TMotionAttackData;

/////////////////////////////////////////////////////////////////////////////////
// Character Attaching Collision Data
enum ECollisionType
{
    COLLISION_TYPE_NONE,

    COLLISION_TYPE_BODY,
    COLLISION_TYPE_ATTACKING,
    COLLISION_TYPE_DEFENDING,
    COLLISION_TYPE_SPLASH,
};
enum ECollisionShape
{
    COLLISION_SHAPE_SPHERE,
    COLLISION_SHAPE_CYLINDER,
};

typedef struct SCollisionData
{
    int iCollisionType;
    CSphereCollisionInstanceVector SphereDataVector;

    SCollisionData()
    {
    }
    virtual ~SCollisionData()
    {
    }
} TCollisionData;

typedef struct SAttachingEffectData
{
    std::string strFileName;
    Vector3 v3Position;
    Vector3 v3Rotation;

    SAttachingEffectData()
    {
    }
    virtual ~SAttachingEffectData()
    {
    }
} TAttachingEffectData;

typedef struct SAttachingObjectData
{
    std::string strFileName;

    SAttachingObjectData()
    {
    }
    virtual ~SAttachingObjectData()
    {
    }
} TAttachingObjectData;

typedef std::vector<TCollisionData> TCollisionDataVector;

void DestroySystem();

void SaveAttackData(FILE *File, int iTabCount, const TAttackData &c_rData);
void SaveMotionAttackData(FILE *File, int iTabCount, const TMotionAttackData &c_rData);
void SaveCollisionData(FILE *File, int iTabCount, const TCollisionData &c_rCollisionData);
void SaveEffectData(FILE *File, int iTabCount, const TAttachingEffectData &c_rEffectData);
void SaveObjectData(FILE *File, int iTabCount, const TAttachingObjectData &c_rObjectData);

void SaveAttackData(storm::File &File, int iTabCount, const TAttackData &c_rData);
void SaveMotionAttackData(storm::File &File, int iTabCount, const TMotionAttackData &c_rData);
void SaveCollisionData(storm::File &File, int iTabCount, const TCollisionData &c_rCollisionData);
void SaveEffectData(storm::File &File, int iTabCount, const TAttachingEffectData &c_rEffectData);
void SaveObjectData(storm::File &File, int iTabCount, const TAttachingObjectData &c_rObjectData);

bool LoadAttackData(CTextFileLoader &rTextFileLoader, TAttackData *pData);
bool LoadAttackData(const GroupTextGroup *group, TAttackData *pData);
bool LoadMotionAttackData(const GroupTextGroup *group, TMotionAttackData *pData);
bool LoadMotionAttackData(CTextFileLoader &rTextFileLoader, TMotionAttackData *pData);
bool LoadCollisionData(const GroupTextGroup *group, TCollisionData *pCollisionData);
bool LoadCollisionData(CTextFileLoader &rTextFileLoader, TCollisionData *pCollisionData);
bool LoadEffectData(CTextFileLoader &rTextFileLoader, TAttachingEffectData *pEffectData);
bool LoadObjectData(CTextFileLoader &rTextFileLoader, TAttachingObjectData *pObjectData);

/////////////////////////////////////////////////////////////////////////////////
// Attaching Data
enum EAttachingDataType
{
    ATTACHING_DATA_TYPE_NONE,

    ATTACHING_DATA_TYPE_COLLISION_DATA,
    ATTACHING_DATA_TYPE_EFFECT,
    ATTACHING_DATA_TYPE_OBJECT,

    ATTACHING_DATA_TYPE_MAX_NUM,
};

typedef struct SAttachingData
{
    SAttachingData()
        : dwType(0), isAttaching(false), dwAttachingModelIndex(0), pCollisionData(NULL), pEffectData(NULL),
          pObjectData(NULL)
    {
    }
    uint32_t dwType;

    bool isAttaching;
    uint32_t dwAttachingModelIndex;
    std::string strAttachingBoneName;

    TCollisionData *pCollisionData;
    TAttachingEffectData *pEffectData;
    TAttachingObjectData *pObjectData;
} TAttachingData;

typedef std::vector<TAttachingData> TAttachingDataVector;

void SaveAttachingData(FILE *File, int iTabCount, const TAttachingDataVector &c_rAttachingDataVector);
void SaveAttachingData(storm::File &File, int iTabCount, const TAttachingDataVector &c_rAttachingDataVector);

bool LoadAttachingData(CTextFileLoader &rTextFileLoader, TAttachingDataVector *pAttachingDataVector);

extern CDynamicPool<TCollisionData> g_CollisionDataPool;
extern CDynamicPool<TAttachingEffectData> g_EffectDataPool;
extern CDynamicPool<TAttachingObjectData> g_ObjectDataPool;
};     // namespace NRaceData
#endif /* METIN2_CLIENT_GAMELIB_GAMETYPE_H */
