set(SOURCE_FILES
	${METIN2_SHARE}/WinMsgBoxErrorHandlers.cpp

	AuthPackets.cpp
	AuthPackets.hpp
	AuthSocket.cpp
	AuthSocket.hpp
	CameraProcedure.cpp
	ClientItemGrid.cpp
	ClientItemGrid.hpp
	GameType.cpp
	GameType.h
	GameData.cpp
	GameData.hpp
	GuildMarkDownloader.cpp
	GuildMarkDownloader.h
	GuildMarkUploader.cpp
	GuildMarkUploader.h
	HWIDManager.cpp
	HWIDManager.h
	InstanceBase.cpp
	InstanceBase.h
	InstanceBaseBattle.cpp
	InstanceBaseEffect.cpp
	InstanceBaseEvent.cpp
	InstanceBaseMotion.cpp
	InstanceBaseMovement.cpp
	InstanceBaseTransform.cpp
	InsultChecker.cpp
	InsultChecker.h
	Locale.cpp
	Locale.h
	MarkImage.cpp
	MarkImage.h
	MarkManager.cpp
	MarkManager.h
	NetworkActorManager.cpp
	NetworkActorManager.h
	PythonAdmin.cpp
	PythonAdmin.h
	PythonAdminModule.cpp
	PythonApplication.cpp
	PythonApplication.h
	PythonApplicationCamera.cpp
	PythonApplicationCursor.cpp
	PythonApplicationEvent.cpp
	PythonApplicationLogo.cpp
	PythonApplicationModule.cpp
	PythonApplicationProcedure.cpp
	PythonApplicationWebPage.cpp
	PythonBackground.cpp
	PythonBackground.h
	PythonBackgroundModule.cpp
	PythonBindings.cpp
	PythonBindings.h
	PythonCharacterManager.cpp
	PythonCharacterManager.h
	PythonCharacterManagerModule.cpp
	PythonCharacterModule.cpp
	PythonChat.cpp
	PythonChat.h
	PythonChatModule.cpp
	PythonCube.cpp
	PythonCube.hpp
	PythonDungeonInfo.cpp
	PythonDungeonInfo.h
	PythonEffectModule.cpp
	PythonEmojiManager.cpp
	PythonEmojiManager.h
	PythonEventManager.cpp
	PythonEventManager.h
	PythonEventManagerMoudle.cpp
	PythonExceptionSender.cpp
	PythonExceptionSender.h
	PythonExchange.cpp
	PythonExchange.h
	PythonExchangeModule.cpp
	PythonFlyModule.cpp
	PythonFishingManager.cpp
	PythonFishingManager.h
	PythonGameEventManagerModule.cpp
	PythonGuild.cpp
	PythonGuild.h
	PythonGuildStorage.cpp
	PythonGuildStorage.h
	PythonItem.cpp
	PythonItem.h
	PythonItemModule.cpp
	PythonItemRenderTargetManager.cpp
	PythonItemRenderTargetManager.h
	PythonMessenger.cpp
	PythonMessenger.h
	PythonMiniMap.cpp
	PythonMiniMap.h
	PythonMiniMapModule.cpp
	PythonMyShopDecoManager.cpp
	PythonMyShopDecoManager.h
	PythonNetworkDatagram.cpp
	PythonNetworkDatagram.h
	PythonNetworkDatagramModule.cpp
	PythonNetworkStream.cpp
	PythonNetworkStream.h
	PythonNetworkStreamCommand.cpp
	PythonNetworkStreamEvent.cpp
	PythonNetworkStreamModule.cpp
	PythonNetworkStreamPhaseGame.cpp
	PythonNetworkStreamPhaseGameActor.cpp
	PythonNetworkStreamPhaseGameItem.cpp
	PythonNetworkStreamPhaseHandShake.cpp
	PythonNetworkStreamPhaseLoading.cpp
	PythonNetworkStreamPhaseLogin.cpp
	PythonNetworkStreamPhaseOffline.cpp
	PythonNetworkStreamPhaseSelect.cpp
	PythonNonPlayerModule.cpp
	PythonOfflineShop.cpp
	PythonOfflineShop.h
	PythonOfflineShopModule.cpp
	PythonPackModule.cpp
	PythonPlayer.cpp
	PythonPlayer.h
	PythonPlayerEventHandler.cpp
	PythonPlayerEventHandler.h
	PythonPlayerInput.cpp
	PythonPlayerInputKeyboard.cpp
	PythonPlayerInputMouse.cpp
	PythonPlayerModule.cpp
	PythonPlayerSkill.cpp
	PythonPrivateShopSearch.cpp
	PythonPrivateShopSearch.h
	PythonProfilerModule.cpp
	PythonQuest.cpp
	PythonQuest.h
	PythonSafeBox.cpp
	PythonSafeBox.h
	PythonShop.cpp
	PythonShop.h
	PythonSkill.cpp
	PythonSkill.h
	PythonSoundManagerModule.cpp
	PythonSystemModule.cpp
	PythonTextTail.cpp
	PythonTextTail.h
	PythonTextTailModule.cpp
	PythonWikiModelViewManager.cpp
	PythonWikiModelViewManager.h
	PythonWikiModule.cpp
	ServerStateChecker.cpp
	ServerStateChecker.h
	ServerStateCheckerModule.cpp
	Test.h
	UserInterface.cpp
	resource.h
)


add_executable(metin2client_run WIN32 ${SOURCE_FILES} UserInterface.rc)
target_precompile_headers(metin2client_run PRIVATE "StdAfx.h" "StdAfx.cpp")
set_target_properties(metin2client_run PROPERTIES LINK_FLAGS "/level='requireAdministrator' /uiAccess='false'")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_target_properties(metin2client_run PROPERTIES OUTPUT_NAME "Diamas")
else ()
	set_target_properties(metin2client_run PROPERTIES OUTPUT_NAME "Diamas")
endif ()

target_link_libraries(metin2client_run  PUBLIC
	vstl
	storm
	metin2_base
	metin2_net
	metin2_pak
	metin2_winsup
	cefdll_wrapper
	python27_static
	pybind11::pybind11
	pybind11::embed
	pybind11::windows_extras
	pybind11::opt_size
	
	
	metin2client_base
	metin2client_lib
	metin2client_granny
	metin2client_ui
	metin2client_effects
	metin2client_game
	metin2client_miles
	metin2client_terrain
	metin2client_scriptlib
	metin2client_speedtree
	metin2client_spherelib
	metin2client_webview
	metin2client_script


	version
	imagehlp
	winmm
	imm32
	dinput8
	dxguid
	ws2_32
	Crypt32
	miles
	legacy_stdio_definitions.lib
	${DIRECT3D_LIBRARY}

	
	lzo-pro
	lz4::lz4
	spdlog::spdlog
	effolkronium_random
	devil_static
	Microsoft.GSL::GSL
)

create_target_launcher(metin2client_run
    RUNTIME_LIBRARY_DIRS "./"
    WORKING_DIRECTORY ${METIN2_CLIENT_RUN_DIR}
)
