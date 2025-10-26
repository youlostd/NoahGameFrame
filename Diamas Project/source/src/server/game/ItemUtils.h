#ifndef METIN2_SERVER_GAME_ITEMUTILS_H
#define METIN2_SERVER_GAME_ITEMUTILS_H

#include <Config.hpp>
#include <variant>

#include <game/ItemTypes.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CItem;
class CHARACTER;
bool IsSummonableZone(int map_index);
static bool IS_MONKEY_DUNGEON(int map_index);
bool IsSummonItem(int vnum);

bool ConflictsWithEquipped(CHARACTER *ch, const CItem *item,
                           const CItem *except,
                           std::initializer_list<int16_t> cells);

bool CanRemove(CHARACTER *ch, const CItem *item);

bool CanAdd(CHARACTER *ch, const CItem *item, const TItemPos &pos,
            const CItem *except = nullptr, bool skipTimeChecks = false);
bool CanModifyItem(const CItem *item);
bool CanStack(const CItem *from, const CItem *to);
CountType Stack(CItem *from, CItem *to, CountType count);
CountType GetItemMaxCount(std::variant<const CItem *, const uint32_t> v);
//
// Toggle items
//

bool ActivateToggleItem(CHARACTER *ch, CItem *item);
void DeactivateToggleItem(CHARACTER *ch, CItem *item);

namespace events
{
namespace Item
{
namespace Toggle
{
void OnCreate(CItem *item);
void OnLoad(CHARACTER *ch, CItem *item);
void OnRemove(CHARACTER *ch, CItem *item);
bool OnUse(CHARACTER *ch, CItem *item);
}

void OnCreate(CItem *item);
void OnLoad(CHARACTER *ch, CItem *item);
void OnRemove(CHARACTER *ch, CItem *item);
bool OnUse(CHARACTER *ch, CItem *item);
void OnEquip(CHARACTER *ch, CItem *item);
bool OnUnequip(CHARACTER *ch, CItem *item);
}
}

CItem *FindToggleItem(CHARACTER *ch, bool active,
                      int32_t subType = -1,
                      int32_t group = -1,
                      CItem *except = nullptr);
void ProcessAutoRecoveryItem(CHARACTER *ch, CItem *item);

bool CanTradeRestrictedItem(CItem *item, CHARACTER *player);
bool CanTradeRestrictedItem(const TItemTable *item, CHARACTER *player);

bool CanModifyCostumeAttr(CItem *item, CItem *itemTo);
bool CanModifyItemAttr(CItem *item, CItem *itemTo, CHARACTER *ch);
bool CanUseItem(CHARACTER *ch, const CItem *item);
bool CheckItemSex(CHARACTER *ch, const CItem *item);
bool CheckItemEmpire(CHARACTER *ch, const CItem *item);

bool CheckItemLimits(CHARACTER *ch, const CItem *item, bool showMessages);
bool CheckItemRace(CHARACTER *ch, const CItem *item);

bool CanEquipItem(CHARACTER *ch, const CItem *item,
                  const CItem *except = nullptr, bool skipTimeChecks = false, bool showMessages = true);
bool CanUnequipItem(CHARACTER *ch, const CItem *item, const TItemPos &swapCell = NPOS);
bool IsOpenPvPSash(const CItem *sash);
uint8_t GetRefineLevel(const TItemTable *proto);
int16_t FindEquipmentCell(CHARACTER *ch, const CItem *item);
bool IsEquipmentCell(const CItem *item, uint8_t cell);
bool SwitchbotCheckItem(CItem *item);
bool SwitchbotIsRareConfigured(CHARACTER *ch, uint32_t slotIndex);

#endif /* METIN2_SERVER_GAME_ITEMUTILS_H */
