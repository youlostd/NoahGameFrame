#ifndef METIN2_GAME_ITEMTYPES_HPP
#define METIN2_GAME_ITEMTYPES_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/ItemConstants.hpp>
#include <game/Constants.hpp>
#include <game/Types.hpp>

#include <xml/Types.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include <unordered_map>
#include <string>
#include <vector>

METIN2_BEGIN_NS

#pragma pack(push, 1)

typedef struct SDropInfo
{
	uint32_t dwVNum;
	int32_t iMinLevel, iMaxLevel;
	CountType count;
	float rarity;
} TDropInfo;

typedef std::vector<TDropInfo> TVecDropInfo;

struct TPlayerItemAttribute {
    ApplyType bType;
    ApplyValue sValue;
};

struct ItemLimit
{
	uint8_t type;
	int32_t value;
};

struct ItemApply
{
	ApplyType type;
	ApplyValue value;
};

struct ItemRefineType
{
	uint32_t refinedVnum;
	uint16_t refineSet;
};


struct ItemAttrProto
{
	static const uint32_t kVersion = 1;

	ApplyType apply;
	uint32_t prob;
	ApplyValue values[5];
	uint8_t maxBySet[ATTRIBUTE_SET_MAX_NUM];
};

struct RefineMaterial
{
	uint32_t vnum;
	uint32_t count;
};

struct RefineEnhanceMaterial
{
	uint32_t vnum;
	uint32_t count;
	uint8_t prob;
};

struct RefineProto
{
	static const uint32_t kVersion = 2;

	uint32_t id;
	uint32_t flags;
	Gold cost;
	uint8_t prob;
	std::vector<RefineMaterial> materials;
	std::vector<RefineEnhanceMaterial> enhance_materials;

};

struct TWikiRefineInfo {
	RefineMaterial materials[REFINE_MATERIAL_MAX_NUM];
	uint8_t material_count;
	Gold price;
};

typedef struct SWikiItemInfo {
	bool isSet;
	bool hasData;

	bool bIsCommon;
	uint32_t dwOrigin;


	std::vector<TWikiRefineInfo> pRefineData;
	std::vector<ItemVnum> pChestInfo;
	std::vector<TWikiItemOriginInfo> pOriginInfo;

	SWikiItemInfo()
	{
		isSet = false;
		hasData = false;
	}
} TWikiItemInfo;

struct ItemIdRange
{
	// Assigned range [min, max]
	uint32_t min;
	uint32_t max;

	// First unused ID in the range
	uint32_t start;
};

constexpr ItemIdRange kNullRange{ 0, 0, 0 };

inline bool operator==(const ItemIdRange& a, const ItemIdRange& b)
{
	return a.min == b.min && a.max == b.max && a.start == b.start;
}

inline bool operator!=(const ItemIdRange& a, const ItemIdRange& b)
{
	return !(a == b);
}

inline bool DoRangesCollide(const ItemIdRange& a, const ItemIdRange& b)
{
	return b.max >= a.min && b.min <= a.max;
}


struct ItemPosition
{
	uint8_t window_type;
	uint16_t cell;

	ItemPosition()
		: window_type(INVENTORY)
		, cell(std::numeric_limits<uint16_t>::max())
	{
		// ctor
	}

	ItemPosition(uint8_t window_type, uint16_t cell)
		: window_type(window_type)
		, cell(cell)
	{
		// ctor
	}

	explicit ItemPosition(std::tuple<uint8_t, uint16_t> t)
		: window_type(std::get<0>(t))
		, cell(std::get<1>(t))
	{
		// ctor
	}

	bool IsValidItemPosition() const
	{
		switch (window_type)
		{
		case RESERVED_WINDOW:
			return false;

		case INVENTORY:
			return cell < INVENTORY_MAX_NUM;
		case EQUIPMENT:
			return cell < WEAR_MAX_NUM;
		case SWITCHBOT:
			return cell < SWITCHBOT_SLOT_COUNT;
		case BELT_INVENTORY:
			return cell < BELT_INVENTORY_SLOT_COUNT;
		case DRAGON_SOUL_INVENTORY:
			return cell < (DRAGON_SOUL_INVENTORY_MAX_NUM);

		case SAFEBOX:
		case MALL:
		case DESTROYED_WINDOW:
			return false;
		default:
			return false;
		}
		return false;
	}
	
	bool IsEquipPosition() const
	{
		return window_type == EQUIPMENT;
	}

	bool IsSwitchbotPosition() const {
		return window_type == SWITCHBOT;
	}
	
	bool IsDragonSoulEquipPosition() const
	{
		return (DRAGON_SOUL_EQUIP_SLOT_START >= cell) && (DRAGON_SOUL_EQUIP_SLOT_END < cell) && (EQUIPMENT == window_type);
	}

	bool IsBeltInventoryPosition() const
	{
		return window_type == BELT_INVENTORY;
	}

	bool IsDefaultInventoryPosition() const
	{
		return INVENTORY == window_type && cell < INVENTORY_MAX_NUM;
	}

	bool IsNormalInventoryPosition() const
	{
		return INVENTORY == window_type && cell <= INVENTORY_PAGE_SIZE * NORMAL_INVENTORY_MAX_PAGE;
	}

	bool operator==(const struct ItemPosition& rhs) const
	{
		return (window_type == rhs.window_type) && (cell == rhs.cell);
	}
	bool operator!=(const struct ItemPosition& rhs) const
	{
		return (window_type != rhs.window_type) || (cell != rhs.cell);
	}
	bool operator<(const struct ItemPosition& rhs) const
	{
		return (window_type < rhs.window_type) || ((window_type == rhs.window_type) && (cell < rhs.cell));
	}
};

using TItemPos = ItemPosition;
using SItemPos = ItemPosition;

const ItemPosition NPOS (RESERVED_WINDOW, std::numeric_limits<uint16_t>::max());


//BOOST_FUSION_ADAPT_STRUCT(
//	ItemPosition,
//	window_type,
//	cell
//)

struct ClientItemData
{
	SocketValue GetSocket(uint32_t index) const
	{
		return sockets[index];
	}

	const TPlayerItemAttribute& GetAttr(uint32_t index) const
	{
		return attrs[index];
	}

	operator bool() const
	{
		return vnum != 0;
	}

	uint8_t highlighted = 0;
	uint32_t id;
	uint32_t vnum;
	uint32_t transVnum;
	int32_t nSealDate;
	CountType count;
	SocketValue sockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute attrs[ITEM_ATTRIBUTE_MAX_NUM];
};



typedef struct SPlayerItem {
	uint32_t id;
	uint8_t window;
	uint16_t pos;

	ClientItemData data;

	uint32_t owner;
	bool is_gm_owner;
	bool is_blocked;
	SocketValue price;

} TPlayerItem;

typedef struct SShopItemTable {
	uint32_t vnum;
	uint32_t transVnum;
	CountType count;

	TItemPos pos;     // PC 상점에만 이용
	Gold price;       // PC, shop_table_ex.txt 상점에만 이용
	uint8_t display_pos; // PC, shop_table_ex.txt 상점에만 이용, 보일 위치.

	bool operator!=(const struct SShopItemTable& other)
	{
		return (vnum != other.vnum || count != other.count ||
		        pos != other.pos || price != other.price ||
		        display_pos != other.display_pos);
	}
} TShopItemTable;

typedef struct SShopTable {
	uint32_t dwVnum;
	uint32_t dwNPCVnum;

	uint8_t byItemCount;
	TShopItemTable items[SHOP_HOST_ITEM_MAX_NUM];
} TShopTable;

typedef struct SShopTableEx : SShopTable
{
	std::string name;
	uint32_t coinType;
	uint32_t coinVnum = 0;
} TShopTableEx;

typedef struct SPlayerShopTable {
	uint32_t aid;
	uint32_t pid;
	uint32_t polyVnum;
	uint32_t titleType;
	char playerName[CHARACTER_NAME_MAX_LEN + 1];
	char shopName[SHOP_SIGN_MAX_LEN + 1];
	long x;
	long y;
	long mapIndex;
	int channel;
	uint32_t openTime;
	bool closed;
	TShopItemTable items[SHOP_HOST_ITEM_MAX_NUM];

	bool operator!=(const struct SPlayerShopTable& other)
	{
		if (aid != other.aid || pid != other.pid ||
		    strcmp(shopName, other.shopName) != 0 || x != other.x ||
		    y != other.y || channel != other.channel ||
		    mapIndex != other.mapIndex || closed != other.closed ||
		    openTime != other.openTime || polyVnum != other.polyVnum ||
		    titleType != other.titleType)
			return true;

		for (size_t i = 0; i < sizeof(items) / sizeof(TShopItemTable); ++i) {
			if (items[i] != other.items[i])
				return true;
		}

		return false;
	}
} TPlayerShopTable;

typedef struct SPlayerShopUpdate {
	uint32_t aid;
	uint32_t pid;
	uint32_t polyVnum;
	uint32_t titleType;
	char playerName[CHARACTER_NAME_MAX_LEN + 1];
	char shopName[SHOP_SIGN_MAX_LEN + 1];
	long x;
	long y;
	long mapIndex;
	int channel;
	uint32_t openTime;
	bool closed;
} TPlayerShopUpdate;

typedef struct SPlayerShopUpdateItem {
	uint32_t aid;
	uint32_t pid;
	uint32_t index;
	TShopItemTable item;
} TPlayerShopUpdateItem;

struct ReadShopTable : TPlayerShopTable {
	int offlineMinutes;
	int premiumMinutes;
};

typedef struct SPlayerShopTableCache : TPlayerShopTable {
	Gold goldStash;
	int offlineMinutesLeft;
	int premiumMinutesLeft;

} TPlayerShopTableCache;

typedef struct SOfflineShopItemData
{
	ClientItemData info;
	uint32_t	dwPosition;
	long long	llPrice;
} TOfflineShopItemData;


struct ShopSearchItemData
{
    TOfflineShopItemData data;
    uint32_t ownerVid;
    char ownerName[CHARACTER_NAME_MAX_LEN];
    uint32_t mapIndex;
    uint8_t channel;
};



#pragma pack(pop)


bool ParseProto(RefineProto& entry, const XmlNode* node);
XmlNode* FormatProto(const RefineProto& entry, XmlMemoryPool* pool);

bool ParseProto(ItemAttrProto& entry, const XmlNode* node);

METIN2_END_NS

namespace ShopSearchFilters
{
enum ShopSearchFilters : int32_t {
    NotSet = -1,
    Everything,
    Weapons,
    Armor,
    Costume,
    PetsAndMounts,
    DragonSoul,
    Chests,
    Useful,
};
}

namespace std
{

template <>
struct hash<METIN2_NS::ItemPosition>
{
	size_t operator()(const METIN2_NS::ItemPosition& p) const
	{
		uint32_t val = p.window_type;
		val <<= 16;
		val += p.cell;

		return val;
	}
};

}



BOOST_FUSION_ADAPT_STRUCT(
	METIN2_NS::ItemRefineType,
	refinedVnum,
	refineSet
);

BOOST_FUSION_ADAPT_STRUCT(
	METIN2_NS::ItemLimit,
	type,
	value
);

BOOST_FUSION_ADAPT_STRUCT(
	METIN2_NS::ItemApply,
	type,
	value
);

BOOST_FUSION_ADAPT_STRUCT(
	TPlayerItemAttribute,
	bType,
	sValue
);

BOOST_FUSION_ADAPT_STRUCT(
	TDropInfo,
	dwVNum,
	iMinLevel,
	iMaxLevel,
	count,
	rarity
);

BOOST_FUSION_ADAPT_STRUCT(
	METIN2_NS::ItemAttrProto,
	apply,
	prob,
	values,
	maxBySet
);

BOOST_FUSION_ADAPT_STRUCT(
	METIN2_NS::RefineMaterial,
	vnum,
	count
);

BOOST_FUSION_ADAPT_STRUCT(
	METIN2_NS::RefineEnhanceMaterial,
	vnum,
	count,
	prob
);

BOOST_FUSION_ADAPT_STRUCT(
	RefineProto,
	id,
	flags,
	cost,
	prob,
	materials,
	enhance_materials
);

BOOST_FUSION_ADAPT_STRUCT(
	ItemIdRange,
	min,
	max,
	start
);

BOOST_FUSION_ADAPT_STRUCT(
	ItemPosition,
	window_type,
	cell
);


BOOST_FUSION_ADAPT_STRUCT(
	ClientItemData,
	id,
	highlighted,
	vnum,
	transVnum,
	nSealDate,
	count,
	sockets,
	attrs
)

BOOST_FUSION_ADAPT_STRUCT(
    TWikiRefineInfo,
    materials,
    material_count,
    price
)

BOOST_FUSION_ADAPT_STRUCT(
    TShopItemTable,
    vnum,
    transVnum,
    count,
    pos,
    price,
    display_pos
)
BOOST_FUSION_ADAPT_STRUCT(
	TWikiItemOriginInfo,
	vnum,
	is_mob
);

BOOST_FUSION_ADAPT_STRUCT(
    TOfflineShopItemData,
    info,
    dwPosition,
    llPrice
)
BOOST_FUSION_ADAPT_STRUCT(
    ShopSearchItemData,
    data,
    ownerVid,
    ownerName,
    mapIndex,
    channel
)

#endif // METIN2_GAME_ITEMTYPES_HPP
