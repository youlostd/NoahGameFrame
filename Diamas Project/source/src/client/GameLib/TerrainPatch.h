// TerrainPatch.h: interface for the CTerrainPatch class.
//
//////////////////////////////////////////////////////////////////////

#ifndef METIN2_CLIENT_GAMELIB_TERRAINPATCH_HPP
#define METIN2_CLIENT_GAMELIB_TERRAINPATCH_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../PRTerrainLib/Terrain.h"
#include "../eterlib/GrpVertexBuffer.h"

#pragma pack(push)
#pragma pack(1)

struct HardwareTransformPatch_SSourceVertex
{
    Vector3 kPosition;
    Vector3 kNormal;
};

struct SWaterVertex
{
    float x, y, z; // position
    uint32_t dwDiffuse;
};
#pragma pack(pop)

class CTerrainPatch
{
  public:
    enum
    {
        PATCH_TYPE_PLAIN = 0,
        PATCH_TYPE_HILL,
        PATCH_TYPE_CLIFF,
    };

    enum
    {
        TERRAIN_VERTEX_COUNT = (CTerrainImpl::PATCH_XSIZE + 1) * (CTerrainImpl::PATCH_YSIZE + 1)
    };

  public:
    CTerrainPatch()
    {
        m_kVB.idx = kInvalidHandle;
        m_WaterVertexBuffer.idx = kInvalidHandle;
        Clear();
    }
    ~CTerrainPatch()
    {
        Clear();
    }

    void Clear();

    void ClearID()
    {
        SetID(0xFFFFFFFF);
    }

    void SetMinX(float fMinX)
    {
        m_fMinX = fMinX;
    }
    float GetMinX()
    {
        return m_fMinX;
    }

    void SetMaxX(float fMaxX)
    {
        m_fMaxX = fMaxX;
    }
    float GetMaxX()
    {
        return m_fMaxX;
    }

    void SetMinY(float fMinY)
    {
        m_fMinY = fMinY;
    }
    float GetMinY()
    {
        return m_fMinY;
    }

    void SetMaxY(float fMaxY)
    {
        m_fMaxY = fMaxY;
    }
    float GetMaxY()
    {
        return m_fMaxY;
    }

    void SetMinZ(float fMinZ)
    {
        m_fMinZ = fMinZ;
    }
    float GetMinZ()
    {
        return m_fMinZ;
    }

    void SetMaxZ(float fMaxZ)
    {
        m_fMaxZ = fMaxZ;
    }
    float GetMaxZ()
    {
        return m_fMaxZ;
    }

    bool IsUse()
    {
        return m_bUse;
    }
    void SetUse(bool bUse)
    {
        m_bUse = bUse;
    }

    bool IsWaterExist()
    {
        return m_bWaterExist;
    }
    void SetWaterExist(bool bWaterExist)
    {
        m_bWaterExist = bWaterExist;
    }

    uint32_t GetID()
    {
        return m_dwID;
    }
    void SetID(uint32_t dwID)
    {
        m_dwID = dwID;
    }

    void SetType(uint8_t byType)
    {
        m_byType = byType;
    }
    uint8_t GetType()
    {
        return m_byType;
    }

    void NeedUpdate(bool bNeedUpdate)
    {
        m_bNeedUpdate = bNeedUpdate;
    }
    bool NeedUpdate()
    {
        return m_bNeedUpdate;
    }

    UINT GetWaterFaceCount();

    void BuildTerrainVertexBuffer(HardwareTransformPatch_SSourceVertex *akSrcVertex);
    void BuildWaterVertexBuffer(SWaterVertex *akSrcVertex, UINT uWaterVertexCount);

  protected:
    void __BuildHardwareTerrainVertexBuffer(HardwareTransformPatch_SSourceVertex *akSrcVertex);

  private:
    float m_fMinX;
    float m_fMaxX;
    float m_fMinY;
    float m_fMaxY;
    float m_fMinZ;
    float m_fMaxZ;
    bool m_bUse;
    bool m_bWaterExist;
    uint32_t m_dwID;
    uint32_t m_dwWaterPriCount;

    VertexBufferHandle m_WaterVertexBuffer;
    uint8_t m_byType;

    bool m_bNeedUpdate;
    uint32_t m_dwVersion;

  public:
    VertexBufferHandle GetWaterVertexBufferPointer()
    {
        return m_WaterVertexBuffer;
    }

  public:
    VertexBufferHandle HardwareTransformPatch_GetVertexBufferPtr()
    {
        return m_kVB;
    }

  protected:
    VertexBufferHandle m_kVB;
};

class CTerrainPatchProxy
{
  public:
    CTerrainPatchProxy();
    virtual ~CTerrainPatchProxy();

    void Clear();

    void SetCenterPosition(const Vector3 &c_rv3Center);

    bool IsIn(const Vector3 &c_rv3Target, float fRadius);

    bool isUsed()
    {
        return m_bUsed;
    }
    void SetUsed(bool bUsed)
    {
        m_bUsed = bUsed;
    }

    short GetPatchNum()
    {
        return m_sPatchNum;
    }
    void SetPatchNum(short sPatchNum)
    {
        m_sPatchNum = sPatchNum;
    }

    uint8_t GetTerrainNum()
    {
        return m_byTerrainNum;
    }
    void SetTerrainNum(uint8_t byTerrainNum)
    {
        m_byTerrainNum = byTerrainNum;
    }

    void SetTerrainPatch(CTerrainPatch *pTerrainPatch)
    {
        m_pTerrainPatch = pTerrainPatch;
    }

    bool isWaterExists();

    UINT GetWaterFaceCount();

    float GetMinX();
    float GetMaxX();
    float GetMinY();
    float GetMaxY();
    float GetMinZ();
    float GetMaxZ();

    // Vertex Buffer
    VertexBufferHandle GetWaterVertexBufferPointer();
    VertexBufferHandle HardwareTransformPatch_GetVertexBufferPtr();

  protected:
    bool m_bUsed;
    short m_sPatchNum; // Patch Number

    uint8_t m_byTerrainNum;

    CTerrainPatch *m_pTerrainPatch;

    Vector3 m_v3Center;
};

inline bool CTerrainPatchProxy::isWaterExists()
{
    return m_pTerrainPatch->IsWaterExist();
}

inline float CTerrainPatchProxy::GetMinX()
{
    return m_pTerrainPatch->GetMinX();
}

inline float CTerrainPatchProxy::GetMaxX()
{
    return m_pTerrainPatch->GetMaxX();
}

inline float CTerrainPatchProxy::GetMinY()
{
    return m_pTerrainPatch->GetMinY();
}

inline float CTerrainPatchProxy::GetMaxY()
{
    return m_pTerrainPatch->GetMaxY();
}

inline float CTerrainPatchProxy::GetMinZ()
{
    return m_pTerrainPatch->GetMinZ();
}

inline float CTerrainPatchProxy::GetMaxZ()
{
    return m_pTerrainPatch->GetMaxZ();
}

inline VertexBufferHandle CTerrainPatchProxy::GetWaterVertexBufferPointer()
{
    return m_pTerrainPatch->GetWaterVertexBufferPointer();
}

#endif
