#include "pythonitem.h"
#include "../EffectLib/EffectManager.h"
#include "../EterBase/StepTimer.h"
#include "../EterLib/Engine.hpp"
#include "../eterBase/Timer.h"
#include "../eterlib/GrpMath.h"
#include "../gamelib/ItemManager.h"
#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"
#include "PythonTextTail.h"
#include "base/Remotery.h"
#include "stdafx.h"

const float c_fDropStartHeight = 100.0f;
const float c_fDropTime = 0.5f;

std::string CPythonItem::TGroundItemInstance::ms_astDropSoundFileName[DROPSOUND_NUM];

void CPythonItem::GetInfo(std::string *pstInfo)
{
    pstInfo->append(
        fmt::format("Item: Inst {}, Pool {}", m_GroundItemInstanceMap.size(), m_GroundItemInstancePool.GetCapacity()));
}

void CPythonItem::TGroundItemInstance::Clear()
{
    stOwnership = "";
    ThingInstance.Clear();
    CEffectManager::Instance().DestroyEffectInstance(dwEffectInstanceIndex);
}

void CPythonItem::TGroundItemInstance::__PlayDropSound(uint32_t eItemType, const Vector3 &c_rv3Pos)
{
    if (eItemType >= DROPSOUND_NUM)
        return;

    CSoundManager::Instance().PlaySound3D(c_rv3Pos.x, c_rv3Pos.y, c_rv3Pos.z,
                                          ms_astDropSoundFileName[eItemType].c_str());
}

bool CPythonItem::TGroundItemInstance::Update()
{
    if (bAnimEnded)
        return false;
    if (dwEndTime < DX::StepTimer::instance().GetTotalMillieSeconds())
    {
        ThingInstance.SetRotationQuaternion(qEnd);

        /*Vector3 v3Adjust = -v3Center;
        Matrix mat;
        DirectX::SimpleMath::MatrixRotationYawPitchRoll(&mat,
        DirectX::XMConvertToRadians(rEnd.y),
        DirectX::XMConvertToRadians(rEnd.x),
        DirectX::XMConvertToRadians(rEnd.z));
        D3DXVec3TransformCoord(&v3Adjust,&v3Adjust,&mat);*/

        Quaternion qAdjust(-v3Center.x, -v3Center.y, -v3Center.z, 0.0f);
        Quaternion qc;
        qEnd.Conjugate(qc);
        qAdjust = qAdjust * qEnd;
        qAdjust = qc * qAdjust;

        ThingInstance.SetPosition(v3EndPosition.x + qAdjust.x, v3EndPosition.y + qAdjust.y,
                                  v3EndPosition.z + qAdjust.z);
        // ThingInstance.Update();
        bAnimEnded = true;

        __PlayDropSound(eDropSoundType, v3EndPosition);
    }
    else
    {
        uint64_t time = DX::StepTimer::instance().GetTotalMillieSeconds() - dwStartTime;
        uint64_t etime = dwEndTime - DX::StepTimer::instance().GetTotalMillieSeconds();
        float rate = time * 1.0f / (dwEndTime - dwStartTime);

        Vector3 v3NewPosition = v3EndPosition; // = rate*(v3EndPosition - v3StartPosition) +
        // v3StartPosition;
        v3NewPosition.z += 100 - 100 * rate * (3 * rate - 2); //-100*(rate-1)*(3*rate+2);

        Quaternion q = Quaternion::CreateFromAxisAngle(v3RotationAxis, etime * 0.03f * (-1 + rate * (3 * rate - 2)));
        // ThingInstance.SetRotation(rEnd.y + etime*rStart.y, rEnd.x +
        // etime*rStart.x, rEnd.z + etime*rStart.z);
        q = qEnd * q;

        ThingInstance.SetRotationQuaternion(q);
        Quaternion qAdjust(-v3Center.x, -v3Center.y, -v3Center.z, 0.0f);
        Quaternion qc;
        q.Conjugate(qc);
        qAdjust = qAdjust * q;
        qAdjust = qc * qAdjust;

        ThingInstance.SetPosition(v3NewPosition.x + qAdjust.x, v3NewPosition.y + qAdjust.y,
                                  v3NewPosition.z + qAdjust.z);

        /*Vector3 v3Adjust = -v3Center;
        Matrix mat;
        DirectX::SimpleMath::MatrixRotationYawPitchRoll(&mat,
        DirectX::XMConvertToRadians(rEnd.y + etime*rStart.y),
        DirectX::XMConvertToRadians(rEnd.x + etime*rStart.x),
        DirectX::XMConvertToRadians(rEnd.z + etime*rStart.z));

        D3DXVec3TransformCoord(&v3Adjust,&v3Adjust,&mat);
        //Tracef("%f %f %f\n",v3Adjust.x,v3Adjust.y,v3Adjust.z);
        v3NewPosition += v3Adjust;
        ThingInstance.SetPosition(v3NewPosition.x, v3NewPosition.y,
        v3NewPosition.z);*/
    }
    ThingInstance.Transform();
    ThingInstance.Deform();
    return !bAnimEnded;
}

void CPythonItem::Update(const POINT &c_rkPtMouse)
{
    TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.begin();
    for (; itor != m_GroundItemInstanceMap.end(); ++itor)
    {
        if (itor->second->dwVirtualNumber == 435224356)
            continue;

        itor->second->Update();
    }

    m_dwPickedItemID = __Pick(c_rkPtMouse);
}

void CPythonItem::Render()
{
    rmt_ScopedCPUSample(RenderGroundItems, 0);

    CPythonGraphic::Instance().SetDiffuseOperation();
    TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.begin();
    for (; itor != m_GroundItemInstanceMap.end(); ++itor)
    {
        if (itor->second->dwVirtualNumber == 435224356)
            continue;

        CGraphicThingInstance &rInstance = itor->second->ThingInstance;
        // rInstance.Update();
        rInstance.Render();
        rInstance.BlendRender();
    }
}

void CPythonItem::SetUseSoundFileName(uint32_t eItemType, const std::string &c_rstFileName)
{
    if (eItemType >= USESOUND_NUM)
        return;

    // SPDLOG_DEBUG("SetUseSoundFile %d : %s", eItemType, c_rstFileName.c_str());

    m_astUseSoundFileName[eItemType] = c_rstFileName;
}

void CPythonItem::SetDropSoundFileName(uint32_t eItemType, const std::string &c_rstFileName)
{
    if (eItemType >= DROPSOUND_NUM)
        return;

    SPDLOG_DEBUG("SetDropSoundFile {0} : {1}", eItemType, c_rstFileName.c_str());

    SGroundItemInstance::ms_astDropSoundFileName[eItemType] = c_rstFileName;
}

void CPythonItem::PlayUseSound(uint32_t dwItemID)
{
    // CItemManager& rkItemMgr=CItemManager::Instance();

    CItemData *pkItemData;
    if (!CItemManager::Instance().GetItemDataPointer(dwItemID, &pkItemData))
        return;

    uint32_t eItemType = __GetUseSoundType(*pkItemData);
    if (eItemType == USESOUND_NONE)
        return;
    if (eItemType >= USESOUND_NUM)
        return;

    CSoundManager::Instance().PlaySound2D(m_astUseSoundFileName[eItemType].c_str());
}

void CPythonItem::PlayDropSound(uint32_t dwItemID)
{
    // CItemManager& rkItemMgr=CItemManager::Instance();

    CItemData *pkItemData;
    if (!CItemManager::Instance().GetItemDataPointer(dwItemID, &pkItemData))
        return;

    uint32_t eItemType = __GetDropSoundType(*pkItemData);
    if (eItemType >= DROPSOUND_NUM)
        return;

    CSoundManager::Instance().PlaySound2D(SGroundItemInstance::ms_astDropSoundFileName[eItemType].c_str());
}

void CPythonItem::PlayUsePotionSound()
{
    CSoundManager::Instance().PlaySound2D(m_astUseSoundFileName[USESOUND_POTION].c_str());
}

uint32_t CPythonItem::__GetDropSoundType(const CItemData &c_rkItemData)
{
    switch (c_rkItemData.GetType())
    {
    case ITEM_WEAPON:
        switch (c_rkItemData.GetWeaponType())
        {
        case WEAPON_BOW:
            return DROPSOUND_BOW;
            break;
        case WEAPON_ARROW:
        case WEAPON_QUIVER:
            return DROPSOUND_DEFAULT;
            break;
        default:
            return DROPSOUND_WEAPON;
            break;
        }
        break;
    case ITEM_ARMOR:
        switch (c_rkItemData.GetSubType())
        {
        case ARMOR_NECK:
        case ARMOR_EAR:
            return DROPSOUND_ACCESSORY;
            break;
        case ARMOR_BODY:
            return DROPSOUND_ARMOR;
        default:
            return DROPSOUND_DEFAULT;
            break;
        }
        break;
    default:
        return DROPSOUND_DEFAULT;
        break;
    }

    return DROPSOUND_DEFAULT;
}

uint32_t CPythonItem::__GetUseSoundType(const CItemData &c_rkItemData)
{
    switch (c_rkItemData.GetType())
    {
    case ITEM_WEAPON:
        switch (c_rkItemData.GetWeaponType())
        {
        case WEAPON_BOW:
            return USESOUND_BOW;
            break;
        case WEAPON_ARROW:
        case WEAPON_QUIVER:
            return USESOUND_DEFAULT;
            break;
        default:
            return USESOUND_WEAPON;
            break;
        }
        break;
    case ITEM_ARMOR:
        switch (c_rkItemData.GetSubType())
        {
        case ARMOR_NECK:
        case ARMOR_EAR:
            return USESOUND_ACCESSORY;
            break;
        case ARMOR_BODY:
            return USESOUND_ARMOR;
        default:
            return USESOUND_DEFAULT;
            break;
        }
        break;
    case ITEM_USE:
        switch (c_rkItemData.GetSubType())
        {
        case USE_ABILITY_UP:
            return USESOUND_POTION;
            break;
        case USE_POTION:
            return USESOUND_NONE;
            break;
        case USE_TALISMAN:
            return USESOUND_PORTAL;
            break;
        default:
            return USESOUND_DEFAULT;
            break;
        }
        break;
    default:
        return USESOUND_DEFAULT;
        break;
    }

    return USESOUND_DEFAULT;
}

void CPythonItem::CreateItem(uint32_t dwVirtualID, uint32_t dwVirtualNumber, float x, float y, float z, bool bDrop)
{
    auto pItemData = CItemManager::Instance().GetProto(dwVirtualNumber);
    if (!pItemData)
        return;

    auto pItemModel = pItemData->GetDropModelThing();

    TGroundItemInstance *pGroundItemInstance = m_GroundItemInstancePool.Alloc();
    pGroundItemInstance->dwVirtualNumber = dwVirtualNumber;

    bool bStabGround = false;

    if (bDrop)
    {
        z = CPythonBackground::Instance().GetHeight(x, y) + 10.0f;

        if (pItemData->GetType() == ITEM_WEAPON &&
            (pItemData->GetSubType() == WEAPON_SWORD || pItemData->GetSubType() == WEAPON_ARROW ||
             pItemData->GetSubType() == WEAPON_QUIVER))
            bStabGround = true;

        bStabGround = false;
        pGroundItemInstance->bAnimEnded = false;
    }
    else
    {
        pGroundItemInstance->bAnimEnded = true;
    }

    {
        // attaching effect
        auto &mgr = CEffectManager::Instance();
        if (!CItemManager::instance().IsRareItem(dwVirtualNumber))
            pGroundItemInstance->dwEffectInstanceIndex = mgr.CreateEffect(m_dwDropItemEffectID);
        else
            pGroundItemInstance->dwEffectInstanceIndex = mgr.CreateEffect(m_dwDropItemSpecialEffectID);

        Matrix matWorld = Matrix::CreateTranslation(x, -y, z);

        auto* effect = mgr.GetEffectInstance(pGroundItemInstance->dwEffectInstanceIndex);
        if (effect)
            effect->SetGlobalMatrix(matWorld);

        pGroundItemInstance->eDropSoundType = __GetDropSoundType(*pItemData);
    }

    Vector3 normal;
    if (!CPythonBackground::Instance().GetNormal(int(x), int(y), &normal))
        normal = Vector3(0.0f, 0.0f, 1.0f);

    pGroundItemInstance->ThingInstance.Clear();
    pGroundItemInstance->ThingInstance.ReserveModelThing(1);
    pGroundItemInstance->ThingInstance.ReserveModelInstance(1);
    pGroundItemInstance->ThingInstance.RegisterModelThing(0, pItemModel);
    pGroundItemInstance->ThingInstance.SetModelInstance(0, 0, 0);
    if (bDrop)
    {
        pGroundItemInstance->v3EndPosition = Vector3(x, -y, z);
        pGroundItemInstance->ThingInstance.SetPosition(0, 0, 0);
    }
    else
        pGroundItemInstance->ThingInstance.SetPosition(x, -y, z);

    pGroundItemInstance->ThingInstance.Update();
    pGroundItemInstance->ThingInstance.Transform();
    pGroundItemInstance->ThingInstance.Deform();

    if (bDrop)
    {
        Vector3 vMin, vMax;
        pGroundItemInstance->ThingInstance.GetBoundBox(&vMin, &vMax);
        pGroundItemInstance->v3Center = (vMin + vMax) * 0.5f;

        std::pair<float, int> f[3] = {std::make_pair(vMax.x - vMin.x, 0), std::make_pair(vMax.y - vMin.y, 1),
                                      std::make_pair(vMax.z - vMin.z, 2)};

        std::sort(f, f + 3);

        // int no_rotation_axis=-1;

        Vector3 rEnd;

        if (/*f[1].first-f[0].first < (f[2].first-f[0].first)*0.30f*/ bStabGround)
        {
            // 뾰족
            if (f[2].second == 0) // axis x
            {
                rEnd.y = 90.0f + GetRandom(-15.0f, 15.0f);
                rEnd.x = GetRandom(0.0f, 360.0f);
                rEnd.z = GetRandom(-15.0f, 15.0f);
            }
            else if (f[2].second == 1) // axis y
            {
                rEnd.y = GetRandom(0.0f, 360.0f);
                rEnd.x = GetRandom(-15.0f, 15.0f);
                rEnd.z = 180.0f + GetRandom(-15.0f, 15.0f);
            }
            else // axis z
            {
                rEnd.y = 180.0f + GetRandom(-15.0f, 15.0f);
                rEnd.x = 0.0f + GetRandom(-15.0f, 15.0f);
                rEnd.z = GetRandom(0.0f, 360.0f);
            }
        }
        else
        {
            // 넓적
            // 땅의 노말의 영향을 받을 것
            if (f[0].second == 0)
            {
                // y,z = by normal
                pGroundItemInstance->qEnd =
                    RotationArc(Vector3(((float)GetRandom(0, 1)) * 2 - 1 + GetRandom(-0.1f, 0.1f),
                                            0 + GetRandom(-0.1f, 0.1f), 0 + GetRandom(-0.1f, 0.1f)),
                                Vector3(0, 0, 1) /*normal*/);
            }
            else if (f[0].second == 1)
            {
                pGroundItemInstance->qEnd = RotationArc(
                    Vector3(0 + GetRandom(-0.1f, 0.1f), ((float)GetRandom(0, 1)) * 2 - 1 + GetRandom(-0.1f, 0.1f),
                                0 + GetRandom(-0.1f, 0.1f)),
                    Vector3(0, 0, 1) /*normal*/);
            }
            else
            {
                pGroundItemInstance->qEnd =
                    RotationArc(Vector3(0 + GetRandom(-0.1f, 0.1f), 0 + GetRandom(-0.1f, 0.1f),
                                            ((float)GetRandom(0, 1)) * 2 - 1 + GetRandom(-0.1f, 0.1f)),
                                Vector3(0, 0, 1) /*normal*/);
            }
        }
        // D3DXQuaternionRotationYawPitchRoll(&pGroundItemInstance->qEnd, rEnd.y,
        // rEnd.x, rEnd.z );
        float rot = GetRandom(0.0f, 2 * 3.1415926535f);
        Quaternion q(0, 0, cosf(rot), sinf(rot));
        pGroundItemInstance->qEnd = pGroundItemInstance->qEnd * q;
        q = RotationArc(Vector3(0, 0, 1), normal);
        pGroundItemInstance->qEnd = pGroundItemInstance->qEnd * q;

        pGroundItemInstance->dwStartTime = DX::StepTimer::instance().GetTotalMillieSeconds();
        pGroundItemInstance->dwEndTime = pGroundItemInstance->dwStartTime + 300;
        pGroundItemInstance->v3RotationAxis.x = sinf(rot + 0); // GetRandom(0.4f,0.7f) * (2*(int)(random()%2) - 1);
        pGroundItemInstance->v3RotationAxis.y = cosf(rot + 0); // GetRandom(0.4f,0.7f) * (2*(int)(random()%2) - 1);
        pGroundItemInstance->v3RotationAxis.z = 0;             // GetRandom(0.4f,0.7f) * (2*(int)(random()%2) - 1);

        /*
        switch (no_rotation_axis)
        {
        case 0:
                pGroundItemInstance->rStart.x = 0;
                break;
        case 1:
                pGroundItemInstance->rStart.y = 0;
                break;
        case 2:
                pGroundItemInstance->rStart.z = 0;
                break;
        }*/

        Vector3 v3Adjust = -pGroundItemInstance->v3Center;
        Matrix mat = Matrix::CreateFromQuaternion(pGroundItemInstance->qEnd);
        /*DirectX::SimpleMath::MatrixRotationYawPitchRoll(&mat,
                DirectX::XMConvertToRadians(pGroundItemInstance->rEnd.y),
                DirectX::XMConvertToRadians(pGroundItemInstance->rEnd.x),
                DirectX::XMConvertToRadians(pGroundItemInstance->rEnd.z));*/

        v3Adjust = Vector3::Transform(v3Adjust, mat);
        // Tracef("%f %f %f\n",v3Adjust.x,v3Adjust.y,v3Adjust.z);
        // pGroundItemInstance->v3EndPosition += v3Adjust;
        // pGroundItemInstance->rEnd.z += pGroundItemInstance->v3Center.z;
    }

    pGroundItemInstance->ThingInstance.Show();

    m_GroundItemInstanceMap.insert(TGroundItemInstanceMap::value_type(dwVirtualID, pGroundItemInstance));

    CPythonTextTail &rkTextTail = CPythonTextTail::Instance();
    rkTextTail.RegisterItemTextTail(dwVirtualID, pItemData->GetName(), &pGroundItemInstance->ThingInstance);
}

void CPythonItem::SetOwnership(uint32_t dwVID, const char *c_pszName)
{
    TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.find(dwVID);

    if (m_GroundItemInstanceMap.end() == itor)
        return;

    TGroundItemInstance *pGroundItemInstance = itor->second;
    pGroundItemInstance->stOwnership.assign(c_pszName);

    CPythonTextTail &rkTextTail = CPythonTextTail::Instance();
    rkTextTail.SetItemTextTailOwner(dwVID, c_pszName);
}

bool CPythonItem::GetOwnership(uint32_t dwVID, const char **c_pszName)
{
    TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.find(dwVID);

    if (m_GroundItemInstanceMap.end() == itor)
        return false;

    TGroundItemInstance *pGroundItemInstance = itor->second;
    *c_pszName = pGroundItemInstance->stOwnership.c_str();

    return true;
}

void CPythonItem::DeleteAllItems()
{
    CPythonTextTail &rkTextTail = CPythonTextTail::Instance();

    TGroundItemInstanceMap::iterator i;
    for (i = m_GroundItemInstanceMap.begin(); i != m_GroundItemInstanceMap.end(); ++i)
    {
        TGroundItemInstance *pGroundItemInst = i->second;
        rkTextTail.DeleteItemTextTail(i->first);
        pGroundItemInst->Clear();
        m_GroundItemInstancePool.Free(pGroundItemInst);
    }
    m_GroundItemInstanceMap.clear();
}

void CPythonItem::DeleteItem(uint32_t dwVirtualID)
{
    TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.find(dwVirtualID);
    if (m_GroundItemInstanceMap.end() == itor)
        return;

    TGroundItemInstance *pGroundItemInstance = itor->second;
    pGroundItemInstance->Clear();
    m_GroundItemInstancePool.Free(pGroundItemInstance);
    m_GroundItemInstanceMap.erase(itor);

    // Text Tail
    CPythonTextTail::Instance().DeleteItemTextTail(dwVirtualID);
}

bool CPythonItem::GetCloseMoney(const TPixelPosition &c_rPixelPosition, uint32_t *pdwItemID, uint32_t dwDistance)
{
    uint32_t dwCloseItemID = 0;
    uint32_t dwCloseItemDistance = 1000 * 1000;

    TGroundItemInstanceMap::iterator i;
    for (i = m_GroundItemInstanceMap.begin(); i != m_GroundItemInstanceMap.end(); ++i)
    {
        TGroundItemInstance *pInstance = i->second;

        if (i->second->dwVirtualNumber == 435224356)
            continue;

        if (pInstance->dwVirtualNumber != VNUM_MONEY)
            continue;

        uint32_t dwxDistance = uint32_t(c_rPixelPosition.x - pInstance->v3EndPosition.x);
        uint32_t dwyDistance = uint32_t(c_rPixelPosition.y - (-pInstance->v3EndPosition.y));
        uint32_t dwDistance = uint32_t(dwxDistance * dwxDistance + dwyDistance * dwyDistance);

        if (dwxDistance * dwxDistance + dwyDistance * dwyDistance < dwCloseItemDistance)
        {
            dwCloseItemID = i->first;
            dwCloseItemDistance = dwDistance;
        }
    }

    if (dwCloseItemDistance > float(dwDistance) * float(dwDistance))
        return false;

    *pdwItemID = dwCloseItemID;

    return true;
}

uint32_t GetAntiFlagByRace(uint32_t race)
{
    switch (race)
    {
    case MAIN_RACE_WARRIOR_M:
    case MAIN_RACE_WARRIOR_W:
        return ITEM_ANTIFLAG_WARRIOR;
    case MAIN_RACE_ASSASSIN_W:
    case MAIN_RACE_ASSASSIN_M:
        return ITEM_ANTIFLAG_ASSASSIN;
    case MAIN_RACE_SURA_M:
    case MAIN_RACE_SURA_W:
        return ITEM_ANTIFLAG_SURA;
    case MAIN_RACE_SHAMAN_W:
    case MAIN_RACE_SHAMAN_M:
        return ITEM_ANTIFLAG_SHAMAN;
    case MAIN_RACE_WOLFMAN_M:
        return ITEM_ANTIFLAG_WOLFMAN;
    default:
        break;
    }

    return 0;
}

bool CPythonItem::IsIgnoredItem(DWORD dwVirtualNumber)
{
    CItemData *pkItemData;
    if (!CItemManager::instance().GetItemDataPointer(dwVirtualNumber, &pkItemData))
    {
        return true;
    }

    if (dwVirtualNumber == 435224356)
        return true;

    bool ignore_pickup_sword = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_SWORD);
    bool ignore_pickup_dagger = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_DAGGER);
    bool ignore_pickup_bow = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_BOW);
    bool ignore_pickup_twohand = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_TWO_HAND);
    bool ignore_pickup_bell = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_BELL);
    bool ignore_pickup_fan = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_FAN);

    bool ignore_pickup_armor = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_ARMOR);
    bool ignore_pickup_head = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_HEAD);
    bool ignore_pickup_shield = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_SHIELD);
    bool ignore_pickup_wrist = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_WRIST);
    bool ignore_pickup_foots = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_FOOTS);
    bool ignore_pickup_neck = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_NECK);
    bool ignore_pickup_ear = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_EAR);
    bool ignore_pickup_etc = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_ETC);
    bool ignore_pickup_talisman = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_TALISMAN);
    bool ignore_pickup_ring = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_RING);
    bool ignore_pickup_elixir = Engine::GetSettings().GetPickupIgnore(CClientConfig::PICKUP_IGNORE_ELIXIR);

    if (pkItemData->GetType() == ITEM_WEAPON)
    {
        bool ignored = false;

        switch (pkItemData->GetSubType())
        {
        case WEAPON_SWORD:
            ignored = ignore_pickup_sword;
            break;
        case WEAPON_DAGGER:
            ignored = ignore_pickup_dagger;
            break;
        case WEAPON_BOW:
            ignored = ignore_pickup_bow;
            break;
        case WEAPON_TWO_HANDED:
            ignored = ignore_pickup_twohand;
            break;
        case WEAPON_BELL:
            ignored = ignore_pickup_bell;
            break;
        case WEAPON_FAN:
            ignored = ignore_pickup_fan;
            break;
        }

        if (ignored)
        {
            return true;
        }
    }

    if(pkItemData->GetType() == ITEM_RING)
        return ignore_pickup_ring;

    if(pkItemData->GetType() == ITEM_TALISMAN)
        return ignore_pickup_talisman;

    if(pkItemData->GetType() == ITEM_TOGGLE && pkItemData->GetSubType() == TOGGLE_AFFECT)
        return ignore_pickup_elixir;

    if (pkItemData->GetType() == ITEM_ARMOR)
    {
        bool ignored = false;

        switch (pkItemData->GetSubType())
        {
        case ARMOR_BODY:
            ignored = ignore_pickup_armor;
            break;
        case ARMOR_HEAD:
            ignored = ignore_pickup_head;
            break;
        case ARMOR_SHIELD:
            ignored = ignore_pickup_shield;
            break;
        case ARMOR_WRIST:
            ignored = ignore_pickup_wrist;
            break;
        case ARMOR_FOOTS:
            ignored = ignore_pickup_foots;
            break;
        case ARMOR_NECK:
            ignored = ignore_pickup_neck;
            break;
        case ARMOR_EAR:
            ignored = ignore_pickup_ear;
            break;
        }

        if (ignored)
        {
            return true;
        }
    }

    if ((pkItemData->GetType() != ITEM_WEAPON && pkItemData->GetType() != ITEM_ARMOR) && ignore_pickup_etc)
    {
        return true;
    }

    return false;
}

std::vector<uint32_t> CPythonItem::GetCloseItem(const std::string &myName, const TPixelPosition &c_rPixelPosition,
                                                uint32_t *pdwItemID, uint32_t dwPickDistance)
{
    std::vector<uint32_t> pickupItems;
    pickupItems.reserve(20);

    uint32_t dwCloseItemDistance = 1000 * 1000;

    for (const auto &p : m_GroundItemInstanceMap)
    {
        if (p.second && IsIgnoredItem(p.second->dwVirtualNumber))
            continue;

        {
            // Dont collect armor/weapons if enabled option
            if (!Engine::GetSettings().IsCollectEquipment())
            {
                CItemData *pItemData;
                if (!CItemManager::Instance().GetItemDataPointer(
                        CPythonItem::Instance().GetVirtualNumberOfGroundItem(p.first), &pItemData))
                    continue;

                if (Engine::GetSettings().IsCollectUseableEquipment())
                {
                    auto inst = CPythonCharacterManager::instance().GetMainInstancePtr();
                    if (!inst)
                        continue;

                    if (pItemData->IsEquipment() && pItemData->IsAntiFlag(GetAntiFlagByRace(inst->GetRace())))
                        continue;
                }
                else
                {
                    if (pItemData->IsEquipment())
                        continue;
                }
            }
            // End of system
            if (p.second->dwVirtualNumber == 435224356)
                continue;

            pickupItems.push_back(p.first);
        }
    }
    return pickupItems;
}

bool CPythonItem::GetGroundItemPosition(uint32_t dwVirtualID, TPixelPosition *pPosition)
{
    TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.find(dwVirtualID);
    if (m_GroundItemInstanceMap.end() == itor)
        return FALSE;

    TGroundItemInstance *pInstance = itor->second;

    const Vector3 &rkD3DVct3 = pInstance->ThingInstance.GetPosition();

    pPosition->x = +rkD3DVct3.x;
    pPosition->y = -rkD3DVct3.y;
    pPosition->z = +rkD3DVct3.z;

    return TRUE;
}

uint32_t CPythonItem::__Pick(const POINT &c_rkPtMouse)
{
    auto &mgr = CPythonTextTail::Instance();
    auto id = mgr.Pick(c_rkPtMouse.x, c_rkPtMouse.y);

    if (-1 != id)
        return id;

    for (const auto &p : m_GroundItemInstanceMap)
    {
        float fu, fv, ft;
        if (p.second->ThingInstance.Intersect(&fu, &fv, &ft))
            return p.first;
    }

    return 0xffffffff;
}

bool CPythonItem::GetPickedItemID(uint32_t *pdwPickedItemID)
{
    if (INVALID_ID == m_dwPickedItemID)
        return false;

    *pdwPickedItemID = m_dwPickedItemID;
    return true;
}

uint32_t CPythonItem::GetVirtualNumberOfGroundItem(uint32_t dwVID)
{
    TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.find(dwVID);

    if (itor == m_GroundItemInstanceMap.end())
        return 0;
    else
        return itor->second->dwVirtualNumber;
}

void CPythonItem::BuildNoGradeNameData(int iType)
{
    /*
    CMapIterator<std::string, CItemData *> itor =
    CItemManager::Instance().GetItemNameMapIterator();

    m_NoGradeNameItemData.clear();
    m_NoGradeNameItemData.reserve(1024);

    while (++itor)
    {
    CItemData * pItemData = *itor;
    if (iType == pItemData->GetType())
            m_NoGradeNameItemData.push_back(pItemData);
    }
    */
}

uint32_t CPythonItem::GetNoGradeNameDataCount()
{
    return m_NoGradeNameItemData.size();
}

CItemData *CPythonItem::GetNoGradeNameDataPtr(uint32_t dwIndex)
{
    if (dwIndex >= m_NoGradeNameItemData.size())
        return NULL;

    return m_NoGradeNameItemData[dwIndex];
}

void CPythonItem::Destroy()
{
    DeleteAllItems();
    m_GroundItemInstancePool.Clear();
}

void CPythonItem::Create()
{
    CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/dropitem/dropitem.mse", &m_dwDropItemEffectID);
    CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/dropitem/dropitem2.mse",
                                              &m_dwDropItemSpecialEffectID);
}

CPythonItem::CPythonItem() : m_dwDropItemEffectID(0), m_dwDropItemSpecialEffectID(0), m_nMouseX(0), m_nMouseY(0)
{
    m_GroundItemInstancePool.SetName("CDynamicPool<TGroundItemInstance>");
    m_dwPickedItemID = INVALID_ID;
}

CPythonItem::~CPythonItem()
{
    assert(m_GroundItemInstanceMap.empty());
}
