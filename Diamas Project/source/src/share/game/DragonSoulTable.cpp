#include "DragonSoulTable.hpp"
#include <SpdLog.hpp>
#include <set>

#include <base/GroupTextTree.hpp>
#include <game/Constants.hpp>
#include <storm/StringFlags.hpp>

#include "game/ItemConstants.hpp"

storm::StringValueTable<uint32_t> kDragonSoulGrades[] = {
	{ "GRADE_NORMAL", DRAGON_SOUL_GRADE_NORMAL },
	{ "GRADE_BRILLIANT", DRAGON_SOUL_GRADE_BRILLIANT },
	{ "GRADE_RARE", DRAGON_SOUL_GRADE_RARE },
	{ "GRADE_ANCIENT", DRAGON_SOUL_GRADE_ANCIENT },
	{ "GRADE_LEGENDARY", DRAGON_SOUL_GRADE_LEGENDARY },
	{ "GRADE_MYTH", DRAGON_SOUL_GRADE_MYTHICAL },
	{ "GRADE_HERO", DRAGON_SOUL_GRADE_HEROIC },
	{ "GRADE_MAX", DRAGON_SOUL_GRADE_MAX },
};

storm::StringValueTable<uint32_t> kDragonSoulSteps[] = {
	{ "STEP_LOWEST,", DRAGON_SOUL_STEP_LOWEST, },
	{ "STEP_LOW,", DRAGON_SOUL_STEP_LOW, },
	{ "STEP_MID,", DRAGON_SOUL_STEP_MID, },
	{ "STEP_HIGH,", DRAGON_SOUL_STEP_HIGH, },
	{ "STEP_HIGHEST,", DRAGON_SOUL_STEP_HIGHEST, },
	{ "STEP_MAX,", DRAGON_SOUL_STEP_MAX, },
};


const std::string g_astGradeName[] =
{
	"grade_normal",
	"grade_brilliant",
	"grade_rare",
	"grade_ancient",
	"grade_legendary",
	"grade_myth",
	"grade_hero",
};

const std::string g_astStepName[] =
{
	"step_lowest",
	"step_low",
	"step_mid",
	"step_high",
	"step_highest",
};

bool DragonSoulTable::Read(GroupTextReader& reader) {
	if (!ReadVnumMapper(reader))
		return false;
	
	if (!ReadBasicApplys(reader))
		return false;
	
	if (!ReadAdditionalApplys(reader))
		return false;
	
	if (!ReadApplyNumSettings(reader))
		return false;
	
	if (!ReadWeightTable(reader))
		return false;
	
	if (!ReadRefineGradeTables(reader))
		return false;
	
	if (!ReadRefineStepTables(reader))
		return false;
	
	if (!ReadRefineStrengthTables(reader))
		return false;
	
	if (!ReadDragonHeartExtTables(reader))
		return false;
	
	if (!ReadDragonSoulExtTables(reader))
		return false;

	return true;
}

bool DragonSoulTable::ReadDragonSoulTableFile(const char* c_pszFileName)
{
	GroupTextMemory mem(storm::GetDefaultAllocator());
	GroupTextReader reader(&mem);

	if (!reader.LoadFile(c_pszFileName)) {
		spdlog::error("Failed to load table file");
		return false;
	}

	if(!Read(reader))
		return false;

	if (CheckApplyNumSettings()
		&& CheckWeightTables()
		&& CheckRefineGradeTables()
		&& CheckRefineStepTables()
		&& CheckRefineStrengthTables()
		&& CheckDragonHeartExtTables()
		&& CheckDragonSoulExtTables())
	{
		SPDLOG_INFO("SyntaxCheck dragon_soul_table.txt [OK]");
		return true;
	}

	spdlog::error("DragonSoul table Check failed.");
	return false;
}

bool DragonSoulTable::ReadDragonSoulTableMemory(std::string_view data)
{
	GroupTextMemory mem(storm::GetDefaultAllocator());
	GroupTextReader reader(&mem);

	if (!reader.LoadString(data)) {
		spdlog::error("Failed to load table file");
		return false;
	}

	if(!Read(reader))
		return false;
	
	if (CheckApplyNumSettings()
		&& CheckWeightTables()
		&& CheckRefineGradeTables()
		&& CheckRefineStepTables()
		&& CheckRefineStrengthTables()
		&& CheckDragonHeartExtTables()
		&& CheckDragonSoulExtTables())
	{
		SPDLOG_INFO("SyntaxCheck dragon_soul_table.txt [OK]");
		return true;
	}

	spdlog::error("DragonSoul table Check failed.");
	return false;
}

bool DragonSoulTable::GetDragonSoulGroupName(uint8_t bType, std::string& stGroupName) const
{
	DragonSoulTable::TMapTypeToName::const_iterator it = m_map_type_to_name.find(bType);
	if (it != m_map_type_to_name.end())
	{
		stGroupName = it->second;
		return true;
	}
	else
	{
		spdlog::error("Invalid DragonSoul Type({0})", bType);
		return false;
	}
}

bool DragonSoulTable::ReadVnumMapper(GroupTextReader& reader)
{
	const auto vnumMapper = reader.GetList("VnumMapper");
	if (vnumMapper != nullptr) {
		std::vector<storm::StringRef> tokens;
		int32_t                    lineNum = 1;
		for (const auto& line : vnumMapper->GetLines()) {
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < 2) {
				SPDLOG_WARN("Ignoring line {} in VnumMapper invalid token count", lineNum);
				continue;
			}

			std::string stDragonSoulName(tokens[0]);
			uint32_t    type = 0;
			if (!storm::ParseNumber(tokens[1], type)) {
				SPDLOG_WARN("Invalid line {} in VnumMapper invalid token count", lineNum);
				continue;
			}

			m_map_name_to_type.emplace(stDragonSoulName, type);
			m_map_type_to_name.emplace(type, stDragonSoulName);
			m_vecDragonSoulTypes.emplace_back(type);
			m_vecDragonSoulNames.emplace_back(stDragonSoulName);

			++lineNum;

		}

	}

	return true;
}

bool DragonSoulTable::ReadBasicApplys(GroupTextReader& reader)
{
	const auto basicApplies = reader.GetGroup("BasicApplys");
	if (!basicApplies)
		return false;

	for (const auto& name : m_vecDragonSoulNames) {
		const auto applyList = basicApplies->GetList(name);
		if (!applyList)
			continue;

		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;

		TVecApplys vecApplys;

		for (const auto& line : applyList->GetLines()) {
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < 2) {
				SPDLOG_WARN("Ignoring line {} in BasicApplys invalid token count", lineNum);
				return false;
			}

			std::string applyName(tokens[0]);

			uint32_t applyType = 0;
			if (!GetApplyTypeValue(applyName, applyType)) {
				SPDLOG_WARN("Invalid line {} in BasicApplys invalid apply type {}", lineNum, applyName);
				return false;

			}

			PointValue applyValue = 0;
			if (!storm::ParseNumber(tokens[1], applyValue)) {
				SPDLOG_WARN("Invalid line {} in VnumMapper invalid apply value {}", lineNum, tokens[1]);
				return false;
			}

			vecApplys.emplace_back(applyType, applyValue);

		}

		m_map_basic_applys_group.emplace(m_map_name_to_type[name], vecApplys);

	}

	return true;
}

bool DragonSoulTable::ReadAdditionalApplys(GroupTextReader& reader)
{
	const auto basicApplies = reader.GetGroup("AdditionalApplys");
	if (!basicApplies)
		return false;

	for (const auto& name : m_vecDragonSoulNames) {
		const auto applyList = basicApplies->GetList(name);
		if (!applyList)
			continue;

		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;

		TVecApplys vecApplys;

		for (const auto& line : applyList->GetLines()) {
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < 3) {
				SPDLOG_WARN("Ignoring line {} in AdditionalApplys invalid token count", lineNum);
				return false;
			}

			std::string applyName(tokens[0]);

			uint32_t applyType = 0;
			if (!GetApplyTypeValue(applyName, applyType)) {
				SPDLOG_WARN("Invalid line {} in AdditionalApplys invalid apply type {}", lineNum, applyName);
				return false;

			}

			PointValue applyValue = 0;
			if (!storm::ParseNumber(tokens[1], applyValue)) {
				SPDLOG_WARN("Invalid line {} in AdditionalApplys invalid apply value {}", lineNum, tokens[1]);
				return false;
			}

			float prob = 0;
			if (!storm::ParseNumber(tokens[2], prob)) {
				SPDLOG_WARN("Invalid line {} in AdditionalApplys invalid probability {}", lineNum, tokens[2]);
				return false;
			}

			vecApplys.emplace_back(applyType, applyValue, prob);

		}

		m_map_additional_applys_group.emplace(m_map_name_to_type[name], vecApplys);

	}

	return true;
}

bool DragonSoulTable::ReadApplyNumSettings(GroupTextReader& reader)
{
	const auto settings = reader.GetGroup("ApplyNumSettings");
	if (!settings)
		return false;

	for (const auto& name : m_vecDragonSoulNames) {
		const auto list = settings->GetList(name);
		if (!list)
			continue;

		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;
		ApplyNumSetting applySet{};

		for (int kind = 0; kind < list->GetLineCount(); ++kind) {
			const auto& line = list->GetLine(kind);
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < DRAGON_SOUL_GRADE_MAX) {
				SPDLOG_WARN("Ignoring line {} in ApplyNumSettings invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
				return false;
			}

			for (int grade = 0; grade < DRAGON_SOUL_GRADE_MAX; ++grade) {
				uint8_t value = 0;
				if (!storm::ParseNumber(tokens[grade], value)) {
					SPDLOG_WARN("Invalid line {} in ApplyNumSettings invalid apply value {}", lineNum, tokens[grade]);
					return false;
				}

				applySet.val[kind][grade] = value;
			}

			++lineNum;
		}

		m_applyNumSetting.emplace(m_map_name_to_type[name], applySet);

	}

	const auto list = settings->GetList("Default");
	if (list) {
		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;
		ApplyNumSetting applySet{};

		for (int i = 0; i < list->GetLineCount(); ++i) {
			const auto& line = list->GetLine(i);
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < DRAGON_SOUL_GRADE_MAX) {
				SPDLOG_WARN("Ignoring line {} in ApplyNumSettings invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
				return false;
			}

			for (int j = 0; j < DRAGON_SOUL_GRADE_MAX; ++j) {
				uint8_t value = 0;
				if (!storm::ParseNumber(tokens[j], value)) {
					SPDLOG_WARN("Invalid line {} in ApplyNumSettings invalid apply value {}", lineNum, tokens[j]);
					return false;
				}

				applySet.val[i][j] = value;
			}

			++lineNum;
		}

		m_applyNumSetting.emplace(0, applySet);
	}


	return true;

}

bool DragonSoulTable::ReadWeightTable(GroupTextReader& reader)
{
	const auto settings = reader.GetGroup("WeightTables");
	if (!settings)
		return false;

	for (const auto& name : m_vecDragonSoulNames) {
		const auto grp = settings->GetGroup(name);
		if (!grp)
			continue;

		WeightTable weights{};

		for (int grade = 0; grade < DRAGON_SOUL_GRADE_MAX; ++grade) {
			const auto& gradeName = g_astGradeName[grade];

			const auto list = grp->GetList(gradeName);
			if (!list) {
				spdlog::error("WeightTable {} is missing grade {}", name, gradeName);
				return false;
			}

			std::vector<storm::StringRef> tokens;
			uint16_t lineNum = 1;

			for (int i = 0; i < list->GetLineCount(); ++i) {
				const auto& line = list->GetLine(i);
				tokens.clear();
				storm::ExtractArguments(line, tokens);

				if (tokens.size() < DRAGON_SOUL_STRENGTH_MAX) {
					SPDLOG_WARN("Ignoring line {} in ApplyNumSettings invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
					return false;
				}

				for (int j = 0; j < DRAGON_SOUL_STRENGTH_MAX; ++j) {
					uint8_t value = 0;
					if (!storm::ParseNumber(tokens[j], value)) {
						SPDLOG_WARN("Invalid line {} in ApplyNumSettings invalid apply value {}", lineNum, tokens[j]);
						return false;
					}

					weights.val[grade][i][j] = value;
				}

				++lineNum;
			}


		}

		m_weightMap.emplace(m_map_name_to_type[name], weights);

	}

	const auto grp = settings->GetGroup("Default");
	if (grp) {
					WeightTable weights{};

		for (int grade = 0; grade < DRAGON_SOUL_GRADE_MAX; ++grade) {
			const auto& gradeName = g_astGradeName[grade];

			const auto list = grp->GetList(gradeName);
			if (!list) {
				spdlog::error("WeightTable {} is missing grade {}", "Default", gradeName);
				return false;
			}

			std::vector<storm::StringRef> tokens;
			uint16_t lineNum = 1;

			for (int i = 0; i < list->GetLineCount(); ++i) {
				const auto& line = list->GetLine(i);
				tokens.clear();
				storm::ExtractArguments(line, tokens);

				if (tokens.size() < DRAGON_SOUL_STRENGTH_MAX) {
					SPDLOG_WARN("Ignoring line {} in ApplyNumSettings invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
					return false;
				}

				for (int j = 0; j < DRAGON_SOUL_STRENGTH_MAX; ++j) {
					uint8_t value = 0;
					if (!storm::ParseNumber(tokens[j], value)) {
						SPDLOG_WARN("Invalid line {} in ApplyNumSettings invalid apply value {}", lineNum, tokens[j]);
						return false;
					}

					weights.val[grade][i][j] = value;
				}

				++lineNum;
			}

		}
		m_weightMap.emplace(0, weights);

	}


	return true;

}

bool DragonSoulTable::ReadRefineGradeTables(GroupTextReader& reader)
{
	const auto settings = reader.GetGroup("RefineGradeTables");
	if (!settings)
		return false;

	for (const auto& name : m_vecDragonSoulNames) {
		const auto list = settings->GetList(name);
		if (!list)
			continue;

		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;
		std::vector<RefineGrade> vecRefineGrade;

		for (int kind = 0; kind < list->GetLineCount(); ++kind) {
			const auto& line = list->GetLine(kind);
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < DRAGON_SOUL_GRADE_MAX + 2) {
				SPDLOG_WARN("Ignoring line {} in RefineGradeTables invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
				return false;
			}

			RefineGrade refineGrade{};

			if (!storm::ParseNumber(tokens[0], refineGrade.needCount)) {
				SPDLOG_WARN("Invalid line {} in RefineGradeTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			if (!storm::ParseNumber(tokens[1], refineGrade.fee)) {
				SPDLOG_WARN("Invalid line {} in RefineGradeTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			for (int grade = 2; grade < DRAGON_SOUL_GRADE_MAX + 2; ++grade) {
				uint8_t value = 0;
				if (!storm::ParseNumber(tokens[grade], value)) {
					SPDLOG_WARN("Invalid line {} in RefineGradeTables invalid apply value {}", lineNum, tokens[grade]);
					return false;
				}

				refineGrade.prob[grade - 2] = value;
			}

			vecRefineGrade.push_back(refineGrade);

			++lineNum;
		}

		m_refineGradeTable.emplace(m_map_name_to_type[name], vecRefineGrade);

	}

	const auto list = settings->GetList("Default");
	if (list) {
		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;
		std::vector<RefineGrade> vecRefineGrade;

		for (int kind = 0; kind < list->GetLineCount(); ++kind) {
			const auto& line = list->GetLine(kind);
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < DRAGON_SOUL_GRADE_MAX + 2) {
				SPDLOG_WARN("Ignoring line {} in RefineGradeTables invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
				return false;
			}

			RefineGrade refineGrade{};

			if (!storm::ParseNumber(tokens[0], refineGrade.needCount)) {
				SPDLOG_WARN("Invalid line {} in RefineGradeTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			if (!storm::ParseNumber(tokens[1], refineGrade.fee)) {
				SPDLOG_WARN("Invalid line {} in RefineGradeTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			for (int grade = 2; grade < DRAGON_SOUL_GRADE_MAX + 2; ++grade) {
				uint8_t value = 0;
				if (!storm::ParseNumber(tokens[grade], value)) {
					SPDLOG_WARN("Invalid line {} in RefineGradeTables invalid apply value {}", lineNum, tokens[grade]);
					return false;
				}

				refineGrade.prob[grade - 2] = value;
			}

			vecRefineGrade.push_back(refineGrade);

			++lineNum;
		}

		m_refineGradeTable.emplace(0, vecRefineGrade);
	}


	return true;
}

bool DragonSoulTable::ReadRefineStepTables(GroupTextReader& reader)
{
	const auto settings = reader.GetGroup("RefineStepTables");
	if (!settings)
		return false;

	for (const auto& name : m_vecDragonSoulNames) {
		const auto list = settings->GetList(name);
		if (!list)
			continue;

		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;
		std::vector<RefineStep> vecRefineStep;

		for (int kind = 0; kind < list->GetLineCount(); ++kind) {
			const auto& line = list->GetLine(kind);
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < DRAGON_SOUL_STEP_MAX + 2) {
				SPDLOG_WARN("Ignoring line {} in RefineStepTables invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
				return false;
			}

			RefineStep refineStep{};

			if (!storm::ParseNumber(tokens[0], refineStep.needCount)) {
				SPDLOG_WARN("Invalid line {} in RefineStepTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			if (!storm::ParseNumber(tokens[1], refineStep.fee)) {
				SPDLOG_WARN("Invalid line {} in RefineStepTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			for (int step = 2; step < DRAGON_SOUL_STEP_MAX + 2; ++step) {
				uint8_t value = 0;
				if (!storm::ParseNumber(tokens[step], value)) {
					SPDLOG_WARN("Invalid line {} in RefineStepTables invalid apply value {}", lineNum, tokens[step]);
					return false;
				}

				refineStep.prob[step - 2] = value;
			}

			vecRefineStep.push_back(refineStep);

			++lineNum;
		}

		m_refineStepTable.emplace(m_map_name_to_type[name], vecRefineStep);

	}

	const auto list = settings->GetList("Default");
	if (list) {
		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;
		std::vector<RefineStep> vecRefineStep;

		for (int kind = 0; kind < list->GetLineCount(); ++kind) {
			const auto& line = list->GetLine(kind);
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < DRAGON_SOUL_STEP_MAX + 2) {
				SPDLOG_WARN("Ignoring line {} in RefineStepTables invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
				return false;
			}

			RefineStep refineStep{};

			if (!storm::ParseNumber(tokens[0], refineStep.needCount)) {
				SPDLOG_WARN("Invalid line {} in RefineStepTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			if (!storm::ParseNumber(tokens[1], refineStep.fee)) {
				SPDLOG_WARN("Invalid line {} in RefineStepTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			for (int step = 2; step < DRAGON_SOUL_STEP_MAX + 2; ++step) {
				uint8_t value = 0;
				if (!storm::ParseNumber(tokens[step], value)) {
					SPDLOG_WARN("Invalid line {} in RefineStepTables invalid apply value {}", lineNum, tokens[step]);
					return false;
				}

				refineStep.prob[step - 2] = value;
			}

			vecRefineStep.push_back(refineStep);

			++lineNum;
		}

		m_refineStepTable.emplace(0, vecRefineStep);
	}


	return true;
}

bool DragonSoulTable::ReadRefineStrengthTables(GroupTextReader& reader)
{
	const auto settings = reader.GetGroup("RefineStrengthTables");
	if (!settings)
		return false;
	for (const auto& name : m_vecDragonSoulNames) {
		const auto list = settings->GetList(name);
		if (!list)
			continue;

		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;

		std::unordered_map<uint32_t, RefineStrength> mapRefineStrength;

		for (int kind = 0; kind < list->GetLineCount(); ++kind) {
			const auto& line = list->GetLine(kind);
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < DRAGON_SOUL_STRENGTH_MAX + 2) {
				SPDLOG_WARN("Ignoring line {} in RefineStrengthTables invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
				return false;
			}

			RefineStrength refineStrength;

			uint32_t subType;

			if (!GetItemSubTypeValue(tokens[0], ITEM_MATERIAL, subType)) {
				SPDLOG_WARN("Invalid line {} in RefineStrengthTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			if (!storm::ParseNumber(tokens[1], refineStrength.fee)) {
				SPDLOG_WARN("Invalid line {} in RefineStrengthTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			for (int step = 2; step < DRAGON_SOUL_STRENGTH_MAX + 2; ++step) {
				uint8_t value = 0;
				if (!storm::ParseNumber(tokens[step], value)) {
					SPDLOG_WARN("Invalid line {} in RefineStrengthTables invalid apply value {}", lineNum, tokens[step]);
					return false;
				}

				refineStrength.prob[step - 2] = value;
			}

			mapRefineStrength.emplace(subType, refineStrength);

			++lineNum;
		}

		m_refineStrengthTable.emplace(m_map_name_to_type[name], mapRefineStrength);

	}

	const auto list = settings->GetList("Default");
	if (list) {
		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;

		std::unordered_map<uint32_t, RefineStrength> mapRefineStrength;

		for (int kind = 0; kind < list->GetLineCount(); ++kind) {
			const auto& line = list->GetLine(kind);
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < DRAGON_SOUL_STRENGTH_MAX + 2) {
				SPDLOG_WARN("Ignoring line {} in RefineStrengthTables invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
				return false;
			}

			RefineStrength refineStrength;

			uint32_t subType;

			if (!GetItemSubTypeValue(tokens[0], ITEM_MATERIAL, subType)) {
				SPDLOG_WARN("Invalid line {} in RefineStrengthTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			if (!storm::ParseNumber(tokens[1], refineStrength.fee)) {
				SPDLOG_WARN("Invalid line {} in RefineStrengthTables invalid need count {}", lineNum, tokens[0]);
				return false;
			}


			for (int step = 2; step < DRAGON_SOUL_STRENGTH_MAX + 2; ++step) {
				uint8_t value = 0;
				if (!storm::ParseNumber(tokens[step], value)) {
					SPDLOG_WARN("Invalid line {} in RefineStrengthTables invalid apply value {}", lineNum, tokens[step]);
					return false;
				}

				refineStrength.prob[step - 2] = value;
			}

			mapRefineStrength.emplace(subType, refineStrength);

			++lineNum;
		}

		m_refineStrengthTable.emplace(0, mapRefineStrength);
	}


	return true;
}

bool DragonSoulTable::ReadDragonHeartExtTables(GroupTextReader& reader)
{
	const auto settings = reader.GetGroup("DragonHeartExtTables");
	if (!settings)
		return false;

	for (const auto& name : m_vecDragonSoulNames) {
		const auto grp = settings->GetGroup(name);
		if (!grp)
			continue;

		const auto chargingTokens = grp->GetTokens("Charging");
		if (!chargingTokens)
			continue;

		std::vector<float> charging;
		for (const auto& token : *chargingTokens) {
			float val;
			storm::ParseNumber(token, val);
			charging.push_back(val);
		}


		m_dragonHeartExtCharging.emplace(m_map_name_to_type[name], charging);


		const auto gradeDataList = grp->GetList("GradeData");
		if (!gradeDataList)
			continue;

		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;
		DragonHeartExtTable dh{};

		for (int i = 0; i < gradeDataList->GetLineCount(); ++i) {
			const auto& line = gradeDataList->GetLine(i);
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < DRAGON_SOUL_STEP_MAX) {
				SPDLOG_WARN("Ignoring line {} in ApplyNumSettings invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
				return false;
			}

			for (int j = 0; j < DRAGON_SOUL_STEP_MAX; ++j) {
				uint8_t value = 0;
				if (!storm::ParseNumber(tokens[j], value)) {
					SPDLOG_WARN("Invalid line {} in ApplyNumSettings invalid apply value {}", lineNum, tokens[j]);
					return false;
				}

				dh.val[i][j] = value;
			}


			++lineNum;
		}

		m_dragonHeartExtTable.emplace(m_map_name_to_type[name], dh);

	}

	const auto grp = settings->GetGroup("Default");
	if (grp) {
		const auto chargingTokens = grp->GetTokens("Charging");
		if (!chargingTokens)
			return false;

		std::vector<float> charging;
		for (const auto& token : *chargingTokens) {
			float val;
			storm::ParseNumber(token, val);
			charging.push_back(val);
		}


		m_dragonHeartExtCharging.emplace(0, charging);


		const auto gradeDataList = grp->GetList("GradeData");
		if (!gradeDataList)
			return false;

		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;
		DragonHeartExtTable dh{};

		for (int i = 0; i < gradeDataList->GetLineCount(); ++i) {
			const auto& line = gradeDataList->GetLine(i);
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < DRAGON_SOUL_STEP_MAX) {
				SPDLOG_WARN("Ignoring line {} in ApplyNumSettings invalid token count needs {} tokens", lineNum, DRAGON_SOUL_GRADE_MAX);
				return false;
			}

			for (int j = 0; j < DRAGON_SOUL_STEP_MAX; ++j) {
				uint8_t value = 0;
				if (!storm::ParseNumber(tokens[j], value)) {
					SPDLOG_WARN("Invalid line {} in ApplyNumSettings invalid apply value {}", lineNum, tokens[j]);
					return false;
				}

				dh.val[i][j] = value;
			}


			++lineNum;
		}

		m_dragonHeartExtTable.emplace(0, dh);
	}




	return true;
}

bool DragonSoulTable::ReadDragonSoulExtTables(GroupTextReader& reader)
{

	const auto settings = reader.GetGroup("DragonSoulExtTables");
	if (!settings)
		return false;

	for (const auto& name : m_vecDragonSoulNames) {
		const auto list = settings->GetList(name);
		if (!list)
			continue;

		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;
		DragonSoulExtTables tbl{};

		for (const auto& line : list->GetLines()) {
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < 3)
				return false;

			uint32_t grade;
			if (!storm::ParseStringWithTable(tokens[0], grade, kDragonSoulGrades)) {
				spdlog::error("Unknown DS Grade {} in DragonSoulExtTables", tokens[0]);
				return false;
			}

			storm::ParseNumber(tokens[1], tbl.grades[grade].prob);
			storm::ParseNumber(tokens[2], tbl.grades[grade].byProduct);

			++lineNum;
		}

		m_dragonSoulExtTable.emplace(m_map_name_to_type[name], tbl);

	}

	const auto name = "Default";
	const auto list = settings->GetList(name);
	if (list) {
		std::vector<storm::StringRef> tokens;
		uint16_t lineNum = 1;
		DragonSoulExtTables tbl{};

		for (const auto& line : list->GetLines()) {
			tokens.clear();
			storm::ExtractArguments(line, tokens);

			if (tokens.size() < 3)
				return false;

			uint32_t grade;
			if (!storm::ParseStringWithTable(tokens[0], grade, kDragonSoulGrades)) {
				spdlog::error("Unknown DS Grade {} in DragonSoulExtTables", tokens[0]);
				return false;
			}

			storm::ParseNumber(tokens[1], tbl.grades[grade].prob);
			storm::ParseNumber(tokens[2], tbl.grades[grade].byProduct);

			++lineNum;
		}

		m_dragonSoulExtTable.emplace(m_map_name_to_type[name], tbl);
	}

	return true;
}

bool DragonSoulTable::CheckApplyNumSettings()
{
	for (size_t i = 0; i < m_vecDragonSoulTypes.size(); i++)
	{
		for (int j = 0; j < DRAGON_SOUL_GRADE_MAX; j++)
		{
			int basis, add_min, add_max;
			if (!GetApplyNumSettings(m_vecDragonSoulTypes[i], j, basis, add_min, add_max))
			{
				spdlog::error("In {0} group of ApplyNumSettings, values in Grade({1}) row is invalid.",
					m_vecDragonSoulNames[i].c_str(), g_astGradeName[j].c_str());
				return false;
			}
		}
	}


	return true;
}

bool DragonSoulTable::CheckWeightTables()
{
	for (size_t i = 0; i < m_vecDragonSoulTypes.size(); i++)
	{
		for (int j = 0; j < DRAGON_SOUL_GRADE_MAX; j++)
		{
			for (int k = 0; k < DRAGON_SOUL_STEP_MAX; k++)
			{
				for (int l = 0; l < DRAGON_SOUL_STRENGTH_MAX; l++)
				{
					float fWeight;
					if (!GetWeight(m_vecDragonSoulTypes[i], j, k, l, fWeight))
					{
						spdlog::error("In {} group of WeightTables, value(Grade({}), Step({}), Strength({}) is invalid.",
							m_vecDragonSoulNames[i].c_str(), g_astGradeName[j].c_str(), g_astStepName[k].c_str(), l);
					}
				}
			}
		}
	}

	return true;
}

bool DragonSoulTable::CheckRefineGradeTables()
{
	for (size_t i = 0; i < m_vecDragonSoulTypes.size(); i++)
	{
		for (int j = 0; j < DRAGON_SOUL_GRADE_MAX - 1; j++)
		{
			CountType need_count;
			Gold fee;
			std::vector <float> vec_probs;
			if (!GetRefineGradeValues(m_vecDragonSoulTypes[i], j, need_count, fee, vec_probs))
			{
				spdlog::error("In {0} group of RefineGradeTables, values in Grade({1}) row is invalid.",
					m_vecDragonSoulNames[i].c_str(), g_astGradeName[j].c_str());
				return false;
			}
			if (need_count < 1)
			{
				spdlog::error("In {0} group of RefineGradeTables, need_count of Grade({1}) is less than 1.",
					m_vecDragonSoulNames[i].c_str(), g_astGradeName[j].c_str());
				return false;
			}
			if (fee < 0)
			{
				spdlog::error("In {0} group of RefineGradeTables, fee of Grade({1}) is less than 0.",
					m_vecDragonSoulNames[i].c_str(), g_astGradeName[j].c_str());
				return false;
			}
			if (DRAGON_SOUL_GRADE_MAX != vec_probs.size())
			{
				spdlog::error("In {0} group of RefineGradeTables, probability list size is not {1}.", m_vecDragonSoulNames[i].c_str(), DRAGON_SOUL_GRADE_MAX);
				return false;
			}
			for (size_t k = 0; k < vec_probs.size(); k++)
			{
				if (vec_probs[k] < 0.f)
				{
					spdlog::error("In {0} group of RefineGradeTables, probability(index : {0}) is less than 0.", m_vecDragonSoulNames[i].c_str(), k);
					return false;
				}
			}
		}
	}

	return true;
}

bool DragonSoulTable::CheckRefineStepTables()
{
	for (size_t i = 0; i < m_vecDragonSoulTypes.size(); i++)
	{
		for (int j = 0; j < DRAGON_SOUL_STEP_MAX - 1; j++)
		{
			CountType need_count;
			Gold fee;
			std::vector <float> vec_probs;
			if (!GetRefineStepValues(m_vecDragonSoulTypes[i], j, need_count, fee, vec_probs))
			{
				spdlog::error("In {0} group of RefineStepTables, values in Step(%s) row is invalid.",
					m_vecDragonSoulNames[i].c_str(), g_astStepName[j].c_str());
				return false;
			}
			if (need_count < 1)
			{
				spdlog::error("In {0} group of RefineStepTables, need_count of Step(%s) is less than 1.",
					m_vecDragonSoulNames[i].c_str(), g_astStepName[j].c_str());
				return false;
			}
			if (fee < 0)
			{
				spdlog::error("In %s group of RefineStepTables, fee of Step(%s) is less than 0.",
					m_vecDragonSoulNames[i].c_str(), g_astStepName[j].c_str());
				return false;
			}
			if (DRAGON_SOUL_STEP_MAX != vec_probs.size())
			{
				spdlog::error("In {0} group of RefineStepTables, probability list size is not {1}.",
					m_vecDragonSoulNames[i].c_str(), DRAGON_SOUL_STEP_MAX);
				return false;
			}
			for (size_t k = 0; k < vec_probs.size(); k++)
			{
				if (vec_probs[k] < 0.f)
				{
					spdlog::error("In %s group of RefineStepTables, probability(index : %d) is less than 0.",
						m_vecDragonSoulNames[i].c_str(), k);
					return false;
				}
			}
		}
	}


	return true;
}


bool DragonSoulTable::CheckRefineStrengthTables()
{
	/*
	 *
	for (size_t i = 0; i < m_vecDragonSoulTypes.size(); i++)
	{
		for (int j = MATERIAL_DS_REFINE_NORMAL; j <= MATERIAL_DS_REFINE_HOLLY; j++)
		{
			Gold fee;
			float prob;
			for (int k = 0; k < DRAGON_SOUL_STRENGTH_MAX - 1; k++)
			{
				if (!GetRefineStrengthValues(m_vecDragonSoulTypes[i], j, k, fee, prob))
				{
					spdlog::error("In %s group of RefineStrengthTables, value(Material(%s), Strength(%d)) or fee are invalid.",
						m_vecDragonSoulNames[i].c_str(), g_astMaterialName[j].c_str(), k);
					return false;
				}
				if (fee < 0)
				{
					spdlog::error("In %s group of RefineStrengthTables, fee of Material(%s) is less than 0.",
						m_vecDragonSoulNames[i].c_str(), g_astMaterialName[j].c_str());
					return false;
				}
				if (prob < 0.f)
				{
					spdlog::error("In %s group of RefineStrengthTables, probability(Material(%s), Strength(%d)) is less than 0.",
						m_vecDragonSoulNames[i].c_str(), g_astMaterialName[j].c_str(), k);
					return false;
				}
			}
		}
	}
		 */

	return true;
}

bool DragonSoulTable::CheckDragonHeartExtTables()
{

	for (size_t i = 0; i < m_vecDragonSoulTypes.size(); i++)
	{
		for (int j = 0; j < DRAGON_SOUL_GRADE_MAX; j++)
		{
			std::vector <float> vec_chargings;
			std::vector <float> vec_probs;

			if (!GetDragonHeartExtValues(m_vecDragonSoulTypes[i], j, vec_chargings, vec_probs))
			{
				spdlog::error("In %s group of DragonHeartExtTables, CHARGING row or Grade(%s) row are invalid.",
					m_vecDragonSoulNames[i].c_str(), g_astGradeName[j].c_str());
				return false;
			}
			if (vec_chargings.size() != vec_probs.size())
			{
				spdlog::error("In %s group of DragonHeartExtTables, CHARGING row size(%d) are not equal Grade(%s) row size(%d).",
					m_vecDragonSoulNames[i].c_str(), vec_chargings.size(), vec_probs.size());
				return false;
			}
			for (size_t k = 0; k < vec_chargings.size(); k++)
			{
				if (vec_chargings[k] < 0.f)
				{
					spdlog::error("In %s group of DragonHeartExtTables, CHARGING value(index : %d) is less than 0",
						m_vecDragonSoulNames[i].c_str(), k);
					return false;
				}
			}
			for (size_t k = 0; k < vec_probs.size(); k++)
			{
				if (vec_probs[k] < 0.f)
				{
					spdlog::error("In %s group of DragonHeartExtTables, Probability(Grade : %s, index : %d) is less than 0",
						m_vecDragonSoulNames[i].c_str(), g_astGradeName[j].c_str(), k);
					return false;
				}
			}
		}
	}

	return true;
}

bool DragonSoulTable::CheckDragonSoulExtTables()
{
	// Group DragonSoulExtTables Reading.

	for (size_t i = 0; i < m_vecDragonSoulTypes.size(); i++)
	{
		for (int j = 0; j < DRAGON_SOUL_GRADE_MAX; j++)
		{
			float prob;
			uint32_t by_product;
			if (!GetDragonSoulExtValues(m_vecDragonSoulTypes[i], j, prob, by_product))
			{
				spdlog::error("In %s group of DragonSoulExtTables, Grade(%s) row is invalid.",
					m_vecDragonSoulNames[i].c_str(), g_astGradeName[j].c_str());
				return false;
			}
			if (prob < 0.f)
			{
				spdlog::error("In %s group of DragonSoulExtTables, Probability(Grade : %s) is less than 0",
					m_vecDragonSoulNames[i].c_str(), g_astGradeName[j].c_str());
				return false;
			}
			/*if (0 != by_product && NULL == ITEM_MANAGER::instance().GetTable(by_product))
			{
				spdlog::error("In %s group of DragonSoulExtTables, ByProduct(%d) of Grade %s is not exist.",
					m_vecDragonSoulNames[i].c_str(), by_product, g_astGradeName[j].c_str());
				return false;
			}*/
		}
	}
	return true;
}

bool DragonSoulTable::GetBasicApplys(uint8_t ds_type, TVecApplys& vec_basic_applys)
{
	auto it = m_map_basic_applys_group.find(ds_type);
	if (m_map_basic_applys_group.end() == it)
	{
		return false;
	}
	vec_basic_applys = it->second;
	return true;
}

bool DragonSoulTable::GetAdditionalApplys(uint8_t ds_type, TVecApplys& vec_additional_applys)
{
	auto it = m_map_additional_applys_group.find(ds_type);
	if (m_map_additional_applys_group.end() == it)
	{
		return false;
	}
	vec_additional_applys = it->second;
	return true;
}

bool DragonSoulTable::GetApplyNumSettings(uint8_t ds_type, uint8_t grade_idx, int& basis, int& add_min, int& add_max)
{
	if (grade_idx >= DRAGON_SOUL_GRADE_MAX)
	{
		spdlog::error("Invalid dragon soul grade_idx({0}).", grade_idx);
		return false;
	}

	if (auto it = m_applyNumSetting.find(ds_type); it != m_applyNumSetting.end())
	{
		basis = it->second.val[0][grade_idx];
		add_min = it->second.val[1][grade_idx];
		add_max = it->second.val[2][grade_idx];
	}
	else {
		it = m_applyNumSetting.find(0);
		if (it != m_applyNumSetting.end()) {
			basis = it->second.val[0][grade_idx];
			add_min = it->second.val[1][grade_idx];
			add_max = it->second.val[2][grade_idx];
		}
		else {
			return false;
		}
	}

	return true;
}

bool DragonSoulTable::GetWeight(uint8_t ds_type, uint8_t grade_idx, uint8_t step_index, uint8_t strength_idx, float& fWeight)
{
	if (grade_idx >= DRAGON_SOUL_GRADE_MAX || step_index >= DRAGON_SOUL_STEP_MAX || strength_idx >= DRAGON_SOUL_STRENGTH_MAX)
	{
		spdlog::error("Invalid dragon soul grade_idx({}) step_index({}) strength_idx({}).", grade_idx, step_index, strength_idx);
		return false;
	}

	if (grade_idx >= DRAGON_SOUL_GRADE_MAX)
	{
		spdlog::error("Invalid dragon soul grade_idx({0}).", grade_idx);
		return false;
	}

	if (auto it = m_weightMap.find(ds_type); it != m_weightMap.end())
	{
		fWeight = it->second.val[grade_idx][step_index][strength_idx];
	}
	else {
		it = m_weightMap.find(0);
		if (it != m_weightMap.end()) {
			fWeight = it->second.val[grade_idx][step_index][strength_idx];

		}
		else {
			return false;
		}
	}

	return true;
}

bool DragonSoulTable::GetRefineGradeValues(uint8_t ds_type, uint8_t grade_idx, CountType& need_count, Gold& fee, std::vector<float>& vec_probs)
{
	if (grade_idx >= DRAGON_SOUL_GRADE_MAX - 1)
	{
		spdlog::error("Invalid dragon soul grade_idx({0}).", grade_idx);
		return false;
	}

	std::string stDragonSoulName;
	if (!GetDragonSoulGroupName(ds_type, stDragonSoulName))
	{
		spdlog::error("Invalid dragon soul type({0}).", ds_type);
		return false;
	}
	if (auto it = m_refineGradeTable.find(ds_type); it != m_refineGradeTable.end())
	{
		fee = it->second[grade_idx].fee;
		need_count = it->second[grade_idx].needCount;
		for (const auto& prob : it->second[grade_idx].prob)
			vec_probs.push_back(prob);
	}
	else {
		it = m_refineGradeTable.find(0);
		if (it != m_refineGradeTable.end()) {
			fee = it->second[grade_idx].fee;
			need_count = it->second[grade_idx].needCount;
			for (const auto& prob : it->second[grade_idx].prob)
				vec_probs.push_back(prob);
		}
		else {
			return false;
		}
	}

	return true;
}

bool DragonSoulTable::GetRefineStepValues(uint8_t ds_type, uint8_t step_idx, CountType& need_count, Gold& fee, std::vector<float>& vec_probs)
{
	if (step_idx >= DRAGON_SOUL_STEP_MAX - 1 && step_idx != 4)
	{
		spdlog::error("Invalid dragon soul step_idx(%d).", step_idx);
		return false;
	}

	std::string stDragonSoulName;
	if (!GetDragonSoulGroupName(ds_type, stDragonSoulName))
	{
		spdlog::error("Invalid dragon soul type(%d).", ds_type);
		return false;
	}

	if (auto it = m_refineStepTable.find(ds_type); it != m_refineStepTable.end())
	{
		fee = it->second[step_idx].fee;
		need_count = it->second[step_idx].needCount;
		for (const auto& prob : it->second[step_idx].prob)
			vec_probs.push_back(prob);
	}
	else {
		it = m_refineStepTable.find(0);
		if (it != m_refineStepTable.end()) {
			fee = it->second[step_idx].fee;
			need_count = it->second[step_idx].needCount;
			for (const auto& prob : it->second[step_idx].prob)
				vec_probs.push_back(prob);
		}
		else {
			return false;
		}
	}

	return true;
}

bool DragonSoulTable::GetRefineStrengthValues(uint8_t ds_type, uint8_t material_type, uint8_t strength_idx, Gold& fee, float& prob)
{
	if (material_type < MATERIAL_DS_REFINE_NORMAL || material_type > MATERIAL_DS_REFINE_HOLLY)
	{
		spdlog::error("Invalid dragon soul material_type(%d).", material_type);
		return false;
	}

	std::string stDragonSoulName;
	if (!GetDragonSoulGroupName(ds_type, stDragonSoulName))
	{
		spdlog::error("Invalid dragon soul type(%d).", ds_type);
		return false;
	}


	if (auto it = m_refineStrengthTable.find(ds_type); it != m_refineStrengthTable.end())
	{
		if (auto it2 = it->second.find(material_type); it2 != it->second.end()) {
			fee = it2->second.fee;
			prob = it2->second.prob[strength_idx];
		}
		else {
			return false;
		}

	}
	else {
		it = m_refineStrengthTable.find(0);
		if (it != m_refineStrengthTable.end())
		{
			if (auto it2 = it->second.find(material_type); it2 != it->second.end()) {
				fee = it2->second.fee;
				prob = it2->second.prob[strength_idx];
			}
			else {
				return false;
			}

		}
		else {
			return false;
		}
	}

	return true;
}

bool DragonSoulTable::GetDragonHeartExtValues(uint8_t ds_type, uint8_t grade_idx, std::vector<float>& vec_chargings, std::vector<float>& vec_probs)
{
	if (grade_idx >= DRAGON_SOUL_GRADE_MAX)
	{
		spdlog::error("Invalid dragon soul grade_idx(%d).", grade_idx);
		return false;
	}

	std::string stDragonSoulName;
	if (!GetDragonSoulGroupName(ds_type, stDragonSoulName))
	{
		spdlog::error("Invalid dragon soul type(%d).", ds_type);
		return false;
	}

	if (m_dragonHeartExtCharging.find(ds_type) != m_dragonHeartExtCharging.end()) {
		vec_chargings = m_dragonHeartExtCharging[ds_type];
	}
	else {
		vec_chargings = m_dragonHeartExtCharging[0];
	}

	if (m_dragonHeartExtTable.find(ds_type) != m_dragonHeartExtTable.end()) {
		const auto& tbl = m_dragonHeartExtTable[ds_type];
		for (const auto& val : tbl.val[grade_idx])
			vec_probs.push_back(val);
	}
	else {
		const auto& tbl = m_dragonHeartExtTable[0];
		for (const auto& val : tbl.val[grade_idx])
			vec_probs.push_back(val);
	}

	return true;
}

bool DragonSoulTable::GetDragonSoulExtValues(uint8_t ds_type, uint8_t grade_idx, float& prob, uint32_t& by_product)
{
	if (grade_idx >= DRAGON_SOUL_GRADE_MAX)
	{
		spdlog::error("Invalid dragon soul grade_idx(%d).", grade_idx);
		return false;
	}

	std::string stDragonSoulName;
	if (!GetDragonSoulGroupName(ds_type, stDragonSoulName))
	{
		spdlog::error("Invalid dragon soul type(%d).", ds_type);
		return false;
	}

	if (m_dragonSoulExtTable.find(ds_type) != m_dragonSoulExtTable.end()) {
		const auto& tbl = m_dragonSoulExtTable[ds_type];
		const auto& val = tbl.grades[grade_idx];
		prob = val.prob;
		by_product = val.byProduct;
	}
	else {
		const auto& tbl = m_dragonSoulExtTable[0];
		const auto& val = tbl.grades[grade_idx];
		prob = val.prob;
		by_product = val.byProduct;
	}

	return true;
}

#ifdef ENABLE_DS_SET
uint8_t DragonSoulTable::GetBasicApplyCount(uint8_t iType)
{
	TVecApplys vec_Applys;
	if (!GetBasicApplys(iType, vec_Applys))
	{
		return 0;
	}

	return vec_Applys.size();
}

bool DragonSoulTable::GetBasicApplyValue(uint8_t iType, uint16_t iApplyType, ApplyValue& iApplyValue)
{
	TVecApplys vec_Applys;
	if (!GetBasicApplys(iType, vec_Applys))
	{
		return false;
	}

	for (const auto& it : vec_Applys)
	{
		if (it.apply_type == iApplyType)
		{
			iApplyValue = it.apply_value;
		}
	}

	return false;
}

bool DragonSoulTable::GetAdditionalApplyValue(uint8_t iType, uint16_t iApplyType, ApplyValue& iApplyValue)
{
	TVecApplys vec_Applys;
	if (!GetAdditionalApplys(iType, vec_Applys))
	{
		return false;
	}

	for (const auto& it : vec_Applys)
	{
		if (it.apply_type == iApplyType)
		{
			iApplyValue = it.apply_value;
		}
	}

	return false;
}

ApplyValue DragonSoulTable::GetDSSetValue(uint8_t iAttributeIndex, uint16_t iApplyType, uint8_t iType, uint8_t iSetGrade)
{
    auto fWeight = GetDSSetWeight(iType, iSetGrade);
	
	ApplyValue iSetValue;
	if (iAttributeIndex < GetBasicApplyCount(iType)) {
		iSetValue = GetDSBasicApplyValue(iType, iApplyType);
	}
	else {
		iSetValue = GetDSAdditionalApplyValue(iType, iApplyType);
	}

	fWeight /= 100.0f;
	if (iSetValue <= 0.0f)
		return 0;

	return iSetValue * fWeight;
}

float DragonSoulTable::GetDSSetWeight(uint8_t iDSType, uint8_t iSetGrade)
{
	if (iSetGrade < DRAGON_SOUL_GRADE_ANCIENT)
	{
		return 0;
	}

	if (iSetGrade >= DRAGON_SOUL_GRADE_MAX)
	{
		return 0;
	}
	static uint8_t s_dssSetLimit[DRAGON_SOUL_GRADE_MAX] = {
	    0, 0, 0, 1, 2, 3, 4,
	};
	float fWeight = 0.0f;
	GetWeight(iDSType, s_dssSetLimit[iSetGrade], 0, 0, fWeight);

	return fWeight;
}
uint8_t DragonSoulTable::GetDSBasicApplyCount(uint8_t iDSType)
{
	TVecApplys vec_Applys;
	if (!GetBasicApplys(iDSType, vec_Applys))
	{
		return false;
	}

	return vec_Applys.size();
}
ApplyValue DragonSoulTable::GetDSBasicApplyValue(uint8_t iDSType, ApplyType applyType)
{
	ApplyValue applyValue = 0;
	GetBasicApplyValue(iDSType, applyType, applyValue);

	return applyValue;
}

ApplyValue DragonSoulTable::GetDSAdditionalApplyValue(uint8_t iDSType, ApplyType applyType)
{
	ApplyValue applyValue = 0;
	GetAdditionalApplyValue(iDSType, applyType, applyValue);

	return applyValue;
}


#endif
