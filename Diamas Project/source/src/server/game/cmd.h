#ifndef METIN2_SERVER_GAME_CMD_H
#define METIN2_SERVER_GAME_CMD_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CHARACTER;

#define ACMD(name)  void (name)(CHARACTER* ch, const char *argument, int cmd, int subcmd)
#define CMD_NAME(name) cmd_info[cmd].command
#define CMDTOKEN(x, y) if (!strcasecmp(x, y))

#define COMMANDINFO(msg) ch->ChatPacket(CHAT_TYPE_INFO, msg);

#define CHECK_COMMAND(Parser, params, msg)                                                                             \
    SITokenParser Parser;                                                                                              \
    Parser.ReadLine(argument);                                                                                         \
    if (params > 0 && Parser.GetTokenNum() < params)                                                                   \
    {                                                                                                                  \
        ch->ChatPacket(CHAT_TYPE_INFO, msg);                                                                           \
        return;                                                                                                        \
    }

#define SEND_HELP(msg)    ch->ChatPacket(CHAT_TYPE_INFO, msg);

#define CHECK_ONLY_COMMAND(Parser, params, msg) \
	if (Parser.GetTokenNum() < params)\
{\
	ch->ChatPacket(CHAT_TYPE_INFO, msg);\
	return false;\
}

struct command_info
{
    const char *command;
    void (*command_pointer)(CHARACTER *ch, const char *argument, int cmd, int subcmd);
    int subcmd;
    int minimum_position;
    int gm_level;
    const char *desc;
};

struct GotoInfo
{
    std::string st_name;

    uint8_t empire;
    int mapIndex;
    uint32_t x, y;

    GotoInfo()
    {
        st_name.clear();
        empire = 0;
        mapIndex = 0;

        x = 0;
        y = 0;
    }

    GotoInfo(const GotoInfo &c_src) { __copy__(c_src); }

    void operator=(const GotoInfo &c_src) { __copy__(c_src); }

    void __copy__(const GotoInfo &c_src)
    {
        st_name = c_src.st_name;
        empire = c_src.empire;
        mapIndex = c_src.mapIndex;

        x = c_src.x;
        y = c_src.y;
    }
};

extern struct command_info cmd_info[];

extern void interpret_command(CHARACTER *ch, const char *argument, size_t len);
extern void interpreter_set_privilege(const char *cmd, int lvl);
extern void interpreter_load_config(const char *filename);

enum SCMD_ACTION
{
    SCMD_SLAP,
    SCMD_KISS,
    SCMD_FRENCH_KISS,
    SCMD_HUG,
    SCMD_LONG_HUG,
    SCMD_SHOLDER,
    SCMD_FOLD_ARM
};

enum SCMD_CMD
{
    SCMD_LOGOUT,
    SCMD_QUIT,
    SCMD_PHASE_SELECT,
    SCMD_SHUTDOWN,
    SCMD_CHANGE_CHANNEL,
};

enum SCMD_RESTART
{
    SCMD_RESTART_TOWN,
    SCMD_RESTART_HERE,
#if defined(WJ_COMBAT_ZONE)
	SCMD_RESTART_COMBAT_ZONE,
#endif
    SCMD_REVIVE,
};

enum SCMD_XMAS
{
    SCMD_XMAS_BOOM,
    SCMD_XMAS_SNOW,
    SCMD_XMAS_SANTA,
};

extern void Shutdown(int iSec);
extern void SendWhisperAll(const char *c_pszBuf);
extern void SendBigNotice(const char *c_pszBuf, int mapIndex);
extern void SendLog(const char *c_pszBuf);         // 운영자에게만 공지
extern void BroadcastNotice(const char *c_pszBuf); // 전 서버에 공지

extern void BroadcastWhisperAll(const char *c_pszBuf);

// LUA_ADD_BGM_INFO
void CHARACTER_SetBGMVolumeEnable();
void CHARACTER_AddBGMInfo(unsigned mapIndex, const char *name, float vol);
// END_OF_LUA_ADD_BGM_INFO

// LUA_ADD_GOTO_INFO
extern void CHARACTER_AddGotoInfo(const std::string &c_st_name, uint8_t empire, int mapIndex, uint32_t x, uint32_t y);
// END_OF_LUA_ADD_GOTO_INFO
#endif /* METIN2_SERVER_GAME_CMD_H */
