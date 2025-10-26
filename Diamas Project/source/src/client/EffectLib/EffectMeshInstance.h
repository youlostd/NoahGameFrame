#ifndef METIN2_CLIENT_EFFECTLIB_EFFECTMESHINSTANCE_H
#define METIN2_CLIENT_EFFECTLIB_EFFECTMESHINSTANCE_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../eterlib/GrpScreen.h"
#include "../eterlib/GrpImageInstance.h"
#include "EffectElementBaseInstance.h"
#include "FrameController.h"
#include "EffectMesh.h"

class CEffectMeshInstance final : public CEffectElementBaseInstance
{
public:
    // NOTE : Mesh 단위 텍스춰 데이타의 인스턴스이다.
    typedef struct STextureInstance
    {
        CFrameController TextureFrameController;
        std::vector<std::unique_ptr<CGraphicImageInstance>> TextureInstanceVector;
    } TTextureInstance;

public:
    CEffectMeshInstance();
    virtual ~CEffectMeshInstance();

public:
    static void DestroySystem();

    static CEffectMeshInstance *New();
    static void Delete(CEffectMeshInstance *pkMeshInstance);

    static CDynamicPool<CEffectMeshInstance> ms_kPool;
    bool isActive();

protected:
    void OnSetDataPointer(CEffectElementBase *pElement);

    void OnInitialize();
    void OnDestroy();

    bool OnUpdate(double fElapsedTime);
    void OnRender(uint32_t addColor);

protected:
    CEffectMeshScript *m_pMeshScript;
    CEffectMesh::Ptr m_pEffectMesh;

    CFrameController m_MeshFrameController;
    std::vector<TTextureInstance> m_TextureInstanceVector;
};
#endif /* METIN2_CLIENT_EFFECTLIB_EFFECTMESHINSTANCE_H */
