/*********************************************************************
 * date        : 2006.11.20
 * file        : cube.h
 * author      : mhh
 * description : 큐브시스템
 */

#ifndef METIN2_SERVER_GAME_CUBE_H
#define METIN2_SERVER_GAME_CUBE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/Types.hpp>

class CHARACTER;
bool Cube_init();

bool Cube_make(CHARACTER *ch, GenericVnum cubeVnum);

void Cube_open(CHARACTER *ch);
void Cube_close(CHARACTER *ch);

#endif /* METIN2_SERVER_GAME_CUBE_H */
