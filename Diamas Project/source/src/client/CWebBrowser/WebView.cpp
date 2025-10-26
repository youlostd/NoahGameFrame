#include "WebManager.h"
#include "WebView.h"

WebView::WebView(CGraphicImageTexture *tex, uint32_t id)
    : m_id(id), m_texture(tex)
{
}

WebView::~WebView()
{
    m_pWebView = nullptr;

    if (WebManager::instance().GetFocusedWebView() == this)
    {
        WebManager::instance().SetFocusedWebView(nullptr);
    }

    m_RenderData.cv.notify_all();
}

void WebView::Initialise(HWND parent)
{
    CefBrowserSettings browserSettings;
    browserSettings.windowless_frame_rate = 60;
    browserSettings.javascript_access_clipboard = cef_state_t::STATE_DISABLED;
    browserSettings.javascript_dom_paste = cef_state_t::STATE_DISABLED;
    browserSettings.file_access_from_file_urls = cef_state_t::STATE_DISABLED;
    browserSettings.webgl = cef_state_t::STATE_ENABLED;
    browserSettings.plugins = cef_state_t::STATE_DISABLED;
    browserSettings.javascript = cef_state_t::STATE_ENABLED;

    CefWindowInfo windowInfo;
    windowInfo.SetAsWindowless(parent);

    CefBrowserHost::CreateBrowser(windowInfo, this, "", browserSettings, nullptr, nullptr);
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefLifeSpanHandler::OnBeforePopup              //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefLifeSpanHandler.html#OnBeforePopup(CefRefPtr%3CCefBrowser%3E,CefRefPtr%3CCefFrame%3E,constCefString&,constCefString&,constCefPopupFeatures&,CefWindowInfo&,CefRefPtr%3CCefClient%3E&,CefBrowserSettings&,bool*)
// //
//                                                                //
////////////////////////////////////////////////////////////////////

void WebView::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    m_pWebView = nullptr;

    // Remove focused web view reference
    if (WebManager::instance().GetFocusedWebView() == this)
        WebManager::instance().SetFocusedWebView(nullptr);
}

bool WebView::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString &target_url,
                            const CefString &target_frame_name, WindowOpenDisposition target_disposition,
                            bool user_gesture,
                            const CefPopupFeatures &popupFeatures, CefWindowInfo &windowInfo,
                            CefRefPtr<CefClient> &client,
                            CefBrowserSettings &settings, CefRefPtr<CefDictionaryValue> &extra_info,
                            bool *no_javascript_access)
{
    // ATTENTION: This method is called on the IO thread

    // Trigger the popup/new tab event
    std::string strTagetURL = target_url.ToString();
    std::string strOpenerURL = frame->GetURL().ToString();

    // Queue event to run on the main thread
    WebManager::instance().NotifyPopup(strTagetURL, strOpenerURL);

    // Block popups generally
    return true;
}

void WebView::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    // Set web view reference
    m_pWebView = browser;
}

void WebView::CloseBrowser()
{
    // CefBrowserHost::CloseBrowser calls the destructor after the browser has been destroyed
    m_bBeingDestroyed = true;

    // Make sure we don't dead lock the CEF render thread
    m_RenderData.cv.notify_all();

    if (m_pWebView)
        m_pWebView->GetHost()->CloseBrowser(true);
}

void WebView::OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType paintType,
                      const CefRenderHandler::RectList &dirtyRects, const void *buffer, int width, int height)
{
    if (m_bBeingDestroyed)
        return;

    {
        std::lock_guard<std::mutex> lock(m_RenderData.dataMutex);

        // Copy popup buffer
        if (paintType == PET_POPUP)
        {
            memcpy(m_RenderData.popupBuffer.get(), buffer, width * height * 4);
            return; // We don't have to wait as we've copied the buffer already
        }

        // Store render data
        m_RenderData.buffer = buffer;
        m_RenderData.width = width;
        m_RenderData.height = height;
        m_RenderData.dirtyRects = dirtyRects;
        m_RenderData.changed = true;
    }

    // Wait for the main thread to handle drawing the texture
    std::unique_lock<std::mutex> lock(m_RenderData.cvMutex);
    m_RenderData.cv.wait(lock);
}

void WebView::OnBeforeContextMenu(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model)
{
    model->Clear();
}

void WebView::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
    if (m_bBeingDestroyed)
        return;

    rect.x = 0;
    rect.y = 0;
    rect.width = m_texture->GetWidth();
    rect.height = m_texture->GetHeight();

    return;
}

void WebView::UpdateTexture()
{
    if (!m_pWebView)
        return;
    if (!m_texture)
        return;

    std::lock_guard<std::mutex> lock(m_RenderData.dataMutex);

    auto pSurface = m_texture->GetD3DTexture();
    if (m_bBeingDestroyed || !pSurface)
        return;

    // Discard current buffer if size doesn't match
    // This happens when resizing the browser as OnPaint is called asynchronously
    //if (m_RenderData.changed && (m_texture->GetWidth() != m_RenderData.width || m_texture->GetWidth() != m_RenderData.height))
    //	m_RenderData.changed = false;

    if (m_RenderData.changed || m_RenderData.popupShown)
    {
        D3DLOCKED_RECT LockedRect;
        pSurface->LockRect(0, &LockedRect, NULL, D3DLOCK_DISCARD);

        // Dirty rect implementation, don't use this as loops are significantly slower than memcpy
        auto surfaceData = static_cast<byte *>(LockedRect.pBits);
        auto sourceData = static_cast<const byte *>(m_RenderData.buffer);
        auto pitch = LockedRect.Pitch;

        // Update view area
        if (m_RenderData.changed)
        {
            // Update changed state
            m_RenderData.changed = false;

            if (m_RenderData.dirtyRects.size() > 0 && m_RenderData.dirtyRects[0].width == m_RenderData.width &&
                m_RenderData.dirtyRects[0].height == m_RenderData.height)
            {
                // Update whole texture
                memcpy(surfaceData, sourceData, m_RenderData.width * m_RenderData.height * 4);
            }
            else
            {
                // Update dirty rects
                for (auto &rect : m_RenderData.dirtyRects)
                {
                    for (int y = rect.y; y < rect.y + rect.height; ++y)
                    {
                        int index = y * pitch + rect.x * 4;
                        memcpy(&surfaceData[index], &sourceData[index], rect.width * 4);
                    }
                }
            }
        }

        // Update popup area (override certain areas of the view texture)
        bool popupSizeMismatches = m_RenderData.popupRect.x + m_RenderData.popupRect.width >= (int)m_texture->GetWidth()
                                   ||
                                   m_RenderData.popupRect.y + m_RenderData.popupRect.height >= (int)m_texture->
                                   GetHeight();

        if (m_RenderData.popupShown && !popupSizeMismatches)
        {
            auto popupPitch = m_RenderData.popupRect.width * 4;
            for (int y = 0; y < m_RenderData.popupRect.height; ++y)
            {
                int sourceIndex = y * popupPitch;
                int destIndex = (y + m_RenderData.popupRect.y) * pitch + m_RenderData.popupRect.x * 4;

                memcpy(&surfaceData[destIndex], &m_RenderData.popupBuffer[sourceIndex], popupPitch);
            }
        }

        // Unlock surface
        pSurface->UnlockRect(0);
    }

    // Resume CEF render thread
    m_RenderData.cv.notify_all();
}

void WebView::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
    std::lock_guard<std::mutex> lock{m_RenderData.dataMutex};
    m_RenderData.popupShown = show;

    // Free popup buffer memory if hidden
    if (!show)
        m_RenderData.popupBuffer.reset();
}

////////////////////////////////////////////////////////////////////
//                                                                //
// Implementation: CefRenderHandler::OnPopupSize                  //
// http://magpcss.org/ceforum/apidocs3/projects/(default)/CefRenderHandler.html#OnPopupSize(CefRefPtr<CefBrowser>,constCefRect&) //
//                                                                //
////////////////////////////////////////////////////////////////////
void WebView::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect &rect)
{
    std::lock_guard<std::mutex> lock{m_RenderData.dataMutex};

    // Update rect
    m_RenderData.popupRect = rect;

    // Resize buffer
    m_RenderData.popupBuffer.reset(new byte[rect.width * rect.height * 4]);
}

void WebView::Focus(bool state)
{
    if (m_pWebView)
    {
        m_pWebView->GetHost()->SetFocus(state);
        m_pWebView->GetHost()->SendFocusEvent(state);
    }

    if (state)
        WebManager::instance().SetFocusedWebView(this);
    else if (WebManager::instance().GetFocusedWebView() == this)
        WebManager::instance().SetFocusedWebView(nullptr);
}

CGraphicImageTexture *WebView::GetTexture() const
{
    return m_texture;
}

void WebView::InjectMouseMove(int iPosX, int iPosY)
{
    if (!m_pWebView)
        return;

    CefMouseEvent mouseEvent;
    mouseEvent.x = iPosX;
    mouseEvent.y = iPosY;

    // Set modifiers from mouse states (yeah, using enum values as indices isn't best practise, but it's the easiest solution here)
    if (m_mouseButtonStates[BROWSER_MOUSEBUTTON_LEFT])
        mouseEvent.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
    if (m_mouseButtonStates[BROWSER_MOUSEBUTTON_MIDDLE])
        mouseEvent.modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
    if (m_mouseButtonStates[BROWSER_MOUSEBUTTON_RIGHT])
        mouseEvent.modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;

    m_pWebView->GetHost()->SendMouseMoveEvent(mouseEvent, false);

    m_vecMousePosition.x = iPosX;
    m_vecMousePosition.y = iPosY;
}

void WebView::InjectMouseDown(eWebBrowserMouseButton mouseButton)
{
    if (!m_pWebView)
        return;

    CefMouseEvent mouseEvent;
    mouseEvent.x = m_vecMousePosition.x;
    mouseEvent.y = m_vecMousePosition.y;

    // Save mouse button states
    m_mouseButtonStates[static_cast<int>(mouseButton)] = true;

    m_pWebView->GetHost()->SendMouseClickEvent(mouseEvent, static_cast<CefBrowserHost::MouseButtonType>(mouseButton),
                                               false, 1);
}

void WebView::InjectMouseUp(eWebBrowserMouseButton mouseButton)
{
    if (!m_pWebView)
        return;

    CefMouseEvent mouseEvent;
    mouseEvent.x = m_vecMousePosition.x;
    mouseEvent.y = m_vecMousePosition.y;

    // Save mouse button states
    m_mouseButtonStates[static_cast<int>(mouseButton)] = false;

    m_pWebView->GetHost()->SendMouseClickEvent(mouseEvent, static_cast<CefBrowserHost::MouseButtonType>(mouseButton),
                                               true, 1);
}

void WebView::InjectMouseWheel(int iScrollVert, int iScrollHorz)
{
    if (!m_pWebView)
        return;

    CefMouseEvent mouseEvent;
    mouseEvent.x = m_vecMousePosition.x;
    mouseEvent.y = m_vecMousePosition.y;

    m_pWebView->GetHost()->SendMouseWheelEvent(mouseEvent, iScrollVert, iScrollHorz) ;
}

void WebView::InjectKeyboardEvent(const CefKeyEvent &keyEvent)
{
    if (m_pWebView)
        m_pWebView->GetHost()->SendKeyEvent(keyEvent);
}

void WebView::ClearTexture() const
{
    if (!m_texture)
        return;

    IDirect3DSurface9 *pD3DSurface;
    m_texture->GetD3DTexture()->GetSurfaceLevel(0, &pD3DSurface);
    if (!pD3DSurface)
        return;

    D3DLOCKED_RECT LockedRect;
    D3DSURFACE_DESC SurfaceDesc;

    pD3DSurface->GetDesc(&SurfaceDesc);
    pD3DSurface->LockRect(&LockedRect, NULL, 0);

    memset(LockedRect.pBits, 0xFF, SurfaceDesc.Width * SurfaceDesc.Height * 4);
    pD3DSurface->UnlockRect();
}

void WebView::LoadUrl(const char *url)
{
    if (!m_pWebView)
        return;

    auto pFrame = m_pWebView->GetMainFrame();
    pFrame->LoadURL(url);
}
