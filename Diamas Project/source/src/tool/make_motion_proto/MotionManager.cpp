#include <SpdLog.hpp>
#include "MotionManager.hpp"
#include "MotionList.hpp"
#include "Motion.hpp"
#include "Race.hpp"
#include "GrannyUtil.hpp"

#include <game/MotionTypes.hpp>
#include <game/MobTypes.hpp>


#include <xml/Document.hpp>
#include <xml/ErrorHandling.hpp>

#include <storm/io/FileUtil.hpp>
#include <storm/io/Path.hpp>
#include <storm/Tokenizer.hpp>
#include <storm/StringUtil.hpp>
#include <storm/io/TextFileLoader.hpp>

namespace
{

static storm::String GetPathFromPcRace(uint32_t race)
{
	switch (race) {
		case MAIN_RACE_WARRIOR_M: return "d:/ymir work/pc/warrior/";
		case MAIN_RACE_WARRIOR_W: return "d:/ymir work/pc2/warrior/";
		case MAIN_RACE_ASSASSIN_M: return "d:/ymir work/pc/assassin/";
		case MAIN_RACE_ASSASSIN_W: return "d:/ymir work/pc2/assassin/";
		case MAIN_RACE_SURA_M: return "d:/ymir work/pc/sura/";
		case MAIN_RACE_SURA_W: return "d:/ymir work/pc2/sura/";
		case MAIN_RACE_SHAMAN_M: return "d:/ymir work/pc/shaman/";
		case MAIN_RACE_SHAMAN_W: return "d:/ymir work/pc2/shaman/";
		case MAIN_RACE_WOLFMAN_M: return "d:/ymir work/pc3/wolfman/";
		default: return "";
	}
}

static storm::StringRef GetMsmFromPcRace(uint32_t race)
{
	switch (race) {
		case MAIN_RACE_WARRIOR_M: return "warrior_m.msm";
		case MAIN_RACE_WARRIOR_W: return "warrior_w.msm";
		case MAIN_RACE_ASSASSIN_M: return "assassin_m.msm";
		case MAIN_RACE_ASSASSIN_W: return "assassin_w.msm";
		case MAIN_RACE_SURA_M: return "sura_m.msm";
		case MAIN_RACE_SURA_W: return "sura_w.msm";
		case MAIN_RACE_SHAMAN_M: return "shaman_m.msm";
		case MAIN_RACE_SHAMAN_W: return "shaman_w.msm";
		case MAIN_RACE_WOLFMAN_M: return "wolfman_m.msm";
		default: return "";
	}
}

}


bool MotionManager::Load(const storm::StringRef& npcList)
{
	if (!LoadPcMotions())
		return false;

	if (!LoadFromNpcList(npcList))
		return false;

	return true;
}

void MotionManager::UpdateAll()
{
	auto f = [this] (Race& race) {
		UpdateMotions(race);
	};

	std::for_each(m_races.begin(), m_races.end(), f);
}

bool MotionManager::LoadPcMotions()
{
	for (uint32_t race = 0; race < MAIN_RACE_MAX_NUM; ++race) {
		const storm::String path = GetPathFromPcRace(race);
		const storm::StringRef msm = GetMsmFromPcRace(race);

		Race r(race);

		if (!r.LoadModelScript((msm))) {
			SPDLOG_ERROR("Failed to load PC MSM '{0}'", msm);
			return false;
		}

		LoadPcRaceMode(MOTION_MODE_GENERAL, path + "general", r);
		LoadPcRaceMode(MOTION_MODE_TWOHAND_SWORD, path + "twohand_sword", r);
		LoadPcRaceMode(MOTION_MODE_ONEHAND_SWORD, path + "onehand_sword", r);
		LoadPcRaceMode(MOTION_MODE_DUALHAND_SWORD, path + "dualhand_sword", r);
		LoadPcRaceMode(MOTION_MODE_BOW, path + "bow", r);
		LoadPcRaceMode(MOTION_MODE_BELL, path + "bell", r);
		LoadPcRaceMode(MOTION_MODE_FAN, path + "fan", r);
		LoadPcRaceMode(MOTION_MODE_CLAW, path + "claw", r);

		LoadPcRaceMode(MOTION_MODE_HORSE, path + "horse", r);
		LoadPcRaceMode(MOTION_MODE_HORSE_TWOHAND_SWORD,
		               path + "horse_twohand_sword", r);
		LoadPcRaceMode(MOTION_MODE_HORSE_ONEHAND_SWORD,
		               path + "horse_onehand_sword", r);
		LoadPcRaceMode(MOTION_MODE_HORSE_DUALHAND_SWORD,
		               path + "horse_dualhand_sword", r);
		LoadPcRaceMode(MOTION_MODE_HORSE_BOW, path + "horse_bow", r);
		LoadPcRaceMode(MOTION_MODE_HORSE_BELL, path + "horse_bell", r);
		LoadPcRaceMode(MOTION_MODE_HORSE_FAN, path + "horse_fan", r);
		LoadPcRaceMode(MOTION_MODE_HORSE_CLAW, path + "horse_claw", r);

		LoadPcRaceMode(MOTION_MODE_FISHING, path + "fishing", r, false);
		LoadPcRaceMode(MOTION_MODE_WEDDING_DRESS, path + "wedding", r, false);

		m_races.push_back(std::move(r));
	}

	return true;
}

bool MotionManager::LoadFromNpcList(const storm::StringRef& filename)
{
	storm::TextFileLoader loader(storm::GetDefaultAllocator());

	bsys::error_code ec;
	loader.Load(filename, ec);

	if (ec) {
		SPDLOG_ERROR("Failed to load NPC list '{0}' with {1}",
		          filename, ec.message());

		return false;
	}

	std::vector<storm::String> lines;
	storm::Tokenize(std::string_view(loader.GetText()),
	                "\r\n",
	                lines);

	std::vector<storm::String> args;

	int i = 0;
	for (const auto& line : lines) {

		args.clear();
		storm::Tokenize(line,
		                "\t",
		                args);

		if (args.empty())
			continue;

		if (args.empty())
			continue;

		if (args.size() != 2) {
			SPDLOG_ERROR(
			          "NPC List '{0}' line {1} has only {2} tokens",
			          filename, i, args.size());
			return false;
		}

		uint32_t vnum;
		if (!storm::ParseNumber(args[0], vnum)) {
			SPDLOG_ERROR(
			          "NPC List '{0}' line {1} vnum is invalid",
			          filename, i);
			return false;
		}

		LoadNpcRace(vnum, args[1]);
	}

	return true;
}


bool MotionManager::LoadNpcRace(uint32_t vnum, const storm::StringRef& msmPath)
{
	const auto path = storm::SplitPath(msmPath).first;
	const auto filename = storm::String(path) + "/motlist.txt";

	MotionList motionList;
	if (!motionList.Load(filename))
		return false;

	Race r(vnum);

	if (!r.LoadModelScript(msmPath))
		return false;

	LoadMotion(motionList, path, MOTION_MODE_GENERAL, MOTION_WALK, r);
	LoadMotion(motionList, path, MOTION_MODE_GENERAL, MOTION_RUN, r);
	LoadMotion(motionList, path, MOTION_MODE_GENERAL, MOTION_NORMAL_ATTACK, r);

	LoadMotion(motionList, path, MOTION_MODE_GENERAL, MOTION_SPECIAL_1, r);
	LoadMotion(motionList, path, MOTION_MODE_GENERAL, MOTION_SPECIAL_2, r);
	LoadMotion(motionList, path, MOTION_MODE_GENERAL, MOTION_SPECIAL_3, r);
	LoadMotion(motionList, path, MOTION_MODE_GENERAL, MOTION_SPECIAL_4, r);
	LoadMotion(motionList, path, MOTION_MODE_GENERAL, MOTION_SPECIAL_5, r);

	m_races.push_back(std::move(r));
	return true;
}

void MotionManager::LoadPcRaceMode(uint32_t mode, const storm::String& path,
                                   Race& race, bool allowAttack)
{
	LoadMotion(path + "/run.msa", mode, MOTION_RUN, race);
	LoadMotion(path + "/walk.msa", mode, MOTION_WALK, race);

	if (!allowAttack)
		return;

	if (mode == MOTION_MODE_GENERAL ||
	    mode == MOTION_MODE_BOW || mode == MOTION_MODE_HORSE_BOW) {
		LoadMotion(path + "/attack.msa", mode, MOTION_COMBO_ATTACK_1, race);
		LoadMotion(path + "/attack_1.msa", mode, MOTION_COMBO_ATTACK_1, race);
		return;
	}

	LoadMotion(path + "/combo_01.msa", mode, MOTION_COMBO_ATTACK_1, race);
	LoadMotion(path + "/combo_02.msa", mode, MOTION_COMBO_ATTACK_2, race);
	LoadMotion(path + "/combo_03.msa", mode, MOTION_COMBO_ATTACK_3, race);
	LoadMotion(path + "/combo_04.msa", mode, MOTION_COMBO_ATTACK_4, race);
	LoadMotion(path + "/combo_05.msa", mode, MOTION_COMBO_ATTACK_5, race);
	LoadMotion(path + "/combo_06.msa", mode, MOTION_COMBO_ATTACK_6, race);
	LoadMotion(path + "/combo_07.msa", mode, MOTION_COMBO_ATTACK_7, race);
	LoadMotion(path + "/combo_08.msa", mode, MOTION_COMBO_ATTACK_8, race);
}

void MotionManager::LoadMotion(const MotionList& list,
                               const storm::StringRef& path,
                               uint32_t mode, uint32_t index,
                               Race& r)
{
	const auto key = MakeMotionKey(mode, index);

	const auto files = list.GetMotions(key);
	if (!files)
		return;

	storm::String animationScript;
	for (auto it = files->begin(), end = files->end(); it != end; ++it) {
		animationScript.assign(path.data(), path.size());
		animationScript += '/';
		animationScript += *it;

		Motion motion;
		if (!motion.LoadFile(animationScript))
			continue;

		r.AddMotion(key, motion);
	}
}

void MotionManager::LoadMotion(const storm::StringRef& filename,
                               uint32_t mode, uint32_t index,
                               Race& r)
{
	const auto key = MakeMotionKey(mode, index);

	Motion motion;
	if (!motion.LoadFile(filename))
		return;

	r.AddMotion(key, motion);
}

void MotionManager::UpdateMotions(Race& race)
{
	const auto& modelPath = race.GetModelPath();
	if (modelPath.empty()) {
		SPDLOG_ERROR("{0} has no model", race.GetVnum());
		return;
	}

	GrannyAnimationProcessor processor;
	if (!processor.LoadModel(modelPath))
		return;

	auto inner = [this, &processor] (Motion& motion) {
		const auto filename = motion.GetFilename();

		AnimationInfo info;
		if (!processor.ProcessAnimation(filename, info))
			return;

		motion.SetDuration(info.duration);
		motion.SetAccumulation(info.accumulation);
	};

	auto outer = [&inner] (Race::MotionMap::value_type& p)
	{ std::for_each(p.second.begin(), p.second.end(), inner); };

	auto& motions = race.GetMotionMap();
	std::for_each(motions.begin(), motions.end(), outer);
}


