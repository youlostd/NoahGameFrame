#include "PythonApplication.h"
#include "StdAfx.h"
//#include <shellapi.h>
#undef GetNextSibling
#undef GetFirstChild
#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif
#ifdef _WIN32
#undef BOOST_ASIO_HAS_IOCP
#define BOOST_ASIO_HAS_IOCP 1
#endif
#include <boost/asio/io_service.hpp>

#include "include/cef_client.h"
#include <pak/Vfs.hpp>

#include "../EterBase/lzo.h"
#include "../MilesLib/SoundVfs.hpp"
#include "HWIDManager.h"
#include <il/il.h>

#ifndef _DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_ERROR
#endif

#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
//#include <crash_reporter/CrashRpt.h>
#include "imgui.h"

#include <VersionHelpers.h>
#include <base/Clipboard.hpp>

#include "PythonBindings.h"
#include "base/Remotery.h"

#include <base/Clipboard.hpp>
#include <base/Console.hpp>
#include <base/DumpVersion.hpp>
#include <base/SimpleApp.hpp>
#include <windows/ComUtil.hpp>

#include <Version.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <filesystem>

#include "pak/DiskVfsProvider.hpp"
#include "pak/Vfs.hpp"
#include "pak/pak/PakVfsProvider.hpp"
namespace fs = std::filesystem;

extern "C"
{
    extern int _fltused;
    volatile int _AVOID_FLOATING_POINT_LIBRARY_BUG = _fltused;
    __declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
};

std::shared_ptr<spdlog::logger> logger;

std::array<std::string, 39> paks = {"pack/property",
                                    "pack/ui",
                                    "pack/environment",
                                    "pack/special",
                                    "pack/bgm",
                                    "pack/sound",
                                    "pack/map",
                                    "pack/icon",
                                    "pack/textureset",
                                    "pack/item",
                                    "pack/guild",
                                    "pack/tree",
                                    "pack/npc2",
                                    "pack/npc",
                                    "pack/npc_pet",
                                    "pack/npc_mount",
                                    "pack/terrain",
                                    "pack/zone",
                                    "pack/effect",
                                    "pack/monster2",
                                    "pack/monster",
                                    "pack/pc2_assassin",
                                    "pack/pc2_shaman",
                                    "pack/pc2_sura",
                                    "pack/pc2_warrior",
                                    "pack/pc2_common",
                                    "pack/pc_assassin",
                                    "pack/pc_shaman",
                                    "pack/pc_sura",
                                    "pack/pc_warrior",
                                    "pack/pc_common",
                                    "pack/pc_ridack",
                                    "pack/pc_plechito",
                                    "pack/locale_m2",
                                    "pack/update_effect",
                                    "pack/game_patch_update1",
                                    "pack/root"

};

extern "C" void initpygame();

void AddPythonBuiltins()
{
    PyObject *builtins = PyImport_ImportModule("__builtin__");

    // MARK_BUG_FIX
    PyModule_AddIntConstant(builtins, "ERROR_MARK_UPLOAD_NEED_RECONNECT",
                            CPythonNetworkStream::ERROR_MARK_UPLOAD_NEED_RECONNECT);
    PyModule_AddIntConstant(builtins, "ERROR_MARK_CHECK_NEED_RECONNECT",
                            CPythonNetworkStream::ERROR_MARK_CHECK_NEED_RECONNECT);
    // END_OF_MARK_BUG_FIX

    PyModule_AddIntMacro(builtins, DS_SUB_HEADER_REFINE_FAIL);
    PyModule_AddIntMacro(builtins, DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE);
    PyModule_AddIntMacro(builtins, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL);
    PyModule_AddIntMacro(builtins, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY);
    PyModule_AddIntMacro(builtins, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL);
    PyModule_AddIntMacro(builtins, DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL);
    PyModule_AddIntMacro(builtins, DS_SUB_HEADER_REFINE_SUCCEED);
    PyModule_AddIntMacro(builtins, PART_MAIN);
    PyModule_AddIntMacro(builtins, PART_WEAPON);
    PyModule_AddIntMacro(builtins, PART_HEAD);
    PyModule_AddIntMacro(builtins, PART_WEAPON_LEFT);
    PyModule_AddIntMacro(builtins, PART_HAIR);
    PyModule_AddIntMacro(builtins, PART_ACCE);
    PyModule_AddIntMacro(builtins, PART_BODY_EFFECT);
    PyModule_AddIntMacro(builtins, PART_WEAPON_EFFECT);
    PyModule_AddIntMacro(builtins, PART_WING_EFFECT);
    PyModule_AddIntMacro(builtins, OBJECT_MATERIAL_MAX_NUM);
    PyModule_AddIntMacro(builtins, BUILDING_HEADQUARTERS);
    PyModule_AddIntMacro(builtins, BUILDING_FACILITY);
    PyModule_AddIntMacro(builtins, BUILDING_OBJECT);
    PyModule_AddIntMacro(builtins, BUILDING_WALL);
    PyModule_AddIntMacro(builtins, BUILDING_BUILD_IN);

    PyModule_AddIntConstant(builtins, "PHASE_WINDOW_LOGIN", CPythonNetworkStream::PHASE_WINDOW_LOGIN);
    PyModule_AddIntConstant(builtins, "PHASE_WINDOW_SELECT", CPythonNetworkStream::PHASE_WINDOW_SELECT);
    PyModule_AddIntConstant(builtins, "PHASE_WINDOW_CREATE", CPythonNetworkStream::PHASE_WINDOW_CREATE);
    PyModule_AddIntConstant(builtins, "PHASE_WINDOW_LOAD", CPythonNetworkStream::PHASE_WINDOW_LOAD);
    PyModule_AddIntConstant(builtins, "PHASE_WINDOW_GAME", CPythonNetworkStream::PHASE_WINDOW_GAME);
    PyModule_AddIntConstant(builtins, "PHASE_WINDOW_EMPIRE", CPythonNetworkStream::PHASE_WINDOW_EMPIRE);
    PyModule_AddIntConstant(builtins, "PHASE_WINDOW_LOGO", CPythonNetworkStream::PHASE_WINDOW_LOGO);
    PyModule_AddIntConstant(builtins, "EMPIRE_A", 1);
    PyModule_AddIntConstant(builtins, "EMPIRE_B", 2);
    PyModule_AddIntConstant(builtins, "EMPIRE_C", 3);

    Py_DECREF(builtins);
}

bool RunMainScript(CPythonLauncher &pyLauncher)
{
    // initpygame();
    AddPythonBuiltins();
    return pyLauncher.Run();
}

bool InitializeCef(CefMainArgs &mainArgs)
{
    {
        CefSettings settings;

        // checkout detailed settings options
        // http://magpcss.org/ceforum/apidocs/projects/%28default%29/_cef_settings_t.html nearly all the settings can be
        // set via args too. settings.multi_threaded_message_loop = true; // not supported, except windows
        // CefString(&settings.browser_subprocess_path).FromASCII("sub_proccess path, by default uses and starts this
        // executeable as child");
        //
        //
        // settings.log_severity = LOGSEVERITY_DEFAULT;

        settings.multi_threaded_message_loop = true;
        settings.windowless_rendering_enabled = true;

        settings.no_sandbox = true;

        bool result = CefInitialize(mainArgs, settings, nullptr, nullptr);
        // CefInitialize creates a sub-proccess and executes the same executeable, as calling CefInitialize, if not set
        // different in settings.browser_subprocess_path if you create an extra program just for the childproccess you
        // only have to call CefExecuteProcess(...) in it.
        if (!result)
        {
            // throw std::exception("Error initializing CEF based embedded browser");
            return false;
        }
    }

    return true;
}

void GrannyError(granny_log_message_type Type, granny_log_message_origin Origin, char const *SourceFile,
                 granny_int32x SourceLine, char const *Message, void *UserData)
{
    // FIXME: Fix the actual problems instead of filtering errors derived from incorrect models we can't do anything
    // about atm
    if (Type == 2 && Origin == 51)
        return;

    // SPDLOG_DEBUG("Granny says ({0}, {1}): {2} (in {3} on line {4})\n", Type, Origin, Message, SourceFile,
    // SourceLine);
}

class Main : public SimpleApp
{
  public:
    Main(HINSTANCE hInstance, asio::io_service& io_service);
    ~Main();

    int Run(int argc, const char **argv);

  private:
    bool ParseArguments(int argc, const char **argv);

    void SetupDefaultLoggers();

    HINSTANCE m_hInstance;
    ScopedComInitialization m_comInit;
    asio::io_service& m_ioService;

    Vfs m_vfs;
    PakVfsProvider m_pak;
    DiskVfsProvider m_disk;

};

Main::Main(HINSTANCE hInstance, asio::io_service& io_service) : m_hInstance(hInstance), m_ioService(io_service)
{
    const auto now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    srand(millis);
    srandom(millis);

    ClipboardInit();

    for (const auto& pak : paks) {
     const std::string nPakName{pak + ".pak"};
        m_pak.AddArchive(nPakName);
    }

    m_vfs.RegisterProvider(&m_pak);
    m_vfs.RegisterProvider(&m_disk);

    SetVfs(&m_vfs);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
}

Main::~Main()
{
    SetVfs(nullptr);

    CefShutdown();
    ImGui::DestroyContext();
}

int Main::Run(int argc, const char **argv)
{

    Remotery *rmt;
    rmt_CreateGlobalInstance(&rmt);


    try
    {
        if (!ParseArguments(argc, argv))
            return 1;
    }
    catch (std::exception &e)
    {
        SPDLOG_ERROR("args: {0}", e.what());
        return 1;
    }

    SPDLOG_INFO("Version: {0}", kHumanVersionString);

    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);

    SetErrorMode(SEM_FAILCRITICALERRORS);

    ilInit();

    granny_log_callback Callback;
    Callback.Function = GrannyError;
    Callback.UserData = 0;
    GrannySetLogCallback(&Callback);

#ifndef DEBUG
    // There's nothing we can do about outdated files in release-mode.
    GrannyFilterMessage(GrannyFileReadingLogMessage, false);
#endif

    CTextFileLoader::SetCacheMode();

    auto app = std::make_unique<CPythonApplication>(m_ioService);
    app->Initialize(m_hInstance);

    bool ret = false;

    {
        CPythonLauncher pyLauncher;

        ret = RunMainScript(pyLauncher);

        app->Clear();
    }
    rmt_DestroyGlobalInstance(rmt);
    return !ret;
}

void SetupDefaultLoggers()
{
#ifdef ENABLE_CONSOLE
    CreateConsoleWindow();
#endif
    std::error_code ec;
    fs::path p("logs");
    if (fs::exists(p, ec) && fs::is_directory(p, ec))
    {
        fs::directory_iterator end;
        for (fs::directory_iterator it(p, ec); it != end; ++it)
        {

            if (fs::is_regular_file(it->status()) && (it->path().extension().compare(".txt") == 0))
            {
                fs::remove(it->path(), ec);
            }
        }
    }
    const auto now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const auto secs = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(duration).count());

#ifdef ENABLE_CONSOLE
    spdlog::init_thread_pool(8192, 1);

    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto rotating_syserr_sink =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/" + secs + "_err.txt", 1024 * 1024 * 10, 3);
    auto rotating_syslog_sink =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/" + secs + "_log.txt", 1024 * 1024 * 10, 3);

    std::vector<spdlog::sink_ptr> sinks{stdout_sink, rotating_syserr_sink, rotating_syslog_sink};
    auto logger = std::make_shared<spdlog::async_logger>("client", sinks.begin(), sinks.end(), spdlog::thread_pool(),
                                                         spdlog::async_overflow_policy::block);
    spdlog::register_logger(logger);

    logger->set_level(spdlog::level::debug);
    sinks[0]->set_level(spdlog::level::debug); // console. Allow everything.  Default value
    sinks[1]->set_level(spdlog::level::err);   //  syserr
    sinks[2]->set_level(spdlog::level::debug); //  log
#else
    auto logger = spdlog::basic_logger_mt<spdlog::async_factory>("client", "logs/" + secs + "_err.txt");
    logger->set_level(spdlog::level::err);

#endif
    logger->flush_on(spdlog::level::err);
    spdlog::set_default_logger(logger);
}

bool Main::ParseArguments(int argc, const char **argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce a help message")("version,V", "output the version number");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

    if (vm.count("help"))
    {
        std::cout << desc;
        return false;
    }

    if (vm.count("version"))
    {
        DumpVersion();
        return false;
    }

    po::notify(vm);

    return true;
}

//
// Large part of this code's copyright (2003-2012) belongs to Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Distributed under the Boost Software License, Version 1.0. (Copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "stdafx.h"
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#include "obfuscator.hpp"

extern "C" int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    if (!IsWindows7SP1OrGreater())
    {
        return 0;
    }

    HWIDMANAGER::InitHardwareId();

    CefMainArgs mainArgs;
    if (!InitializeCef(mainArgs))
        return 1;


    {
        int result = CefExecuteProcess(mainArgs, nullptr, nullptr);
        // checkout CefApp, derive it and set it as second parameter, for more control on
        // command args and resources.
        if (result >= 0) // child proccess has endend, so exit.
        {


            return result;
        }
        if (result == -1)
        {
            asio::io_service service;
    
            try
            {
                SetupDefaultLoggers();
            }
            catch (const spdlog::spdlog_ex &ex)
            {
                std::string msg = "Failed to open log file with: ";
                msg += ex.what();

                MessageBoxA(NULL, msg.c_str(), METIN2_BRAND_NAME, MB_OK | MB_ICONERROR);
                return 1;
            }
            
            Main main(hInstance, service);
            auto val = main.Run(__argc, const_cast<const char **>(__argv));
            spdlog::shutdown();
            return val;
        }
    }



    return 1;
}
