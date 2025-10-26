#include <game/ItemTypes.hpp>

#include <xml/Util.hpp>

#include <base/Exception.hpp>

#include <storm/Util.hpp>
#include <storm/StringUtil.hpp>
#include <vector>

METIN2_BEGIN_NS


namespace
{

void ParseVnum(const XmlNode* node, uint32_t& start, uint32_t& end)
{
	if (TryGetAttribute(node, "vnum", start)) {
		end = 1;
		return;
	}

	GetAttribute(node, "vnum-begin", start);
	GetAttribute(node, "vnum-end", end);

	// TODO(tim): Exception
	if (start > end)
		spdlog::error("Vnum range start is larger than end");

	end -= start;
}


void ParseVector(const XmlNode* node, float& x, float& y, float& z)
{
	GetAttribute(node, "x", x);
	GetAttribute(node, "y", y);
	GetAttribute(node, "z", z);
}

void FormatVector(XmlMemoryPool* pool, XmlNode* node, float x, float y, float z)
{
	AddAttribute(pool, node, "x", x);
	AddAttribute(pool, node, "y", y);
	AddAttribute(pool, node, "z", z);
}

}

bool ParseProto(RefineProto& entry, const XmlNode* node)
{
	storm::StringRef str;

	//
	// Root node
	//

	GetAttribute(node, "id", entry.id);
	GetAttribute(node, "cost", entry.cost);
	GetAttribute(node, "prob", entry.prob);


	if (node->first_attribute("flags"))
		ParseStringTableEntry(node, "flags", &GetRefineFlagsValue, entry.flags);

	//
	// <material> nodes
	//

	uint32_t i = 0;
	for (auto m = node->first_node("material"); m; m = m->next_sibling("material")) {
		i++;

		RefineMaterial mat;
		GetAttribute(m, "vnum", mat.vnum);
		GetAttribute(m, "count", mat.count);
		entry.materials.push_back(mat);
	}

	uint32_t i2 = 0;
	for (auto m = node->first_node("enhance_material"); m; m = m->next_sibling("enhance_material")) {
		i2++;

		RefineEnhanceMaterial mat;
		GetAttribute(m, "vnum", mat.vnum);
		GetAttribute(m, "count", mat.count);
		GetAttribute(m, "prob", mat.prob);
		entry.enhance_materials.push_back(mat);
	}
	return true;
}

static bool HasMaterials(const RefineProto& entry)
{
	for (const auto& mat : entry.materials) {
		if (mat.vnum != 0)
			return true;
	}

	return false;
}

XmlNode* FormatProto(const RefineProto& entry, XmlMemoryPool* pool)
{
	auto node = pool->allocate_node(rapidxml::node_element, "set");

	storm::StringRef strref;
	storm::String str;

	//
	// Root node
	//

	AddAttribute(pool, node, "id", entry.id);
	AddAttribute(pool, node, "cost", entry.cost);
	AddAttribute(pool, node, "prob", entry.prob);

	if (entry.flags != 0) {
		storm::String str;
		ThrowErrorIfNot(GetRefineFlagsString(str, entry.flags),
		                "Failed to format flags: {0}",
		                entry.flags);
		AddAttribute(pool, node, "flags", str.c_str());
	}

	//
	// <material> nodes
	//

	if (HasMaterials(entry)) {
		for (const auto& material : entry.materials) {
			if (material.vnum == 0)
				continue;

			auto mat = pool->allocate_node(rapidxml::node_element, "mat");

			AddAttribute(pool, mat, "value", material.vnum);
			AddAttribute(pool, mat, "value", material.count);

			node->append_node(mat);
		}
	}

	return node;
}

bool ParseProto(ItemAttrProto& entry, const XmlNode* node)
{
	storm::StringRef str;
	uint32_t val;

	//
	// Root node
	//

	ParseStringTableEntry(node, "apply",
	                      &GetApplyTypeValue,
	                      entry.apply);

	GetAttribute(node, "prob", entry.prob);

	//
	// <values> node
	//

	auto v = node->first_node("values");
	for (auto m = v->first_node(); m; m = m->next_sibling()) {
		GetAttribute(m, "index", val);
		METIN2_XML_PARSE_ASSERT(m, val < STORM_ARRAYSIZE(entry.values));
		GetAttribute(m, "value", entry.values[val]);
	}

	//
	// <sets> node
	//

	auto s = node->first_node("sets");
	for (auto m = s->first_node(); m; m = m->next_sibling()) {
		ParseStringTableEntry(m, "type",
		                      &GetItemAttributeSetValue,
		                      val);
		METIN2_XML_PARSE_ASSERT(m, val < STORM_ARRAYSIZE(entry.maxBySet));

		GetAttribute(m, "max", entry.maxBySet[val]);
	}

	return true;
}




METIN2_END_NS
