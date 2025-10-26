#include "utils.h"
#include "char.h"
#include "char_manager.h"
#include "motion.h"
#include <game/GamePacket.hpp>
#include "GBufferManager.h"
#include "wedding.h"
#include "config.h"
#include "main.h"
#include "PacketUtils.hpp"
#include "war_map.h"

#ifdef ENABLE_MESSENGER_BLOCK
#include "messenger_manager.h"
#endif

#define NEED_TARGET	(1 << 0)
#define NEED_PC		(1 << 1)
#define WOMAN_ONLY	(1 << 2)
#define OTHER_SEX_ONLY	(1 << 3)
#define SELF_DISARM	(1 << 4)
#define TARGET_DISARM	(1 << 5)
#define BOTH_DISARM	(SELF_DISARM | TARGET_DISARM)

struct emotion_type_s
{
    const char *command;
    const char *command_to_client;
    long flag;
    float extra_delay;
} emotion_types[] = {
    {"키스", "french_kiss", NEED_PC | BOTH_DISARM, 2.0f},
    {"뽀뽀", "kiss", NEED_PC | BOTH_DISARM, 1.5f},
    {"따귀", "slap", NEED_PC, 1.5f},
    {"박수", "clap", 0, 1.0f},
    {"와", "cheer1", 0, 1.0f},
    {"만세", "cheer2", 0, 1.0f},

    // DANCE
    {"댄스1", "dance1", 0, 1.0f},
    {"댄스2", "dance2", 0, 1.0f},
    {"댄스3", "dance3", 0, 1.0f},
    {"댄스4", "dance4", 0, 1.0f},
    {"댄스5", "dance5", 0, 1.0f},
    {"댄스6", "dance6", 0, 1.0f},
    {"댄스7", "dance7", 0, 1.0f},

    // END_OF_DANCE
    {"축하", "congratulation", 0, 1.0f},
    {"용서", "forgive", 0, 1.0f},
    {"화남", "angry", 0, 1.0f},
    {"유혹", "attractive", 0, 1.0f},
    {"슬픔", "sad", 0, 1.0f},
    {"브끄", "shy", 0, 1.0f},
    {"응원", "cheerup", 0, 1.0f},
    {"질투", "banter", 0, 1.0f},
    {"기쁨", "joy", 0, 1.0f},
    {"throw_money", "throw_money", 0, 1.0f},
    {"doze", "doze", 0, 1.0f},
    {"exercise", "exercise", 0, 1.0f},
    {"selfie", "selfie", 0, 1.0f},
    {"pushup", "pushup", 0, 1.0f},

    {"\n", "\n", 0, 0.0f},
    /*
    //{ "키스",		NEED_PC | OTHER_SEX_ONLY | BOTH_DISARM,		MOTION_ACTION_FRENCH_KISS,	 1.0f },
    { "뽀뽀",		NEED_PC | OTHER_SEX_ONLY | BOTH_DISARM,		MOTION_ACTION_KISS,		 1.0f },
    { "껴안기",		NEED_PC | OTHER_SEX_ONLY | BOTH_DISARM,		MOTION_ACTION_SHORT_HUG,	 1.0f },
    { "포옹",		NEED_PC | OTHER_SEX_ONLY | BOTH_DISARM,		MOTION_ACTION_LONG_HUG,		 1.0f },
    { "어깨동무",	NEED_PC | SELF_DISARM,				MOTION_ACTION_PUT_ARMS_SHOULDER, 0.0f },
    { "팔짱",		NEED_PC	| WOMAN_ONLY | SELF_DISARM,		MOTION_ACTION_FOLD_ARM,		 0.0f },
    { "따귀",		NEED_PC | SELF_DISARM,				MOTION_ACTION_SLAP,		 1.5f },

    { "휘파람",		0,						MOTION_ACTION_CHEER_01,		 0.0f },
    { "만세",		0,						MOTION_ACTION_CHEER_02,		 0.0f },
    { "박수",		0,						MOTION_ACTION_CHEER_03,		 0.0f },

    { "호호",		0,						MOTION_ACTION_LAUGH_01,		 0.0f },
    { "킥킥",		0,						MOTION_ACTION_LAUGH_02,		 0.0f },
    { "우하하",		0,						MOTION_ACTION_LAUGH_03,		 0.0f },

    { "엉엉",		0,						MOTION_ACTION_CRY_01,		 0.0f },
    { "흑흑",		0,						MOTION_ACTION_CRY_02,		 0.0f },

    { "인사",		0,						MOTION_ACTION_GREETING_01,	0.0f },
    { "바이",		0,						MOTION_ACTION_GREETING_02,	0.0f },
    { "정중인사",	0,						MOTION_ACTION_GREETING_03,	0.0f },

    { "비난",		0,						MOTION_ACTION_INSULT_01,	0.0f },
    { "모욕",		SELF_DISARM,					MOTION_ACTION_INSULT_02,	0.0f },
    { "우웩",		0,						MOTION_ACTION_INSULT_03,	0.0f },

    { "갸우뚱",		0,						MOTION_ACTION_ETC_01,		0.0f },
    { "끄덕끄덕",	0,						MOTION_ACTION_ETC_02,		0.0f },
    { "도리도리",	0,						MOTION_ACTION_ETC_03,		0.0f },
    { "긁적긁적",	0,						MOTION_ACTION_ETC_04,		0.0f },
    { "퉤",		0,						MOTION_ACTION_ETC_05,		0.0f },
    { "뿡",		0,						MOTION_ACTION_ETC_06,		0.0f },
     */
};

std::set<std::pair<uint32_t, uint32_t>> s_emotion_set;

ACMD(do_emotion_allow)
{
    if (ch->GetArena())
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "대련장에서 사용하실 수 없습니다.");
        return;
    }

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    uint32_t val = 0;
    str_to_number(val, arg1);

#ifdef ENABLE_MESSENGER_BLOCK
	CHARACTER* tch = g_pCharManager->Find(val);
	if (!tch)
		return;

	if (MessengerManager::instance().IsBlocked_Target(ch->GetName(), tch->GetName()))
	{
		SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You blocked %s", tch->GetName());
		return;
	}
	if (MessengerManager::instance().IsBlocked_Me(ch->GetName(), tch->GetName()))
	{
		SendI18nChatPacket(ch, CHAT_TYPE_INFO, "%s blocked you", tch->GetName());
		return;
	}
#endif

    s_emotion_set.insert(std::make_pair(ch->GetVID(), val));
}

bool CHARACTER_CanEmotion(CHARACTER &rch)
{
    if (gConfig.disbaleEmotionMask)
        return true;

    // 결혼식 맵에서는 사용할 수 있다.
    if (marriage::WeddingManager::instance().IsWeddingMap(rch.GetMapIndex()))
        return true;

    // 열정의 가면 착용시 사용할 수 있다.
    if (rch.IsEquipUniqueItem(UNIQUE_ITEM_EMOTION_MASK))
        return true;

    if (rch.IsEquipUniqueItem(UNIQUE_ITEM_EMOTION_MASK2))
        return true;

    if (CWarMapManager::instance().IsWarMap(rch.GetMapIndex()))
        return false;

    return false;
}

ACMD(do_emotion)
{
    int i;
    {
        if (ch->IsRiding())
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "말을 탄 상태에서 감정표현을 할 수 없습니다.");
            return;
        }

        if (ch->IsPolymorphed())
            return;
    }

    for (i = 0; *emotion_types[i].command != '\n'; ++i)
    {
        if (!strcmp(cmd_info[cmd].command, emotion_types[i].command))
            break;

        if (!strcmp(cmd_info[cmd].command, emotion_types[i].command_to_client))
            break;
    }

    if (*emotion_types[i].command == '\n')
    {
        SPDLOG_ERROR("cannot find emotion");
        return;
    }

    if (!CHARACTER_CanEmotion(*ch))
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "열정의 가면을 착용시에만 할 수 있습니다.");
        return;
    }

    if (!ch->IsNextEmotionPulse())
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You need to wait some time for the next emotion.");
        return;
    }

    if (IS_SET(emotion_types[i].flag, WOMAN_ONLY) && SEX_MALE == GetSexByRace(ch->GetRaceNum()))
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "여자만 할 수 있습니다.");
        return;
    }

#ifdef ENABLE_MONEY_THROW_WITH_COINS
	if(!strcmp(emotion_types[i].command, "throw_money"))
	{
		auto chars = ch->FindCharactersInRange(2000);
		if(chars.size() < 1)
		{
			SendI18nChatPacket(ch, CHAT_TYPE_INFO, "There is nobody to throw money at in range.");
			return;
		}

		if(ch->GetCash("cash") - 1000 < 0)
		{
			SendI18nChatPacket(ch, CHAT_TYPE_INFO, "You dont have enough cash to throw.");
			return;
		}

		auto coinsFee = 100;
		auto coinsAmountPerChar = (1000 - coinsFee) / chars.size();

		for (auto tch : chars)
			tch->ChangeCash(coinsAmountPerChar, "cash", true);

		ch->ChangeCash(1000, "cash", false);

	}
#endif

    char arg1[256];
    one_argument(argument, arg1, sizeof(arg1));

    CHARACTER *victim = nullptr;

    if (*arg1)
        victim = ch->FindCharacterInView(arg1, IS_SET(emotion_types[i].flag, NEED_PC));

    if (IS_SET(emotion_types[i].flag, NEED_TARGET | NEED_PC))
    {
        if (!victim)
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "그런 사람이 없습니다.");
            return;
        }
    }

    if (victim)
    {
        if (!victim->IsPC() || victim == ch)
            return;

        if (victim->IsPolymorphed())
            return;

        if (victim->IsRiding())
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "말을 탄 상대와 감정표현을 할 수 없습니다.");
            return;
        }

        long distance = DISTANCE_APPROX(ch->GetX() - victim->GetX(), ch->GetY() - victim->GetY());

        if (distance < 10)
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "너무 가까이 있습니다.");
            return;
        }

        if (distance > 500)
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "너무 멀리 있습니다");
            return;
        }

        if (IS_SET(emotion_types[i].flag, OTHER_SEX_ONLY))
        {
            if (GetSexByRace(ch->GetRaceNum()) == GetSexByRace(victim->GetRaceNum()) && !gConfig.
                enableSameGenderEmotions)
            {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO, "이성간에만 할 수 있습니다.");
                return;
            }
        }

        if (IS_SET(emotion_types[i].flag, NEED_PC))
        {
            if (s_emotion_set.find(std::make_pair(victim->GetVID(), ch->GetVID())) == s_emotion_set.end())
            {
                if (true == marriage::CManager::instance().IsMarried(ch->GetPlayerID()))
                {
                    const marriage::TMarriage *marriageInfo = marriage::CManager::instance().Get(ch->GetPlayerID());

                    const uint32_t other = marriageInfo->GetOther(ch->GetPlayerID());

                    if (0 == other || other != victim->GetPlayerID())
                    {
                        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "이 행동은 상호동의 하에 가능 합니다.");
                        return;
                    }
                }
                else
                {
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "이 행동은 상호동의 하에 가능 합니다.");
                    return;
                }
            }

            s_emotion_set.insert(std::make_pair(ch->GetVID(), victim->GetVID()));
        }
    }

    std::string chatbuf = fmt::format("{} {} {}", emotion_types[i].command_to_client,
                                      (uint32_t)ch->GetVID(), victim ? (uint32_t)victim->GetVID() : 0);

    TPacketGCChat pack_chat;
    pack_chat.type = CHAT_TYPE_COMMAND;
    pack_chat.id = 0;
    pack_chat.message = chatbuf;

    PacketAround(ch->GetViewMap(), ch, HEADER_GC_CHAT, pack_chat);

    ch->UpdateEmotionPulse();

    if (victim)
        SPDLOG_TRACE("ACTION: %s TO %s", emotion_types[i].command, victim->GetName());
    else
        SPDLOG_TRACE("ACTION: %s", emotion_types[i].command);
}
