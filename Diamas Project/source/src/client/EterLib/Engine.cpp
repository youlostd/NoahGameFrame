#include "StdAfx.h"
#include "Engine.hpp"

namespace
{
KeyboardInput *s_keyboard = nullptr;
}

FontManager fontManager;
FontManager& Engine::m_fontManager = fontManager;

CClientConfig clientConfig;
CClientConfig& Engine::m_config = clientConfig;
CGraphicDevice* Engine::m_device = NULL;

KeyboardInput &GetKeyboardInput()
{
    assert(nullptr != s_keyboard && "KeyboardInput not set");
    return *s_keyboard;
}

void SetKeyboardInput(KeyboardInput *keyboard)
{
    s_keyboard = keyboard;
}
