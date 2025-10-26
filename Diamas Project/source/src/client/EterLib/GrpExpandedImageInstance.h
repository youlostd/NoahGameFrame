#pragma once

#include "GrpImageInstance.h"

class CGraphicExpandedImageInstance : public CGraphicImageInstance
{
public:
    static DWORD Type();

    enum ERenderingMode
    {
        RENDERING_MODE_NORMAL,
        RENDERING_MODE_SCREEN,
        RENDERING_MODE_COLOR_DODGE,
        RENDERING_MODE_MODULATE,
    };

public:
    CGraphicExpandedImageInstance() = default;
    explicit CGraphicExpandedImageInstance(std::string_view filename);
    virtual ~CGraphicExpandedImageInstance();

    void Destroy() override;

    void SetDepth(float fDepth);
    void SetOrigin();
    void SetOrigin(float fx, float fy);
    void SetRotation(float fRotation);
    void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
    void SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom);
    void SetRenderingMode(int iMode);
    void OnRenderWithCoolTimeBox(float fTime) override;

protected:
    void OnRender() override;
    void OnSetImagePointer() override;
    bool OnIsType(DWORD dwType) override;

protected:
    float m_fDepth{};
    Vector2 m_v2Origin{};
    float m_fRotation{};
    RECT m_RenderingRect{};
    int m_iRenderingMode{RENDERING_MODE_NORMAL};
};
