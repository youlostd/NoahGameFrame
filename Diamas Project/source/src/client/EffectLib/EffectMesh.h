#ifndef METIN2_CLIENT_EFFECTLIB_EFFECTMESH_H
#define METIN2_CLIENT_EFFECTLIB_EFFECTMESH_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../eterlib/GrpScreen.h"
#include "../eterlib/Resource.h"
#include "../eterlib/GrpImageInstance.h"
#include "../eterLib/TextFileLoader.h"

#include "Type.h"
#include "EffectElementBase.h"
#include <base/SimpleMath.h>

class CEffectMesh final : public ConcreteResource<CEffectMesh>
{
public:
    typedef struct SEffectFrameData
    {
        uint8_t byChangedFrame;
        float fVisibility;
        uint32_t dwVertexCount;
        uint32_t dwTextureVertexCount;
        uint32_t dwIndexCount;
        std::vector<TPTVertex> PTVertexVector;
    } TEffectFrameData;

    typedef struct SEffectMeshData
    {
        char szObjectName[32];
        char szDiffuseMapFileName[128];

        std::vector<TEffectFrameData> EffectFrameDataVector;
        std::vector<CGraphicImage::Ptr> pImageVector;

        static SEffectMeshData *New();
        static void Delete(SEffectMeshData *pkData);

        static void DestroySystem();

        static CDynamicPool<SEffectMeshData> ms_kPool;
    } TEffectMeshData;

    CEffectMesh();
    virtual ~CEffectMesh();

    virtual void Clear();

    virtual bool Load(int iSize, const void *c_pvBuf);

    uint32_t GetFrameCount();
    uint32_t GetMeshCount();
    TEffectMeshData *GetMeshDataPointer(uint32_t dwMeshIndex);

    std::vector<CGraphicImage::Ptr> *GetTextureVectorPointer(uint32_t dwMeshIndex);
    std::vector<CGraphicImage::Ptr> &GetTextureVectorReference(uint32_t dwMeshIndex);

    // Exceptional function for tool
    bool GetMeshElementPointer(uint32_t dwMeshIndex, TEffectMeshData **ppMeshData);

protected:
    bool OnLoad(int iSize, const void *c_pvBuf);

    bool __LoadData_Ver001(int iSize, const uint8_t *c_pbBuf);
    bool __LoadData_Ver002(int iSize, const uint8_t *c_pbBuf);

protected:
    int m_iGeomCount;
    int m_iFrameCount;
    std::vector<TEffectMeshData *> m_pEffectMeshDataVector;

    bool m_isData;
};

class CEffectMeshScript : public CEffectElementBase
{
public:
    typedef struct SMeshData
    {
        uint8_t byBillboardType;

        bool bBlendingEnable;
        uint8_t byBlendingSrcType;
        uint8_t byBlendingDestType;
        bool bTextureAlphaEnable;

        uint8_t byColorOperationType;
        DirectX::SimpleMath::Color ColorFactor;

        bool bTextureAnimationLoopEnable;
        float fTextureAnimationFrameDelay;

        uint32_t dwTextureAnimationStartFrame;

        TTimeEventTableFloat TimeEventAlpha;

        SMeshData()
        {
            TimeEventAlpha.clear();
        }
    } TMeshData;

    typedef std::vector<TMeshData> TMeshDataVector;

public:
    CEffectMeshScript();
    virtual ~CEffectMeshScript();

    const char *GetMeshFileName();

    void ReserveMeshData(uint32_t dwMeshCount);
    bool CheckMeshIndex(uint32_t dwMeshIndex);
    bool GetMeshDataPointer(uint32_t dwMeshIndex, TMeshData **ppMeshData);
    int GetMeshDataCount();

    int GetBillboardType(uint32_t dwMeshIndex);
    bool isBlendingEnable(uint32_t dwMeshIndex);
    uint8_t GetBlendingSrcType(uint32_t dwMeshIndex);
    uint8_t GetBlendingDestType(uint32_t dwMeshIndex);
    bool isTextureAlphaEnable(uint32_t dwMeshIndex);
    bool GetColorOperationType(uint32_t dwMeshIndex, uint8_t *pbyType);
    bool GetColorFactor(uint32_t dwMeshIndex, DirectX::SimpleMath::Color *pColor);
    bool GetTimeTableAlphaPointer(uint32_t dwMeshIndex, TTimeEventTableFloat **pTimeEventAlpha);

    bool isMeshAnimationLoop();
    int GetMeshAnimationLoopCount();
    float GetMeshAnimationFrameDelay();
    bool isTextureAnimationLoop(uint32_t dwMeshIndex);
    float GetTextureAnimationFrameDelay(uint32_t dwMeshIndex);
    uint32_t GetTextureAnimationStartFrame(uint32_t dwMeshIndex);

protected:
    void OnClear();
    bool OnIsData();
    bool OnLoadScript(CTextFileLoader &rTextFileLoader);

protected:
    int m_isMeshAnimationLoop;
    int m_iMeshAnimationLoopCount;
    float m_fMeshAnimationFrameDelay;
    TMeshDataVector m_MeshDataVector;

    std::string m_strMeshFileName;

public:
    static void DestroySystem();

    static CEffectMeshScript *New();
    static void Delete(CEffectMeshScript *pkData);

    static CDynamicPool<CEffectMeshScript> ms_kPool;
};
#endif /* METIN2_CLIENT_EFFECTLIB_EFFECTMESH_H */
