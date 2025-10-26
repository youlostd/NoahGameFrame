#include "WebManager.h"

#include <Python.h>

#include "WebView.h"
#include "../ScriptLib/StdAfx.h"
#include "../ScriptLib/PythonUtils.h"

bool isKeyDown(WPARAM wParam)
{
    return (GetKeyState(wParam) & 0x8000) != 0;
}

int GetCefKeyboardModifiers(WPARAM wParam, LPARAM lParam)
{
    int modifiers = 0;
    if (isKeyDown(VK_SHIFT))
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    if (isKeyDown(VK_CONTROL))
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    if (isKeyDown(VK_MENU))
        modifiers |= EVENTFLAG_ALT_DOWN;

    // Low bit set from GetKeyState indicates "toggled".
    if (::GetKeyState(VK_NUMLOCK) & 1)
        modifiers |= EVENTFLAG_NUM_LOCK_ON;
    if (::GetKeyState(VK_CAPITAL) & 1)
        modifiers |= EVENTFLAG_CAPS_LOCK_ON;

    switch (wParam)
    {
    case VK_RETURN:
        if ((lParam >> 16) & KF_EXTENDED)
            modifiers |= EVENTFLAG_IS_KEY_PAD;
        break;
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
        if (!((lParam >> 16) & KF_EXTENDED))
            modifiers |= EVENTFLAG_IS_KEY_PAD;
        break;
    case VK_NUMLOCK:
    case VK_NUMPAD0:
    case VK_NUMPAD1:
    case VK_NUMPAD2:
    case VK_NUMPAD3:
    case VK_NUMPAD4:
    case VK_NUMPAD5:
    case VK_NUMPAD6:
    case VK_NUMPAD7:
    case VK_NUMPAD8:
    case VK_NUMPAD9:
    case VK_DIVIDE:
    case VK_MULTIPLY:
    case VK_SUBTRACT:
    case VK_ADD:
    case VK_DECIMAL:
    case VK_CLEAR:
        modifiers |= EVENTFLAG_IS_KEY_PAD;
        break;
    case VK_SHIFT:
        if (isKeyDown(VK_LSHIFT))
            modifiers |= EVENTFLAG_IS_LEFT;
        else if (isKeyDown(VK_RSHIFT))
            modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    case VK_CONTROL:
        if (isKeyDown(VK_LCONTROL))
            modifiers |= EVENTFLAG_IS_LEFT;
        else if (isKeyDown(VK_RCONTROL))
            modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    case VK_MENU:
        if (isKeyDown(VK_LMENU))
            modifiers |= EVENTFLAG_IS_LEFT;
        else if (isKeyDown(VK_RMENU))
            modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    case VK_LWIN:
        modifiers |= EVENTFLAG_IS_LEFT;
        break;
    case VK_RWIN:
        modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    }
    return modifiers;
}

CefRefPtr<WebView> WebManager::CreateWebView(uint32_t width, uint32_t height)
{
    auto index = GetEmptyIndex();
    auto texture = new CGraphicImageTexture;
    if (!texture)
        return nullptr;

    texture->Create(width, height, D3DFMT_A8R8G8B8);

    auto [iter, success] = m_impl->m_webViewTextures.emplace(index, texture);
    if (success)
    {
        CefRefPtr<WebView> pWebView = new WebView((*iter).second, index);
        auto [iter, success] = m_impl->m_webViews.emplace(index, pWebView);
        if (success)
        {
            return pWebView;
        }
    }

    return nullptr;
}

void WebManager::DestroyWebView(uint32_t index)
{
    auto it = m_impl->m_webViews.find(index);
    if (it == m_impl->m_webViews.end())
        return;

    CefRefPtr<WebView> pWebView = it->second;
    if (pWebView)
    {
        // pWebView->Release(); // Do not release since other references get corrupted then
        pWebView->CloseBrowser();
        m_impl->m_webViews.erase(it);
    }
}

void WebManager::DestroyAllWebViews()
{
    for (auto [index, webView] : m_impl->m_webViews)
    {
        if (webView)
        {
            webView->CloseBrowser();
        }
    }
    m_impl->m_webViews.clear();
}

int WebManager::GetEmptyIndex()
{
    static int iMaxIndex = 1;

    if (iMaxIndex > 2100000000)
        iMaxIndex = 1;

    int iNextIndex = iMaxIndex++;
    while (m_impl->m_webViewTextures.find(iNextIndex) != m_impl->m_webViewTextures.end())
        iNextIndex++;

    return iNextIndex;
}

void WebManager::ProcessInputMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_focusedWebView)
        return;

    CefKeyEvent keyEvent;
    keyEvent.windows_key_code = wParam;
    keyEvent.native_key_code = lParam;
    keyEvent.modifiers = GetCefKeyboardModifiers(wParam, lParam);
    keyEvent.is_system_key = uMsg == WM_SYSCHAR || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP;

    if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
        keyEvent.type = cef_key_event_type_t::KEYEVENT_RAWKEYDOWN;
    else if (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP)
        keyEvent.type = cef_key_event_type_t::KEYEVENT_KEYUP;
    else if (uMsg == WM_CHAR || uMsg == WM_SYSCHAR)
        keyEvent.type = cef_key_event_type_t::KEYEVENT_CHAR;

    m_focusedWebView->InjectKeyboardEvent(keyEvent);
}

void WebManager::NotifyPopup(const std::string &strTagetUrl, const std::string &strOpenerUrl)
{
    PyCallClassMemberFunc(m_poInterface, "BINARY_WebViewPopUp",
                          Py_BuildValue("(ss)", strTagetUrl.c_str(), strOpenerUrl.c_str()));
}

void WebManager::SetInterfaceHandler(PyObject *poHandler)
{
    m_poInterface = poHandler;
}

void WebManager::DestroyInterfaceHandler()
{
    m_poInterface = NULL;
}

PyObject *webManagerSetInterfaceHandler(PyObject *poSelf, PyObject *poArgs)
{
    PyObject *poHandler;
    if (!PyTuple_GetObject(poArgs, 0, &poHandler))
        return Py_BuildException();

    WebManager::Instance().SetInterfaceHandler(poHandler);
    Py_RETURN_NONE;
}

PyObject *webManagerDestroyInterfaceHandler(PyObject *poSelf, PyObject *poArgs)
{
    WebManager::Instance().DestroyInterfaceHandler();
    Py_RETURN_NONE;
}

extern "C" void initwebManager()
{
    static PyMethodDef s_methods[] =
    {

        {"SetInterfaceHandler", webManagerSetInterfaceHandler, METH_VARARGS},
        {"DestroyInterfaceHandler", webManagerDestroyInterfaceHandler, METH_VARARGS},

        {NULL, NULL, NULL}
    };

    PyObject *poModule = Py_InitModule("webManager", s_methods);
}
