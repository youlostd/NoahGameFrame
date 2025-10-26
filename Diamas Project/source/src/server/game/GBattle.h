#ifndef METIN2_SERVER_GAME_BATTLE_H
#define METIN2_SERVER_GAME_BATTLE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CHARACTER;
class CItem;

enum EBattleTypes // 상대방 기준
{
    BATTLE_NONE,
    BATTLE_DAMAGE,
    BATTLE_DEFENSE,
    BATTLE_DEAD
};

extern PointValue ApplyAttackBonus(CHARACTER *pkAttacker, CHARACTER *pkVictim, PointValue iAtk);
extern PointValue ApplyResistBonus(CHARACTER *attacker, CHARACTER *victim, PointValue atk);
extern PointValue CalcBattleDamage(PointValue iDam, int iAttackerLev, int iVictimLev);
extern DamageValue CalcMeleeDamage(CHARACTER *pAttacker, CHARACTER *pVictim, bool bIgnoreDefense = false,
                           bool bIgnoreTargetRating = false);
extern int CalcMagicDamage(CHARACTER *pAttacker, CHARACTER *pVictim);
extern int CalcArrowDamage(CHARACTER *pkAttacker, CHARACTER *pkVictim,
                           CItem *pkBow, CItem *pkArrow,
                           bool bIgnoreDefense = false, bool ignoreDistance = false);
extern float CalcAttackRating(CHARACTER *pkAttacker, CHARACTER *pkVictim, bool bIgnoreTargetRating = false);

extern bool battle_is_attackable(CHARACTER *ch, CHARACTER *victim);
extern int battle_melee_attack(CHARACTER *ch, CHARACTER *victim);
extern void battle_end(CHARACTER *ch);

extern void Freeze(CHARACTER *pkAttacker, CHARACTER *pkVictim, int time);

extern bool battle_distance_valid_by_xy(long x, long y, long tx, long ty);
extern bool battle_distance_valid(CHARACTER *ch, CHARACTER *victim);
extern int battle_count_attackers(CHARACTER *ch);

extern void NormalAttackAffect(CHARACTER *pkAttacker, CHARACTER *pkVictim);
extern void ArrowAttackAffect(CHARACTER *pkAttacker, CHARACTER *pkVictim);

// 특성 공격
void AttackAffect(CHARACTER *pkAttacker,
                  CHARACTER *pkVictim,
                  uint8_t att_point,
                  uint32_t immune_flag,
                  uint32_t affect_idx,
                  uint8_t affect_point,
                  int32_t affect_amount,
                  int time,
                  const char *name);

void SkillAttackAffect(CHARACTER *pkVictim,
                       int success_pct,
                       uint32_t immune_flag,
                       uint32_t affect_idx,
                       uint8_t affect_point,
                       int32_t affect_amount,
                       int time,
                       const char *name,
                       bool force = false);

uint32_t GetAttackMotionInputTime(CHARACTER *ch, uint32_t motionKey);
uint32_t GetAttackMotionDuration(CHARACTER *ch, uint32_t motionKey);
#define GET_SPEED_HACK_COUNT(ch)		((ch)->m_speed_hack_count)
#define INCREASE_SPEED_HACK_COUNT(ch)	(++GET_SPEED_HACK_COUNT(ch))
void SET_ATTACK_TIME(CHARACTER *ch, CHARACTER *victim, uint32_t current_time);
void SET_ATTACKED_TIME(CHARACTER *ch, CHARACTER *victim, uint32_t current_time);
bool IS_SPEED_HACK(CHARACTER *ch, CHARACTER *victim, uint32_t current_time, uint32_t motionKey);
#endif /* METIN2_SERVER_GAME_BATTLE_H */
