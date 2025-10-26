#ifndef METIN2_SERVER_GAME_BUILDING_HPP
#define METIN2_SERVER_GAME_BUILDING_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "entity.h"

#include <base/Singleton.hpp>

#include <game/BuildingTypes.hpp>

#include <unordered_map>
#include <vector>

METIN2_BEGIN_NS

class CHARACTER;

namespace building
{
class CLand;

class CObject : public CEntity
{
public:
    CObject(CLand *land, const ObjectProto *pProto, const TObject *pData);
    ~CObject();

    void Destroy();

    virtual void EncodeInsertPacket(CEntity *entity);
    virtual void EncodeRemovePacket(CEntity *entity);

    uint32_t GetID() const { return m_data.id; }

    void SetVID(uint32_t dwVID);

    uint32_t GetVID() const { return m_dwVID; }

    bool Show(int32_t lMapIndex, int32_t x, int32_t y);

    CLand *GetLand() { return m_pkLand; }

    uint32_t GetVnum() const { return m_pProto->vnum; }

    uint32_t GetGroup() const { return m_pProto->groupVnum; }

    void RegenNPC();

    // BUILDING_NPC
    void ApplySpecialEffect();
    void RemoveSpecialEffect();

    void Reconstruct(uint32_t dwVnum);

    CHARACTER *GetNPC() { return m_chNPC; }

    // END_OF_BUILDING_NPC

protected:
    const ObjectProto *m_pProto;
    TObject m_data;
    uint32_t m_dwVID;
    CLand *m_pkLand;
    CHARACTER *m_chNPC;
};

class CLand
{
public:
    CLand(TLand *pData);
    ~CLand();

    void Destroy();

    const TLand &GetData() const;
    void PutData(const TLand *data);

    uint32_t GetID() const { return m_data.id; }

    void SetOwner(uint32_t dwGID);

    uint32_t GetOwner() const { return m_data.guildId; }

    void InsertObject(CObject *pkObj);
    CObject *FindObject(uint32_t dwID);
    CObject *FindObjectByVID(uint32_t dwVID);
    CObject *FindObjectByVnum(uint32_t dwVnum);
    CObject *FindObjectByGroup(uint32_t dwGroupVnum);
    CObject *FindObjectByNPC(CHARACTER *npc);
    void DeleteObject(uint32_t dwID);

    bool RequestUpdateObject(uint32_t dwID, int32_t lMapIndex, int32_t x, int32_t y, float xRot, float yRot, float zRot,
                             bool checkAnother);
    bool RequestCreateObject(uint32_t dwVnum, int32_t lMapIndex, int32_t x, int32_t y, float xRot, float yRot,
                             float zRot, bool checkAnother);
    void RequestDeleteObject(uint32_t dwID);
    void RequestDeleteObjectByVID(uint32_t dwVID);

    void RequestUpdate(uint32_t dwGuild);

    // LAND_CLEAR
    void ClearLand();
    // END_LAND_CLEAR

    // BUILD_WALL
    bool RequestCreateWall(int32_t nMapIndex, float rot);
    void RequestDeleteWall();

    bool RequestCreateWallBlocks(uint32_t dwVnum, int32_t nMapIndex, char wallSize, bool doorEast, bool doorWest,
                                 bool doorSouth, bool doorNorth);
    void RequestDeleteWallBlocks(uint32_t dwVnum);
    // END_BUILD_WALL

    uint32_t GetMapIndex() const { return m_data.mapIndex; }

    uint32_t GetGuildID() const { return m_data.guildId; }

private:
    void DrawWall(uint32_t dwVnum, int32_t nMapIndex, int32_t &centerX, int32_t &centerY, char length, float zRot);

protected:
    TLand m_data;
    std::unordered_map<uint32_t, CObject *> m_map_pkObject;
    std::unordered_map<uint32_t, CObject *> m_map_pkObjectByVID;
};

class CManager : public singleton<CManager>
{
public:
    CManager();
    ~CManager();

    bool Initialize();

    void FinalizeBoot();

    const ObjectProto *GetObjectProto(uint32_t dwVnum) const;

    bool LoadLand(TLand *pTable);
    CLand *FindLand(uint32_t dwID);
    CLand *FindLand(int32_t lMapIndex, int32_t x, int32_t y);
    CLand *FindLandByGuild(uint32_t GID);
    void UpdateLand(TLand *pTable);

    bool LoadObject(TObject *pTable, bool isBoot = false);
    bool UpdateObject(TObject *pTable);

    void DeleteObject(uint32_t dwID);
    void UnregisterObject(CObject *pkObj);

    CObject *FindObjectByVID(uint32_t dwVID);

    void SendLandList(DESC *d, int32_t lMapIndex);

    // LAND_CLEAR
    void ClearLand(uint32_t dwLandID);
    void ClearLandByGuildID(uint32_t dwGuildID);
    // END_LAND_CLEAR

protected:
    std::unordered_map<uint32_t, ObjectProto> m_objectProto;

    std::unordered_map<uint32_t, CLand *> m_map_pkLand;
    std::unordered_map<uint32_t, CObject *> m_map_pkObjByID;
    std::unordered_map<uint32_t, CObject *> m_map_pkObjByVID;
};
}

METIN2_END_NS

#endif
