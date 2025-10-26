#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "questmanager.h"
#include "questevent.h"

namespace quest
{
void CancelTimerEvent(LPEVENT *ppEvent)
{
    quest_event_info *info = static_cast<quest_event_info *>((*ppEvent)->info);

    if (info)
    {
        delete[](info->name);
        info->name = nullptr;
    }

    event_cancel(ppEvent);
}

EVENTFUNC(quest_server_timer_event)
{
    quest_server_event_info *info = static_cast<quest_server_event_info *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("quest_server_timer_event> <Factor> Null pointer");
        return 0;
    }

    CQuestManager &q = CQuestManager::instance();

    if (!q.ServerTimer(info->npc_id, info->arg))
        return THECORE_SECS_TO_PASSES(1) / 2 + 1;

    if (0 == info->time_cycle) // 루프가 아니라면 종료 시킨다.
    {
        q.ClearServerTimerNotCancel(info->name, info->arg);
        delete[](info->name);
        info->name = nullptr;
    }

    return info->time_cycle;
}

EVENTFUNC(quest_timer_event)
{
    quest_event_info *info = static_cast<quest_event_info *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("quest_timer_event> <Factor> Null pointer");
        return 0;
    }

    CQuestManager &q = CQuestManager::instance();

    if (g_pCharManager->FindByPID(info->player_id))
    {
        if (!CQuestManager::instance().Timer(info->player_id, info->npc_id))
            return (THECORE_SECS_TO_PASSES(1) / 2 + 1);

        if (0 == info->time_cycle) // 루프가 아니라면 종료 시킨다.
            goto END_OF_TIMER_EVENT;
    }
    else
    {
    END_OF_TIMER_EVENT:
        PC *pPC = q.GetPC(info->player_id);

        if (pPC)
            pPC->RemoveTimerNotCancel(info->name);
        else
            SPDLOG_ERROR("quest::PC pointer null. player_id: %u", info->player_id);

        delete[](info->name);
        info->name = nullptr;
        return 0;
    }

    return info->time_cycle;
}

LPEVENT quest_create_server_timer_event(const char *name, double when, unsigned int timernpc, bool loop,
                                        unsigned int arg)
{
    const int nameCapacity = strlen(name) + 1;

    long ltime_cycle = (long)(rint(THECORE_SECS_TO_PASSES(when)));

    quest_server_event_info *info = AllocEventInfo<quest_server_event_info>();

    info->npc_id = timernpc;
    info->time_cycle = loop ? ltime_cycle : 0;
    info->arg = arg;
    info->name = new char[nameCapacity];

    if (info->name)
        strlcpy(info->name, name, nameCapacity);

    return event_create(quest_server_timer_event, info, ltime_cycle);
}

LPEVENT quest_create_timer_event(const char *name, unsigned int player_id, double when, unsigned int npc_id, bool loop)
{
    const int nameCapacity = strlen(name) + 1;

    long ltime_cycle = (int)(rint(THECORE_SECS_TO_PASSES(when)));

    quest_event_info *info = AllocEventInfo<quest_event_info>();

    info->player_id = player_id;
    info->npc_id = npc_id;
    info->name = new char[nameCapacity];

    if (info->name)
        strlcpy(info->name, name, nameCapacity);

    SPDLOG_INFO("QUEST timer name %s cycle %d pc %u npc %u loop? %d", name ? name : "<noname>", ltime_cycle, player_id,
                npc_id, loop ? 1 : 0);

    info->time_cycle = loop ? ltime_cycle : 0;
    return event_create(quest_timer_event, info, ltime_cycle);
}
} // namespace quest
