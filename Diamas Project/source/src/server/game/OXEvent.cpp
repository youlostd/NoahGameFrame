#include "constants.h"
#include "config.h"
#include "questmanager.h"
#include "start_position.h"
#include <game/GamePacket.hpp>
#include "GBufferManager.h"
#include "log.h"
#include "char.h"
#include "char_manager.h"
#include "OXEvent.h"
#include "desc.h"
#include "ChatUtil.hpp"
#include "item_manager.h"
#include "PacketUtils.hpp"

bool COXEventManager::Initialize()
{
    m_timedEvent = nullptr;
    m_map_char.clear();
    m_map_attender.clear();
    m_vec_quiz.clear();
    m_list_iplist.clear();

    SetStatus(OXEVENT_FINISH);
    m_eAutoEvent = nullptr;
    return true;
}

void COXEventManager::Destroy()
{
    CloseEvent();

    m_map_char.clear();
    m_map_attender.clear();
    m_vec_quiz.clear();
    m_list_iplist.clear();

    SetStatus(OXEVENT_FINISH);
}

OXEventStatus COXEventManager::GetStatus()
{
    uint8_t ret = quest::CQuestManager::instance().GetEventFlag("oxevent_status");

    switch (ret)
    {
    case 0:
        return OXEVENT_FINISH;

    case 1:
        return OXEVENT_OPEN;

    case 2:
        return OXEVENT_CLOSE;

    case 3:
        return OXEVENT_QUIZ;

    default:
        break;
    }

    return OXEVENT_ERR;
}

void COXEventManager::SetStatus(OXEventStatus status)
{
    uint8_t val = 0;

    switch (status)
    {
    case OXEVENT_OPEN:
        val = 1;
        break;

    case OXEVENT_CLOSE:
        val = 2;
        break;

    case OXEVENT_QUIZ:
        val = 3;
        break;

    case OXEVENT_FINISH:
    case OXEVENT_ERR:
    default:
        val = 0;
        break;
    }
    quest::CQuestManager::instance().RequestSetEventFlag("oxevent_status", val);
}

bool COXEventManager::Enter(CHARACTER *pkChar)
{
    if (GetStatus() == OXEVENT_FINISH)
    {
        SPDLOG_INFO("OXEVENT : map finished. but char enter. %s", pkChar->GetName());
        return false;
    }

    PIXEL_POSITION pos = pkChar->GetXYZ();

    if (pos.x == 26100 && pos.y == 24600)
        return EnterAttender(pkChar);

    if (pos.x == 25900 && pos.y == 28900)
        return EnterAudience(pkChar);

    SPDLOG_INFO("OXEVENT : wrong pos enter %d %d", pos.x, pos.y);
    return false;
}

bool COXEventManager::CheckIpAddress(CHARACTER *ch)
{
    for (auto &it : m_map_attender)
    {
        CHARACTER *tch = g_pCharManager->FindByPID(it.second);
        if (!tch || !tch->GetDesc())
            continue;

        if (!strcmp(ch->GetDesc()->GetHWIDHash().c_str(), tch->GetDesc()->GetHWIDHash().c_str()) && ch->GetMapIndex() ==
            tch->GetMapIndex())
        {
            LogManager::Instance().HackLog("MULTI_IP_OX", ch);
            ch->GoHome();
            return false;
        }
    }

    return true;
}

void COXEventManager::RemoveFromAttenderList(uint32_t dwPID) { m_map_attender.erase(dwPID); }

bool COXEventManager::EnterAttender(CHARACTER *pkChar)
{
    uint32_t pid = pkChar->GetPlayerID();

    if (CheckIpAddress(pkChar))
    {
        pkChar->SetPolymorph(101);
        m_map_char.insert(std::make_pair(pid, pid));
        m_map_attender.insert(std::make_pair(pid, pid));
        return true;
    }
    return false;
}

bool COXEventManager::EnterAudience(CHARACTER *pkChar)
{
    uint32_t pid = pkChar->GetPlayerID();
    pkChar->SetObserverMode(true);
    pkChar->SetPolymorph(101);
    m_map_char.emplace(pid, pid);

    return true;
}

bool COXEventManager::AddQuiz(uint8_t level, const char *pszQuestion, bool answer)
{
    if (m_vec_quiz.size() < (size_t)level + 1)
        m_vec_quiz.resize(level + 1);

    struct tag_Quiz tmpQuiz{};
    tmpQuiz.level = level;
    strlcpy(tmpQuiz.Quiz, pszQuestion, sizeof(tmpQuiz.Quiz));
    tmpQuiz.answer = answer;

    m_vec_quiz[level].push_back(tmpQuiz);
    return true;
}

bool COXEventManager::ShowQuizList(CHARACTER *pkChar)
{
    int c = 0;

    for (size_t i = 0; i < m_vec_quiz.size(); ++i)
    {
        for (size_t j = 0; j < m_vec_quiz[i].size(); ++j, ++c)
        {
            pkChar->ChatPacket(CHAT_TYPE_INFO, "%d %s %s", m_vec_quiz[i][j].level, m_vec_quiz[i][j].Quiz,
                               m_vec_quiz[i][j].answer ? LC_TEXT_LC("참", GetLocale(pkChar)) : LC_TEXT_LC("거짓", GetLocale(pkChar)));
        }
    }

    SendI18nChatPacket(pkChar, CHAT_TYPE_INFO, "총 퀴즈 수: %d", c);
    return true;
}

void COXEventManager::ClearQuiz()
{
    for (unsigned int i = 0; i < m_vec_quiz.size(); ++i) { m_vec_quiz[i].clear(); }

    m_vec_quiz.clear();
}

void COXEventManager::ClearAttenders() { m_map_attender.clear(); }

EVENTINFO(OXEventInfoData)
{
    bool answer;

    OXEventInfoData()
        : answer(false)
    {
    }
};

EVENTFUNC(oxevent_timer)
{
    static uint8_t flag = 0;
    OXEventInfoData *info = static_cast<OXEventInfoData *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("oxevent_timer> <Factor> Null pointer");
        return 0;
    }

    switch (flag)
    {
    case 0:
        SendI18nNoticeMap("10초뒤 판정하겠습니다.", OXEVENT_MAP_INDEX, true);
        flag++;
        return THECORE_SECS_TO_PASSES(10);

    case 1:
        SendI18nNoticeMap("정답은", OXEVENT_MAP_INDEX, true);

        if (info->answer == true)
        {
            COXEventManager::instance().CheckAnswer(true);
            SendI18nNoticeMap("O 입니다", OXEVENT_MAP_INDEX, true);
        }
        else
        {
            COXEventManager::instance().CheckAnswer(false);
            SendI18nNoticeMap("X 입니다", OXEVENT_MAP_INDEX, true);
        }

        SendI18nNoticeMap("5초 뒤 틀리신 분들을 바깥으로 이동 시키겠습니다.", OXEVENT_MAP_INDEX, true);

        flag++;
        return THECORE_SECS_TO_PASSES(5);

    case 2:
        COXEventManager::instance().WarpToAudience();
        COXEventManager::instance().SetStatus(OXEVENT_CLOSE);
        SendI18nNoticeMap("다음 문제 준비해주세요.", OXEVENT_MAP_INDEX, true);
        flag = 0;
        break;
    }
    return 0;
}

bool COXEventManager::Quiz(unsigned char level, int timelimit)
{
    if (m_vec_quiz.empty())
        return false;
    if (level > m_vec_quiz.size())
        level = m_vec_quiz.size() - 1;
    if (m_vec_quiz[level].empty())
        return false;

    if (timelimit < 0)
        timelimit = 30;

    int idx = Random::get<int>(0, m_vec_quiz[level].size() - 1);

    SendI18nNoticeMap("문제 입니다.", OXEVENT_MAP_INDEX, true);
    SendBigOXNoticeSpecial(m_vec_quiz[level][idx].Quiz);
    SendI18nNoticeMap("맞으면 O, 틀리면 X로 이동해주세요", OXEVENT_MAP_INDEX, true);

    if (m_timedEvent != nullptr) { event_cancel(&m_timedEvent); }

    auto *answer = AllocEventInfo<OXEventInfoData>();

    answer->answer = m_vec_quiz[level][idx].answer;

    timelimit -= 15;
    m_timedEvent = event_create(oxevent_timer, answer, THECORE_SECS_TO_PASSES(timelimit));

    SetStatus(OXEVENT_QUIZ);

    m_vec_quiz[level].erase(m_vec_quiz[level].begin() + idx);
    return true;
}

bool COXEventManager::CheckAnswer(bool answer)
{
    if (m_map_attender.empty())
        return true;

    m_map_miss.clear();

    int rect[4];
    if (!answer)
    {
        rect[0] = 22200;
        rect[1] = 22900;
        rect[2] = 25900;
        rect[3] = 26400;
    }
    else
    {
        rect[0] = 26200;
        rect[1] = 22900;
        rect[2] = 29900;
        rect[3] = 26400;
    }

    auto iter = m_map_attender.begin();
    for (; iter != m_map_attender.end();)
    {
        auto pkChar = g_pCharManager->FindByPID(iter->second);
        if (pkChar != nullptr)
        {
            PIXEL_POSITION pos = pkChar->GetXYZ();

            if (pos.x < rect[0] || pos.x > rect[2] || pos.y < rect[1] || pos.y > rect[3])
            {
                auto iter_tmp = iter;
                iter++;
                m_map_attender.erase(iter_tmp);
                m_map_miss.emplace(pkChar->GetPlayerID(), pkChar->GetPlayerID());
            }
            else
            {
                SendI18nChatPacket(pkChar, CHAT_TYPE_INFO, "정답입니다!");
                char chatbuf[256];
                int len = std::snprintf(chatbuf, sizeof(chatbuf),
                                        "%s %u %u", Random::get(0, 1) == 1 ? "cheer1" : "cheer2",
                                        (uint32_t)pkChar->GetVID(), 0);

                if (len < 0 || len >= (int)sizeof(chatbuf))
                    len = sizeof(chatbuf) - 1;

                ++len;

                TPacketGCChat pack_chat = {};
                pack_chat.type = CHAT_TYPE_COMMAND;
                pack_chat.id = 0;
                pack_chat.message = chatbuf;

                TEMP_BUFFER buf;
                buf.write(&pack_chat, sizeof(TPacketGCChat));
                buf.write(chatbuf, len);

                PacketAround(pkChar->GetViewMap(), pkChar, HEADER_GC_CHAT, pack_chat);

                ++iter;
            }
        }
        else
        {
            auto err = m_map_char.find(iter->first);
            if (err != m_map_char.end())
                m_map_char.erase(err);

            auto err2 = m_map_miss.find(iter->first);
            if (err2 != m_map_miss.end())
                m_map_miss.erase(err2);

            auto iter_tmp = iter;
            ++iter;
            m_map_attender.erase(iter_tmp);
        }
    }
    return true;
}

void COXEventManager::WarpToAudience()
{
    if (m_map_miss.size() <= 0)
        return;

    auto iter = m_map_miss.begin();
    for (; iter != m_map_miss.end(); ++iter)
    {
        auto pkChar = g_pCharManager->FindByPID(iter->second);

        if (pkChar != nullptr)
        {
            switch (Random::get(0, 3))
            {
            case 0:
                pkChar->Show(OXEVENT_MAP_INDEX, 25900, 28900);
                break;
            case 1:
                pkChar->Show(OXEVENT_MAP_INDEX, 20500, 28100);
                break;
            case 2:
                pkChar->Show(OXEVENT_MAP_INDEX, 26200, 20500);
                break;
            case 3:
                pkChar->Show(OXEVENT_MAP_INDEX, 32100, 28100);
                break;
            default:
                pkChar->Show(OXEVENT_MAP_INDEX, 25900, 28900);
                break;
            }
            pkChar->SetPolymorph(101);
        }
    }

    m_map_miss.clear();
}

bool COXEventManager::CloseEvent()
{
    if (m_timedEvent != nullptr) { event_cancel(&m_timedEvent); }

    auto iter = m_map_char.begin();
    for (; iter != m_map_char.end(); ++iter)
    {
        auto pkChar = g_pCharManager->FindByPID(iter->second);

        if (pkChar != nullptr)
        {
            pkChar->SetPolymorph(0);
            pkChar->SetObserverMode(false);
            pkChar->GoHome();
        }
    }

    m_map_char.clear();

    return true;
}

bool COXEventManager::LogWinner()
{
    auto iter = m_map_attender.begin();

    for (; iter != m_map_attender.end(); ++iter)
    {
        CHARACTER *pkChar = g_pCharManager->FindByPID(iter->second);

        if (pkChar)
            LogManager::instance().CharLog(pkChar, 0, "OXEVENT", "LastManStanding");
    }

    return true;
}

CHARACTER *COXEventManager::GetWinner()
{
    auto iter = m_map_attender.begin();

    for (; iter != m_map_attender.end(); ++iter)
    {
        CHARACTER *pkChar = g_pCharManager->FindByPID(iter->second);

        if (pkChar)
            return pkChar;
    }

    return nullptr;
}

bool COXEventManager::GiveItemToAttender(uint32_t dwItemVnum, uint8_t count)
{
    auto iter = m_map_attender.begin();

    for (; iter != m_map_attender.end(); ++iter)
    {
        CHARACTER *pkChar = g_pCharManager->FindByPID(iter->second);

        if (pkChar)
        {
            quest::CQuestManager::instance().BeginOtherPCBlock(pkChar->GetPlayerID());
            pkChar->AutoGiveItem(dwItemVnum, count);
            LogManager::instance().ItemLog(pkChar->GetPlayerID(), 0, count, dwItemVnum, "OXEVENT_REWARD", "",
                                           pkChar->GetDesc()->GetHostName().c_str(), dwItemVnum);
            quest::CQuestManager::instance().EndOtherPCBlock();
        }
    }

    return true;
}

enum EOXEventStatuses
{
    OXEVENT_AUTO_REGISTRATION,
    OXEVENT_AUTO_MIN_ATTENDERS,
    OXEVENT_AUTO_QUIZ,
    OXEVENT_AUTO_CHECK_ATTENDERS,
    OXEVENT_AUTO_REWARD_OR_FINISH,
    OXEVENT_AUTO_FINISH,
    OXEVENT_AUTO_MAX_NUM
};

EVENTINFO(oxevent_automatic_event_info)
{
    int state;
    int retryTimes;
    int iRegTime;
    uint32_t dwMinAttenders;
    uint32_t dwRemainAttender;
    int iMaxQuestions;
    int iPrizeVnum;
    uint32_t iQuestions;
    uint8_t bPrizeCount;
    COXEventManager *pOXEvent;
};

EVENTFUNC(oxevent_automatic_event_func)
{
    oxevent_automatic_event_info *info = dynamic_cast<oxevent_automatic_event_info *>(event->info);

    if (info == nullptr)
    {
        SPDLOG_ERROR("oxevent_timer> <Factor> Null pointer");
        return 0;
    }
    COXEventManager *pOXMgr = info->pOXEvent;
    if (!pOXMgr)
    {
        SPDLOG_ERROR("oxmanager is null!!");
        return 0;
    }
    int iStatus = info->state;

    switch (iStatus)
    {
    case OXEVENT_AUTO_REGISTRATION: {
        info->state = OXEVENT_AUTO_MIN_ATTENDERS;
        BroadcastNoticeSpecial(fmt::format("OX: Half the registration time has passed time left: %d.#{}",
                                           info->iRegTime / 2).c_str());
        return THECORE_SECS_TO_PASSES((info->iRegTime * 60) / 2);
        // status OXEVENT_AUTO_HALF_REGISTRATION
    }
    case OXEVENT_AUTO_MIN_ATTENDERS: {
        if (pOXMgr->GetAttenderCount() >= info->dwMinAttenders)
        {
            // broadcast map notice
            BroadcastNoticeSpecial("OX: Registration has been closed.");
            info->state = OXEVENT_AUTO_QUIZ;
            quest::CQuestManager::instance().SetEventFlag("oxevent_chat", 0);
            pOXMgr->SetStatus(OXEVENT_CLOSE);
            return THECORE_SECS_TO_PASSES(5);
        }
        if (info->retryTimes >= 4)
        {
            BroadcastNoticeSpecial("OX: Event ended because of the minimum required participants hasn't been reached.");
            info->state = OXEVENT_AUTO_FINISH;
            pOXMgr->SetStatus(OXEVENT_FINISH);
            quest::CQuestManager::instance().SetEventFlag("oxevent_chat", 0);
            return THECORE_SECS_TO_PASSES(5);
        }
        ++info->retryTimes;
        BroadcastNoticeSpecial(
            "OX: The participants number is lower than the minimum limit. One minute added to registration phase.");
        return THECORE_SECS_TO_PASSES(60);
    }
    case OXEVENT_AUTO_QUIZ: {
        info->state = OXEVENT_AUTO_CHECK_ATTENDERS;
        ++info->iQuestions;
        SendBigOXNoticeSpecial(fmt::format("OX: Participants: %d#{}", pOXMgr->GetAttenderCount()).c_str());
        SendBigOXNoticeSpecial(fmt::format("OX: Question: %d/%d#{};{}", info->iQuestions, info->iMaxQuestions).c_str());
        pOXMgr->Quiz(1, 30);
        return THECORE_SECS_TO_PASSES(40);
        // OXEVENT_AUTO_CHECK_ATTENDERS
    }
    case OXEVENT_AUTO_CHECK_ATTENDERS: {
        if (pOXMgr->GetAttenderCount() == info->dwRemainAttender || pOXMgr->GetAttenderCount() == 0)
        {
            info->state = OXEVENT_AUTO_REWARD_OR_FINISH;
            return THECORE_SECS_TO_PASSES(1);
        }
        info->state = OXEVENT_AUTO_QUIZ;
        return THECORE_SECS_TO_PASSES(2);
        // status OXEVENT_AUTO_QUIZ / OXEVENT_AUTO_REWARD_AND_CLOSE
    }
    case OXEVENT_AUTO_REWARD_OR_FINISH: {
        // Get Winner and award him
        char cBuf[200];
        if (pOXMgr->GetAttenderCount() > 0)
        {
            CHARACTER *pWinner = pOXMgr->GetWinner();
            pOXMgr->GiveItemToAttender(info->iPrizeVnum, info->bPrizeCount);
            BroadcastNoticeSpecial(
                fmt::format("OX: %s has won the event. Congratulations!#{}", pWinner->GetName()).c_str());
        }
        else
        {
            SendBigOXNoticeSpecial("OX: All the participants have lost everyone will be teleported to their village.");
        }
        info->state = OXEVENT_AUTO_FINISH;
        pOXMgr->SetStatus(OXEVENT_FINISH);
        return THECORE_SECS_TO_PASSES(5);

        // function close then set timer to null.
    }
    case OXEVENT_AUTO_FINISH: {
        pOXMgr->CloseEvent();
        return 0;
    }
    }
    return 0;
}

void COXEventManager::StartAutomaticOX()
{
    if (m_eAutoEvent)
        return;

    if (GetStatus() != OXEVENT_FINISH)
        return;
    int iRegMinutes = 10;
    int iMinAttenders = 10;
    int iRemainAttenders = 1;
    int iQuestions = 100;
    int iPrizeVnum = 80014;
    int iCount = 1;

    if (quest::CQuestManager::instance().GetEventFlag("oxevent_register_time") > 4)
        iRegMinutes = quest::CQuestManager::instance().GetEventFlag("oxevent_register_time");
    if (quest::CQuestManager::instance().GetEventFlag("oxevent_min_attenders") > 5)
        iMinAttenders = quest::CQuestManager::instance().GetEventFlag("oxevent_min_attenders");
    if (quest::CQuestManager::instance().GetEventFlag("oxevent_remain_attender") > 0)
        iRemainAttenders = quest::CQuestManager::instance().GetEventFlag("oxevent_remain_attender");
    if (quest::CQuestManager::instance().GetEventFlag("oxevent_max_question") > 20)
        iQuestions = quest::CQuestManager::instance().GetEventFlag("oxevent_max_question");
    if (quest::CQuestManager::instance().GetEventFlag("oxevent_reward_vnum") > 0)
    {
        auto item = ITEM_MANAGER::instance().GetTable(
            quest::CQuestManager::instance().GetEventFlag("oxevent_reward_vnum"));
        if (item)
        {
            iPrizeVnum = quest::CQuestManager::instance().GetEventFlag("oxevent_reward_vnum");
            if (quest::CQuestManager::instance().GetEventFlag("oxevent_reward_count") > 0)
                iCount = quest::CQuestManager::instance().GetEventFlag("oxevent_reward_count");
        }
    }
    auto item = ITEM_MANAGER::instance().GetTable(iPrizeVnum);
    ClearQuiz();
    //ClearAttenders();
    //quest::CQuestManager::instance().SetEventFlag("oxevent_attenders", 0);
    char script[256];
    std::snprintf(script, sizeof(script), "%s/oxquiz.lua", GetLocaleService().GetDefaultLocale().path.c_str());
    int result = lua_dofile(quest::CQuestManager::instance().GetLuaState(), script);
    if (result != 0)
        return;
    SetStatus(OXEVENT_OPEN);
    BroadcastNotice("OX event has started talk to Uriel to paticipate.");

    if (item)
    {
        BroadcastNoticeSpecial(fmt::format("OX: The reward is %sx%s.#{};{}", TextTag::itemname(item->dwVnum), iCount).c_str());
    }

    BroadcastNoticeSpecial(fmt::format("OX: %s minutes left until the end of the registration phase.#{}", iRegMinutes).c_str());

    quest::CQuestManager::instance().SetEventFlag("oxevent_chat", 1);


    oxevent_automatic_event_info *info = AllocEventInfo<oxevent_automatic_event_info>();
    info->state = OXEVENT_AUTO_REGISTRATION;
    info->iRegTime = iRegMinutes;
    info->dwMinAttenders = iMinAttenders;
    info->dwRemainAttender = iRemainAttenders;
    info->iMaxQuestions = iQuestions;
    info->iPrizeVnum = iPrizeVnum;
    info->bPrizeCount = iCount;
    info->retryTimes = 0;
    info->iQuestions = 0;
    info->pOXEvent = this;
    m_eAutoEvent = event_create(oxevent_automatic_event_func, info, THECORE_SECS_TO_PASSES((iRegMinutes * 60) / 2));
}
