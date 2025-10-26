#pragma once

#include "GrpImage.h"
#include "GrpIndexBuffer.h"
#include "Pool.h"

class CGraphicImageInstance : public CGraphicBase
{
public:
    static DWORD Type();
    bool IsType(DWORD dwType);

public:
    CGraphicImageInstance() = default;
    explicit CGraphicImageInstance(std::string_view filename);
    virtual ~CGraphicImageInstance();

    virtual void Destroy();

    void Render();
    void SetScalePivotCenter(bool bScalePivotCenter);

    void SetDiffuseColor(float fr, float fg, float fb, float fa);
    void SetPosition(float fx, float fy);
    void SetInverse();
    void SetScale(float fx, float fy);
    void SetScale(Vector2 scale);

    void SetImagePointer(CGraphicImage::Ptr r);
    void SetFilename(const std::string &r);
    void ReloadImagePointer(CGraphicImage::Ptr pImage);

    bool IsEmpty() const;

    int GetWidth() const;
    int GetHeight() const;
    void SetDiffuseColor(const Color &color);
    const Vector2& GetScale() const { return m_v2Scale; }

    CGraphicTexture *GetTexturePointer() const;
    const CGraphicTexture &GetTextureReference() const;
    CGraphicImage::Ptr GetGraphicImagePointer() const;

    bool operator ==(const CGraphicImageInstance &rhs) const;
    virtual void OnRenderWithCoolTimeBox(float fTime);

protected:

    virtual void OnRender();
    virtual void OnSetImagePointer();

    virtual bool OnIsType(DWORD dwType);

    DirectX::SimpleMath::Color m_DiffuseColor{1.0f, 1.0f, 1.0f, 1.0f};
    Vector2                    m_v2Position{};
    bool                       m_bIsInverse{};
    bool m_bScalePivotCenter{};

    CGraphicImage::Ptr         m_roImage;
    Vector2                    m_v2Scale{1.0f, 1.0f};
    std::string                m_fileName;
};
