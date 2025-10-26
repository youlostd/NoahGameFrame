#ifndef METIN2_SERVER_GAME_BLUEDRAGON_H
#define METIN2_SERVER_GAME_BLUEDRAGON_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CHARACTER;
extern int BlueDragon_StateBattle(CHARACTER *);
extern time_t UseBlueDragonSkill(CHARACTER *, unsigned int);
extern int BlueDragon_Damage(CHARACTER *me, CHARACTER *attacker, int dam);

#endif /* METIN2_SERVER_GAME_BLUEDRAGON_H */
