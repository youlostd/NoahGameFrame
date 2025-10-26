#include "constants.h"
#include "sectree_manager.h"
#include "item_manager.h"
#include "GBufferManager.h"
#include "Config.hpp"
#include "char.h"
#include "char_manager.h"
#include "guild.h"
#include "guild_manager.h"
#include "desc.h"
#include "desc_manager.h"
#include "questmanager.h"
#include "building.h"
#include "DbCacheSocket.hpp"
#include "vector.h"

#include <game/GamePacket.hpp>

#include <base/ClientDb.hpp>

#include <boost/math/constants/constants.hpp>

METIN2_BEGIN_NS

enum
{
    // ADD_SUPPLY_BUILDING
    BUILDING_INCREASE_GUILD_MEMBER_COUNT_SMALL = 14061,
    BUILDING_INCREASE_GUILD_MEMBER_COUNT_MEDIUM = 14062,
    BUILDING_INCREASE_GUILD_MEMBER_COUNT_LARGE = 14063,
    // END_OF_ADD_SUPPLY_BUILDING

    FLAG_VNUM = 14200,
    WALL_DOOR_VNUM = 14201,
    WALL_BACK_VNUM = 14202,
    WALL_LEFT_VNUM = 14203,
    WALL_RIGHT_VNUM = 14204,
};

using namespace building;

CObject::CObject(CLand *land, const ObjectProto *pProto, const TObject *pData)
    : CEntity{ENTITY_OBJECT}, m_pProto(pProto)
      , m_dwVID(0)
      , m_pkLand(land)
      , m_chNPC(nullptr) { std::memcpy(&m_data, pData, sizeof(TObject)); }

CObject::~CObject() { Destroy(); }

void CObject::Destroy()
{
    SECTREE_MANAGER::instance().ForAttrRegion(GetMapIndex(),
                                              GetX() + m_pProto->region[0],
                                              GetY() + m_pProto->region[1],
                                              GetX() + m_pProto->region[2],
                                              GetY() + m_pProto->region[3],
                                              m_data.xRot, m_data.yRot, m_data.zRot,
                                              ATTR_OBJECT,
                                              ATTR_REGION_MODE_REMOVE);

    CEntity::Destroy();

    if (GetSectree())
        GetSectree()->RemoveEntity(this);

    // <Factor> NPC should be destroyed in CHARACTER_MANAGER
    // BUILDING_NPC
    /*
    if (m_chNPC) {
        M2_DESTROY_CHARACTER(m_chNPC);
    }
    */

    RemoveSpecialEffect();
    // END_OF_BUILDING_NPC
}

// BUILDING_NPC
void CObject::Reconstruct(uint32_t dwVnum)
{
    const auto r = SECTREE_MANAGER::instance().GetMapRegion(m_data.mapIndex);
    if (!r)
        return;

    CLand *pLand = GetLand();
    pLand->RequestDeleteObject(GetID());
    pLand->RequestCreateObject(dwVnum, m_data.mapIndex, m_data.x, m_data.y, m_data.xRot, m_data.yRot, m_data.zRot,
                               false);
}

// END_OF_BUILDING_NPC

void CObject::EncodeInsertPacket(CEntity *entity)
{
    DESC *d;
    if (!(d = entity->GetDesc()))
        return;

	SPDLOG_DEBUG("ObjectInsertPacket vid {0} vnum {1} rot {2} {3} {4}",
			  m_dwVID, m_data.vnum, m_data.xRot, m_data.yRot, m_data.zRot);

    TPacketGCCharacterAdd pack = {};
    pack.dwVID = m_dwVID;
    pack.bType = CHAR_TYPE_BUILDING;
    pack.angle = m_data.zRot;
    pack.x = GetX();
    pack.y = GetY();
    pack.z = GetZ();
    pack.wRaceNum = m_data.vnum;
    pack.guildID = m_pkLand->GetGuildID();

    // 빌딩 회전 정보(벽일때는 문 위치)를 변환
    pack.xRot = unsigned(m_data.xRot);
    pack.yRot = unsigned(m_data.yRot);

    d->Send(HEADER_GC_CHARACTER_ADD, pack);
}

void CObject::EncodeRemovePacket(CEntity *entity)
{
    DESC *d;

    if (!(d = entity->GetDesc()))
        return;

    SPDLOG_DEBUG("ObjectRemovePacket vid {0}", m_dwVID);

    TPacketGCCharacterDelete pack;
    pack.id = m_dwVID;

    d->Send(HEADER_GC_CHARACTER_DEL, pack);
}

void CObject::SetVID(uint32_t dwVID) { m_dwVID = dwVID; }

bool CObject::Show(int32_t lMapIndex, int32_t x, int32_t y)
{
    SECTREE *tree = SECTREE_MANAGER::instance().Get(lMapIndex, x, y);

    if (!tree)
    {
        SPDLOG_ERROR("cannot find sectree by {0}x{1} mapindex {2}", x, y, lMapIndex);
        return false;
    }

    if (GetSectree())
    {
        GetSectree()->RemoveEntity(this);
        ViewCleanup();
    }

    m_data.mapIndex = lMapIndex;
    m_data.x = x;
    m_data.y = y;

    SetMapIndex(lMapIndex);
    SetXYZ(x, y, 0);

    tree->InsertEntity(this);
    UpdateSectree();

    SECTREE_MANAGER::instance().ForAttrRegion(GetMapIndex(),
                                              x + m_pProto->region[0],
                                              y + m_pProto->region[1],
                                              x + m_pProto->region[2],
                                              y + m_pProto->region[3],
                                              m_data.xRot, m_data.yRot, m_data.zRot,
                                              ATTR_OBJECT,
                                              ATTR_REGION_MODE_SET);
    return true;
}

void CObject::ApplySpecialEffect()
{
    // ADD_SUPPLY_BUILDING
    if (m_pProto->vnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_SMALL ||
        m_pProto->vnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_MEDIUM ||
        m_pProto->vnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_LARGE)
    {
        auto guild = CGuildManager::instance().FindGuild(m_pkLand->GetOwner());
        if (guild)
        {
            switch (m_pProto->vnum)
            {
            case BUILDING_INCREASE_GUILD_MEMBER_COUNT_SMALL:
                guild->SetMemberCountBonus(6);
                break;

            case BUILDING_INCREASE_GUILD_MEMBER_COUNT_MEDIUM:
                guild->SetMemberCountBonus(12);
                break;

            case BUILDING_INCREASE_GUILD_MEMBER_COUNT_LARGE:
                guild->SetMemberCountBonus(18);
                break;
            }

            if (gConfig.IsHostingMap(m_pkLand->GetMapIndex()))
                guild->BroadcastMemberCountBonus();
        }
    }
    // END_OF_ADD_SUPPLY_BUILDING
}

void CObject::RemoveSpecialEffect()
{
    // ADD_SUPPLY_BUILDING
    if (m_pProto->vnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_SMALL ||
        m_pProto->vnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_MEDIUM ||
        m_pProto->vnum == BUILDING_INCREASE_GUILD_MEMBER_COUNT_LARGE)
    {
        auto guild = CGuildManager::instance().FindGuild(m_pkLand->GetOwner());
        if (guild)
        {
            guild->SetMemberCountBonus(0);

            if (gConfig.IsHostingMap(m_pkLand->GetMapIndex()))
                guild->BroadcastMemberCountBonus();
        }
    }
    // END_OF_ADD_SUPPLY_BUILDING
}

// BUILDING_NPC
void CObject::RegenNPC()
{
    if (0 == m_pProto->npcVnum)
        return;

    auto guild = CGuildManager::instance().FindGuild(m_pkLand->GetOwner());
    if (!guild)
        return;

    // TODO(tim): This looks horribly broken!
    const float rot = m_data.zRot * 2.0f * boost::math::constants::pi<float>() / 360.0f;

    const int x = 0;
    const int y = std::max(m_pProto->region[1], m_pProto->region[3]) + 300;
    const int newX = (int)((x * cosf(rot)) + (y * sinf(rot)));
    const int newY = (int)((y * cosf(rot)) - (x * sinf(rot)));

    m_chNPC = g_pCharManager->SpawnMob(m_pProto->npcVnum,
                                       GetMapIndex(),
                                       GetX() + newX,
                                       GetY() + newY,
                                       GetZ(),
                                       false,
                                       (int)m_data.zRot);

    if (!m_chNPC)
    {
        SPDLOG_ERROR("Cannot create guild npc");
        return;
    }

    m_chNPC->SetGuild(guild);

    // 힘의 신전일 경우 길드 레벨을 길마에게 저장해놓는다
    if (m_pProto->vnum == 14061 ||
        m_pProto->vnum == 14062 ||
        m_pProto->vnum == 14063)
    {
        auto pPC = quest::CQuestManager::instance().GetPC(guild->GetMasterPID());
        if (pPC)
            pPC->SetFlag("alter_of_power.build_level", guild->GetLevel());
    }
}

// END_OF_BUILDING_NPC

////////////////////////////////////////////////////////////////////////////////////

CLand::CLand(TLand *pData) { memcpy(&m_data, pData, sizeof(TLand)); }

CLand::~CLand() { Destroy(); }

void CLand::Destroy()
{
    auto it = m_map_pkObject.begin();

    while (it != m_map_pkObject.end())
    {
        CObject *pkObj = (it++)->second;
        CManager::instance().UnregisterObject(pkObj);
        delete pkObj;
    }

    m_map_pkObject.clear();
    m_map_pkObjectByVID.clear();
}

const TLand &CLand::GetData() const { return m_data; }

void CLand::PutData(const TLand *data)
{
    std::memcpy(&m_data, data, sizeof(m_data));

    if (!m_data.guildId)
        return;

    const TMapRegion *r = SECTREE_MANAGER::instance().GetMapRegion(m_data.mapIndex);
    if (!r)
        return;

    const auto chars = g_pCharManager->GetCharactersByRaceNum(20040);
    for (auto ch : chars)
    {
        if (ch->GetMapIndex() != m_data.mapIndex)
            continue;

        int x = ch->GetX();
        int y = ch->GetY();

        if (x > m_data.x + m_data.width || x < m_data.x)
            continue;

        if (y > m_data.y + m_data.height || y < m_data.y)
            continue;

        M2_DESTROY_CHARACTER(ch);
    }
}

void CLand::InsertObject(CObject *pkObj)
{
    m_map_pkObject.insert(std::make_pair(pkObj->GetID(), pkObj));
    m_map_pkObjectByVID.insert(std::make_pair(pkObj->GetVID(), pkObj));
}

CObject *CLand::FindObject(uint32_t dwID)
{
    auto it = m_map_pkObject.find(dwID);
    if (it == m_map_pkObject.end())
        return nullptr;

    return it->second;
}

CObject *CLand::FindObjectByGroup(uint32_t dwGroupVnum)
{
    for (auto it = m_map_pkObject.begin(); it != m_map_pkObject.end(); ++it)
    {
        CObject *pObj = it->second;
        if (pObj->GetGroup() == dwGroupVnum)
            return pObj;
    }

    return nullptr;
}

CObject *CLand::FindObjectByVnum(uint32_t dwVnum)
{
    for (auto it = m_map_pkObject.begin(); it != m_map_pkObject.end(); ++it)
    {
        CObject *pObj = it->second;
        if (pObj->GetVnum() == dwVnum)
            return pObj;
    }

    return nullptr;
}

// BUILDING_NPC
CObject *CLand::FindObjectByNPC(CHARACTER *npc)
{
    if (!npc)
        return nullptr;

    for (auto it = m_map_pkObject.begin(); it != m_map_pkObject.end(); ++it)
    {
        CObject *pObj = it->second;
        if (pObj->GetNPC() == npc)
            return pObj;
    }

    return nullptr;
}

// END_OF_BUILDING_NPC

CObject *CLand::FindObjectByVID(uint32_t dwVID)
{
    auto it = m_map_pkObjectByVID.find(dwVID);
    if (it == m_map_pkObjectByVID.end())
        return nullptr;

    return it->second;
}

void CLand::DeleteObject(uint32_t dwID)
{
    CObject *pkObj;

    if (!(pkObj = FindObject(dwID)))
        return;

    SPDLOG_DEBUG("Land::DeleteObject {0}", dwID);
    CManager::instance().UnregisterObject(pkObj);
    M2_DESTROY_CHARACTER(pkObj->GetNPC());

    m_map_pkObject.erase(dwID);
    m_map_pkObjectByVID.erase(dwID);

    delete pkObj;
}

struct FIsIn
{
    int32_t sx, sy;
    int32_t ex, ey;

    bool bIn;

    FIsIn(int32_t sx, int32_t sy, int32_t ex, int32_t ey)
        : sx(sx), sy(sy), ex(ex), ey(ey), bIn(false)
    {
    }

    void operator()(CEntity *ent)
    {
        if (ent->IsType(ENTITY_CHARACTER))
        {
            CHARACTER *ch = (CHARACTER *)ent;
            if (ch->IsMonster()) { return; }
            if (sx <= ch->GetX() && ch->GetX() <= ex
                && sy <= ch->GetY() && ch->GetY() <= ey) { bIn = true; }
        }
    }
};

bool CLand::RequestUpdateObject(uint32_t dwID, int32_t lMapIndex, int32_t x, int32_t y, float xRot, float yRot,
                                float zRot, bool checkAnother)
{
    auto object = FindObject(dwID);
    if (!object)
    {
        SPDLOG_ERROR("Invalid Object  {0}", dwID);
        return false;
    }

    SECTREE_MANAGER &rkSecTreeMgr = SECTREE_MANAGER::instance();

    auto pkProto = CManager::instance().GetObjectProto(object->GetVnum());
    if (!pkProto)
    {
        SPDLOG_ERROR("Invalid Object vnum {0}", object->GetVnum());
        return false;
    }

    const TMapRegion *r = rkSecTreeMgr.GetMapRegion(lMapIndex);
    if (!r)
        return false;

    SPDLOG_DEBUG("RequestUpdateObject(vnum={0}, map={1}, pos=({2},{3}), rot=({4},{5},{6}) region({7}x{9})",
                dwID, lMapIndex, x, y, xRot, yRot, zRot, r->width, r->width);

    int sx = m_data.x;
    int ex = sx + m_data.width;
    int sy = m_data.y;
    int ey = sy + m_data.height;

    int osx = x + pkProto->region[0];
    int osy = y + pkProto->region[1];
    int oex = x + pkProto->region[2];
    int oey = y + pkProto->region[3];

    int32_t tsx = osx, tex = oex, tsy = osy, tey = oey;
    RotateRegion(tsx, tsy, tex, tey, xRot, yRot, zRot);

    if (tsx < sx || tex > ex || tsy < sy || tey > ey)
    {
        SPDLOG_ERROR("invalid position: object is outside of land region\n"
                     "LAND: {0} {1} ~ {2} {3}\nOBJ: {4} {5} ~ {6} {7}",
                     sx, sy, ex, ey, tsx, tsy, tex, tey);
        return false;
    }

    // ADD_BUILDING_ROTATION
    if (checkAnother)
    {
        if (rkSecTreeMgr.ForAttrRegion(lMapIndex, osx, osy, oex, oey,
                                       xRot, yRot, zRot,
                                       ATTR_OBJECT, ATTR_REGION_MODE_CHECK))
        {
            SPDLOG_ERROR("another object already exist");
            return false;
        }

        FIsIn f(tsx, tsy, tex, tey);
        rkSecTreeMgr.GetMap(lMapIndex)->for_each(f);
        if (f.bIn)
        {
            SPDLOG_ERROR("another object already exist");
            return false;
        }
    }
    // END_OF_BUILDING_NPC

    TPacketGDUpdateObject p;
    p.dwID = dwID;
    p.dwLandID = m_data.id;
    p.lMapIndex = lMapIndex;
    p.x = x;
    p.y = y;
    p.xRot = xRot;
    p.yRot = yRot;
    p.zRot = zRot;
    db_clientdesc->DBPacket(HEADER_GD_UPDATE_OBJECT, 0, &p, sizeof(TPacketGDUpdateObject));
    return true;
}

bool CLand::RequestCreateObject(uint32_t dwVnum, int32_t lMapIndex, int32_t x, int32_t y, float xRot, float yRot,
                                float zRot, bool checkAnother)
{
    SECTREE_MANAGER &rkSecTreeMgr = SECTREE_MANAGER::instance();

    auto pkProto = CManager::instance().GetObjectProto(dwVnum);
    if (!pkProto)
    {
        SPDLOG_ERROR("Invalid Object vnum {0}", dwVnum);
        return false;
    }

    const TMapRegion *r = rkSecTreeMgr.GetMapRegion(lMapIndex);
    if (!r)
        return false;

    SPDLOG_DEBUG("RequestCreateObject(vnum={0}, map={1}, pos=({2},{3}), rot=({4},{5},{6}) region({7}x{9})",
                dwVnum, lMapIndex, x, y, xRot, yRot, zRot, r->width, r->width);

    int sx = m_data.x;
    int ex = sx + m_data.width;
    int sy = m_data.y;
    int ey = sy + m_data.height;

    int osx = x + pkProto->region[0];
    int osy = y + pkProto->region[1];
    int oex = x + pkProto->region[2];
    int oey = y + pkProto->region[3];

    int32_t tsx = osx, tex = oex, tsy = osy, tey = oey;
    RotateRegion(tsx, tsy, tex, tey, xRot, yRot, zRot);

    if (tsx < sx || tex > ex || tsy < sy || tey > ey)
    {
        SPDLOG_ERROR("invalid position: object is outside of land region\n"
                     "LAND: {0} {1} ~ {2} {3}\nOBJ: {4} {5} ~ {6} {7}",
                     sx, sy, ex, ey, tsx, tsy, tex, tey);
        return false;
    }

    // ADD_BUILDING_ROTATION
    if (checkAnother)
    {
        if (rkSecTreeMgr.ForAttrRegion(lMapIndex, osx, osy, oex, oey,
                                       xRot, yRot, zRot,
                                       ATTR_OBJECT, ATTR_REGION_MODE_CHECK))
        {
            SPDLOG_ERROR("another object already exist");
            return false;
        }

        FIsIn f(tsx, tsy, tex, tey);
        rkSecTreeMgr.GetMap(lMapIndex)->for_each(f);
        if (f.bIn)
        {
            SPDLOG_ERROR("another object already exist");
            return false;
        }
    }
    // END_OF_BUILDING_NPC

    TPacketGDCreateObject p;
    p.dwVnum = dwVnum;
    p.dwLandID = m_data.id;
    p.lMapIndex = lMapIndex;
    p.x = x;
    p.y = y;
    p.xRot = xRot;
    p.yRot = yRot;
    p.zRot = zRot;
    db_clientdesc->DBPacket(HEADER_GD_CREATE_OBJECT, 0, &p, sizeof(TPacketGDCreateObject));
    return true;
}

void CLand::RequestDeleteObject(uint32_t dwID)
{
    if (!FindObject(dwID))
    {
        SPDLOG_ERROR("no object by id {0}", dwID);
        return;
    }

    db_clientdesc->DBPacket(HEADER_GD_DELETE_OBJECT, 0, &dwID, sizeof(uint32_t));
    SPDLOG_DEBUG("RequestDeleteObject id {0}", dwID);
}

void CLand::RequestDeleteObjectByVID(uint32_t dwVID)
{
    CObject *pkObj;

    if (!(pkObj = FindObjectByVID(dwVID)))
    {
        SPDLOG_ERROR("no object by vid {0}", dwVID);
        return;
    }

    uint32_t dwID = pkObj->GetID();
    db_clientdesc->DBPacket(HEADER_GD_DELETE_OBJECT, 0, &dwID, sizeof(uint32_t));
    SPDLOG_DEBUG("RequestDeleteObject vid {0} id {1}", dwVID, dwID);
}

void CLand::SetOwner(uint32_t dwGuild)
{
    if (m_data.guildId != dwGuild)
    {
        m_data.guildId = dwGuild;
        RequestUpdate(dwGuild);
    }
}

void CLand::RequestUpdate(uint32_t dwGuild)
{
    uint32_t a[2];

    a[0] = GetID();
    a[1] = dwGuild;

    db_clientdesc->DBPacket(HEADER_GD_UPDATE_LAND, 0, &a[0], sizeof(uint32_t) * 2);
    SPDLOG_DEBUG("RequestUpdate id {0} guild {1}", a[0], a[1]);
}

////////////////////////////////////////////////////////////////////////////////////

CManager::CManager()
{
}

CManager::~CManager()
{
    auto it = m_map_pkLand.begin();
    for (; it != m_map_pkLand.end(); ++it)
        delete it->second;
}

bool CManager::Initialize()
{
    m_objectProto.clear();

    std::vector<ObjectProto> v;
    if (!LoadClientDbFromFile("data/object_proto_server", v))
        return false;

    for (const auto &item : v)
        m_objectProto.emplace(item.vnum, item);

    for (auto it = m_objectProto.begin(); it != m_objectProto.end(); ++it)
    {
        for (uint32_t i = 0; i < OBJECT_MATERIAL_MAX_NUM; ++i)
        {
            if (!it->second.materials[i].itemVnum)
                break;

            if (!ITEM_MANAGER::instance().GetTable(it->second.materials[i].itemVnum))
            {
                SPDLOG_ERROR(
                    "Unknown material {0} for object {1}",
                    it->second.materials[i].itemVnum, it->first);
                return false;
            }
        }
    }
    return true;
}

void CManager::FinalizeBoot()
{
    auto it = m_map_pkObjByID.begin();

    while (it != m_map_pkObjByID.end())
    {
        CObject *pkObj = (it++)->second;

        pkObj->Show(pkObj->GetMapIndex(), pkObj->GetX(), pkObj->GetY());
        // BUILDING_NPC
        pkObj->RegenNPC();
        pkObj->ApplySpecialEffect();
        // END_OF_BUILDING_NPC
    }

    // BUILDING_NPC
    SPDLOG_INFO("BuildingManager:: Finalizing Boot");
    // END_OF_BUILDING_NPC

    auto it2 = m_map_pkLand.begin();

    while (it2 != m_map_pkLand.end())
    {
        CLand *pkLand = (it2++)->second;

        const TLand &r = pkLand->GetData();

        // LAND_MASTER_LOG
        SPDLOG_DEBUG("LandMaster map_index={0} pos=({1}, {2})", r.mapIndex, r.x, r.y);
        // END_OF_LAND_MASTER_LOG

        if (r.guildId != 0)
            continue;

        if (!gConfig.IsHostingMap(r.mapIndex))
            continue;

        const TMapRegion *region = SECTREE_MANAGER::instance().GetMapRegion(r.mapIndex);
        if (!region)
            continue;

        g_pCharManager->SpawnMob(20040, r.mapIndex, r.x + (r.width / 2), r.y + (r.height / 2), 0);
    }
}

const ObjectProto *CManager::GetObjectProto(uint32_t dwVnum) const
{
    auto it = m_objectProto.find(dwVnum);
    if (it == m_objectProto.end())
        return nullptr;

    return &it->second;
}

bool CManager::LoadLand(TLand *pTable) // from DB
{
    // MapAllow에 없는 맵의 땅일지라도 load를 해야한다.
    //	건물(object)이 어느 길드에 속해 있는지 알기 위해서는 건물이 세위진 땅이 어느 길드 소속인지 알아한다.
    //	만약 땅을 load해 놓지 않으면 길드 건물이 어느 길드에 소속된 건지 알지 못해서
    //	길드 건물에 의한 길드 버프를 받지 못한다.
    //if (!g_config.IsHostingMap(pTable->mapIndex))
    //	return false;

    CLand *pkLand = new CLand(pTable);
    m_map_pkLand.insert(std::make_pair(pkLand->GetID(), pkLand));

    SPDLOG_DEBUG("LAND: {0} map {1} {2}x{3} w {4} h {5}",
                pTable->id, pTable->mapIndex, pTable->x, pTable->y, pTable->width, pTable->height);

    return true;
}

void CManager::UpdateLand(TLand *pTable)
{
    CLand *pkLand = FindLand(pTable->id);

    if (!pkLand)
    {
        SPDLOG_ERROR("cannot find land by id {0}", pTable->id);
        return;
    }

    pkLand->PutData(pTable);

    const DESC_MANAGER::DESC_SET &cont = DESC_MANAGER::instance().GetClientSet();

    auto it = cont.begin();

    TPacketGCLandList p;
    TLandPacketElement e;

    e.dwID = pTable->id;
    e.x = pTable->x;
    e.y = pTable->y;
    e.width = pTable->width;
    e.height = pTable->height;
    e.dwGuildID = pTable->guildId;

    SPDLOG_INFO("BUILDING: UpdateLand {0} pos {1}x{2} guild {3}", e.dwID, e.x, e.y, e.dwGuildID);
    p.lands.emplace_back(e);

    CGuild *guild = CGuildManager::instance().FindGuild(pTable->guildId);
    while (it != cont.end())
    {
        const auto &d = *(it++);

        if (d->GetCharacter() && d->GetCharacter()->GetMapIndex() == pTable->mapIndex)
        {
            // we must send the guild name first
            d->GetCharacter()->SendGuildName(guild);
            d->Send(HEADER_GC_LAND_LIST, p);
        }
    }

}

CLand *CManager::FindLand(uint32_t dwID)
{
    auto it = m_map_pkLand.find(dwID);
    if (it == m_map_pkLand.end())
        return nullptr;

    return it->second;
}

CLand *CManager::FindLand(int32_t lMapIndex, int32_t x, int32_t y)
{
    SPDLOG_INFO("BUILDING: FindLand {0} {1} {2}", lMapIndex, x, y);

    const TMapRegion *r = SECTREE_MANAGER::instance().GetMapRegion(lMapIndex);

    if (!r)
        return nullptr;

    auto it = m_map_pkLand.begin();

    while (it != m_map_pkLand.end())
    {
        CLand *pkLand = (it++)->second;
        const TLand &r = pkLand->GetData();

        if (r.mapIndex != lMapIndex)
            continue;

        if (x < r.x || y < r.y)
            continue;

        if (x > r.x + r.width || y > r.y + r.height)
            continue;

        return pkLand;
    }

    return nullptr;
}

CLand *CManager::FindLandByGuild(uint32_t GID)
{
    auto it = m_map_pkLand.begin();

    while (it != m_map_pkLand.end())
    {
        CLand *pkLand = (it++)->second;

        if (pkLand->GetData().guildId == GID)
            return pkLand;
    }

    return nullptr;
}

bool CManager::UpdateObject(TObject *pTable) // from DB
{
    CLand *pkLand = FindLand(pTable->landId);

    if (!pkLand)
    {
        SPDLOG_INFO("Cannot find land by id {0}", pTable->landId);
        return false;
    }

    auto pkProto = GetObjectProto(pTable->vnum);
    if (!pkProto)
    {
        SPDLOG_ERROR("Cannot find object {0} in prototype (id {1})", pTable->vnum, pTable->id);
        return false;
    }

    SPDLOG_INFO("OBJUPD: id {0} vnum {1} map {2} pos {3}x{4}", pTable->id, pTable->vnum, pTable->mapIndex, pTable->x,
                pTable->y);

    CObject *pkObj = pkLand->FindObject(pTable->id);
    if (pkObj) { pkObj->Show(pTable->mapIndex, pTable->x, pTable->y); }

    return true;
}

bool CManager::LoadObject(TObject *pTable, bool isBoot) // from DB
{
    CLand *pkLand = FindLand(pTable->landId);

    if (!pkLand)
    {
        SPDLOG_INFO("Cannot find land by id {0}", pTable->landId);
        return false;
    }

    auto pkProto = GetObjectProto(pTable->vnum);
    if (!pkProto)
    {
        SPDLOG_ERROR("Cannot find object {0} in prototype (id {1})", pTable->vnum, pTable->id);
        return false;
    }

    SPDLOG_INFO("OBJ: id {0} vnum {1} map {2} pos {3}x{4}", pTable->id, pTable->vnum, pTable->mapIndex, pTable->x,
                pTable->y);

    CObject *pkObj = new CObject(pkLand, pkProto, pTable);

    uint32_t dwVID = g_pCharManager->AllocVID();
    pkObj->SetVID(dwVID);

    m_map_pkObjByVID.insert(std::make_pair(dwVID, pkObj));
    m_map_pkObjByID.insert(std::make_pair(pTable->id, pkObj));

    pkLand->InsertObject(pkObj);

    if (!isBoot)
        pkObj->Show(pTable->mapIndex, pTable->x, pTable->y);
    else
    {
        pkObj->SetMapIndex(pTable->mapIndex);
        pkObj->SetXYZ(pTable->x, pTable->y, 0);
    }

    // BUILDING_NPC
    if (!isBoot)
    {
        if (pkProto->npcVnum)
            pkObj->RegenNPC();

        pkObj->ApplySpecialEffect();
    }
    // END_OF_BUILDING_NPC

    return true;
}

void CManager::DeleteObject(uint32_t dwID) // from DB
{
    SPDLOG_INFO("OBJ_DEL: {0}", dwID);

    auto it = m_map_pkObjByID.find(dwID);
    if (it == m_map_pkObjByID.end())
        return;

    it->second->GetLand()->DeleteObject(dwID);
}

CObject *CManager::FindObjectByVID(uint32_t dwVID)
{
    auto it = m_map_pkObjByVID.find(dwVID);
    if (it == m_map_pkObjByVID.end())
        return nullptr;

    return it->second;
}

void CManager::UnregisterObject(CObject *pkObj)
{
    m_map_pkObjByID.erase(pkObj->GetID());
    m_map_pkObjByVID.erase(pkObj->GetVID());
}

void CManager::SendLandList(DESC *d, int32_t lMapIndex)
{
    std::vector<TLandPacketElement> es;

    auto it = m_map_pkLand.begin();
    
    while (it != m_map_pkLand.end())
    {
        CLand *pkLand = (it++)->second;
        const TLand &r = pkLand->GetData();

        if (r.mapIndex != lMapIndex)
            continue;

        //
        CHARACTER *ch = d->GetCharacter();
        if (ch)
        {
            CGuild *guild = CGuildManager::instance().FindGuild(r.guildId);
            ch->SendGuildName(guild);
        }
        //
        TLandPacketElement e;
        e.dwID = r.id;
        e.x = r.x;
        e.y = r.y;
        e.width = r.width;
        e.height = r.height;
        e.dwGuildID = r.guildId;

        es.emplace_back(e);
        
    }


    if (!es.empty())
    {
        TPacketGCLandList p;
        p.lands = es;
        d->Send(HEADER_GC_LAND_LIST, p);
    }
}

// LAND_CLEAR
void CManager::ClearLand(uint32_t dwLandID)
{
    CLand *pLand = FindLand(dwLandID);

    if (pLand == nullptr)
    {
        SPDLOG_INFO("LAND_CLEAR: there is no LAND id like {0}", dwLandID);
        return;
    }

    pLand->ClearLand();

    SPDLOG_INFO("LAND_CLEAR: request Land Clear. LandID: {0}", pLand->GetID());
}

void CManager::ClearLandByGuildID(uint32_t dwGuildID)
{
    CLand *pLand = FindLandByGuild(dwGuildID);

    if (pLand == nullptr)
    {
        SPDLOG_INFO("LAND_CLEAR: there is no GUILD id like {0}", dwGuildID);
        return;
    }

    pLand->ClearLand();

    SPDLOG_INFO("LAND_CLEAR: request Land Clear. LandID: {0}", pLand->GetID());
}

void CLand::ClearLand()
{
    auto iter = m_map_pkObject.begin();

    while (iter != m_map_pkObject.end())
    {
        RequestDeleteObject(iter->second->GetID());
        iter++;
    }

    SetOwner(0);

    const auto &r = GetData();
    g_pCharManager->SpawnMob(20040, r.mapIndex, r.x + (r.width / 2), r.y + (r.height / 2), 0);
}

// END_LAND_CLEAR

// BUILD_WALL
void CLand::DrawWall(uint32_t dwVnum, int32_t nMapIndex, int32_t &x, int32_t &y, char length, float zRot)
{
    int rot = (int)zRot;
    rot = ((rot % 360) / 90) * 90;

    int dx = 0, dy = 0;

    switch (rot)
    {
    case 0:
        dx = -500;
        dy = 0;
        break;

    case 90:
        dx = 0;
        dy = 500;
        break;

    case 180:
        dx = 500;
        dy = 0;
        break;

    case 270:
        dx = 0;
        dy = -500;
        break;
    }

    for (int i = 0; i < length; i++)
    {
        this->RequestCreateObject(dwVnum, nMapIndex, x, y, 0, 0, rot, false);
        x += dx;
        y += dy;
    }
}

bool CLand::RequestCreateWall(int32_t nMapIndex, float rot)
{
    const bool WALL_ANOTHER_CHECKING_ENABLE = false;

    const TLand &land = GetData();

    int center_x = land.x + land.width / 2;
    int center_y = land.y + land.height / 2;

    int wall_x = center_x;
    int wall_y = center_y;
    int wall_half_w = 1000;
    int wall_half_h = 1362;

    if (rot == 0.0f) // 남쪽 문
    {
        int door_x = wall_x;
        int door_y = wall_y + wall_half_h;
        RequestCreateObject(WALL_DOOR_VNUM, nMapIndex, wall_x, wall_y + wall_half_h, door_x, door_y, 0.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_BACK_VNUM, nMapIndex, wall_x, wall_y - wall_half_h, door_x, door_y, 0.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_LEFT_VNUM, nMapIndex, wall_x - wall_half_w, wall_y, door_x, door_y, 0.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_RIGHT_VNUM, nMapIndex, wall_x + wall_half_w, wall_y, door_x, door_y, 0.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
    }
    else if (rot == 180.0f) // 북쪽 문
    {
        int door_x = wall_x;
        int door_y = wall_y - wall_half_h;
        RequestCreateObject(WALL_DOOR_VNUM, nMapIndex, wall_x, wall_y - wall_half_h, door_x, door_y, 180.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_BACK_VNUM, nMapIndex, wall_x, wall_y + wall_half_h, door_x, door_y, 0.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_LEFT_VNUM, nMapIndex, wall_x - wall_half_w, wall_y, door_x, door_y, 0.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_RIGHT_VNUM, nMapIndex, wall_x + wall_half_w, wall_y, door_x, door_y, 0.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
    }
    else if (rot == 90.0f) // 동쪽 문
    {
        int door_x = wall_x + wall_half_h;
        int door_y = wall_y;
        RequestCreateObject(WALL_DOOR_VNUM, nMapIndex, wall_x + wall_half_h, wall_y, door_x, door_y, 90.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_BACK_VNUM, nMapIndex, wall_x - wall_half_h, wall_y, door_x, door_y, 90.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_LEFT_VNUM, nMapIndex, wall_x, wall_y - wall_half_w, door_x, door_y, 90.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_RIGHT_VNUM, nMapIndex, wall_x, wall_y + wall_half_w, door_x, door_y, 90.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
    }
    else if (rot == 270.0f) // 서쪽 문
    {
        int door_x = wall_x - wall_half_h;
        int door_y = wall_y;
        RequestCreateObject(WALL_DOOR_VNUM, nMapIndex, wall_x - wall_half_h, wall_y, door_x, door_y, 90.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_BACK_VNUM, nMapIndex, wall_x + wall_half_h, wall_y, door_x, door_y, 90.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_LEFT_VNUM, nMapIndex, wall_x, wall_y - wall_half_w, door_x, door_y, 90.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(WALL_RIGHT_VNUM, nMapIndex, wall_x, wall_y + wall_half_w, door_x, door_y, 90.0f,
                            WALL_ANOTHER_CHECKING_ENABLE);
    }
    // TODO: This is for guild flags more or less
    // properly should implement or remove this
    if (gConfig.testServer)
    {
        RequestCreateObject(FLAG_VNUM, nMapIndex, land.x + 50, land.y + 50, 0, 0, 0.0, WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(FLAG_VNUM, nMapIndex, land.x + land.width - 50, land.y + 50, 0, 0, 90.0,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(FLAG_VNUM, nMapIndex, land.x + land.width - 50, land.y + land.height - 50, 0, 0, 180.0,
                            WALL_ANOTHER_CHECKING_ENABLE);
        RequestCreateObject(FLAG_VNUM, nMapIndex, land.x + 50, land.y + land.height - 50, 0, 0, 270.0,
                            WALL_ANOTHER_CHECKING_ENABLE);
    }
    return true;
}

void CLand::RequestDeleteWall()
{
    auto iter = m_map_pkObject.begin();

    while (iter != m_map_pkObject.end())
    {
        unsigned id = iter->second->GetID();
        unsigned vnum = iter->second->GetVnum();

        switch (vnum)
        {
        case WALL_DOOR_VNUM:
        case WALL_BACK_VNUM:
        case WALL_LEFT_VNUM:
        case WALL_RIGHT_VNUM:
            RequestDeleteObject(id);
            break;
        }

        if (gConfig.testServer)
        {
            if (FLAG_VNUM == vnum)
                RequestDeleteObject(id);
        }

        iter++;
    }
}

bool CLand::RequestCreateWallBlocks(uint32_t dwVnum, int32_t nMapIndex, char wallSize, bool doorEast, bool doorWest,
                                    bool doorSouth, bool doorNorth)
{
    const TLand &r = GetData();

    int32_t startX = r.x + (r.width / 2) - (1300 + wallSize * 500);
    int32_t startY = r.y + (r.height / 2) + (1300 + wallSize * 500);

    uint32_t corner = dwVnum - 4;
    uint32_t wall = dwVnum - 3;
    uint32_t door = dwVnum - 1;

    bool checkAnother = false;
    int32_t *ptr = nullptr;
    int delta = 1;
    int rot = 270;

    bool doorOpen[4];
    doorOpen[0] = doorWest;
    doorOpen[1] = doorNorth;
    doorOpen[2] = doorEast;
    doorOpen[3] = doorSouth;

    if (wallSize > 3)
        wallSize = 3;
    else if (wallSize < 0)
        wallSize = 0;

    for (int i = 0; i < 4; i++, rot -= 90)
    {
        switch (i)
        {
        case 0:
            delta = -1;
            ptr = &startY;
            break;
        case 1:
            delta = 1;
            ptr = &startX;
            break;
        case 2:
            ptr = &startY;
            delta = 1;
            break;
        case 3:
            ptr = &startX;
            delta = -1;
            break;
        }

        this->RequestCreateObject(corner, nMapIndex, startX, startY, 0, 0, rot, checkAnother);

        *ptr = *ptr + (700 * delta);

        if (doorOpen[i])
        {
            this->DrawWall(wall, nMapIndex, startX, startY, wallSize, rot);

            *ptr = *ptr + (700 * delta);

            this->RequestCreateObject(door, nMapIndex, startX, startY, 0, 0, rot, checkAnother);

            *ptr = *ptr + (1300 * delta);

            this->DrawWall(wall, nMapIndex, startX, startY, wallSize, rot);
        }
        else { this->DrawWall(wall, nMapIndex, startX, startY, wallSize * 2 + 4, rot); }

        *ptr = *ptr + (100 * delta);
    }

    return true;
}

void CLand::RequestDeleteWallBlocks(uint32_t dwID)
{
    auto iter = m_map_pkObject.begin();

    uint32_t corner = dwID - 4;
    uint32_t wall = dwID - 3;
    uint32_t door = dwID - 1;
    uint32_t dwVnum = 0;

    while (iter != m_map_pkObject.end())
    {
        dwVnum = iter->second->GetVnum();

        if (dwVnum == corner || dwVnum == wall || dwVnum == door) { RequestDeleteObject(iter->second->GetID()); }
        iter++;
    }
}

// END_BUILD_WALL

METIN2_END_NS
