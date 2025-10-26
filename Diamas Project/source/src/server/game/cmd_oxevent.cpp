#include "utils.h"
#include "char.h"
#include "OXEvent.h"
#include "questmanager.h"
#include "questlua.h"
#include "config.h"
#include "locale_service.h"
#include "cmd.h"
#include "main.h"

ACMD(do_oxevent_show_quiz)
{
    ch->ChatPacket(CHAT_TYPE_INFO, "===== OX QUIZ LIST =====");
    COXEventManager::instance().ShowQuizList(ch);
    ch->ChatPacket(CHAT_TYPE_INFO, "===== OX QUIZ LIST END =====");
}

ACMD(do_oxevent_log)
{
    if (COXEventManager::instance().LogWinner() == false)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "OX이벤트의 나머지 인원을 기록하였습니다.");
    }
    else { SendI18nChatPacket(ch, CHAT_TYPE_INFO, "OX이벤트의 나머지 인원 기록을 실패했습니다."); }
}

ACMD(do_oxevent_get_attender)
{
    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "현재 남은 참가자수 : %d", COXEventManager::instance().GetAttenderCount());
}
