#ifndef METIN2_CLIENT_CWEBBROWSER_WEBMANAGER_IMPL_H
#define METIN2_CLIENT_CWEBBROWSER_WEBMANAGER_IMPL_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <unordered_map>
#include "WebView.h"

struct WebManager_impl
{
    std::unordered_map<uint32_t, CefRefPtr<WebView>> m_webViews;
    std::unordered_map<uint32_t, CGraphicImageTexture *> m_webViewTextures;
};

#endif /* METIN2_CLIENT_CWEBBROWSER_WEBMANAGER_IMPL_H */
