#ifndef METIN2_SERVER_GAME_MAIN_H
#define METIN2_SERVER_GAME_MAIN_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class MotionManager;
class LocaleService;

MotionManager &GetMotionManager();
LocaleService &GetLocaleService();
#endif /* METIN2_SERVER_GAME_MAIN_H */
