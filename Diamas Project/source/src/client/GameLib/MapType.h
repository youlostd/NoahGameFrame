#ifndef METIN2_CLIENT_GAMELIB_MAPTYPE_H
#define METIN2_CLIENT_GAMELIB_MAPTYPE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../MilesLib/SoundManager.h"
class CProperty;
/////////////////////////////////////////////////////////////////
// Property

enum EPropertyType
{
    PROPERTY_TYPE_NONE,
    PROPERTY_TYPE_TREE,
    PROPERTY_TYPE_BUILDING,
    PROPERTY_TYPE_EFFECT,
    PROPERTY_TYPE_AMBIENCE,
    PROPERTY_TYPE_DUNGEON_BLOCK,
    PROPERTY_TYPE_MAX_NUM,
};

const char c_szPropertyTypeName[PROPERTY_TYPE_MAX_NUM][32] = {"None",   "Tree",     "Building",
                                                              "Effect", "Ambience", "DungeonBlock"};

const char c_szPropertyExtension[PROPERTY_TYPE_MAX_NUM][16] = {".pr", ".prt", ".prb", ".pre", ".pra", ".prd"};

uint32_t GetPropertyType(const char *c_szTypeName);
const char *GetPropertyExtension(uint32_t dwType);

struct TPropertyTree
{
    std::string strName;
    std::string strFileName;

    float fSize;
    float fVariance;
};

struct TPropertyBuilding
{
    std::string strName;
    std::string strFileName;
    std::string strAttributeDataFileName;
    bool isShadowFlag;
};

struct TPropertyEffect
{
    std::string strName;
    std::string strFileName;
};

enum EAmbiencePlayType
{
    AMBIENCE_PLAY_TYPE_ONCE,
    AMBIENCE_PLAY_TYPE_STEP,
    AMBIENCE_PLAY_TYPE_LOOP,
    AMBIENCE_PLAY_TYPE_MAX_NUM,
};

const char c_szPlayTypeName[AMBIENCE_PLAY_TYPE_MAX_NUM][8] = {
    "ONCE",
    "STEP",
    "LOOP",
};

struct TPropertyAmbience
{
    std::string strName;
    std::string strPlayType;

    float fPlayInterval;
    float fPlayIntervalVariation;
    float fMaxVolumeAreaPercentage;
    std::vector<std::string> AmbienceSoundVector;
};

struct TPropertyDungeonBlock
{
    std::string strName;
    std::string strFileName;
    std::string strAttributeDataFileName;
};

bool PropertyTreeDataToString(TPropertyTree *pData, CProperty *pProperty);
bool PropertyTreeStringToData(CProperty *pProperty, TPropertyTree *pData);
bool PropertyBuildingDataToString(TPropertyBuilding *pData, CProperty *pProperty);
bool PropertyBuildingStringToData(CProperty *pProperty, TPropertyBuilding *pData);
bool PropertyEffectDataToString(TPropertyEffect *pData, CProperty *pProperty);
bool PropertyEffectStringToData(CProperty *pProperty, TPropertyEffect *pData);
bool PropertyAmbienceDataToString(TPropertyAmbience *pData, CProperty *pProperty);
bool PropertyAmbienceStringToData(CProperty *pProperty, TPropertyAmbience *pData);
bool PropertyDungeonBlockDataToString(TPropertyDungeonBlock *pData, CProperty *pProperty);
bool PropertyDungeonBlockStringToData(CProperty *pProperty, TPropertyDungeonBlock *pData);

/////////////////////////////////////////////////////////////////

typedef struct SScreenPosition
{
    int x;
    int y;
} TScreenPosition;

typedef Vector3 TPixelPosition;

float SPixelPosition_CalculateDistanceSq3d(const TPixelPosition &c_rkPPosLeft, const TPixelPosition &c_rkPPosRight);

typedef struct SCellPosition
{
    SCellPosition()
    {
        this->x = 0;
        this->y = 0;
    }

    SCellPosition(int x, int y)
    {
        this->x = x;
        this->y = y;
    }

    int x, y;
} TCellPosition;

const int c_Section_xAttributeCellSize = 50;
const int c_Section_yAttributeCellSize = 50;
const int c_Section_xCellSize = 100;
const int c_Section_yCellSize = 100;

#endif /* METIN2_CLIENT_GAMELIB_MAPTYPE_H */
