#ifndef METIN2_TOOL_CONVPROTO_PROTOREADER_HPP
#define METIN2_TOOL_CONVPROTO_PROTOREADER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

// YMIR's ProtoReader.hpp (mainly unmodified)

#include "Format.hpp"

#include <storm/String.hpp>

#include <vector>

#include <game/MobTypes.hpp>

METIN2_BEGIN_NS

bool LoadItemProto(const std::string& filename,
                   std::vector<TItemTable>& items);

bool LoadMobProto(const std::string& filename,
                  std::vector<TMobTable>& mobs);

METIN2_END_NS

#endif
