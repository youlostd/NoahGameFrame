#include <game/BuildingTypes.hpp>

#include <xml/Util.hpp>

#include <base/Exception.hpp>

#include <storm/StringUtil.hpp>

METIN2_BEGIN_NS

bool ParseProto(ObjectProto& entry, const XmlNode* node)
{
	storm::StringRef str;

	//
	// Root node
	//

	GetAttribute(node, "vnum", entry.vnum);
	ParseStringTableEntry(node, "type", &GetBuildingTypeValue, entry.type);
	GetAttribute(node, "name", entry.name);
	GetAttribute(node, "price", entry.price);

	TryGetAttribute(node, "life", entry.life);
	TryGetAttribute(node, "npc", entry.npcVnum);
	TryGetAttribute(node, "group", entry.groupVnum);
	TryGetAttribute(node, "dependent-group", entry.dependOnGroupVnum);
	TryGetAttribute(node, "allow-placement", entry.allowPlacement);

	//
	// <position> node
	//

	auto pos = node->first_node("position");
	if (pos) {
		TryGetAttribute(pos, "start-x", entry.region[0]);
		TryGetAttribute(pos, "start-y", entry.region[1]);
		TryGetAttribute(pos, "end-x", entry.region[2]);
		TryGetAttribute(pos, "end-y", entry.region[3]);
	}

	//
	// <rotation> node
	//

	auto rot = node->first_node("rotation");
	if (rot) {
		TryGetAttribute(rot, "x-limit", entry.rotationLimits[0]);
		TryGetAttribute(rot, "y-limit", entry.rotationLimits[1]);
		TryGetAttribute(rot, "z-limit", entry.rotationLimits[2]);
	}

	//
	// <material> nodes
	//

	uint32_t i = 0;
	for (auto m : nodes(node, "material")) {
		METIN2_XML_PARSE_ASSERT(m, i < OBJECT_MATERIAL_MAX_NUM);
		auto& mat = entry.materials[i++];

		GetAttribute(m, "vnum", mat.itemVnum);
		GetAttribute(m, "count", mat.count);
	}

	//
	// <appearance> nodes
	//

	auto appearance = node->first_node("appearance");
	if (appearance)
		TryGetAttribute(appearance, "msm", entry.msmFilename);

	return true;
}

static bool HasMaterials(const ObjectProto& entry)
{
	for (int i = 0; i < OBJECT_MATERIAL_MAX_NUM; ++i) {
		if (entry.materials[i].itemVnum != 0)
			return true;
	}

	return false;
}

static bool HasPosition(const ObjectProto& entry)
{
	return entry.region[0] != 0 &&
	       entry.region[1] != 0 &&
	       entry.region[2] != 0 &&
	       entry.region[3] != 0;
}

static bool HasRotation(const ObjectProto& entry)
{
	return entry.rotationLimits[0] != 0.0f &&
	       entry.rotationLimits[1] != 0.0f &&
	       entry.rotationLimits[2] != 0.0f;
}

XmlNode* FormatProto(const ObjectProto& entry, XmlMemoryPool* pool)
{
	auto node = pool->allocate_node(rapidxml::node_element, "object");

	storm::StringRef strref;
	storm::String str;

	//
	// Root node
	//

	AddAttribute(pool, node, "vnum", entry.vnum);

	ThrowErrorIfNot(GetBuildingTypeString(strref, entry.type),
	                "Failed to format type: {0}",
	                entry.type);
	AddAttribute(pool, node, "type", strref);

	AddAttribute(pool, node, "name", entry.name);
	AddAttribute(pool, node, "price", entry.price);

	if (entry.life != 0)
		AddAttribute(pool, node, "life", entry.life);

	if (entry.npcVnum != 0)
		AddAttribute(pool, node, "npc", entry.npcVnum);

	if (entry.groupVnum != 0)
		AddAttribute(pool, node, "group", entry.groupVnum);

	if (entry.dependOnGroupVnum != 0)
		AddAttribute(pool, node, "dependent-group", entry.dependOnGroupVnum);

	AddAttribute(pool, node, "allow-placement", entry.allowPlacement);

	//
	// <position> node
	//

	if (HasPosition(entry)) {
		auto pos = pool->allocate_node(rapidxml::node_element, "position");

		if (entry.region[0] != 0)
			AddAttribute(pool, pos, "start-x", entry.region[0]);

		if (entry.region[1] != 0)
			AddAttribute(pool, pos, "start-y", entry.region[1]);

		if (entry.region[2] != 0)
			AddAttribute(pool, pos, "end-x", entry.region[2]);

		if (entry.region[3] != 0)
			AddAttribute(pool, pos, "end-y", entry.region[3]);

		node->append_node(pos);
	}

	//
	// <rotation> node
	//

	if (HasRotation(entry)) {
		auto rot = pool->allocate_node(rapidxml::node_element, "rotation");

		if (entry.rotationLimits[0] != 0.0f)
			AddAttribute(pool, rot, "x-limit", entry.rotationLimits[0]);

		if (entry.rotationLimits[1] != 0.0f)
			AddAttribute(pool, rot, "y-limit", entry.rotationLimits[1]);

		if (entry.rotationLimits[2] != 0.0f)
			AddAttribute(pool, rot, "z-limit", entry.rotationLimits[2]);

		node->append_node(rot);
	}

	//
	// <material> nodes
	//

	if (HasMaterials(entry)) {
		for (int i = 0; i < OBJECT_MATERIAL_MAX_NUM; ++i) {
			if (entry.materials[i].itemVnum == 0)
				continue;

			auto mat = pool->allocate_node(rapidxml::node_element, "material");

			AddAttribute(pool, mat, "vnum", entry.materials[i].itemVnum);
			AddAttribute(pool, mat, "count", entry.materials[i].count);

			node->append_node(mat);
		}
	}

	//
	// <appearance> nodes
	//

	if (!entry.msmFilename.empty()) {
		auto appearance = pool->allocate_node(rapidxml::node_element,
		                                      "appearance");
		AddAttribute(pool, appearance, "msm", entry.msmFilename);
		node->append_node(appearance);
	}

	return node;
}

METIN2_END_NS
