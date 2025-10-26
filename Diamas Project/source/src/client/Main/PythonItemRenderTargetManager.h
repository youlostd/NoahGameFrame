#ifndef METIN2_CLIENT_MAIN_PYTHONITEMRENDERTARGETMANAGER_H
#define METIN2_CLIENT_MAIN_PYTHONITEMRENDERTARGETMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "InstanceBase.h"

class CPythonItemRenderTargetManager : public CGraphicBase, public CSingleton<CPythonItemRenderTargetManager>
{
public:
    CPythonItemRenderTargetManager();
    virtual ~CPythonItemRenderTargetManager();

    void __Initialize();
    void Destroy();

    bool SetModelInstance(CInstanceBase::SCreateData &createData);
    void Select(CInstanceBase::SCreateData &createData);
    bool CreateModelBackground(uint32_t width, uint32_t height);
    void RenderBackground();
    void Drag(int32_t x, int32_t y);
    void Update();
    void Deform();
    void RenderModel();

    void SetFov(float fov)
    {
        m_fov = fov;
    }

    void SetTargetZ(float y)
    {
        m_targetY = y;
    }

    void EnableRotation(bool val)
    {
        m_enableRotation = val;
    }

    void SetModelZoom(bool zoom);

    void SetRotSpeed(float speed)
    {
        m_rotSpeed = speed;
    }

    void SetBaseRot(float rot)
    {
        m_baseRotation = rot;
    }

    void Show()
    {
        m_isShow = true;
    }

    void Hide()
    {
        m_isShow = false;
    }

private:
    std::unique_ptr<CInstanceBase> m_pModel;
    std::unique_ptr<CGraphicExpandedImageInstance> m_bgImage;
    float m_modelRotation;
    float m_fov = 10.0f;
    float m_targetY = 0.0f;
    bool m_enableRotation = true;
    float m_zoom = 0.0f;
    float m_minZoom = 0.0f;
    float m_fModelZoomSpeed = 0.0f;
    float m_rotSpeed = 100.0f;
    float m_baseRotation = 0.0f;
    float m_modelHeight = 0.0f;
    bool m_isShow = false;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONITEMRENDERTARGETMANAGER_H */
