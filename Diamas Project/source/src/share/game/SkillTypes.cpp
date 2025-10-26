#include <game/SkillTypes.hpp>
#include <game/SkillConstants.hpp>
#include <game/Constants.hpp>

#include <xml/Util.hpp>

#include <base/Exception.hpp>



bool ParseProto(SkillProto& entry, const XmlNode* node)
{
	storm::StringRef str;

	//
	// Root node
	//

	GetAttribute(node, "vnum", entry.vnum);
	GetAttribute(node, "name", entry.name);

	ParseStringTableEntry(node, "type", &GetSkillTypeValue, entry.type);
	ParseStringTableEntry(node, "attr-type", &GetSkillAttrTypeValue, entry.attrType);

	if (node->first_attribute("flags"))
		ParseStringTableEntry(node, "flags", &GetSkillFlagsValue, entry.flags);

	GetAttribute(node, "cooltime", entry.cooltime);

	auto level = node->first_node("level");
	if (level) {
		TryGetAttribute(level, "step", entry.levelStep);
		TryGetAttribute(level, "max", entry.maxLevel);
		TryGetAttribute(level, "limit", entry.levelLimit);
	}

	auto cost = node->first_node("cost");
	if (cost) {
		TryGetAttribute(cost, "sp", entry.spCost);
		TryGetAttribute(cost, "grand-master-sp", entry.grandMasterSpCost);
	}

	auto target = node->first_node("target");
	if (target) {
		TryGetAttribute(target, "range", entry.targetRange);
		TryGetAttribute(target, "max-hit", entry.maxHitCount);
		TryGetAttribute(target, "splash-range", entry.splashRange);

		TryGetAttribute(target, "splash-dmg-poly", entry.splashDamage);
	}

	auto property = node->first_node("first-property");
	if (property) {
		if (property->first_attribute("point")) {
			ParseStringTableEntry(property, "point", &GetPointTypeValue,
			                      entry.point);
		}

		TryGetAttribute(property, "value", entry.value);
		TryGetAttribute(property, "master-value", entry.masterValue);
		TryGetAttribute(property, "affect-flag", entry.affectFlag);
		TryGetAttribute(property, "duration", entry.duration);
		TryGetAttribute(property, "duration-sp", entry.durationSpCost);
	}

	property = node->first_node("second-property");
	if (property) {
		if (property->first_attribute("point")) {
			ParseStringTableEntry(property, "point", &GetPointTypeValue,
			                      entry.point2);
		}

		TryGetAttribute(property, "value", entry.value2);
		TryGetAttribute(property, "affect-flag", entry.affectFlag2);
		TryGetAttribute(property, "duration", entry.duration2);
	}

	property = node->first_node("third-property");
	if (property) {
		if (property->first_attribute("point")) {
			ParseStringTableEntry(property, "point", &GetPointTypeValue,
			                      entry.point3);
		}

		TryGetAttribute(property, "value", entry.value3);
		TryGetAttribute(property, "affect-flag", entry.affectFlag);
		TryGetAttribute(property, "duration", entry.duration3);
	}

	// Requirements Node

	auto s = node->first_node("requirements");
	if(s) {
		for (auto m = s->first_node(); m; m = m->next_sibling()) {
			if(!m)
				continue;
			
			SkillRequirement rq = {};
			GetAttribute(m, "skill-vnum", rq.skillVnum);
			GetAttribute(m, "skill-level", rq.skillLevel);
			entry.requirements.emplace_back(rq);
		}
	}

	return true;
}

XmlNode* FormatProto(const SkillProto& entry, XmlMemoryPool* pool)
{
	auto node = pool->allocate_node(rapidxml::node_element, "skill");

	storm::StringRef strref;
	storm::String str;

	//
	// Root node
	//

	AddAttribute(pool, node, "vnum", entry.vnum);

	ThrowErrorIfNot(GetSkillTypeString(strref, entry.type),
	                "Failed to format skill type: {0}",
	                entry.type);
	AddAttribute(pool, node, "type", strref);

	AddAttribute(pool, node, "name", entry.name);

	ThrowErrorIfNot(GetSkillAttrTypeString(strref, entry.attrType),
	                "Failed to format skill attr-type: {0}",
	                entry.attrType);
	AddAttribute(pool, node, "attr-type", strref);

	if (entry.flags != 0) {
		ThrowErrorIfNot(GetSkillFlagsString(str, entry.flags),
		                "Failed to format skill flags: {0}",
		                entry.flags);
		AddAttribute(pool, node, "flags", str);
	}

	if (entry.cooltime.size())
		AddAttribute(pool, node, "cooltime", entry.cooltime);

	if (!entry.levelStep.empty() || entry.maxLevel != 0 || entry.levelLimit != 0) {
		auto level = pool->allocate_node(rapidxml::node_element, "level");

		if (!entry.levelStep.empty())
			AddAttribute(pool, level, "step", entry.levelStep);

		if (entry.maxLevel != 0)
			AddAttribute(pool, level, "max", entry.maxLevel);

		if (entry.levelLimit != 0)
			AddAttribute(pool, level, "limit", entry.levelLimit);

		node->append_node(level);
	}

	if (entry.spCost.size() || entry.grandMasterSpCost.size()) {
		auto cost = pool->allocate_node(rapidxml::node_element, "cost");

		if (entry.spCost.size())
			AddAttribute(pool, cost, "sp", entry.spCost);

		if (entry.grandMasterSpCost.size())
			AddAttribute(pool, cost, "grand-master-sp", entry.grandMasterSpCost);

		node->append_node(cost);
	}

	const bool needsTarget = entry.targetRange != 0 ||
	                         entry.maxHitCount != 0 ||
	                         entry.splashRange != 0 ||
	                         entry.splashDamage.size();

	if (needsTarget) {
		auto target = pool->allocate_node(rapidxml::node_element, "target");

		if (entry.targetRange != 0)
			AddAttribute(pool, target, "range", entry.targetRange);

		if (entry.maxHitCount != 0)
			AddAttribute(pool, target, "max-hit", entry.maxHitCount);

		if (entry.splashRange != 0)
			AddAttribute(pool, target, "splash-range", entry.splashRange);

		if (entry.splashDamage.size())
			AddAttribute(pool, target, "splash-dmg-poly", entry.splashDamage);

		node->append_node(target);
	}

	const bool needsProp1 = entry.point != 0 ||
	                        entry.affectFlag != 0 ||
	                        entry.value.size() ||
	                        entry.masterValue.size() ||
	                        entry.duration.size() ||
	                        entry.durationSpCost.size();

	if (needsProp1) {
		auto prop = pool->allocate_node(rapidxml::node_element, "first-property");

		if (entry.point != 0) {
			ThrowErrorIfNot(GetPointTypeString(strref, entry.point),
			                "Failed to format point type: {0}",
			                entry.point);
			AddAttribute(pool, prop, "point", strref);
		}

		if (entry.value.size())
			AddAttribute(pool, prop, "value", entry.value);

		if (entry.masterValue.size())
			AddAttribute(pool, prop, "master-value", entry.masterValue);

		if (entry.affectFlag != 0)
			AddAttribute(pool, prop, "affect-flag", entry.affectFlag);

		if (entry.duration.size())
			AddAttribute(pool, prop, "duration", entry.duration);

		if (entry.durationSpCost.size())
			AddAttribute(pool, prop, "duration-sp", entry.durationSpCost);

		node->append_node(prop);
	}

	const bool needsProp2 = entry.point2 != 0 ||
	                        entry.affectFlag2 != 0 ||
	                        entry.value2.size() ||
	                        entry.duration2.size();

	if (needsProp2) {
		auto prop = pool->allocate_node(rapidxml::node_element, "second-property");

		if (entry.point2 != 0) {
			ThrowErrorIfNot(GetPointTypeString(strref, entry.point2),
			                "Failed to format point2 type: {0}",
			                entry.point2);
			AddAttribute(pool, prop, "point", strref);
		}

		if (entry.value2.size())
			AddAttribute(pool, prop, "value", entry.value2);

		if (entry.affectFlag2 != 0)
			AddAttribute(pool, prop, "affect-flag", entry.affectFlag2);

		if (entry.duration2.size())
			AddAttribute(pool, prop, "duration", entry.duration2);

		node->append_node(prop);
	}

	const bool needsProp3 = entry.point3 != 0 ||
	                        entry.affectFlag3 != 0 ||
	                        entry.value3.size() ||
	                        entry.duration3.size();

	if (needsProp3) {
		auto prop = pool->allocate_node(rapidxml::node_element, "third-property");

		if (entry.point3 != 0) {
			ThrowErrorIfNot(GetPointTypeString(strref, entry.point3),
			                "Failed to format point3 type: {0}",
			                entry.point3);
			AddAttribute(pool, prop, "point", strref);
		}

		if (entry.value3.size())
			AddAttribute(pool, prop, "value", entry.value3);

		if (entry.affectFlag3 != 0)
			AddAttribute(pool, prop, "affect-flag", entry.affectFlag3);

		if (entry.duration3.size())
			AddAttribute(pool, prop, "duration", entry.duration3);

		node->append_node(prop);
	}

	if(!entry.requirements.empty()) {
		auto requirements = pool->allocate_node(rapidxml::node_element, "requirements");

		for (auto& rq : entry.requirements) {
			auto requirement = pool->allocate_node(rapidxml::node_element, "requirement");

			AddAttribute(pool, requirement, "skill-vnum", rq.skillVnum);
			AddAttribute(pool, requirement, "skill-level", rq.skillLevel);

	
			requirements->append_node(requirement);
		}

		node->append_node(requirements);
	}

	return node;
}


