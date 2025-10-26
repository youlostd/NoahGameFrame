#include "StdAfx.h"
#include "../eterLib/ResourceManager.h"
#include "../Main/GameType.h"

#include "ItemData.h"

CGraphicThing::Ptr CItemData::GetModelThing()
{
    EnsureAssetsLoaded();
    return m_modelThing;
}

CGraphicThing::Ptr CItemData::GetDropModelThing()
{
    EnsureAssetsLoaded();
    return m_dropModelThing;
}

CGraphicImage::Ptr CItemData::GetIconImage()
{
    EnsureAssetsLoaded();
    return m_iconImage;
}

std::string CItemData::GetIconImageFileName()
{
    EnsureAssetsLoaded();
    if (m_iconImage)
        return m_iconImage->GetFileName();
    return "";
}

void CItemData::EnsureAssetsLoaded()
{
    if (!m_loaded)
    {
        LoadModels();
        LoadIcon();
        m_loaded = true;
    }
}

uint32_t CItemData::GetAttachingDataCount()
{
    return m_AttachingDataVector.size();
}

bool CItemData::GetCollisionDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData **c_ppAttachingData)
{
    if (dwIndex >= GetAttachingDataCount())
        return FALSE;

    if (NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA != m_AttachingDataVector[dwIndex].dwType)
        return FALSE;

    *c_ppAttachingData = &m_AttachingDataVector[dwIndex];
    return TRUE;
}

bool CItemData::GetAttachingDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData **c_ppAttachingData)
{
    if (dwIndex >= GetAttachingDataCount())
        return FALSE;

    *c_ppAttachingData = &m_AttachingDataVector[dwIndex];
    return TRUE;
}

void CItemData::SetName(const std::string &c_rstName)
{
    m_strName = c_rstName;
}

void CItemData::SetSummary(const std::string &c_rstSumm)
{
    m_strSummary = c_rstSumm;
}

void CItemData::SetDescription(const std::string &c_rstDesc)
{
    m_strDescription = c_rstDesc;
}

void CItemData::SetDefaultItemData(const std::string &iconName, const std::string &modelFileName)
{
    if (!modelFileName.empty())
        m_strDropModelFileName = modelFileName;
    else
        m_strDropModelFileName = "d:/ymir work/item/etc/item_bag.gr2";

    m_strModelFileName = modelFileName;
    m_strIconFileName = iconName;
    m_strDescription.clear();
    m_strSummary.clear();
    m_strName.clear();

    m_loaded = false;
}

void CItemData::LoadModels()
{
    if (!m_strModelFileName.empty())
        m_modelThing = CResourceManager::Instance().LoadResource<CGraphicThing>(m_strModelFileName);

    m_dropModelThing = CResourceManager::Instance().LoadResource<CGraphicThing>(m_strDropModelFileName);
}

void CItemData::LoadIcon()
{
    auto &resMgr = CResourceManager::Instance();

    if (!m_strIconFileName.empty())
    {
        m_iconImage = resMgr.LoadResource<CGraphicImage>(m_strIconFileName);
        if (m_iconImage)
            return;

        SPDLOG_ERROR("Failed to load icon {} for item {}", m_strIconFileName, m_ItemTable.dwVnum);
    }

    m_iconImage = resMgr.LoadResource<CGraphicImage>("icon/item/27995.tga");
    if (m_iconImage)
        return;

    SPDLOG_WARN("Failed to load default icon for {}", m_ItemTable.dwVnum);
}

void CItemData::SetItemTableData(const TItemTable &pItemTable)
{
    m_ItemTable = pItemTable;
}

const TItemTable *CItemData::GetTable() const
{
    return &m_ItemTable;
}

uint32_t CItemData::GetIndex() const
{
    return m_ItemTable.dwVnum;
}

const char *CItemData::GetName() const
{
    if (m_strName.empty())
        return m_ItemTable.szLocaleName;
    return m_strName.c_str();
}

const char *CItemData::GetDescription() const
{
    return m_strDescription.c_str();
}

const char *CItemData::GetSummary() const
{
    return m_strSummary.c_str();
}

uint8_t CItemData::GetType() const
{
    return m_ItemTable.bType;
}

uint8_t CItemData::GetSubType() const
{
    return m_ItemTable.bSubType;
}

int CItemData::GetAttributeSetIndex()
{
    if (GetType() == ITEM_WEAPON)
    {
        switch (GetSubType())
        {
        case WEAPON_ARROW:
        case WEAPON_QUIVER:
            return -1;

        default:
            return ATTRIBUTE_SET_WEAPON;
        }
    }

    if (GetType() == ITEM_COSTUME)
    {
        switch (GetSubType())
        {
        case COSTUME_HAIR:
            return ATTRIBUTE_SET_COSTUME_HAIR;
        case COSTUME_BODY:
            return ATTRIBUTE_SET_COSTUME_BODY;
        case COSTUME_WEAPON:
            return ATTRIBUTE_SET_COSTUME_WEAPON;

        default:
            break;
        }
    }

    if (GetType() == ITEM_ARMOR)
    {
        switch (GetSubType())
        {
        case ARMOR_BODY:
            //			case COSTUME_BODY: // 코스츔 갑옷은 일반 갑옷과 동일한 Attribute Set을 이용하여 랜덤속성 붙음 (ARMOR_BODY == COSTUME_BODY)
            return ATTRIBUTE_SET_BODY;

        case ARMOR_WRIST:
            return ATTRIBUTE_SET_WRIST;

        case ARMOR_FOOTS:
            return ATTRIBUTE_SET_FOOTS;

        case ARMOR_NECK:
            return ATTRIBUTE_SET_NECK;

        case ARMOR_HEAD:
            //			case COSTUME_HAIR: // 코스츔 헤어는 일반 투구 아이템과 동일한 Attribute Set을 이용하여 랜덤속성 붙음 (ARMOR_HEAD == COSTUME_HAIR)
            return ATTRIBUTE_SET_HEAD;

        case ARMOR_SHIELD:
            return ATTRIBUTE_SET_SHIELD;

        case ARMOR_EAR:
            return ATTRIBUTE_SET_EAR;
        }
    }

    return -1;
}

bool CItemData::IsAttrChangeableCostume() const
{
    if (GetType() != ITEM_COSTUME)
        return false;

    switch (GetSubType())
    {
    case COSTUME_BODY:
    case COSTUME_HAIR:
    case COSTUME_WEAPON:
        return true;
    default:
        break;
    }
    return false;
}

#define DEF_STR(x) #x

const char *CItemData::GetUseTypeString() const
{
    if (GetType() != ITEM_USE)
        return "NOT_USE_TYPE";

    switch (GetSubType())
    {
    case USE_TUNING:
        return DEF_STR(USE_TUNING);
    case USE_DETACHMENT:
        return DEF_STR(USE_DETACHMENT);
    case USE_CLEAN_SOCKET:
        return DEF_STR(USE_CLEAN_SOCKET);
    case USE_CHANGE_ATTRIBUTE:
        return DEF_STR(USE_CHANGE_ATTRIBUTE);
    case USE_ADD_ATTRIBUTE:
        return DEF_STR(USE_ADD_ATTRIBUTE);
    case USE_ADD_ATTRIBUTE2:
        return DEF_STR(USE_ADD_ATTRIBUTE2);
    case USE_ADD_ACCESSORY_SOCKET:
        return DEF_STR(USE_ADD_ACCESSORY_SOCKET);
    case USE_PUT_INTO_ACCESSORY_SOCKET:
        return DEF_STR(USE_PUT_INTO_ACCESSORY_SOCKET);
    case USE_PUT_INTO_BELT_SOCKET:
        return DEF_STR(USE_PUT_INTO_BELT_SOCKET);
    case USE_PUT_INTO_RING_SOCKET:
        return DEF_STR(USE_PUT_INTO_RING_SOCKET);
    case USE_CHANGE_COSTUME_ATTR:
        return DEF_STR(USE_CHANGE_COSTUME_ATTR);
    case USE_RESET_COSTUME_ATTR:
        return DEF_STR(USE_RESET_COSTUME_ATTR);
    case USE_ADD_ATTRIBUTE_RARE:
        return DEF_STR(USE_ADD_ATTRIBUTE_RARE);
    case USE_CHANGE_ATTRIBUTE_RARE:
        return DEF_STR(USE_CHANGE_ATTRIBUTE_RARE);
    case USE_ENHANCE_TIME:
        return DEF_STR(USE_ENHANCE_TIME);
    case USE_CHANGE_ATTRIBUTE_PERM:
        return DEF_STR(USE_CHANGE_ATTRIBUTE_PERM);
    case USE_ADD_ATTRIBUTE_PERM:
        return DEF_STR(USE_ADD_ATTRIBUTE_PERM);
    case USE_MAKE_ACCESSORY_SOCKET_PERM:
        return DEF_STR(USE_MAKE_ACCESSORY_SOCKET_PERM);
    case USE_SET_TITLE:
        return DEF_STR(USE_SET_TITLE);
    case USE_LEVEL_PET_CHANGE_ATTR:
        return DEF_STR(USE_LEVEL_PET_CHANGE_ATTR);
    case USE_LEVEL_PET_FOOD:
        return DEF_STR(USE_LEVEL_PET_FOOD);
    case USE_BATTLEPASS:
        return DEF_STR(USE_BATTLEPASS);
    case USE_ADD_SOCKETS:
        return DEF_STR(USE_ADD_SOCKETS);
    }
    return "USE_UNKNOWN_TYPE";
}

uint32_t CItemData::GetWeaponType() const
{
    if (GetType() == ITEM_COSTUME && GetSubType() == COSTUME_WEAPON)
        return GetValue(3);

    return m_ItemTable.bSubType;
}

uint8_t CItemData::GetSize() const
{
    return m_ItemTable.bSize;
}

bool CItemData::IsAntiFlag(uint32_t dwFlag) const
{
    return (dwFlag & m_ItemTable.dwAntiFlags) != 0;
}

bool CItemData::IsFlag(uint32_t dwFlag) const
{
    return (dwFlag & m_ItemTable.dwFlags) != 0;
}

bool CItemData::IsWearableFlag(uint32_t dwFlag) const
{
    return (dwFlag & m_ItemTable.dwWearFlags) != 0;
}

bool CItemData::HasNextGrade() const
{
    return 0 != m_ItemTable.dwRefinedVnum;
}

uint32_t CItemData::GetWearFlags() const
{
    return m_ItemTable.dwWearFlags;
}

uint32_t CItemData::GetIBuyItemPrice() const
{
    return m_ItemTable.dwGold;
}

uint32_t CItemData::GetISellItemPrice() const
{
    return m_ItemTable.dwShopBuyPrice;
}

std::optional<TItemLimit> CItemData::GetLimit(uint8_t byIndex) const
{
    if (byIndex >= ITEM_LIMIT_MAX_NUM)
    {
        assert(byIndex < ITEM_LIMIT_MAX_NUM);
        return std::nullopt;
    }

    return m_ItemTable.aLimits[byIndex];
}

uint32_t CItemData::GetAntiFlags() const
{
    return m_ItemTable.dwAntiFlags;
}

uint32_t CItemData::GetFlags() const
{
    return m_ItemTable.dwFlags;
}

std::optional<TItemApply> CItemData::GetApply(uint8_t byIndex) const
{
    if (byIndex >= ITEM_APPLY_MAX_NUM)
    {
        return std::nullopt;
    }

    auto ItemApply = m_ItemTable.aApplies[byIndex];

    if ((APPLY_ATT_SPEED == ItemApply.bType) && (ITEM_WEAPON == GetType()) && (WEAPON_TWO_HANDED == GetSubType()))
    {
        ItemApply.lValue -= 10;
    }

    return ItemApply;
}


long CItemData::GetValue(uint8_t byIndex) const
{
    if (byIndex >= ITEM_VALUES_MAX_NUM)
    {
        assert(byIndex < ITEM_VALUES_MAX_NUM);
        return 0;
    }

    return m_ItemTable.alValues[byIndex];
}

//¼­¹ö¿Í µ¿ÀÏ ¼­¹ö ÇÔ¼ö º¯°æ½Ã °°ÀÌ º¯°æ!!(ÀÌÈÄ¿¡ ÇÕÄ£´Ù)
//SocketCount = 1 ÀÌ¸é ÃÊ±Þ¹«±â
//SocketCount = 2 ÀÌ¸é Áß±Þ¹«±â
//SocketCount = 3 ÀÌ¸é °í±Þ¹«±â
int CItemData::GetSocketCount() const
{
    return m_ItemTable.bGainSocketPct;
}

int CItemData::GetAddonType() const
{
    return m_ItemTable.sAddonType;
}

uint32_t CItemData::GetIconNumber() const
{
    return m_ItemTable.dwVnum;
    //!@#
    //	return m_ItemTable.dwIconNumber;
}

uint32_t CItemData::GetRefinedVnum() const
{
    return m_ItemTable.dwRefinedVnum;
    //!@#
    //	return m_ItemTable.dwIconNumber;
}

uint32_t CItemData::GetRefineSet() const
{
    return m_ItemTable.wRefineSet;
    //!@#
    //	return m_ItemTable.dwIconNumber;
}

UINT CItemData::GetSpecularPoweru() const
{
    return m_ItemTable.bSpecular;
}

float CItemData::GetSpecularPowerf() const
{
    UINT uSpecularPower = GetSpecularPoweru();

    return float(uSpecularPower) / 100.0f;
}

//refine °ªÀº ¾ÆÀÌÅÛ¹øÈ£ ³¡ÀÚ¸®¿Í ÀÏÄ¡ÇÑ´Ù-_-(Å×ÀÌºíÀÌ¿ëÀ¸·Î ¹Ù²Ü ¿¹Á¤)
UINT CItemData::GetRefine() const
{
    return GetIndex() % 10;
}

bool CItemData::IsEquipment() const
{
    switch (GetType())
    {
    case ITEM_WEAPON:
    case ITEM_ARMOR:
        return TRUE;
        break;
    }

    return FALSE;
}

#ifdef ENABLE_SHINING_SYSTEM
void CItemData::SetItemShiningTableData(const TItemShiningTable &data)
{
    m_ItemShiningTable = data;
}

#endif

void CItemData::Clear()
{
    m_strName.clear();
    m_strSummary.clear();
    m_strModelFileName.clear();
    m_strDropModelFileName.clear();
    m_strIconFileName.clear();

    m_modelThing = nullptr;
    m_dropModelThing = nullptr;
    m_iconImage = nullptr;

    memset(&m_ItemTable, 0, sizeof(m_ItemTable));
#ifdef ENABLE_SHINING_SYSTEM
    memset(&m_ItemShiningTable, 0, sizeof(m_ItemShiningTable));
#endif
#ifdef INGAME_WIKI
    m_isValidImage = true;
    m_wikiInfo.isSet = false;
    m_wikiInfo.hasData = false;
    m_isBlacklisted = false;
#endif
}

CItemData::CItemData()
{
    Clear();
}

CItemData::~CItemData()
{
}

bool CItemData::IsSealAbleItem() const
{
    if (GetType() == ITEM_WEAPON)
        return GetSubType() != WEAPON_ARROW && GetSubType() != WEAPON_QUIVER;

    return GetType() == ITEM_ARMOR || GetType() == ITEM_BELT;
}

void CItemData::SetItemTableScaleData(int dwJob, int dwSex, float fScaleX, float fScaleY, float fScaleZ, float fPosX,
                                      float fPosY, float fPosZ)
{
    hasScaleData = true;
    m_ItemScaleTable.scale[dwSex][dwJob].x = fScaleX;
    m_ItemScaleTable.scale[dwSex][dwJob].y = fScaleY;
    m_ItemScaleTable.scale[dwSex][dwJob].z = fScaleZ;
    m_ItemScaleTable.scalePos[dwSex][dwJob].x = fPosX;
    m_ItemScaleTable.scalePos[dwSex][dwJob].y = fPosY;
    m_ItemScaleTable.scalePos[dwSex][dwJob].z = fPosZ;
}

Vector3 &CItemData::GetItemScalePosition(int dwJob, int dwSex)
{
    return m_ItemScaleTable.scalePos[dwSex][dwJob];
}

Vector3 &CItemData::GetItemScale(int dwJob, int dwSex)
{
    return m_ItemScaleTable.scale[dwSex][dwJob];
}

bool CItemData::HasScaleData() const
{
    return hasScaleData;
}

const char *CItemData::GetModelThingPath() const
{
    return m_strModelFileName.c_str();
}
