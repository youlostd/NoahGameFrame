#include <game/CubeTypes.hpp>
#include <game/CubeConstants.hpp>
#include <game/Constants.hpp>

#include <xml/Util.hpp>

#include <base/Exception.hpp>


bool ParseProto(CubeProto& entry, const XmlNode* node)
{
	//
	// Root node
	//

	GetAttribute(node, "vnum", entry.vnum);

	ParseStringTableEntry(node, "category", &GetCubeCategoryValue, entry.category);
	GetAttribute(node, "probability", entry.probability);
	GetAttribute(node, "cost", entry.cost);
	
	// Materials Node

	const auto materials = node->first_node("materials");
	if (materials) {
		int i = 0;
		for (auto e = materials->first_node(); e; e = e->next_sibling()) {
			
			CubeItem mat{};
			GetAttribute(e, "vnum", mat.vnum);
			GetAttribute(e, "count", mat.count);
			TryGetAttribute(e, "transfer-bonus", mat.transferBonus);
			TryGetAttribute(e, "remove-at-failure", mat.removeAtFailure);
			entry.items.push_back(mat);
			
		}
	}
	
	const auto reward = node->first_node("reward");
	if(reward) {
		GetAttribute(reward, "vnum", entry.reward.vnum);
		GetAttribute(reward, "count", entry.reward.count);
	}

	
	return true;
}

XmlNode* FormatProto(const CubeProto& entry, XmlMemoryPool* pool)
{
	auto node = pool->allocate_node(rapidxml::node_element, "recipe");

	storm::StringRef strref;
	storm::String str;

	//
	// Root node
	//

	AddAttribute(pool, node, "vnum", entry.vnum);

	ThrowErrorIfNot(GetCubeCategoryString(strref, entry.category),
	                "Failed to format cube category: {0}",
	                entry.category);
	AddAttribute(pool, node, "category", strref);

	AddAttribute(pool, node, "probability", entry.probability);
	AddAttribute(pool, node, "cost", entry.cost);

	if(!entry.items.empty()) {
		auto materials = pool->allocate_node(rapidxml::node_element, "materials");

		for (auto& mat : entry.items) {
			auto e = pool->allocate_node(rapidxml::node_element, "material");

			AddAttribute(pool, e, "vnum", mat.vnum);
			AddAttribute(pool, e, "count", mat.count);
			if(mat.transferBonus)
				AddAttribute(pool, e, "transfer-bonus", mat.transferBonus);
			if(mat.removeAtFailure)
				AddAttribute(pool, e, "remove-at-failure", mat.removeAtFailure);
	
			materials->append_node(e);
		}

		node->append_node(materials);
	}
	
	auto rewardNode = pool->allocate_node(rapidxml::node_element, "reward");
	AddAttribute(pool, rewardNode, "vnum", entry.reward.vnum);
	AddAttribute(pool, rewardNode, "count", entry.reward.count);
	node->append_node(rewardNode);


	return node;
}


