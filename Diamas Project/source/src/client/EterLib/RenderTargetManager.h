#pragma once
#include <unordered_map>
#include "GrpBase.h"
#include "GrpScreen.h"
#include <base/Singleton.hpp>
#include "GrpRenderTargetTexture.h"

class CRenderTargetManager : public CGraphicBase, public CSingleton<CRenderTargetManager>
{
public:
    enum ERenderTargets
    {
        ILLUSTRADED_BG,
        MYSHOP_DECO,
        RENDER_TARGET_ITEM,
        RENDER_TARGET_MAX,
    };

    CRenderTargetManager();
    virtual ~CRenderTargetManager();

    bool CreateRenderTarget(int width, int height);
    bool CreateRenderTargetWithIndex(int width, int height, uint32_t index);
    CGraphicRenderTargetTexture *CreateWikiRenderTarget(DWORD index, DWORD width, DWORD height);

    bool GetRenderTargetRect(uint32_t index, RECT &rect);
    bool GetWikiRenderTargetRect(DWORD index, RECT &rect);
    bool ChangeRenderTarget(uint32_t index);
    bool ChangeWikiRenderTarget(DWORD index);

    void CreateRenderTargetTextures();
    void ReleaseRenderTargetTextures();

    CGraphicRenderTargetTexture *GetRenderTarget(uint32_t index);
    CGraphicRenderTargetTexture *GetWikiRenderTarget(DWORD index);

    void ResetRenderTarget();
    void ClearRenderTarget(uint32_t color = D3DCOLOR_ARGB(0, 0, 0, 0)) const;
    void Destroy();
private:

    bool CreateGraphicTexture(uint32_t index, uint32_t width, uint32_t height, D3DFORMAT texFormat,
                              D3DFORMAT dephtFormat);

    std::unordered_map<uint32_t, CGraphicRenderTargetTexture *> m_renderTargets;
    std::unordered_map<uint32_t, CGraphicRenderTargetTexture *> m_wikiRenderTargets;

    uint32_t m_widht;
    uint32_t m_height;
    bool m_isShow;
    CGraphicRenderTargetTexture *m_currentRenderTarget;
};
