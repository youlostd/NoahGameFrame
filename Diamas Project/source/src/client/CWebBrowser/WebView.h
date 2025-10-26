#ifndef METIN2_CLIENT_CWEBBROWSER_WEBVIEW_H
#define METIN2_CLIENT_CWEBBROWSER_WEBVIEW_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <cstdint>
#include "../EterLib/GrpImageTexture.h"
#undef GetNextSibling
#undef GetFirstChild
#include <include/cef_client.h>
#include <include/cef_render_handler.h>
#include <condition_variable>

#include <base/SimpleMath.h>
using namespace DirectX::SimpleMath;

class WebView
    : CefClient, public CGraphicBase,
      CefRenderHandler,
      CefLifeSpanHandler,
      CefContextMenuHandler //Add to allow overwrite of context menu functions

{
public:
    enum eWebBrowserMouseButton
    {
        BROWSER_MOUSEBUTTON_LEFT = 0,
        BROWSER_MOUSEBUTTON_MIDDLE = 1,
        BROWSER_MOUSEBUTTON_RIGHT = 2
    };

    WebView(CGraphicImageTexture *tex, uint32_t id);

    virtual ~WebView();

    void Initialise(HWND parent);

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
    {
        return this;
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler()
    {
        return this;
    };

    virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler()
    {
        return this;
    };

    // CefLifeSpawnHandler methods
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               const CefString &target_url,
                               const CefString &target_frame_name,
                               WindowOpenDisposition target_disposition,
                               bool user_gesture,
                               const CefPopupFeatures &popupFeatures,
                               CefWindowInfo &windowInfo,
                               CefRefPtr<CefClient> &client,
                               CefBrowserSettings &settings,
                               CefRefPtr<CefDictionaryValue> &extra_info,
                               bool *no_javascript_access) override;
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    void CloseBrowser();

    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;
    void UpdateTexture();
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect &rect) override;
    void Focus(bool state);
    CGraphicImageTexture *GetTexture() const;

    void InjectMouseMove(int iPosX, int iPosY);
    void InjectMouseDown(eWebBrowserMouseButton mouseButton);
    void InjectMouseUp(eWebBrowserMouseButton mouseButton);
    void InjectMouseWheel(int iScrollVert, int iScrollHorz);
    void InjectKeyboardEvent(const CefKeyEvent &keyEvent);
    void ClearTexture() const;
    void LoadUrl(const char *url);

    virtual void OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType paintType,
                         const CefRenderHandler::RectList &dirtyRects,
                         const void *buffer, int width, int height) override;
    void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                             CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;

    uint32_t m_id = 0;
    CGraphicImageTexture *m_texture = nullptr;
    CefRefPtr<CefBrowser> m_pWebView;

    Vector2 m_v2Position{};
    RECT m_webRect{};
    bool m_mouseButtonStates[3]{};
    POINT m_vecMousePosition{};

    struct
    {
        bool changed = false;
        std::mutex dataMutex;
        std::mutex cvMutex;
        std::condition_variable cv;

        const void *buffer{};
        int width{}, height{};
        CefRenderHandler::RectList dirtyRects;

        CefRect popupRect;
        bool popupShown = false;
        std::unique_ptr<uint8_t[]> popupBuffer;
    } m_RenderData;

    bool m_bBeingDestroyed = false;

public:
IMPLEMENT_REFCOUNTING(WebView);
};
#endif /* METIN2_CLIENT_CWEBBROWSER_WEBVIEW_H */
