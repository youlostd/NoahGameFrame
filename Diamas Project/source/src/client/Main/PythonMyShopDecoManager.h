#ifndef METIN2_CLIENT_MAIN_PYTHONMYSHOPDECOMANAGER_H
#define METIN2_CLIENT_MAIN_PYTHONMYSHOPDECOMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "InstanceBase.h"

class CPythonMyShopDecoManager : public CGraphicBase, public CSingleton<CPythonMyShopDecoManager>
{
    using TCharacterInstanceMap = std::map<uint32_t, CInstanceBase *>;

public:
    CPythonMyShopDecoManager();
    virtual ~CPythonMyShopDecoManager();

    void __Initialize();
    void Destroy();
    CInstanceBase *GetInstancePtr(uint32_t VirtualID);

    bool CreateModelInstance(uint32_t index);
    void SelectModel(uint32_t index);
    bool CreateModelBackground(uint32_t width, uint32_t height);
    void RenderBackground();
    void Update();
    void Deform();
    void RenderModel();

private:
    CInstanceBase *m_pModel;
    std::unique_ptr<CGraphicImageInstance> m_bgImage;
    TCharacterInstanceMap m_mapInstances;
    float m_modelRotation;

public:
    bool m_isShow;

    bool IsShow() const
    {
        return m_isShow;
    }
};
#endif /* METIN2_CLIENT_MAIN_PYTHONMYSHOPDECOMANAGER_H */
