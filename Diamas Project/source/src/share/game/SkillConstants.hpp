#ifndef METIN2_GAME_SKILLCONSTANTS_HPP
#define METIN2_GAME_SKILLCONSTANTS_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

enum SkillLimits
{
	SKILLBOOK_DELAY_MIN = 64800,
	SKILLBOOK_DELAY_MAX = 108000,
	SKILL_MAX_LEVEL = 50,
	SKILL_GROUP_MAX_NUM = 2,
	SKILL_PENALTY_DURATION = 3,
};

enum SkillType
{
	SKILL_TYPE_SECONDARY,
	SKILL_TYPE_WARRIOR,
	SKILL_TYPE_ASSASSIN,
	SKILL_TYPE_SURA,
	SKILL_TYPE_SHAMAN,
	SKILL_TYPE_HORSE,
	SKILL_TYPE_COMMON,
	SKILL_TYPE_WOLFMAN,
	SKILL_TYPE_SKILLTREE,

};

enum SkillUpType
{
	SKILL_UP_BY_POINT,
	SKILL_UP_BY_BOOK,
	SKILL_UP_BY_TRAIN,
	SKILL_UP_BY_QUEST,
};

enum SkillAttrType
{
	SKILL_ATTR_TYPE_NONE,
	SKILL_ATTR_TYPE_NORMAL,
	SKILL_ATTR_TYPE_MELEE,
	SKILL_ATTR_TYPE_RANGE,
	SKILL_ATTR_TYPE_MAGIC,


};

enum SkillGrade
{
	SKILL_NORMAL,
	SKILL_MASTER,
	SKILL_GRAND_MASTER,
	SKILL_PERFECT_MASTER,
	SKILL_LEGENDARY_MASTER,
};

enum ESkillFlags
{
	SKILL_FLAG_ATTACK = (1 << 0), // 공격 기술
	SKILL_FLAG_USE_MELEE_DAMAGE = (1 << 1), // 기본 밀리 타격치를 b 값으로 사용
	SKILL_FLAG_COMPUTE_ATTGRADE = (1 << 2), // 공격등급을 계산한다
	SKILL_FLAG_SELFONLY = (1 << 3), // 자신에게만 쓸 수 있음
	SKILL_FLAG_USE_MAGIC_DAMAGE = (1 << 4), // 기본 마법 타격치를 b 값으로 사용
	SKILL_FLAG_USE_HP_AS_COST = (1 << 5), // HP를 SP대신 쓴다
	SKILL_FLAG_COMPUTE_MAGIC_DAMAGE = (1 << 6),
	SKILL_FLAG_SPLASH = (1 << 7),
	SKILL_FLAG_GIVE_PENALTY = (1 << 8), // 쓰고나면 잠시동안(3초) 2배 데미지를 받는다.
	SKILL_FLAG_USE_ARROW_DAMAGE = (1 << 9), // 기본 화살 타격치를 b 값으로 사용
	SKILL_FLAG_PENETRATE = (1 << 10), // 방어무시
	SKILL_FLAG_IGNORE_TARGET_RATING = (1 << 11), // 상대 레이팅 무시
	SKILL_FLAG_SLOW = (1 << 12), // 슬로우 공격
	SKILL_FLAG_STUN = (1 << 13), // 스턴 공격
	SKILL_FLAG_HP_ABSORB = (1 << 14), // HP 흡수
	SKILL_FLAG_SP_ABSORB = (1 << 15), // SP 흡수
	SKILL_FLAG_FIRE_CONT = (1 << 16), // FIRE 지속 데미지
	SKILL_FLAG_REMOVE_BAD_AFFECT = (1 << 17), // 나쁜효과 제거
	SKILL_FLAG_REMOVE_GOOD_AFFECT = (1 << 18), // 나쁜효과 제거
	SKILL_FLAG_CRUSH = (1 << 19), // 상대방을 날림
	SKILL_FLAG_POISON = (1 << 20), // 독 공격
	SKILL_FLAG_TOGGLE = (1 << 21), // 토글
	SKILL_FLAG_DISABLE_BY_POINT_UP = (1 << 22), // 찍어서 올릴 수 없다.
	SKILL_FLAG_CRUSH_LONG = (1 << 23),	// 상대방을 멀리 날림
	SKILL_FLAG_WIND = (1 << 24), // 바람 속성
	SKILL_FLAG_ELEC = (1 << 25), // 전기 속성
	SKILL_FLAG_FIRE = (1 << 26), // 불 속성
	SKILL_FLAG_CRITICAL = (1 << 27),
	SKILL_FLAG_BLEEDING = (1 << 28),
	SKILL_FLAG_PARTY = (1 << 29),
	SKILL_FLAG_PARTY_AND_ALL = (1 << 30),
};

enum ESkillIndexes
{
	SKILL_RESERVED = 0,

	// 무사 전사 계열
	// A
	SKILL_SAMYEON = 1, // 삼연참(세번베기)
	SKILL_PALBANG, // 팔방풍우
	// S
	SKILL_JEONGWI, // 전귀혼
	SKILL_GEOMKYUNG, // 검경
	SKILL_TANHWAN, // 탄환격

	// 무사 기공 계열
	// A
	SKILL_GIGONGCHAM = 16,	// 기공참
	SKILL_GYOKSAN, // 격산타우
	SKILL_DAEJINGAK, // 대진각
	// S
	SKILL_CHUNKEON, // 천근추
	SKILL_GEOMPUNG, // 검풍

	// 자객 암살 계열
	// A
	SKILL_AMSEOP = 31, // 암습
	SKILL_GUNGSIN, // 궁신탄영
	SKILL_CHARYUN, // 차륜살
	// S
	SKILL_EUNHYUNG, // 은형법
	SKILL_SANGONG, // 산공분

	// 자객 궁수 계열
	// A
	SKILL_YEONSA = 46, // 연사
	SKILL_KWANKYEOK, // 관격술
	SKILL_HWAJO, // 화조파
	// S
	SKILL_GYEONGGONG, // 경공술
	SKILL_GIGUNG, // 기궁

	// 수라 검
	// A
	SKILL_SWAERYUNG = 61, // 쇄령지
	SKILL_YONGKWON, // 용권파
	// S
	SKILL_GWIGEOM, // 귀검
	SKILL_TERROR, // 공포
	SKILL_JUMAGAP, // 주마갑
	SKILL_PABEOB, // 파법술

	// 수라 마법
	// A
	SKILL_MARYUNG = 76, // 마령
	SKILL_HWAYEOMPOK, // 화염폭
	SKILL_MUYEONG, // 무영진
	// S
	SKILL_MANASHIELD, // 흑신수호
	SKILL_TUSOK, // 투속마령
	SKILL_MAHWAN, // 마환격

	// 무당 용신
	// A
	SKILL_BIPABU = 91,
	SKILL_YONGBI, // 용비광사파
	SKILL_PAERYONG, // 패룡나한무
	// S
	//SKILL_BUDONG, // 부동박부
	SKILL_HOSIN, // 호신
	SKILL_REFLECT, // 보호
	SKILL_GICHEON, // 기천대공

	// 무당 뇌신
	// A
	SKILL_NOEJEON = 106, // 뇌전령
	SKILL_BYEURAK, // 벼락
	SKILL_CHAIN, // 체인라이트닝
	// S
	SKILL_JEONGEOP, // 정업인
	SKILL_KWAESOK, // 이동속도업
	SKILL_JEUNGRYEOK, // 증력술

	// 공통 스킬
	// 7
	SKILL_7_A_ANTI_TANHWAN = 112,
	SKILL_7_B_ANTI_AMSEOP,
	SKILL_7_C_ANTI_SWAERYUNG,
	SKILL_7_D_ANTI_YONGBI,

	// 8
	SKILL_8_A_ANTI_GIGONGCHAM,
	SKILL_8_B_ANTI_YEONSA,
	SKILL_8_C_ANTI_MAHWAN,
	SKILL_8_D_ANTI_BYEURAK,

	// 보조 스킬

	SKILL_LEADERSHIP = 121, // 통솔력
	SKILL_COMBO = 122, // 연계기
	SKILL_CREATE = 123, // 제조
	SKILL_MINING = 124,

	SKILL_LANGUAGE1 = 126, // 신수어 능력
	SKILL_LANGUAGE2 = 127, // 천조어 능력
	SKILL_LANGUAGE3 = 128, // 진노어 능력
	SKILL_POLYMORPH = 129, // 둔갑

	SKILL_HORSE = 130, // 승마 스킬
	SKILL_HORSE_SUMMON = 131, // 말 소환 스킬
	SKILL_HORSE_WILDATTACK = 137, // 난무
	SKILL_HORSE_CHARGE = 138, // 돌격
	SKILL_HORSE_ESCAPE = 139, // 탈출
	SKILL_HORSE_WILDATTACK_RANGE = 140, // 난무(활)

	SKILL_ADD_HP = 141, // 증혈
	SKILL_RESIST_PENETRATE = 142, // 철통
	SKILL_ADD_MONSTER = 143,

	// 수인족 스킬
	SKILL_CHAYEOL = 170, // 차열 (공격) : 늑대가 바람을 가르듯 적을 찢어 발긴다 : 총 공격력 %.0f-%.0f 크리티컬 확률 증가, 출혈 확률 %.0f%% 무기 보너스 // 무사 삼연참
	SKILL_SALPOONG = 171, // 살풍 (공격) : 전방으로 죽음을 부르는 바람을 일으킨다. // 무사 검풍
	SKILL_GONGDAB = 172, // 공답참 (공격) : 허공을 딛고 적을 섬멸한다. // 자객 천룡추타 (라고 기획서에 써있는데 무슨스킬인지 모르겠음 -_-;)
	SKILL_PASWAE = 173, // 파쇄 (공격) : 상대의 갑옷을 찢어 발긴다. // 무사 삼연참 (단, 1회만 타격)
	SKILL_JEOKRANG = 174, // 적랑빙의 (버프) : 붉은 늑대의 영혼을 빙의한다. // 공격력 +%.0f, 방어력 -%.0f, 이동 속도 -%.0f, 상대 방어 무시 확률 %.0f%% // 무사 천근추
	SKILL_CHEONGRANG = 175, // 청랑빙의 (버프) : 푸른 늑대의 영혼을 빙의한다. // 공격력 -%.0f, 방어력 -%.0f, 이동 속도 +%.0f, 일정 확률로 스턴 유발

	GUILD_SKILL_START = 151,
	GUILD_SKILL_EYE = 151,
	GUILD_SKILL_BLOOD = 152,
	GUILD_SKILL_BLESS = 153,
	GUILD_SKILL_SEONGHWI = 154,
	GUILD_SKILL_ACCEL = 155,
	GUILD_SKILL_BUNNO = 156,
	GUILD_SKILL_JUMUN = 157,
	GUILD_SKILL_TELEPORT = 158,
	GUILD_SKILL_DOOR = 159,
	GUILD_SKILL_END = 162,

	GUILD_SKILL_COUNT = GUILD_SKILL_END - GUILD_SKILL_START + 1,


	// Anti Warrior ( Dash / Investida )
	SKILL_7_A_ANTI_TANHWAN_2015 = 221,
	// Anti Ninja ( Ambush / Emboscar )
	SKILL_7_B_ANTI_AMSEOP_2015,
	// Anti Sura ( Finger Strike / Ataque do Dedo )
	SKILL_7_C_ANTI_SWAERYUNG_2015,
	// Anti Shaman ( Shooting Dragon / Ataque do Drag? )
	SKILL_7_D_ANTI_YONGBI_2015,

	// Anti Warrior ( Spirit Strike / For? do Golpe )
	SKILL_7_A_ANTI_GIGONGCHAM_2015,
	// Anti Ninja ( Repetitive Shot / Tiro duplo )
	SKILL_7_B_ANTI_YEONSA_2015,
	// Anti Sura ( Dark Orb / Bola da Escurid? )
	SKILL_7_C_ANTI_MAHWAN_2015,
	// Anti Shaman ( Summon Lightning / Invocar Raio )
	SKILL_7_D_ANTI_BYEURAK_2015,
	// Anti Lycan ( Wolf's Breath / Sopro do Lobo )
	SKILL_7_D_ANTI_SALPOONG_2015,


	// Power to Warrior ( Dash / Investida )
	SKILL_8_A_POWER_TANHWAN_2015 = 236,
	// Power to ( Ambush / Emboscar )
	SKILL_8_B_POWER_AMSEOP_2015,
	// Power to Sura ( Finger Strike / Ataque do Dedo )
	SKILL_8_C_POWER_SWAERYUNG_2015,
	// Power to Shaman ( Shooting Dragon / Ataque do Drag? )
	SKILL_8_D_POWER_YONGBI_2015,

	// Power to Warrior ( Spirit Strike / For? do Golpe )
	SKILL_8_A_POWER_GIGONGCHAM_2015,
	// Power to Ninja ( Repetitive Shot / Tiro duplo )
	SKILL_8_B_POWER_YEONSA_2015,
	// Power to Sura ( Dark Orb / Bola da Escurid? )
	SKILL_8_C_POWER_MAHWAN_2015,
	// Power to Shaman ( Summon Lightning / Invocar Raio )
	SKILL_8_D_POWER_BYEURAK_2015,
	// Power to Lycan ( Wolf's Breath / Sopro do Lobo )
	SKILL_8_D_POWER_SALPOONG_2015,

	SKILL_MAX_NUM = 255,

};

bool GetSkillTypeString(storm::StringRef& s, uint32_t val);
bool GetSkillTypeValue(const storm::StringRef& s, uint32_t& val);

bool GetSkillAttrTypeString(storm::StringRef& s, uint32_t val);
bool GetSkillAttrTypeValue(const storm::StringRef& s, uint32_t& val);

bool GetSkillFlagsString(storm::String& s, uint32_t val);
bool GetSkillFlagsValue(const storm::String& s, uint32_t& val);

uint32_t GetSkillPowerByLevel(uint32_t level);
uint32_t GetMasterTypeFromLevel(uint32_t level);
uint32_t GetRelativeSkillLevel(uint32_t grade, uint32_t level);

METIN2_END_NS

#endif
