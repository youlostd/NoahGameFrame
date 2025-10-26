#include "Main.hpp"

#include <game/ProtoXml.hpp>
//#include <game/ProtoClientDb.hpp>
#include <game/ItemTypes.hpp>
#include <game/MobTypes.hpp>
//#include <game/MapTypes.hpp>
#include <game/MotionTypes.hpp>
//#include <game/BuildingTypes.hpp>
//#include <game/SkillTypes.hpp>

#include <vector>

METIN2_BEGIN_NS

namespace
{

template <class T>
bool FormatSimpleProto(const std::string& input,
                       const std::string& output,
                       const char* name,
                       bool verbose)
{
	std::vector<T> items;
	if (!LoadProtoXml(input, items))
		return false;

	return WriteProtoXml(output, items, name);
}

static bool FormatItemProto(const std::string& input,
                            const std::string& output,
                            bool verbose)
{ return FormatSimpleProto<ItemProto>(input, output, "item-proto", verbose); }

static ActionRegistrator item("item", &FormatItemProto);

static bool FormatMobProto(const std::string& input,
                           const std::string& output,
                           bool verbose)
{ return FormatSimpleProto<MobProto>(input, output, "mob-proto", verbose); }

static ActionRegistrator mob("mob", &FormatMobProto);

static bool FormatMotionProto(const std::string& input,
                              const std::string& output,
                              bool verbose)
{ return FormatSimpleProto<MotionProto>(input, output, "motion-proto", verbose); }

static ActionRegistrator motion("motion", &FormatMotionProto);

//static bool FormatObjectProto(const std::string& input,
//                              const std::string& output,
//                              bool verbose)
//{ return FormatSimpleProto<ObjectProto>(input, output, "object-proto", verbose); }

//static ActionRegistrator object("object", &FormatObjectProto);

//static bool FormatSkillProto(const std::string& input,
 //                            const std::string& output,
 //                            bool verbose)
//{ return FormatSimpleProto<SkillProto>(input, output, "skill-proto", verbose); }

//static ActionRegistrator skill("skill", &FormatSkillProto);

//static bool FormatItemAttrProto(const std::string& input,
//                                const std::string& output,
//                                bool verbose)
//{ return FormatSimpleProto<ItemAttrProto>(input, output, "item-attr", verbose); }

//static ActionRegistrator attr("attr", &FormatItemAttrProto);

static bool FormatRefineProto(const std::string& input,
                              const std::string& output,
                              bool verbose)
{ return FormatSimpleProto<RefineProto>(input, output, "refiner-sets", verbose); }

static ActionRegistrator refine("refine", &FormatRefineProto);

static bool FormatRefinerSets(const std::string& input,
                              const std::string& output,
                              bool verbose)
{ return FormatSimpleProto<RefinerSet>(input, output, "refiner-proto", verbose); }

static ActionRegistrator refiner("refiner", &FormatRefinerSets);

//static bool FormatAppearanceProto(const std::string& input,
//                                  const std::string& output,
//                                  bool verbose)
//{ return FormatSimpleProto<AppearanceProto>(input, output, "appearance-proto", verbose); }
//
//static ActionRegistrator appearance("appearance", &FormatAppearanceProto);

// TODO(tim): maps

}

METIN2_END_NS
