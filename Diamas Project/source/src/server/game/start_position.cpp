#include "start_position.h"

char g_nation_name[4][32] =
{
    "",
    "신수국",
    "천조국",
    "진노국",
};

//	LC_TEXT("신수국")
//	LC_TEXT("천조국")
//	LC_TEXT("진노국")

long g_start_map[4] =
{
    0,  // reserved
    1,  // 신수국
    21, // 천조국
    41  // 진노국
};

uint32_t g_start_position[4][3] =
{
    {0, 0, 0}, // reserved
    {59700, 68200, 1},
    {55700, 55500, 21},
    {48000, 73600, 41}
};

uint32_t arena_return_position[4][3] =
{
    {0, 0, 0},
    {40400, 63500, 3},
    {36200, 31800, 23},
    {38000, 47000, 43}
};

uint32_t g_create_position_wolf[4][2] =
{
    {0, 0},           // reserved
    {805300, 932300}, // 신수국
    {907700, 932300}, // 천조국
    {856500, 932300}, // 진노국
};

uint32_t g_create_position[4][3] =
{
    {0, 0, 0},
    {50200, 57900, 1},
    {52070, 64200, 21},
    {35700, 50400, 41}
};
