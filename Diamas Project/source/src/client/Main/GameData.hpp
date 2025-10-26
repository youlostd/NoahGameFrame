#ifndef METIN2_GAMEDATA_HPP
#define METIN2_GAMEDATA_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

auto LoadGameData() -> bool;
auto RegisterSkill(uint8_t race, uint8_t group, uint8_t empire=0) -> void;

#endif /* METIN2_GAMEDATA_HPP */
