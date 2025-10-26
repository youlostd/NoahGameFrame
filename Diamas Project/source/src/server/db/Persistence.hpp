#ifndef METIN2_SERVER_DB_PERSISTENCE_HPP
#define METIN2_SERVER_DB_PERSISTENCE_HPP


#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



#include <vector>
#include <string>
#include <mysql/AsyncSQL.h>
#include <game/DbPackets.hpp>

bool CreatePlayerCreateQuery(const TPlayerCreatePacket& p, std::string& query);
bool CreatePlayerSaveQuery(const TPlayerTable& tab, std::string& query);
bool CreatePlayerLoadQuery(uint32_t pid, std::string& query);
bool CreatePlayerTableFromRes(MYSQL_RES* res, TPlayerTable& tab);
bool CreateItemSaveQuery(const TPlayerItem& item, std::string& query);
bool CreateItemTableFromRes(MYSQL_RES* res, uint32_t owner, std::vector<TPlayerItem>& items);
void CreateQuickslotLoadQuery(uint32_t pid, std::string& query);
void CreateQuickslotTableFromRes(MYSQL_RES* res,
                                 std::vector<QuickslotPair>& quickslots);
void CreateQuickslotSaveQuery(uint32_t pid, uint32_t index,
                              const TQuickslot& slot, std::string& query);
void CreateQuickslotDeleteQuery(uint32_t pid, uint32_t index,
                                std::string& query);
void CreateSkillLoadQuery(uint32_t pid, std::string& query);
void CreateSkillTableFromRes(MYSQL_RES* res, std::vector<SkillPair>& skills);
void CreateSkillSaveQuery(uint32_t pid, uint32_t vnum,
                          const TPlayerSkill& skill, std::string& query);
void CreateSkillDeleteQuery(uint32_t pid, uint32_t vnum, std::string& query);
void CreateTitleDeleteQuery(uint32_t pid, std::string& query);
void CreateTitleLoadQuery(uint32_t pid, std::string& query);
void CreateTitleSaveQuery(uint32_t pid,
                          const TPlayerTitle& title, std::string& query);

#endif
