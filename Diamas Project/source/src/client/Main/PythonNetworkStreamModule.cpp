#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonApplication.h"
#include <pybind11/stl.h>
static std::string gs_stServerInfo;
std::list<std::string> g_kList_strCommand;

void netStreamSetServerInfo(const std::string &serverInfo)
{
    gs_stServerInfo = serverInfo;
}

py::bytes netStreamGetServerInfo()
{
    return gs_stServerInfo;
}

void netPreserveServerCommand(std::string szLine)
{
    g_kList_strCommand.emplace_back(szLine);
}

std::string netGetPreservedServerCommand()
{
    if (g_kList_strCommand.empty())
        return "";

    std::string strCommand = g_kList_strCommand.front();
    g_kList_strCommand.pop_front();

    return strCommand;
}

void netSendPartyExitPacket(CPythonNetworkStream *stream)
{
    auto &rPlayer = CPythonPlayer::instance();

    uint32_t dwVID = rPlayer.GetMainCharacterIndex();
    uint32_t dwPID;
    if (rPlayer.PartyMemberVIDToPID(dwVID, &dwPID))
        stream->SendPartyRemovePacket(dwPID);
}

void init_net(py::module &m)
{
    py::module net = m.def_submodule("net", "Provides network functionality");

#define DEF_PROPERTY_ARRAY(mainType, name, type, size)                                                                 \
    .def_property_readonly(#name, [](const mainType &c) -> const std::array<type, size> & {                            \
        return reinterpret_cast<const std::array<type, size> &>(c.name);                                               \
    })

    net.def("SetServerInfo", &netStreamSetServerInfo);
    net.def("GetServerInfo", &netStreamGetServerInfo);
    net.def("EvaluatePartData", &EvaluatePartData);

    net.def("PreserveCommand", &netPreserveServerCommand);
    net.def("GetPreservedServerCommand", &netGetPreservedServerCommand);

    py::class_<TPlayerItemAttribute>(net, "TPlayerItemAttribute")
        .def(py::init<>())
        .def_readwrite("type", &TPlayerItemAttribute::bType)
        .def_readwrite("value", &TPlayerItemAttribute::sValue);

    py::class_<Part>(net, "Part")
        .def(py::init<>())
        .def_readwrite("vnum", &Part::vnum)
        .def_readwrite("appearance", &Part::appearance)
        .def_readwrite("costume", &Part::costume)
        .def_readwrite("costume_appearance", &Part::costume_appearance);

    py::class_<SimplePlayer>(net, "SimplePlayer")
        .def(py::init<>())
        .def_readwrite("id", &SimplePlayer::id)

        .def_readwrite("empire", &SimplePlayer::empire)
        .def_readwrite("job", &SimplePlayer::byJob)
        .def_readwrite("level", &SimplePlayer::byLevel)
        .def_readwrite("playTime", &SimplePlayer::dwPlayMinutes)
        .def_readwrite("st", &SimplePlayer::byST)
        .def_readwrite("ht", &SimplePlayer::byHT)
        .def_readwrite("dx", &SimplePlayer::byDX)
        .def_readwrite("iq", &SimplePlayer::byIQ)
        DEF_PROPERTY_ARRAY(SimplePlayer, parts, Part, PART_MAX_NUM)
        .def_readwrite("changeName", &SimplePlayer::bChangeName)
        .def_readwrite("changeEmpire", &SimplePlayer::bChangeEmpire)
        .def_readwrite("mapIndex", &SimplePlayer::mapIndex)
        .def_readwrite("x", &SimplePlayer::x)
        .def_readwrite("y", &SimplePlayer::y)
        .def_readwrite("port", &SimplePlayer::port)
        .def_readwrite("skillGroup", &SimplePlayer::skill_group)
        .def_readwrite("lastPlay", &SimplePlayer::last_play);


    auto netStream = py::class_<CPythonNetworkStream, std::shared_ptr<CPythonNetworkStream>>(net, "netStream");
    netStream.def(py::init([]()
    {
        return gPythonNetworkStream;
    }), pybind11::return_value_policy::reference);
    netStream.def("ClearPhaseWindow", &CPythonNetworkStream::ClearPhaseWindow);
    netStream.def("ConnectGameServer", &CPythonNetworkStream::ConnectGameServer);
    netStream.def("IsGamePhase", &CPythonNetworkStream::IsGamePhase);
    netStream.def("Disconnect", &CPythonNetworkStream::Disconnect);
    netStream.def("ExitApp", &CPythonNetworkStream::ExitApplication);
    netStream.def("ExitGame", &CPythonNetworkStream::ExitGame);
    netStream.def("GetGuildID", &CPythonNetworkStream::GetGuildID);
    netStream.def("GetGuildName", &CPythonNetworkStream::GetGuildName);
    netStream.def("GetLoginId", [](const CPythonNetworkStream &self)
    {
        return py::bytes(self.GetLoginId());
    });
    netStream.def("GetMainActorEmpire", &CPythonNetworkStream::GetMainActorEmpire);
    netStream.def("GetMainActorRace", &CPythonNetworkStream::GetMainActorRace);
    netStream.def("GetMainActorSkillGroup", &CPythonNetworkStream::GetMainActorSkillGroup);
    netStream.def("GetPlayer", &CPythonNetworkStream::GetPlayerRef);
    netStream.def("GetPlayerName", &CPythonNetworkStream::GetPlayerName);
    netStream.def("IsInsultIn", &CPythonNetworkStream::IsInsultIn);
    netStream.def("LogOutGame", &CPythonNetworkStream::LogOutGame);
    netStream.def("SendOpenMyShop", &CPythonNetworkStream::OpenMyShop);
    netStream.def("OpenMyShopSearch", &CPythonNetworkStream::OpenMyShopSearch);
    netStream.def("SendAcceRefineAcceptPacket", &CPythonNetworkStream::SendAcceRefineAcceptPacket);
    netStream.def("SendAcceRefineCancelPacket", &CPythonNetworkStream::SendAcceRefineCancelPacket);
    netStream.def("SendAcceRefineCheckinPacket", &CPythonNetworkStream::SendAcceRefineCheckinPacket);
    netStream.def("SendAcceRefineCheckoutPacket", &CPythonNetworkStream::SendAcceRefineCheckoutPacket);
    netStream.def("SendChangeLookCheckinPacket", &CPythonNetworkStream::SendChangeLookCheckinPacket);
    netStream.def("SendChangeLookCheckoutPacket", &CPythonNetworkStream::SendChangeLookCheckoutPacket);
    netStream.def("SendChangeLookAcceptPacket", &CPythonNetworkStream::SendChangeLookAcceptPacket);
    netStream.def("SendChangeLookCancelPacket", &CPythonNetworkStream::SendChangeLookCancelPacket);
    netStream.def("SendAnswerMakeGuildPacket", &CPythonNetworkStream::SendAnswerMakeGuildPacket);
    netStream.def("SendChangeEmpirePacket", &CPythonNetworkStream::SendChangeEmpirePacket);
    netStream.def("SendChangeNamePacket", &CPythonNetworkStream::SendChangeNamePacket);
    netStream.def("SendChangeSkillColorPacket", &CPythonNetworkStream::SendChangeSkillColorPacket);
    netStream.def("SendChatPacketWithType", &CPythonNetworkStream::SendChatPacket);
    netStream.def("SendChatPacket", &CPythonNetworkStream::SendSimpleChatPacket);
    netStream.def("SendCreateCharacterPacket", &CPythonNetworkStream::SendCreateCharacterPacket);
    netStream.def("SendDestroyCharacterPacket", &CPythonNetworkStream::SendDestroyCharacterPacket);
    netStream.def("SendDragonSoulRefinePacket", &CPythonNetworkStream::SendDragonSoulRefinePacket);
    netStream.def("SendEmoticon", &CPythonNetworkStream::SendEmoticon);
    netStream.def("SendEnterGame", &CPythonNetworkStream::SendEnterGame);
    netStream.def("SendExchangeAcceptPacket", &CPythonNetworkStream::SendExchangeAcceptPacket);
    netStream.def("SendExchangeElkAddPacket", &CPythonNetworkStream::SendExchangeElkAddPacket);
    netStream.def("SendExchangeExitPacket", &CPythonNetworkStream::SendExchangeExitPacket);
    netStream.def("SendExchangeItemAddPacket", &CPythonNetworkStream::SendExchangeItemAddPacket);
    netStream.def("SendExchangeItemDelPacket", &CPythonNetworkStream::SendExchangeItemDelPacket);
    netStream.def("SendExchangeStartPacket", &CPythonNetworkStream::SendExchangeStartPacket);
    netStream.def("SendGiveItemPacket", &CPythonNetworkStream::SendGiveItemPacket);
    netStream.def("SendGuildAddMemberPacket", &CPythonNetworkStream::SendGuildAddMemberPacket);
    netStream.def("SendGuildChangeGradeAuthorityPacket", &CPythonNetworkStream::SendGuildChangeGradeAuthorityPacket);
    netStream.def("SendGuildChangeGradeNamePacket", &CPythonNetworkStream::SendGuildChangeGradeNamePacket);
    netStream.def("SendGuildChangeMemberGeneralPacket", &CPythonNetworkStream::SendGuildChangeMemberGeneralPacket);
    netStream.def("SendGuildChangeMemberGradePacket", &CPythonNetworkStream::SendGuildChangeMemberGradePacket);
    netStream.def("SendGuildChargeGSPPacket", &CPythonNetworkStream::SendGuildChargeGSPPacket);
    netStream.def("SendGuildDeleteCommentPacket", &CPythonNetworkStream::SendGuildDeleteCommentPacket);
    netStream.def("SendGuildDepositMoneyPacket", &CPythonNetworkStream::SendGuildDepositMoneyPacket);
    netStream.def("SendGuildInviteAnswerPacket", &CPythonNetworkStream::SendGuildInviteAnswerPacket);
    netStream.def("SendGuildOfferPacket", &CPythonNetworkStream::SendGuildOfferPacket);
    netStream.def("SendGuildPostCommentPacket", &CPythonNetworkStream::SendGuildPostCommentPacket);
    netStream.def("SendGuildRefreshCommentsPacket", &CPythonNetworkStream::SendGuildRefreshCommentsPacket);
    netStream.def("SendGuildRemoveMemberPacket",
                  py::overload_cast<const std::string &>(&CPythonNetworkStream::SendGuildRemoveMemberPacket));
    netStream.def("SendGuildUseSkillPacket", &CPythonNetworkStream::SendGuildUseSkillPacket);
    netStream.def("SendGuildWithdrawMoneyPacket", &CPythonNetworkStream::SendGuildWithdrawMoneyPacket);
    netStream.def("SendItemCombinationPacket", &CPythonNetworkStream::SendItemCombinationPacket);
    netStream.def("SendItemDestroyPacket", &CPythonNetworkStream::SendItemDestroyPacket);
    netStream.def("SendItemMovePacket", &CPythonNetworkStream::SendItemMovePacket);
    netStream.def("SendItemSplitPacket", &CPythonNetworkStream::SendItemSplitPacket);
    netStream.def("SendSetTitlePacket", &CPythonNetworkStream::SendSetTitlePacket);
    netStream.def("SendItemUseMultiplePacket", &CPythonNetworkStream::SendItemUseMultiplePacket);
    netStream.def("SendRemoveMetinPacket", &CPythonNetworkStream::SendRemoveMetinPacket);
    netStream.def("SendItemUsePacket", &CPythonNetworkStream::SendItemUsePacket);
    netStream.def("SendItemUseToItemPacket", &CPythonNetworkStream::SendItemUseToItemPacket);
    netStream.def("SendLoadTargetInfo", &CPythonNetworkStream::SendLoadTargetInfo);
    netStream.def("SendMallCheckoutPacket", &CPythonNetworkStream::SendMallCheckoutPacket);
    netStream.def("SendMessengerAddByNamePacket", &CPythonNetworkStream::SendMessengerAddByNamePacket);
    netStream.def("SendMessengerAddByVIDPacket", &CPythonNetworkStream::SendMessengerAddByVIDPacket);
    netStream.def("SendMessengerRemovePacket", &CPythonNetworkStream::SendMessengerRemovePacket);
    netStream.def("SendMessengerSetBlockPacket", &CPythonNetworkStream::SendMessengerSetBlockPacket);
    netStream.def("SendPartyExitPacket", &netSendPartyExitPacket);
    netStream.def("SendMyShopDecoSet", &CPythonNetworkStream::SendMyShopDecoSet);
    netStream.def("SendMyShopDecoState", &CPythonNetworkStream::SendMyShopDecoState);
    netStream.def("SendOfflineShopWarpPacket", &CPythonNetworkStream::SendOfflineShopWarpPacket);
    netStream.def("SendOnClickPacket", &CPythonNetworkStream::SendOnClickPacket);
    netStream.def("SendPartyInviteAnswerPacket", &CPythonNetworkStream::SendPartyInviteAnswerPacket);
    netStream.def("SendPartyInvitePacket", &CPythonNetworkStream::SendPartyInvitePacket);
    netStream.def("SendPartyParameterPacket", &CPythonNetworkStream::SendPartyParameterPacket);
    netStream.def("SendPartyRemovePacket", &CPythonNetworkStream::SendPartyRemovePacket);
    netStream.def("SendPartySetStatePacket", &CPythonNetworkStream::SendPartySetStatePacket);
    netStream.def("SendPartyUseSkillPacket", &CPythonNetworkStream::SendPartyUseSkillPacket);

    netStream.def("SendPrivateShopSearchOpenFilter", &CPythonNetworkStream::SendPrivateShopSearchOpenFilter);
    netStream.def("SendPrivateShopSearchOpenCategory", &CPythonNetworkStream::SendPrivateShopSearchOpenCategory);
    netStream.def("SendPrivateShopSearchInfoSearch", &CPythonNetworkStream::SendPrivateShopSearchInfoSearch);
    netStream.def("SendPrivateShopSearchOpenPage", &CPythonNetworkStream::SendPrivateShopSearchOpenPage);
    netStream.def("SendPrivateShopSearchBuyItem", &CPythonNetworkStream::SendPrivateShopSearchBuyItem);


    netStream.def("SendQuestCommandPacket", &CPythonNetworkStream::SendQuestCommandPacket);
    netStream.def("SendQuestConfirmPacket", &CPythonNetworkStream::SendQuestConfirmPacket);
    netStream.def("SendQuestInputStringLongPacket", &CPythonNetworkStream::SendQuestInputStringLongPacket);
    netStream.def("SendQuestInputStringPacket", &CPythonNetworkStream::SendQuestInputStringPacket);
    netStream.def("SendRefinePacket", &CPythonNetworkStream::SendRefinePacket);
    netStream.def("SendUpdateSwitchbotAttributePacket", &CPythonNetworkStream::SendUpdateSwitchbotAttributePacket);
    netStream.def("SendSwitchbotSlotActivatePacket", &CPythonNetworkStream::SendSwitchbotSlotActivatePacket);
    netStream.def("SendSwitchbotSlotDeactivatePacket", &CPythonNetworkStream::SendSwitchbotSlotDeactivatePacket);
    netStream.def("SendSafeboxCheckinPacket", &CPythonNetworkStream::SendSafeBoxCheckinPacket);
    netStream.def("SendSafeboxCheckoutPacket", &CPythonNetworkStream::SendSafeBoxCheckoutPacket);
    netStream.def("SendSafeboxItemMovePacket", &CPythonNetworkStream::SendSafeBoxItemMovePacket);
    netStream.def("SendSafeboxMoneyPacket", &CPythonNetworkStream::SendSafeBoxMoneyPacket);
    netStream.def("SendScriptAnswerPacket", &CPythonNetworkStream::SendScriptAnswerPacket);
    netStream.def("SendScriptButtonPacket", &CPythonNetworkStream::SendScriptButtonPacket);
    netStream.def("SendSelectItemPacket", &CPythonNetworkStream::SendSelectItemPacket);
    netStream.def("SendShopBuyPacket", &CPythonNetworkStream::SendShopBuyPacket, py::arg("pos"), py::arg("amount") = 1);
    netStream.def("SendShopEndPacket", &CPythonNetworkStream::SendShopEndPacket);
    netStream.def("SendShopSellPacket", &CPythonNetworkStream::SendShopSellPacket);
    netStream.def("SendShopSellPacketNew", &CPythonNetworkStream::SendShopSellPacketNew);
    netStream.def("SendSkillGroupSelectPacket", &CPythonNetworkStream::SendSkillGroupSelectPacket);
    netStream.def("SendBattlePassAction", &CPythonNetworkStream::SendBattlePassAction);

    netStream.def("SendRequestDungeonInfo", &CPythonNetworkStream::SendRequestDungeonInfo);
    netStream.def("SendRequestDungeonRanking", &CPythonNetworkStream::SendRequestDungeonRanking);
    netStream.def("SendRequestDungeonWarp", &CPythonNetworkStream::SendRequestDungeonWarp);


    netStream.def("SendWhisperPacket", &CPythonNetworkStream::SendWhisperPacket);
    netStream.def("SetHandler", &CPythonNetworkStream::SetHandler);
    netStream.def("SetMarkServer", &CPythonNetworkStream::SetMarkServer);
    netStream.def("SetOffLinePhase", &CPythonNetworkStream::SetOffLinePhase);
    netStream.def("SetPhaseWindow", &CPythonNetworkStream::SetPhaseWindow);
    netStream.def("SetSvPsWindow", &CPythonNetworkStream::SetServerCommandParserWindow);
    netStream.def("StartGame", &CPythonNetworkStream::StartGame);
    netStream.def("ToggleGameDebugInfo", &CPythonNetworkStream::ToggleGameDebugInfo);
    netStream.def("UploadMark", &CPythonNetworkStream::UploadMark);
    netStream.def("UploadSymbol", &CPythonNetworkStream::UploadSymbol);
    netStream.def("Warp", &CPythonNetworkStream::Warp);
    netStream.def("SendChatFilter", &CPythonNetworkStream::SendChatFilter);
    netStream.def("SendAttrPlusLevelPet", &CPythonNetworkStream::SendAttrPlusLevelPet);
    netStream.def("SendOpenLevelPet", &CPythonNetworkStream::SendOpenLevelPet);
    netStream.def("SendCloseLevelPet", &CPythonNetworkStream::SendCloseLevelPet);
    netStream.def("GetMultiState", &CPythonNetworkStream::GetMultiState);

    

    
}
