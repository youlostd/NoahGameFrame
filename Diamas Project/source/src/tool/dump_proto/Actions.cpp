#include "Main.hpp"

#include <game/ProtoClientDb.hpp>
#include <game/ProtoXml.hpp>
#include <game/ItemTypes.hpp>
#include <game/MobTypes.hpp>
//#include <game/MapTypes.hpp>
#include <game/MotionTypes.hpp>
#include <game/Types.hpp>
#include <game/BuildingTypes.hpp>
//#include <game/SkillTypes.hpp
#include <game/ProtoReader.hpp>
#include <game/SkillTypes.hpp>

#include "game/CubeTypes.hpp"

METIN2_BEGIN_NS

namespace
{

template <class T>
bool DumpSimpleProto(const std::string& input,
                     const std::string& output,
                     bool verbose)
{
	std::vector<T> items;
	if (!LoadProtoXml<T>(input, items))
		return false;

	return WriteSimpleProto<T>(output.c_str(), items);
}

template <class T>
bool DumpOldItemProto(const std::string& input,
                     const std::string& output,
                     bool verbose)
{
	std::vector<T> items;
	if (!LoadItemProto(input, items))
		return false;

	return WriteSimpleProto<T>(output.c_str(), items);
}

template <class T>
bool DumpOldMobProto(const std::string& input,
                     const std::string& output,
                     bool verbose)
{
	std::vector<T> items;
	if (!LoadMobProto(input, items))
		return false;

	return WriteSimpleProto<T>(output.c_str(), items);
}
//
// item-proto handling
//

static bool DumpOldServerItemProto(const std::string& input,
                                const std::string& output,
                                bool verbose)
{ return DumpOldItemProto<TItemTable>(input, output, verbose); }

static ActionRegistrator serverItemOld("server-item-old", &DumpOldServerItemProto);

//
// mob-proto handling
//


static bool DumpOldServerMobProto(const std::string& input,
                               const std::string& output,
                               bool verbose)
{ return DumpOldMobProto<TMobTable>(input, output, verbose); }

static ActionRegistrator oldServerMob("server-mob-old", &DumpOldServerMobProto);
//
// motion-proto handling
//

static bool DumpMotionProto(const std::string& input,
                            const std::string& output,
                            bool verbose)
{ return DumpSimpleProto<MotionProto>(input, output, verbose); }

static ActionRegistrator motion("motion", &DumpMotionProto);


//
// object-proto handling
//

static bool DumpObjectProto(const std::string& input,
                            const std::string& output,
                            bool verbose)
{ return DumpSimpleProto<ObjectProto>(input, output, verbose); }

static ActionRegistrator object("object", &DumpObjectProto);


//
// skill-proto handling
//

static bool DumpSkillProto(const std::string& input,
                           const std::string& output,
                           bool verbose)
{ return DumpSimpleProto<SkillProto>(input, output, verbose); }

static ActionRegistrator skill("skill", &DumpSkillProto);



//
// item-attr-proto handling
//

static bool DumpItemAttrProto(const std::string& input,
                              const std::string& output,
                              bool verbose)
{ return DumpSimpleProto<ItemAttrProto>(input, output, verbose); }

static ActionRegistrator attr("item-attr", &DumpItemAttrProto);


//
// cube-proto handling
//

static bool DumpCubeProto(const std::string& input,
                            const std::string& output,
                            bool verbose)
{ return DumpSimpleProto<CubeProto>(input, output, verbose); }

static ActionRegistrator cube("cube", &DumpCubeProto);


//
// refine-proto handling
//

static bool DumpRefineProto(const std::string& input,
                            const std::string& output,
                            bool verbose)
{ return DumpSimpleProto<RefineProto>(input, output, verbose); }

static ActionRegistrator refine("refine", &DumpRefineProto);


}

METIN2_END_NS
