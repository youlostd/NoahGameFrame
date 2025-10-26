#ifndef METIN2_SERVER_GAME_MINING_H
#define METIN2_SERVER_GAME_MINING_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/Types.hpp>

class CHARACTER;
class CItem;

namespace mining
{
LPEVENT CreateMiningEvent(CHARACTER *ch, CHARACTER *load, int count);
uint32_t GetRawOreFromLoad(uint32_t dwLoadVnum);
bool OreRefine(CHARACTER *ch, CHARACTER *npc, CItem *item, Gold cost, int pct, CItem *metinstone_item);
int GetFractionCount();

// REFINE_PICK
int RealRefinePick(CHARACTER *ch, CItem *item);
void CHEAT_MAX_PICK(CHARACTER *ch, CItem *item);
// END_OF_REFINE_PICK

bool IsVeinOfOre(uint32_t vnum);
bool LoadMiningExtraDrops(const std::string& filename);

#ifdef ENABLE_GEM_SYSTEM
	bool GemRefine(CHARACTER* ch, CHARACTER* npc, CItem* item, Gold cost, CItem* metinstone_item);
#endif
}
#endif /* METIN2_SERVER_GAME_MINING_H */
