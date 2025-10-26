#ifndef METIN2_CLIENT_ETERLIB_GRPDEVICE_HPP
#define METIN2_CLIENT_ETERLIB_GRPDEVICE_HPP

#pragma once

#include "GrpBase.h"
#include "GrpDetector.h"

#include "Handle.h"
#include "StateManager.h"

#include <Utils.h>
#include <bx/handlealloc.h>
#include "SharedVertexBuffer.hpp"

class GrpShaderFXPool;
class GrpShaderFX;

enum AttribType
{
    TYPE_GENERIC = 0,
    TYPE_VERTEX = 1,    /**< POSITION */
    TYPE_NORMAL = 2,    /**< NORMAL */
    TYPE_TEXCOORD = 3,  /**< TEXCOORD<i>n</i> */
    TYPE_COLOR = 4,     /**< COLOR<i>n</i> */
    TYPE_TANGENT = 5,   /**< TANGENT */
    TYPE_BINORMAL = 6,  /**< BINORMAL */
    TYPE_POSITIONT = 7, /**< POSITION x,y,z,rhw */
    TYPE_BLENDINDICES = 8,
    TYPE_BLENDWEIGHT = 9,
};

/**
    Vertex attribute format enumeration.  These values are used as input to
    constructing vertex or stream attributes; they map 1:1 with, for example,
    D3DDECLTYPE_<FORMAT> in Direct3D and the 'type' parameter to gl<ARRAY>Pointer()
    in OpenGL.
*/
enum AttribFormat
{
    FORMAT_FLOAT = 0,    /**< FLOAT<i>n</i> */
    FORMAT_UBYTE = 1,    /**< UBYTE */
    FORMAT_UBYTEUN = 2,  /**< UBYTEUN */
    FORMAT_SHORT = 3,    /**< SHORT */
    FORMAT_USHORT = 4,   /**< USHORT */
    FORMAT_SHORTUN = 5,  /**< SHORTUN */
    FORMAT_HALF = 6,     /**< HALF */
    FORMAT_DEC = 7,      /**< DEC */
    FORMAT_UDECUN = 8,   /**< UDECUN */
    FORMAT_D3DCOLOR = 9, /**< D3DCOLOR */
};

struct GraphicVertexAttribute
{
    AttribType type;     /**< Specifies the vertex type. */
    AttribFormat format; /**< Specifies the vertex format (float, bool, int, etc). */
    int size; /**< Specifies the vertex format size (3 for (type)3, for example, where type is float and size is 3, the
                 attribute is a float3). */
};

struct GraphicVertexAttributeEx
{
    GraphicVertexAttributeEx() : stream(0), type(TYPE_GENERIC), usage(0), format(FORMAT_FLOAT), size(0)
    {
    }
    GraphicVertexAttributeEx(int s, AttribType t, int u, AttribFormat f, int z)
        : stream(s), type(t), usage(u), format(f), size(z)
    {
    }
    int stream;          // stream
    AttribType type;     /**< Specifies the vertex type. */
    int usage;           // usage
    AttribFormat format; /**< Specifies the vertex format (float, bool, int, etc). */
    int size; /**< Specifies the vertex format size (3 for (type)3, for example, where type is float and size is 3, the
                 attribute is a float3). */
};

struct GraphicVertexFormat
{
    LPDIRECT3DVERTEXDECLARATION9 vDecl;
    unsigned int vertexSize[4];
};
const int MAX_RENDERTARGET = 4;

struct NewGraphicVertexBuffer
{
    LPDIRECT3DVERTEXBUFFER9 m_ptr;
    uint32_t size;
    DWORD flags;
    uint8_t vertexSize;

    void Destroy();
    void Invalidate();
    bool Restore(LPDIRECT3DDEVICE9 pd3dDevice);
    bool IsDynamic()
    {
        return IS_SET(flags, D3DUSAGE_DYNAMIC);
    }
};

struct NewGraphicIndexBuffer
{
    LPDIRECT3DINDEXBUFFER9 indexBuffer;
    uint32_t nIndices;
    uint8_t indexSize;
    DWORD flags;

    void Invalidate();
    bool Restore(LPDIRECT3DDEVICE9 pd3dDevice);
    bool IsDynamic()
    {
        return IS_SET(flags, D3DUSAGE_DYNAMIC);
    }
};

class CGraphicDevice : public CGraphicBase
{
  public:
    enum EDeviceState
    {
        DEVICESTATE_OK,
        DEVICESTATE_BROKEN,
        DEVICESTATE_NEEDS_RESET,
        DEVICESTATE_NULL
    };

    enum ECreateReturnValues
    {
        CREATE_OK = (1 << 0),
        CREATE_NO_DIRECTX = (1 << 1),
        CREATE_GET_DEVICE_CAPS = (1 << 2),
        CREATE_GET_DEVICE_CAPS2 = (1 << 3),
        CREATE_DEVICE = (1 << 4),
        CREATE_REFRESHRATE = (1 << 5),
        CREATE_ENUM = (1 << 6),
        // 2003. 01. 09. myevan 모드 리스트 얻기 실패
        CREATE_DETECT = (1 << 7),
        // 2003. 01. 09. myevan 모드 선택 실패
        CREATE_NO_TNL = (1 << 8),
        CREATE_BAD_DRIVER = (1 << 9),
        CREATE_FORMAT = (1 << 10),
    };

    CGraphicDevice();
    virtual ~CGraphicDevice();

    void InitBackBufferCount(UINT uBackBufferCount);

    void Destroy();
    int Create(HWND hWnd, int hres, int vres, bool Windowed = true, int bit = 32, int ReflashRate = 0);

    void SetFvF(DWORD fvf);
    void ShaderOff();
    void DrawPDTVertices(D3DPRIMITIVETYPE primitiveType, unsigned nPrimitiveCount, void* pVertex,
                         int              TheNumberOfVertexData);
    void DrawTextVertices(D3DPRIMITIVETYPE primitiveType, unsigned nPrimitiveCount, void* pVertex,
                          int              TheNumberOfVertexData);
    HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
    HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void *pVertexStreamZeroData,
                            UINT VertexStreamZeroStride);
    HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices, UINT startIndex,
                                 UINT primCount, INT baseVertexIndex = 0);
    HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices,
                                   UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat,
                                   CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride);
    // void *LockIndexBuffer(IndexBufferHandle hIndexBuffer, unsigned flags = 0, unsigned nOffsetToLock = 0, unsigned
    // nSizeToLock = 0); bool UnlockIndexBuffer(IndexBufferHandle hIndexBuffer); VertexBufferHandle
    // CreateVertexBuffer(long size, int nVertexSize, const void *data, unsigned flags); void
    // DeleteVertexBuffer(VertexBufferHandle hVertexBuffer); void DeleteIndexBuffer(IndexBufferHandle hIndexBuffer);
    // void SetVertexBuffer(VertexBufferHandle hVertexBuffer, int stream, unsigned offset);
    // void SetIndexBuffer(IndexBufferHandle hIndexBuffer);

    VertexBufferHandle CreateVertexBuffer(const long size, int nVertexSize, const void *data = NULL,
                                          unsigned int flags = 0);
    void DeleteVertexBuffer(VertexBufferHandle& hVertexBuffer);
    void *LockVertexBuffer(const VertexBufferHandle hVertexBuffer, const unsigned int flags = 0,
                           const unsigned int nOffsetToLock = 0, const unsigned int nSizeToLock = 0);
    bool UnlockVertexBuffer(const VertexBufferHandle hVertexBuffer);

    VertexFormatHandle CreateVertexFormat(const GraphicVertexAttribute *attribs, const int nAttribs,
                                          const int nStream = 0, const int nStartTexCoord = 0);
    VertexFormatHandle CreateVertexFormat(const GraphicVertexAttributeEx *attribs, const int nAttribs);
    IndexBufferHandle CreateIndexBuffer(const unsigned int nIndices, D3DFORMAT inf = D3DFMT_INDEX16,
                                        const void *data = NULL, unsigned int flags = 0);
    void DeleteIndexBuffer(IndexBufferHandle& hIndexBuffer);
    void *LockIndexBuffer(const IndexBufferHandle hIndexBuffer, const unsigned int flags = 0,
                          const unsigned int nOffsetToLock = 0, const unsigned int nSizeToLock = 0);
    bool UnlockIndexBuffer(const IndexBufferHandle hIndexBuffer);
    bool SetPDTStream(SPDTVertex *pVertices, UINT uVtxCount);
    bool SetPDTStream(SPDTVertexRaw *pSrcVertices, UINT uVtxCount);
    void SetVertexBuffer(VertexBufferHandle hVertexBuffer, const int stream, const unsigned offset);
    void SetIndexBuffer(IndexBufferHandle _handle);
    void SetVertexFormat(VertexFormatHandle hVertexFormat);

    void SetDepthFunc(D3DCMPFUNC depthFunc);
    void SetDepthEnable(bool bEnable, bool bWriteEnable);
    void Clear(bool bTarget = true, bool bDepth = true, bool bStencil = false, DWORD dwColor = 0, float fDepth = 1.f,
               DWORD dwStencil = 0, DWORD nIdx = 0);
    void SetVertexBufferFreq(UINT stream, UINT FrequencyParameter);
    void SetupForRenderInstancing(VertexBufferHandle hVertexBuffer, UINT nCnt, unsigned offset = 0);

    static EDeviceState GetDeviceState();
    static LPDIRECT3D9 GetDirectx9();
    GrpShaderFX *CreateShaderFX();
    GrpShaderFX *CreateShaderFX(const std::string &strFilename, const std::vector<std::string> &vShaderDefines,
                                GrpShaderFXPool *pShaderPool);
    void DeleteShaderFX(GrpShaderFX *shader);

    void RestoreForRenderInstancing();
    static LPDIRECT3DDEVICE9 GetDevice();

    void OnLostDevice();
    void OnRestoreDevice();
    bool Reset();

    void SetDefaultIndexBuffer(UINT eDefIB);

    bool ResizeBackBuffer(UINT uWidth, UINT uHeight);

  protected:
    void __Initialize();
    void InitDeviceDefault();

    void __InitializeDefaultIndexBufferList();
    void __DestroyDefaultIndexBufferList();
    bool __CreateDefaultIndexBufferList();
    bool __CreateDefaultIndexBuffer(UINT eDefIB, UINT uIdxCount, const WORD *c_awIndices);

    void __DestroyPDTVertexBufferList();

    bool __CreatePDTVertexBufferList();

    DWORD m_uBackBufferCount = 0;
    CStateManager *m_pStateManager{};
    std::vector<GrpShaderFX *> m_vShaderFXs;
    EngineHandle<GraphicVertexFormat> m_vertexFormats;
    bx::HandleAllocT<4096> m_vertexBuffersHandles;
    NewGraphicVertexBuffer m_vertexBuffers[4096]{};
    bx::HandleAllocT<4096> m_indexBufferHandles;
    NewGraphicIndexBuffer m_indexBuffers[4096]{};

    LPDIRECT3DSURFACE9 m_pFrameBuffer{};
    LPDIRECT3DSURFACE9 m_pDepthStencilBuffer{};
    D3DCMPFUNC currentDepthFunc;
    bool currentDepthTestEnable{}, currentDepthWriteEnable{};
    VertexFormatHandle currentVertexFormat{};
    DWORD m_currentFVF{};
    VertexBufferHandle currentVertexBuffer[4]{};
    unsigned int currentOffset[4]{};
    unsigned int currentFrequencyParameter[4]{};
    IndexBufferHandle currentIndexBuffer{};
    unsigned int currentInstanceCount{};

    std::unique_ptr<SharedVertexBuffer> m_sharedVertexBuffer;
    std::unique_ptr<SharedVertexBuffer> m_sharedVertexBufferText;

    // EngineHandle<VertexBufferHandle> m_vertexBuffers;
    // EngineHandle<IndexBufferHandle> m_indexBuffers;
};

#endif
