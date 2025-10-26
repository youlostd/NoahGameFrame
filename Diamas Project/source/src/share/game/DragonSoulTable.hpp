#ifndef METIN2_SERVER_GAME_DRAGON_SOUL_TABLE_H
#define METIN2_SERVER_GAME_DRAGON_SOUL_TABLE_H

#include "Types.hpp"

#include <Config.hpp>


#include "base/GroupTextTree.hpp"
#include "game/game_type.h"
#include <unordered_map>

#include <game/ItemConstants.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



struct SApply
{
	SApply (ApplyType at, ApplyValue av, float p = 0.f) : apply_type(at), apply_value(av), prob(p) {}
	ApplyType apply_type;
	ApplyValue apply_value;
	float prob;
};

struct ApplyNumSetting {
	uint8_t val[3][DRAGON_SOUL_GRADE_MAX];
};

struct WeightTable {
	uint8_t val[DRAGON_SOUL_GRADE_MAX][DRAGON_SOUL_STEP_MAX][DRAGON_SOUL_STRENGTH_MAX];
};

struct RefineGrade {
	uint8_t needCount;
	Gold fee;
	uint8_t prob[DRAGON_SOUL_GRADE_MAX];
};
	
struct RefineStep {
	uint8_t needCount;
	Gold fee;
	uint8_t prob[DRAGON_SOUL_STEP_MAX];
};

struct RefineStrength {
	Gold fee;
	uint8_t prob[DRAGON_SOUL_STRENGTH_MAX];
};

struct DragonHeartExtTable {
	uint8_t val[DRAGON_SOUL_GRADE_MAX][DRAGON_SOUL_STEP_MAX];
};

struct DragonSoulExtTable {
	float prob;
	float byProduct;
};

struct DragonSoulExtTables {
	DragonSoulExtTable grades[DRAGON_SOUL_GRADE_MAX];
};

const int DRAGON_SOUL_ADDITIONAL_ATTR_START_IDX = 3;



class DragonSoulTable
{
public:
	DragonSoulTable() = default;
	~DragonSoulTable() = default;
	typedef std::vector <SApply> TVecApplys;
	typedef std::map <uint8_t, TVecApplys> TMapApplyGroup;

	bool Read(GroupTextReader& reader);
	bool	ReadDragonSoulTableFile(const char * c_pszFileName);
	bool	ReadDragonSoulTableMemory(std::string_view data);

	bool	GetDragonSoulGroupName(uint8_t bType, std::string& stGroupName) const;
	bool ReadVnumMapper(GroupTextReader& reader);
	bool ReadBasicApplys(GroupTextReader& reader);
	bool ReadAdditionalApplys(GroupTextReader& reader);
	bool ReadApplyNumSettings(GroupTextReader& reader);
	bool ReadWeightTable(GroupTextReader& reader);
	bool ReadRefineGradeTables(GroupTextReader& reader);
	bool ReadRefineStepTables(GroupTextReader& reader);
	bool ReadRefineStrengthTables(GroupTextReader& reader);
	bool ReadDragonHeartExtTables(GroupTextReader& reader);
	bool ReadDragonSoulExtTables(GroupTextReader& reader);

	
	
	bool	GetBasicApplys(uint8_t ds_type,  TVecApplys& vec_basic_applys);
	bool	GetAdditionalApplys(uint8_t ds_type,  TVecApplys& vec_additional_attrs);

	bool	GetApplyNumSettings(uint8_t ds_type, uint8_t grade_idx,  int& basis,  int& add_min,  int& add_max);
	bool	GetWeight(uint8_t ds_type, uint8_t grade_idx, uint8_t step_index, uint8_t strength_idx,  float& fWeight);
	bool	GetRefineGradeValues(uint8_t ds_type, uint8_t grade_idx, CountType& need_count,  Gold& fee,  std::vector<float>& vec_probs);
	bool	GetRefineStepValues(uint8_t ds_type, uint8_t step_idx,  CountType& need_count,  Gold& fee,  std::vector<float>& vec_probs);
	bool	GetRefineStrengthValues(uint8_t ds_type, uint8_t material_type, uint8_t strength_idx,  Gold& fee,  float& prob);
	bool	GetDragonHeartExtValues(uint8_t ds_type, uint8_t grade_idx,  std::vector<float>& vec_chargings,  std::vector<float>& vec_probs);
	bool	GetDragonSoulExtValues(uint8_t ds_type, uint8_t grade_idx,  float& prob,  uint32_t& by_product);
#ifdef ENABLE_DS_SET
	uint8_t	GetBasicApplyCount(uint8_t iType);
	bool	GetBasicApplyValue(uint8_t iType, uint16_t iApplyType, ApplyValue& iApplyValue);
	bool	GetAdditionalApplyValue(uint8_t iType, uint16_t iApplyType, ApplyValue& iApplyValue);
    ApplyValue GetDSSetValue(uint8_t iAttributeIndex, uint16_t iApplyType, uint8_t iType, uint8_t iSetGrade);
    float GetDSSetWeight(uint8_t iDSType, uint8_t iSetGrade);
	uint8_t GetDSBasicApplyCount(uint8_t iDSType);
	ApplyValue GetDSBasicApplyValue(uint8_t iDSType, ApplyType applyType);
	ApplyValue GetDSAdditionalApplyValue(uint8_t iDSType, ApplyType applyType);
#endif
private:
	std::string stFileName;


	typedef std::map <std::string, uint8_t> TMapNameToType;
	typedef std::map <uint8_t, std::string> TMapTypeToName;
	using TApplyNumSettingMap = std::unordered_map<uint8_t, ApplyNumSetting>;
	using TWeightMap = std::unordered_map<uint8_t, WeightTable>;
	using TRefineGradeTable = std::unordered_map<uint8_t, std::vector<RefineGrade>>;
	using TRefineStepTable = std::unordered_map<uint8_t, std::vector<RefineStep>>;
	using TRefineStrengthTable = std::unordered_map<uint8_t, std::unordered_map<uint32_t, RefineStrength>>;
	using TDragonHeartExtCharging = std::unordered_map<uint8_t, std::vector<float>>;
	using TDragonHeartExtTable= std::unordered_map<uint8_t, DragonHeartExtTable>;
	using TDragonSoulExtTablesMap= std::unordered_map<uint8_t, DragonSoulExtTables>;

	
	std::vector <std::string> m_vecDragonSoulNames;
	std::vector <uint8_t> m_vecDragonSoulTypes;

	TMapNameToType m_map_name_to_type;
	TMapTypeToName m_map_type_to_name;
	TMapApplyGroup m_map_basic_applys_group;
	TMapApplyGroup m_map_additional_applys_group;
	TApplyNumSettingMap m_applyNumSetting;
	TWeightMap m_weightMap;
	TRefineGradeTable m_refineGradeTable;
	TRefineStepTable m_refineStepTable;
	TRefineStrengthTable m_refineStrengthTable;
	TDragonHeartExtCharging m_dragonHeartExtCharging;
	TDragonHeartExtTable m_dragonHeartExtTable;
	TDragonSoulExtTablesMap m_dragonSoulExtTable;
	
	// table check functions.
	bool	CheckApplyNumSettings();
	bool	CheckWeightTables();
	bool	CheckRefineGradeTables();
	bool	CheckRefineStepTables();
	bool	CheckRefineStrengthTables();
	bool	CheckDragonHeartExtTables();
	bool	CheckDragonSoulExtTables();
};

#endif /* METIN2_SERVER_GAME_DRAGON_SOUL_TABLE_H */
