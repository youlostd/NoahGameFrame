#ifndef METIN2_CLIENT_CWEBBROWSER_WEBMANAGER_H
#define METIN2_CLIENT_CWEBBROWSER_WEBMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#undef GetNextSibling
#undef GetFirstChild

#include <base/Singleton.hpp>
#include "../EterLib/GrpImageTexture.h"

#include <list>
#include <unordered_map>
#include <include/cef_app.h>

#include "../ScriptLib/PyStr.hpp"
#include "WebManager_impl.h"

class WebView;

class WebManager : public CSingleton<WebManager>
{
public:

    WebManager()
    {
        m_impl = std::make_unique<WebManager_impl>();
    }

    CefRefPtr<WebView> CreateWebView(uint32_t width, uint32_t height);
    void DestroyWebView(uint32_t index);
    void DestroyAllWebViews();
    int GetEmptyIndex();

    WebView *GetFocusedWebView()
    {
        return m_focusedWebView;
    }

    void SetFocusedWebView(WebView *webView)
    {
        m_focusedWebView = webView;
    }

    void ProcessInputMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void NotifyPopup(const std::string &strTagetUrl, const std::string &strOpenerUrl);
    void SetInterfaceHandler(PyObject *poHandler);
    void DestroyInterfaceHandler();
    PyObject *m_poInterface;

private:
    WebView *m_focusedWebView = nullptr;
    std::unique_ptr<WebManager_impl> m_impl;
};
#endif /* METIN2_CLIENT_CWEBBROWSER_WEBMANAGER_H */
