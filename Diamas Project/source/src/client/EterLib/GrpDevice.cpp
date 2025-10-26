#include "GrpDevice.h"
#include "../EterBase/Stl.h"
#include "MatrixStack.h"

#include "StdAfx.h"
#define IDV_DIRECTX9

#include "GrpShaderFX.h"
#include "imgui_impl_dx9.h"
#include "RenderTargetManager.h"

#include <bx/bx.h>
#include <bx/macros.h>
#include <Utils.h>

bool GRAPHICS_CAPS_CAN_NOT_DRAW_LINE = false;
bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW = false;
bool GRAPHICS_CAPS_HALF_SIZE_IMAGE = false;
bool GRAPHICS_CAPS_CAN_NOT_TEXTURE_ADDRESS_BORDER = false;
bool GRAPHICS_CAPS_SOFTWARE_TILING = false;

static bool s_graphicsDebuggerPresent = false;

void setGraphicsDebuggerPresent(bool _present)
{
    BX_TRACE("Graphics debugger is %spresent.", _present ? "" : "not ");
    s_graphicsDebuggerPresent = _present;
}

bool isGraphicsDebuggerPresent() { return s_graphicsDebuggerPresent; }

D3DPRESENT_PARAMETERS g_kD3DPP;

void NewGraphicVertexBuffer::Destroy() { M2_SAFE_RELEASE_CHECK(m_ptr); }

void NewGraphicVertexBuffer::Invalidate()
{
    if (!IsDynamic())
        return;
    M2_SAFE_RELEASE(m_ptr, 0);
}

bool NewGraphicVertexBuffer::Restore(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT hr;
    if ((hr = pd3dDevice->CreateVertexBuffer(size, flags, 0, IsDynamic() ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, &m_ptr, NULL)) != D3D_OK)
    {
        SPDLOG_ERROR("Vertex buffer creation failed. hr = {}", hr);
        return false;
    }
    return true;
}

void NewGraphicIndexBuffer::Invalidate() { M2_SAFE_RELEASE_CHECK(indexBuffer); }

bool NewGraphicIndexBuffer::Restore(LPDIRECT3DDEVICE9 pd3dDevice)
{
    if (FAILED(pd3dDevice->CreateIndexBuffer(
            nIndices * indexSize, flags, indexSize == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32,  IsDynamic() ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, &indexBuffer, NULL)))
    {
        return false;
    }
    return true;
}




CGraphicDevice::CGraphicDevice() { __Initialize(); }

CGraphicDevice::~CGraphicDevice() { Destroy(); }

void CGraphicDevice::__Initialize()
{
    ms_iD3DAdapterInfo = D3DADAPTER_DEFAULT;
    ms_iD3DDevInfo = D3DADAPTER_DEFAULT;
    ms_iD3DModeInfo = D3DADAPTER_DEFAULT;

    ms_lpd3d = NULL;
    ms_lpd3dDevice = NULL;

    ms_dwWavingEndTime = 0;
    ms_dwFlashingEndTime = 0;

    m_smallPdtVertexBuffer.idx = kInvalidHandle;
    m_largePdtVertexBuffer.idx = kInvalidHandle;

    m_pStateManager = NULL;

    __InitializeDefaultIndexBufferList();
}

void CGraphicDevice::InitDeviceDefault()
{

    currentVertexFormat = NullEngineHandle;
    currentDepthFunc = D3DCMP_LESSEQUAL;
    currentDepthTestEnable = true;
    currentDepthWriteEnable = true;
}

void CGraphicDevice::SetDepthFunc(D3DCMPFUNC depthFunc)
{
    if (m_deviceNeedsReset)
        return;
    if (depthFunc != currentDepthFunc)
    {
        ms_lpd3dDevice->SetRenderState(D3DRS_ZFUNC, currentDepthFunc = depthFunc);
    }
}

void CGraphicDevice::SetDepthEnable(bool bEnable, bool bWriteEnable)
{
    if (m_deviceNeedsReset)
        return;

    if (bEnable != currentDepthTestEnable)
    {
        ms_lpd3dDevice->SetRenderState(D3DRS_ZENABLE, currentDepthTestEnable = bEnable);
    }
    if (bWriteEnable != currentDepthWriteEnable)
    {
        ms_lpd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, currentDepthWriteEnable = bWriteEnable);
    }
}

bool CGraphicDevice::ResizeBackBuffer(UINT uWidth, UINT uHeight)
{
    if (m_deviceNeedsReset)
        return false;

    if (!ms_lpd3dDevice)
        return false;

    auto &rkD3DPP = ms_d3dPresentParameter;
    if (rkD3DPP.Windowed)
    {
        if (rkD3DPP.BackBufferWidth != uWidth || rkD3DPP.BackBufferHeight != uHeight)
        {
            rkD3DPP.BackBufferWidth = uWidth;
            rkD3DPP.BackBufferHeight = uHeight;

            if (!Reset())
                return false;
        }
    }

    return true;
}

// Format lookup table
static const D3DDECLTYPE declTypes[][4] = {
    D3DDECLTYPE_FLOAT1,    D3DDECLTYPE_FLOAT2, D3DDECLTYPE_FLOAT3,    D3DDECLTYPE_FLOAT4,  D3DDECLTYPE_UNUSED,
    D3DDECLTYPE_UNUSED,    D3DDECLTYPE_UNUSED, D3DDECLTYPE_UBYTE4N,   D3DDECLTYPE_UNUSED,  D3DDECLTYPE_UNUSED,
    D3DDECLTYPE_UNUSED,    D3DDECLTYPE_UBYTE4, D3DDECLTYPE_UNUSED,    D3DDECLTYPE_SHORT2N, D3DDECLTYPE_UNUSED,
    D3DDECLTYPE_SHORT4N,   D3DDECLTYPE_UNUSED, D3DDECLTYPE_USHORT2N,  D3DDECLTYPE_UNUSED,  D3DDECLTYPE_USHORT4N,
    D3DDECLTYPE_UNUSED,    D3DDECLTYPE_SHORT2, D3DDECLTYPE_UNUSED,    D3DDECLTYPE_SHORT4,  D3DDECLTYPE_UNUSED,
    D3DDECLTYPE_FLOAT16_2, D3DDECLTYPE_UNUSED, D3DDECLTYPE_FLOAT16_4, D3DDECLTYPE_UNUSED,  D3DDECLTYPE_UNUSED,
    D3DDECLTYPE_DEC3N,     D3DDECLTYPE_UNUSED, D3DDECLTYPE_UNUSED,    D3DDECLTYPE_UNUSED,  D3DDECLTYPE_UDEC3,
    D3DDECLTYPE_UNUSED,    D3DDECLTYPE_UNUSED, D3DDECLTYPE_UNUSED,    D3DDECLTYPE_UNUSED,  D3DDECLTYPE_D3DCOLOR,
};

static const D3DDECLUSAGE declUsages[] = {
    (D3DDECLUSAGE)(-1),        D3DDECLUSAGE_POSITION,    D3DDECLUSAGE_NORMAL,   D3DDECLUSAGE_TEXCOORD,
    D3DDECLUSAGE_COLOR,        D3DDECLUSAGE_TANGENT,     D3DDECLUSAGE_BINORMAL, D3DDECLUSAGE_POSITIONT,
    D3DDECLUSAGE_BLENDINDICES, D3DDECLUSAGE_BLENDWEIGHT,
};

int vfSizes[] = {
    sizeof(float),          sizeof(unsigned char),  sizeof(unsigned char),
    sizeof(short),          sizeof(unsigned short), sizeof(short),
    sizeof(unsigned short), // half
};

// Small common utility function for the vertex format creation functions below
void fillVertexElement(D3DVERTEXELEMENT9 &vElem, GraphicVertexFormat &vf, const int stream, const AttribType type,
                       const AttribFormat format, const int size, const int usageIndex)
{
    vElem.Stream = stream;
    vElem.Offset = vf.vertexSize[stream];
    vElem.Type = declTypes[format][size - 1];
    vElem.Method = D3DDECLMETHOD_DEFAULT;
    vElem.Usage = declUsages[type];
    vElem.UsageIndex = usageIndex;

    if (format >= FORMAT_DEC)
    {
        vf.vertexSize[stream] += 4;
    }
    else
    {
        vf.vertexSize[stream] += size * vfSizes[format];
    }
}

VertexFormatHandle CGraphicDevice::CreateVertexFormat(const GraphicVertexAttribute *attribs, const int nAttribs,
                                                      const int nStream, const int nStartTexCoord)
{
    if (m_deviceNeedsReset)
        return NullEngineHandle;
    GraphicVertexFormat vf;
    memset(vf.vertexSize, 0, sizeof(vf.vertexSize));

    D3DVERTEXELEMENT9 *vElem = new D3DVERTEXELEMENT9[nAttribs + 1];

    // Fill the D3DVERTEXELEMENT9 array
    int nTexCoords = nStartTexCoord;
    for (int i = 0; i < nAttribs; ++i)
    {
        fillVertexElement(vElem[i], vf, nStream, attribs[i].type, attribs[i].format, attribs[i].size, nTexCoords);

        if (attribs[i].type == TYPE_TEXCOORD)
            ++nTexCoords;
    }
    // Terminating element
    memset(vElem + nAttribs, 0, sizeof(D3DVERTEXELEMENT9));
    vElem[nAttribs].Stream = 0xFF;
    vElem[nAttribs].Type = D3DDECLTYPE_UNUSED;

    HRESULT hr = ms_lpd3dDevice->CreateVertexDeclaration(vElem, &vf.vDecl);
    delete[] vElem;

    if (hr != D3D_OK)
    {
        SPDLOG_ERROR("Couldn't create vertex declaration");
        return NullEngineHandle;
    }
    else
    {
        return m_vertexFormats.Add(vf);
    }
}

VertexFormatHandle CGraphicDevice::CreateVertexFormat(const GraphicVertexAttributeEx *attribs, const int nAttribs)
{
    if (m_deviceNeedsReset)
        return NullEngineHandle;

    GraphicVertexFormat vf;
    memset(vf.vertexSize, 0, sizeof(vf.vertexSize));

    D3DVERTEXELEMENT9 *vElem = new D3DVERTEXELEMENT9[nAttribs + 1];

    // Fill the D3DVERTEXELEMENT9 array
    for (int i = 0; i < nAttribs; ++i)
    {
        fillVertexElement(vElem[i], vf, attribs[i].stream, attribs[i].type, attribs[i].format, attribs[i].size,
                          attribs[i].usage);
    }
    // Terminating element
    memset(vElem + nAttribs, 0, sizeof(D3DVERTEXELEMENT9));
    vElem[nAttribs].Stream = 0xFF;
    vElem[nAttribs].Type = D3DDECLTYPE_UNUSED;

    HRESULT hr = ms_lpd3dDevice->CreateVertexDeclaration(vElem, &vf.vDecl);
    delete[] vElem;

    if (hr != D3D_OK)
    {
        SPDLOG_ERROR("Couldn't create vertex declaration");
        return NullEngineHandle;
    }
    else
    {
        return m_vertexFormats.Add(vf);
    }
}

IndexBufferHandle CGraphicDevice::CreateIndexBuffer(const unsigned int nIndices, D3DFORMAT inf, const void *data,
                                                    unsigned int flags)
{
    if (m_deviceNeedsReset)
        return ENGINE_INVALID_HANDLE;

    if (inf != D3DFMT_INDEX16 && inf != D3DFMT_INDEX32)
    {
        SPDLOG_ERROR("unknown index buffer format");
        return ENGINE_INVALID_HANDLE;
    }
    IndexBufferHandle handle = {m_indexBufferHandles.alloc()};
    auto &ib = m_indexBuffers[handle.idx];
    ib.nIndices = nIndices;
    ib.indexSize = (inf == D3DFMT_INDEX16 ? 2 : 4);
    ib.flags = flags;

    if (!ib.Restore(ms_lpd3dDevice))
    {
        SPDLOG_ERROR("Index buffer creation failed\n");
        return ENGINE_INVALID_HANDLE;
    }

    unsigned int size = ib.nIndices * ib.indexSize;

    // Upload the provided index data if any
    if (data != NULL)
    {
        void *dest;
        if (ib.indexBuffer->Lock(0, size, &dest, ib.IsDynamic() ? D3DLOCK_DISCARD : 0) == D3D_OK)
        {
            memcpy(dest, data, size);
            ib.indexBuffer->Unlock();
        }
        else
        {
            SPDLOG_ERROR("Couldn't lock index buffer");
        }
    }

    return handle;
}

void CGraphicDevice::SetVertexFormat(VertexFormatHandle hVertexFormat)
{
    if (m_deviceNeedsReset)
        return;

    if (hVertexFormat != currentVertexFormat)
    {
        if (hVertexFormat == NullEngineHandle)
        {
            // HRESULT hr = m_pd3dDevice->SetVertexDeclaration(NULL);
            //_ASSERT(hr==D3D_OK);
        }
        else
        {
            HRESULT hr = ms_lpd3dDevice->SetVertexDeclaration(m_vertexFormats[hVertexFormat].vDecl);
            _ASSERT(hr == D3D_OK);
        }

        currentVertexFormat = hVertexFormat;
        m_currentFVF = 0;
    }
}

HRESULT CGraphicDevice::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
    if (m_deviceNeedsReset)
        return S_OK;

    if (!ms_lpd3dDevice)
        return S_OK;
    return (ms_lpd3dDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount));
}

HRESULT CGraphicDevice::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount,
                                        const void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    if (m_deviceNeedsReset)
        return S_OK;

    if (!ms_lpd3dDevice)
        return S_OK;
    SetVertexBuffer(ENGINE_INVALID_HANDLE, 0, 0);
    return ms_lpd3dDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData,
                                           VertexStreamZeroStride);
}

HRESULT CGraphicDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices,
                                             UINT startIndex, UINT primCount, INT baseVertexIndex)
{
    if (m_deviceNeedsReset)
        return S_OK;

    return ms_lpd3dDevice->DrawIndexedPrimitive(PrimitiveType, baseVertexIndex, minIndex, NumVertices, startIndex,
                                                primCount);
}

HRESULT CGraphicDevice::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex,
                                               UINT NumVertexIndices, UINT PrimitiveCount, CONST void *pIndexData,
                                               D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData,
                                               UINT VertexStreamZeroStride)
{
    if (m_deviceNeedsReset)
        return S_OK;

    SetVertexBuffer(ENGINE_INVALID_HANDLE, 0, 0); // ���� ������ �־ ����.
    SetIndexBuffer(ENGINE_INVALID_HANDLE); // ����� �����ϰ� ���� ����setindices()�� ��ȿȭ �Ǵ� ������ �־ ����

    return ms_lpd3dDevice->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertexIndices, PrimitiveCount,
                                                  pIndexData, IndexDataFormat, pVertexStreamZeroData,
                                                  VertexStreamZeroStride);
}

void CGraphicDevice::SetFvF(DWORD fvf)
{
    if (m_deviceNeedsReset)
        return;

    if (m_currentFVF != fvf)
    {
        m_currentFVF = fvf;
        ms_lpd3dDevice->SetFVF(fvf);
        currentVertexFormat = NullEngineHandle;
    }
}

void CGraphicDevice::ShaderOff()
{
    if (m_deviceNeedsReset)
        return;

    ms_lpd3dDevice->SetVertexShader(NULL);
    ms_lpd3dDevice->SetPixelShader(NULL);
}

void *CGraphicDevice::LockVertexBuffer(const VertexBufferHandle hVertexBuffer, const unsigned int flags,
                                       const unsigned int nOffsetToLock, const unsigned int nSizeToLock)
{
    if (m_deviceNeedsReset)
        return nullptr;

    if (!isValid(hVertexBuffer))
        return nullptr;

    if (!m_vertexBuffers[hVertexBuffer.idx].m_ptr)
        return nullptr;

    void *pointer = nullptr;
    HRESULT hr = m_vertexBuffers[hVertexBuffer.idx].m_ptr->Lock(
        nOffsetToLock * m_vertexBuffers[hVertexBuffer.idx].vertexSize,
        nSizeToLock * m_vertexBuffers[hVertexBuffer.idx].vertexSize, &pointer, flags);
    if (hr != D3D_OK)
        return nullptr;
    return pointer;
}

bool CGraphicDevice::UnlockVertexBuffer(const VertexBufferHandle hVertexBuffer)
{
    return (m_vertexBuffers[hVertexBuffer.idx].m_ptr->Unlock() == D3D_OK);
}

void *CGraphicDevice::LockIndexBuffer(const IndexBufferHandle hIndexBuffer, const unsigned int flags,
                                      const unsigned int nOffsetToLock, const unsigned int nSizeToLock)
{
    if (m_deviceNeedsReset)
        return nullptr;

    if(!isValid(hIndexBuffer))
        return nullptr;

    void *pointer = NULL;

    NewGraphicIndexBuffer in = m_indexBuffers[hIndexBuffer.idx];

    HRESULT hr = m_indexBuffers[hIndexBuffer.idx].indexBuffer->Lock(nOffsetToLock * m_indexBuffers[hIndexBuffer.idx].indexSize,
                                                                nSizeToLock * m_indexBuffers[hIndexBuffer.idx].indexSize,
                                                                &pointer, flags);
    if (hr != D3D_OK)
        return NULL;
    return pointer;
}

bool CGraphicDevice::UnlockIndexBuffer(const IndexBufferHandle hIndexBuffer)
{
    if (m_deviceNeedsReset)
        return false;

    if(!isValid(hIndexBuffer))
        return false;

    return (m_indexBuffers[hIndexBuffer.idx].indexBuffer->Unlock() == D3D_OK);
}

bool CGraphicDevice::SetPDTStream(SPDTVertex *pVertices, UINT uVtxCount)
{
    if (m_deviceNeedsReset)
        return false;

    return SetPDTStream((SPDTVertexRaw *)pVertices, uVtxCount);
}

bool CGraphicDevice::SetPDTStream(SPDTVertexRaw *pSrcVertices, UINT uVtxCount)
{
    if (m_deviceNeedsReset)
        return false;

    assert(uVtxCount <= kLargePdtVertexBufferSize);

    if (!uVtxCount)
        return false;
    VertexBufferHandle vb{};

    if (uVtxCount <= kSmallPdtVertexBufferSize)
        vb = GetSmallPdtVertexBuffer();
    else
        vb = GetLargePdtVertexBuffer();

    const auto bytes = sizeof(TPDTVertex) * uVtxCount;

    auto *dst = LockVertexBuffer(vb, D3DLOCK_DISCARD);
    if (!dst)
        return false;
    std::memcpy(dst, pSrcVertices, bytes);
    UnlockVertexBuffer(vb);
    SetVertexBuffer(vb, 0, 0);

    return true;
}

VertexBufferHandle CGraphicDevice::CreateVertexBuffer(const long size, int nVertexSize, const void *data,
                                                      unsigned int flags)
{

    if (m_deviceNeedsReset)
        return ENGINE_INVALID_HANDLE;

    VertexBufferHandle handle = {m_vertexBuffersHandles.alloc()};
    auto &vb = m_vertexBuffers[handle.idx];

    vb.size = size;
    vb.flags = flags;
    vb.vertexSize = nVertexSize;

    if (!vb.Restore(ms_lpd3dDevice))
        return ENGINE_INVALID_HANDLE;

    // Upload the provided vertex data if any
    if (data != NULL)
    {
        void *dest;
        if (vb.m_ptr->Lock(0, size, &dest, vb.IsDynamic() ? D3DLOCK_DISCARD : 0) == D3D_OK)
        {
            memcpy(dest, data, size);
            vb.m_ptr->Unlock();
        }
        else
        {
            SPDLOG_ERROR("Couldn't lock vertex buffer");
        }
    }

    return handle;
}

void CGraphicDevice::DeleteVertexBuffer(VertexBufferHandle& hVertexBuffer)
{
    if (m_deviceNeedsReset)
        return;

    if (!isValid(hVertexBuffer))
        return;

    // �����ɰ��� ���� ���õȰ��̶��, �����Ѵ�
    for (int i = 0; i < 4; ++i)
    {
        if (currentVertexBuffer[i].idx == hVertexBuffer.idx)
        {
            SetVertexBuffer(ENGINE_INVALID_HANDLE, i, 0);
        }
    }

    m_vertexBuffers[hVertexBuffer.idx].Destroy();
    m_vertexBuffers[hVertexBuffer.idx] = {};
    m_vertexBuffersHandles.free(hVertexBuffer.idx);
    hVertexBuffer = ENGINE_INVALID_HANDLE;
}

void CGraphicDevice::DeleteIndexBuffer(IndexBufferHandle& hIndexBuffer)
{
    if (m_deviceNeedsReset)
        return;

    if (!isValid(hIndexBuffer))
        return;

    // 삭제할 것이 현재 선택된 것이다.
    if (hIndexBuffer.idx == currentIndexBuffer.idx)
        SetIndexBuffer(ENGINE_INVALID_HANDLE);

    M2_SAFE_RELEASE_CHECK(m_indexBuffers[hIndexBuffer.idx].indexBuffer);
    m_indexBufferHandles.free(hIndexBuffer.idx);
    hIndexBuffer = ENGINE_INVALID_HANDLE;
}

void CGraphicDevice::SetVertexBuffer(VertexBufferHandle hVertexBuffer, const int stream, const unsigned int offset)
{
    if (m_deviceNeedsReset)
        return;

    if (hVertexBuffer.idx != currentVertexBuffer[stream].idx || offset != currentOffset[stream])
    {
        if (!isValid(hVertexBuffer))
        {
            HRESULT hr = ms_lpd3dDevice->SetStreamSource(stream, NULL, 0, 0);
            _ASSERT(SUCCEEDED(hr));
        }
        else
        {
            int nVertexSize = m_vertexBuffers[hVertexBuffer.idx].vertexSize;
            HRESULT hr = ms_lpd3dDevice->SetStreamSource(stream, m_vertexBuffers[hVertexBuffer.idx].m_ptr, (UINT)offset,
                                                         nVertexSize);
            _ASSERT(SUCCEEDED(hr));
        }

        currentVertexBuffer[stream] = hVertexBuffer;
        currentOffset[stream] = offset;
    }
}

void CGraphicDevice::SetIndexBuffer(IndexBufferHandle hIndexBuffer)
{
    if (m_deviceNeedsReset)
        return;

    if (hIndexBuffer.idx != currentIndexBuffer.idx)
    {
        if (!isValid(hIndexBuffer))
        {
            HRESULT hr = ms_lpd3dDevice->SetIndices(NULL);
            _ASSERT(hr == D3D_OK);
        }
        else
        {
            HRESULT hr = ms_lpd3dDevice->SetIndices(m_indexBuffers[hIndexBuffer.idx].indexBuffer);
            _ASSERT(hr == D3D_OK);
        }

        currentIndexBuffer = hIndexBuffer;
    }
}

CGraphicDevice::EDeviceState CGraphicDevice::GetDeviceState()
{
    if (!ms_lpd3dDevice)
        return DEVICESTATE_NULL;

    HRESULT hr;

    if (FAILED(hr = ms_lpd3dDevice->TestCooperativeLevel()))
    {
        if (D3DERR_DEVICELOST == hr)
            return DEVICESTATE_BROKEN;

        if (D3DERR_DEVICENOTRESET == hr)
        {
            m_deviceNeedsReset = true;
            return DEVICESTATE_NEEDS_RESET;
        }

        return DEVICESTATE_BROKEN;
    }

    return DEVICESTATE_OK;
}

LPDIRECT3D9 CGraphicDevice::GetDirectx9() { return ms_lpd3d; }

GrpShaderFX *CGraphicDevice::CreateShaderFX()
{
    GrpShaderFX *ret = new GrpShaderFX(ms_lpd3dDevice);
    m_vShaderFXs.push_back(ret);
    return ret;
}

GrpShaderFX *CGraphicDevice::CreateShaderFX(const std::string &strFilename,
                                            const std::vector<std::string> &vShaderDefines /* = vector<string> */,
                                            GrpShaderFXPool *pShaderPool /* = NULL */)
{

    GrpShaderFX *ret = new GrpShaderFX(ms_lpd3dDevice);
    SPDLOG_TRACE("Create Shader {}", strFilename);
    ret->SetFromFile(strFilename, vShaderDefines, pShaderPool);
    ret->Setup();

    m_vShaderFXs.push_back(ret);

    return ret;
}

void CGraphicDevice::DeleteShaderFX(GrpShaderFX *shader)
{
    M2_SAFE_RELEASE_CHECK(shader->m_pEffect);

    auto i = find(m_vShaderFXs.begin(), m_vShaderFXs.end(), shader);
    _ASSERT(i != m_vShaderFXs.end());
    m_vShaderFXs.erase(i);

    delete shader;
}

void CGraphicDevice::Clear(bool bTarget, bool bDepth, bool bStencil, DWORD dwColor, float fDepth, DWORD dwStencil,
                           DWORD nIdx)
{
    DWORD flag = (bTarget ? D3DCLEAR_TARGET : 0) | (bDepth ? D3DCLEAR_ZBUFFER : 0) | (bStencil ? D3DCLEAR_STENCIL : 0);
    ms_lpd3dDevice->Clear(nIdx, NULL, flag, dwColor, fDepth, dwStencil);
}

void CGraphicDevice::SetVertexBufferFreq(UINT stream, UINT FrequencyParameter)
{

    if (FrequencyParameter != currentFrequencyParameter[stream])
    {
        HRESULT hr = ms_lpd3dDevice->SetStreamSourceFreq(stream, FrequencyParameter);
        _ASSERT(SUCCEEDED(hr));
        currentFrequencyParameter[stream] = FrequencyParameter;
    }
}

void CGraphicDevice::SetupForRenderInstancing(VertexBufferHandle hVertexBuffer, UINT nCnt, unsigned int offset)
{
    if (m_deviceNeedsReset)
        return;

    if (isValid(hVertexBuffer))
    {
        SetVertexBuffer(ENGINE_INVALID_HANDLE, 1, 0);
        RestoreForRenderInstancing();
    }
    else
    {
        // ½ºÆ®¸² 0ÀÇ SetStreamSourceFreq
        SetVertexBufferFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | nCnt);

        // ½ºÆ®¸² 1ÀÇ ¹öÅØ½º ¹öÆÛ
        SetVertexBuffer(hVertexBuffer, 1, offset);
        SetVertexBufferFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1ul);

        currentInstanceCount = nCnt;
    }
}

void CGraphicDevice::RestoreForRenderInstancing()
{
    if (m_deviceNeedsReset)
        return;

    SetVertexBufferFreq(0, 1);
    SetVertexBufferFreq(1, 1);

    currentInstanceCount = 1;
}

LPDIRECT3DDEVICE9 CGraphicDevice::GetDevice() { return ms_lpd3dDevice; }

void CGraphicDevice::OnLostDevice()
{
    InitDeviceDefault();
    m_pStateManager->SetDefaultState();

    for (auto i = m_vShaderFXs.begin(); i != m_vShaderFXs.end(); ++i)
    {
        (*i)->OnLostDevice();
    }
    ImGui_ImplDX9_InvalidateDeviceObjects();

    for (uint32_t ii = 0; ii < BX_COUNTOF(m_vertexBuffers); ++ii)
    {
        if (m_vertexBuffers[ii].IsDynamic() && m_vertexBuffers[ii].m_ptr)
            m_vertexBuffers[ii].Invalidate();
    }

    for (uint32_t ii = 0; ii < BX_COUNTOF(m_indexBuffers); ++ii)
    {
        if (m_indexBuffers[ii].IsDynamic() && m_indexBuffers[ii].indexBuffer)
            m_indexBuffers[ii].Invalidate();
    }

    CRenderTargetManager::instance().ReleaseRenderTargetTextures();

}

void CGraphicDevice::OnRestoreDevice()
{

    for (std::vector<GrpShaderFX *>::iterator i = m_vShaderFXs.begin(); i != m_vShaderFXs.end(); ++i)
    {
        (*i)->OnResetDevice();
    }
    for (uint32_t ii = 0; ii < BX_COUNTOF(m_vertexBuffers); ++ii)
    {
        if (m_vertexBuffers[ii].IsDynamic())
            m_vertexBuffers[ii].Restore(ms_lpd3dDevice);
    }
    for (uint32_t ii = 0; ii < BX_COUNTOF(m_indexBuffers); ++ii)
    {
        if (m_indexBuffers[ii].IsDynamic())
            m_indexBuffers[ii].Restore(ms_lpd3dDevice);
    }

    InitDeviceDefault();
    m_pStateManager->SetDefaultState();

    ImGui_ImplDX9_CreateDeviceObjects();
    CRenderTargetManager::instance().CreateRenderTargetTextures();

    __CreatePDTVertexBufferList();
}

bool CGraphicDevice::Reset()
{
    OnLostDevice();

    HRESULT hr;
    if (FAILED(hr = ms_lpd3dDevice->Reset(&ms_d3dPresentParameter)))
    {
            SPDLOG_ERROR("Reset::Error {}", hr);

        return false;
    }
    m_deviceNeedsReset = false;

    OnRestoreDevice();
    return true;
}

static DWORD s_MaxTextureWidth, s_MaxTextureHeight;

bool EL3D_ConfirmDevice(D3DCAPS9 &rkD3DCaps, UINT uBehavior, D3DFORMAT /*eD3DFmt*/)
{
    // PUREDEVICE는 GetTransform / GetViewport 등이 되지 않는다.
    if (uBehavior & D3DCREATE_PUREDEVICE)
        return false;

    if (uBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING)
    {
        // DirectionalLight
        if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS))
            return false;

        // PositionalLight
        if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_POSITIONALLIGHTS))
            return false;

        // Software T&L Support - ATI NOT SUPPORT CLIP, USE DIRECTX SOFTWARE PROCESSING CLIPPING
        if (GRAPHICS_CAPS_SOFTWARE_TILING)
        {
            if (!(rkD3DCaps.PrimitiveMiscCaps & D3DPMISCCAPS_CLIPTLVERTS))
                return false;
        }
        else
        {
            // Shadow/Terrain
            if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN))
                return false;
        }
    }

    s_MaxTextureWidth = rkD3DCaps.MaxTextureWidth;
    s_MaxTextureHeight = rkD3DCaps.MaxTextureHeight;
    return true;
}

DWORD GetMaxTextureWidth() { return s_MaxTextureWidth; }

DWORD GetMaxTextureHeight() { return s_MaxTextureHeight; }

int CGraphicDevice::Create(HWND hWnd, int iHres, int iVres, bool Windowed, int /*iBit*/, int iReflashRate)
{
    int iRet = CREATE_OK;

    Destroy();

    ms_iWidth = iHres;
    ms_iHeight = iVres;

    ms_hWnd = hWnd;
    ms_hDC = GetDC(hWnd);
    ms_lpd3d = Direct3DCreate9(D3D_SDK_VERSION);

    if (!ms_lpd3d)
        return CREATE_NO_DIRECTX;

    if (!ms_kD3DDetector.Build(*ms_lpd3d, EL3D_ConfirmDevice))
        return CREATE_ENUM;

    if (!ms_kD3DDetector.Find(800, 600, 32, true, &ms_iD3DModeInfo, &ms_iD3DDevInfo, &ms_iD3DAdapterInfo))
        return CREATE_DETECT;

    std::string stDevList;
    ms_kD3DDetector.GetString(&stDevList);

    D3D_CAdapterInfo *pkD3DAdapterInfo = ms_kD3DDetector.GetD3DAdapterInfop(ms_iD3DAdapterInfo);
    if (!pkD3DAdapterInfo)
    {
        SPDLOG_ERROR("adapter {0} is EMPTY", ms_iD3DAdapterInfo);
        return CREATE_DETECT;
    }

    D3D_SModeInfo *pkD3DModeInfo = pkD3DAdapterInfo->GetD3DModeInfop(ms_iD3DDevInfo, ms_iD3DModeInfo);
    if (!pkD3DModeInfo)
    {
        SPDLOG_ERROR("device {0}, mode {1} is EMPTY", ms_iD3DDevInfo, ms_iD3DModeInfo);
        return CREATE_DETECT;
    }

    D3DADAPTER_IDENTIFIER9 &rkD3DAdapterId = pkD3DAdapterInfo->GetIdentifier();
    if (Windowed && strnicmp(rkD3DAdapterId.Driver, "3dfx", 4) == 0 &&
        22 == pkD3DAdapterInfo->GetDesktopD3DDisplayModer().Format)
    {
        return CREATE_FORMAT;
    }

    if (pkD3DModeInfo->m_dwD3DBehavior == D3DCREATE_SOFTWARE_VERTEXPROCESSING)
    {
        iRet |= CREATE_NO_TNL;
    }

    std::string stModeInfo;
    pkD3DModeInfo->GetString(&stModeInfo);

    // Tracen(stModeInfo.c_str());

    int ErrorCorrection = 0;

RETRY:
    ZeroMemory(&ms_d3dPresentParameter, sizeof(ms_d3dPresentParameter));

    ms_d3dPresentParameter.Windowed = Windowed;
    ms_d3dPresentParameter.BackBufferWidth = iHres;
    ms_d3dPresentParameter.BackBufferHeight = iVres;
    ms_d3dPresentParameter.hDeviceWindow = hWnd;
    ms_d3dPresentParameter.BackBufferCount = m_uBackBufferCount;
    if (Windowed)
    {
        ms_d3dPresentParameter.MultiSampleType = D3DMULTISAMPLE_NONE;
    }
    else
    {
        ms_d3dPresentParameter.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
    }

    ms_d3dPresentParameter.SwapEffect = D3DSWAPEFFECT_DISCARD;

    if (Windowed)
    {
      	ms_d3dPresentParameter.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
        ms_d3dPresentParameter.BackBufferFormat = pkD3DAdapterInfo->GetDesktopD3DDisplayModer().Format;
    }
    else
    {
          ms_d3dPresentParameter.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
        ms_d3dPresentParameter.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

        ms_d3dPresentParameter.BackBufferFormat = pkD3DModeInfo->m_eD3DFmtPixel;
    }

    ms_d3dPresentParameter.EnableAutoDepthStencil = true;
    ms_d3dPresentParameter.AutoDepthStencilFormat = D3DFMT_D24S8;


  ms_dwD3DBehavior = pkD3DModeInfo->m_dwD3DBehavior;

    if (FAILED(ms_hLastResult = ms_lpd3d->CreateDevice(ms_iD3DAdapterInfo, D3DDEVTYPE_HAL, hWnd,
                                                       // 2004. 1. 9 myevan 버텍스 프로세싱 방식 자동 선택 추가
                                                       ms_dwD3DBehavior, &ms_d3dPresentParameter, &ms_lpd3dDevice)))
    {
        switch (ms_hLastResult)
        {
        case D3DERR_INVALIDCALL:
            SPDLOG_ERROR("IDirect3DDevice.CreateDevice - ERROR D3DERR_INVALIDCALL\nThe method call is invalid. For "
                         "example, a method's parameter may have an invalid value.");
            break;
        case D3DERR_NOTAVAILABLE:
            SPDLOG_ERROR("IDirect3DDevice.CreateDevice - ERROR D3DERR_NOTAVAILABLE\nThis device does not support the "
                         "queried technique. ");
            break;
        case D3DERR_OUTOFVIDEOMEMORY:
            SPDLOG_ERROR("IDirect3DDevice.CreateDevice - ERROR D3DERR_OUTOFVIDEOMEMORY\nDirect3D does not have enough "
                         "display memory to perform the operation");
            break;
        default:
            SPDLOG_ERROR("IDirect3DDevice.CreateDevice - ERROR {0}", ms_hLastResult);
            break;
        }

        if (ErrorCorrection)
            return CREATE_DEVICE;

        // 2004. 1. 9 myevan 큰의미 없는 코드인듯.. 에러나면 표시하고 종료하자
        iReflashRate = 0;
        ++ErrorCorrection;
        iRet = CREATE_REFRESHRATE;
        goto RETRY;
    }

    // Check DXT Support Info
    if (ms_lpd3d->CheckDeviceFormat(ms_iD3DAdapterInfo, D3DDEVTYPE_HAL, ms_d3dPresentParameter.BackBufferFormat, 0,
                                    D3DRTYPE_TEXTURE, D3DFMT_DXT1) == D3DERR_NOTAVAILABLE)
    {
        ms_bSupportDXT = false;
    }

    if (ms_lpd3d->CheckDeviceFormat(ms_iD3DAdapterInfo, D3DDEVTYPE_HAL, ms_d3dPresentParameter.BackBufferFormat, 0,
                                    D3DRTYPE_TEXTURE, D3DFMT_DXT3) == D3DERR_NOTAVAILABLE)
    {
        ms_bSupportDXT = false;
    }

    if (ms_lpd3d->CheckDeviceFormat(ms_iD3DAdapterInfo, D3DDEVTYPE_HAL, ms_d3dPresentParameter.BackBufferFormat, 0,
                                    D3DRTYPE_TEXTURE, D3DFMT_DXT5) == D3DERR_NOTAVAILABLE)
    {
        ms_bSupportDXT = false;
    }

    if (FAILED((ms_hLastResult = ms_lpd3dDevice->GetDeviceCaps(&ms_d3dCaps))))
    {
        SPDLOG_ERROR("IDirect3DDevice.GetDeviceCaps - ERROR {0}", ms_hLastResult);
        return CREATE_GET_DEVICE_CAPS2;
    }

    if (!Windowed)
        SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, iHres, iVres, SWP_SHOWWINDOW);

    // Tracef("vertex shader version : %X\n",(DWORD)ms_d3dCaps.VertexShaderVersion);

    ms_lpd3dDevice->GetViewport(&ms_Viewport);

    m_pStateManager = new CStateManager(ms_lpd3dDevice);

    CreateMatrixStack(0, &ms_lpd3dMatStack);
    ms_lpd3dMatStack->LoadIdentity();

    ms_matIdentity = Matrix::Identity;
    ms_matView = Matrix::Identity;
    ms_matProj = Matrix::Identity;
    ms_matInverseView = Matrix::Identity;
    ms_matInverseViewYAxis = Matrix::Identity;
    ms_matScreen0 = Matrix::Identity;
    ms_matScreen1 = Matrix::Identity;
    ms_matScreen2 = Matrix::Identity;

    ms_matScreen0._11 = 1;
    ms_matScreen0._22 = -1;

    ms_matScreen1._41 = 1;
    ms_matScreen1._42 = 1;

    ms_matScreen2._11 = (float)iHres / 2;
    ms_matScreen2._22 = (float)iVres / 2;

    InitDeviceDefault();

    ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);


    if (!__CreateDefaultIndexBufferList())
        return false;

    if (!__CreatePDTVertexBufferList())
        return false;

    GRAPHICS_CAPS_CAN_NOT_TEXTURE_ADDRESS_BORDER = (ms_d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_BORDER) == 0;
    m_sharedVertexBuffer = std::make_unique<SharedVertexBuffer>(this);
    return (iRet);
}

void CGraphicDevice::__DestroyPDTVertexBufferList()
{
    /*
     *
    if (m_smallPdtVertexBuffer)
    {
        m_smallPdtVertexBuffer->Release();
        m_smallPdtVertexBuffer = nullptr;
    }

    if (m_largePdtVertexBuffer)
    {
        m_largePdtVertexBuffer->Release();
        m_largePdtVertexBuffer = nullptr;
    }
     */
}

void CGraphicDevice::DrawPDTVertices(D3DPRIMITIVETYPE primitiveType, unsigned int nPrimitiveCount,void* pVertex,int TheNumberOfVertexData) {
    if(m_sharedVertexBuffer) {
        m_sharedVertexBuffer->Render(primitiveType, nPrimitiveCount, pVertex, sizeof(TPDTVertex), TheNumberOfVertexData);
    }
}
void CGraphicDevice::DrawTextVertices(D3DPRIMITIVETYPE primitiveType, unsigned int nPrimitiveCount,void* pVertex,int TheNumberOfVertexData) {
    if(m_sharedVertexBufferText) {
        m_sharedVertexBufferText->Render(primitiveType, nPrimitiveCount, pVertex, sizeof(TPDTVertex), TheNumberOfVertexData);
    }
}

bool CGraphicDevice::__CreatePDTVertexBufferList()
{
    if (m_deviceNeedsReset)
        return false;

    m_smallPdtVertexBuffer = CreateVertexBuffer(sizeof(TPDTVertex) * kSmallPdtVertexBufferSize, sizeof(TPDTVertex),
                                                nullptr, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY);
    m_largePdtVertexBuffer = CreateVertexBuffer(sizeof(TPDTVertex) * kLargePdtVertexBufferSize, sizeof(TPDTVertex),
                                                nullptr, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY);

    return true;
}

void CGraphicDevice::__InitializeDefaultIndexBufferList()
{
    for (UINT i = 0; i < DEFAULT_IB_NUM; ++i)
        ms_alpd3dDefIB[i] = ENGINE_INVALID_HANDLE;
}

void CGraphicDevice::__DestroyDefaultIndexBufferList() {}

bool CGraphicDevice::__CreateDefaultIndexBuffer(UINT eDefIB, UINT uIdxCount, const WORD *c_awIndices)
{
    if (m_deviceNeedsReset)
        return false;

    assert(!isValid(ms_alpd3dDefIB[eDefIB]));

    ms_alpd3dDefIB[eDefIB] = CreateIndexBuffer(uIdxCount, D3DFMT_INDEX16, c_awIndices);

    return true;
}

void CGraphicDevice::SetDefaultIndexBuffer(UINT eDefIB)
{
    if (m_deviceNeedsReset)
        return;

    if (eDefIB >= DEFAULT_IB_NUM)
        return;

    SetIndexBuffer(ms_alpd3dDefIB[eDefIB]);
}

bool CGraphicDevice::__CreateDefaultIndexBufferList()
{
    static const WORD c_awLineIndices[2] = {
        0,
        1,
    };
    static const WORD c_awLineTriIndices[6] = {
        0, 1, 0, 2, 1, 2,
    };
    static const WORD c_awLineRectIndices[8] = {
        0, 1, 0, 2, 1, 3, 2, 3,
    };
    static const WORD c_awLineCubeIndices[24] = {
        0, 1, 0, 2, 1, 3, 2, 3, 0, 4, 1, 5, 2, 6, 3, 7, 4, 5, 4, 6, 5, 7, 6, 7,
    };
    static const WORD c_awFillTriIndices[3] = {
        0,
        1,
        2,
    };
    static const WORD c_awFillRectIndices[6] = {
        0, 2, 1, 2, 3, 1,
    };
    static const WORD c_awFillCubeIndices[36] = {
        0, 1, 2, 1, 3, 2, 2, 0, 6, 0, 4, 6, 0, 1, 4, 1, 5, 4, 1, 3, 5, 3, 7, 5, 3, 2, 7, 2, 6, 7, 4, 5, 6, 5, 7, 6,
    };
    static const WORD c_awFont[6] = {0, 1, 2, 0, 2, 3};

    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE, 2, c_awLineIndices))
        return false;
    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_TRI, 6, c_awLineTriIndices))
        return false;
    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_RECT, 8, c_awLineRectIndices))
        return false;
    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_CUBE, 24, c_awLineCubeIndices))
        return false;
    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_TRI, 3, c_awFillTriIndices))
        return false;
    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_RECT, 6, c_awFillRectIndices))
        return false;
    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_CUBE, 36, c_awFillCubeIndices))
        return false;
    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FONT, 6, c_awFont))
        return false;
    return true;
}

void CGraphicDevice::InitBackBufferCount(UINT uBackBufferCount) { m_uBackBufferCount = uBackBufferCount; }

void CGraphicDevice::Destroy()
{

    __DestroyPDTVertexBufferList();
    __DestroyDefaultIndexBufferList();

    while (m_vertexFormats.GetCount())
    {
        m_vertexFormats[m_vertexFormats.GetFirst()].vDecl->Release();
        m_vertexFormats.Remove(m_vertexFormats.GetFirst());
    }

  /*  while (m_indexBuffers.GetCount())
    {
        DeleteIndexBuffer(m_indexBuffers.GetFirst());
    }*/
    while (m_vShaderFXs.size())
    {
        DeleteShaderFX(m_vShaderFXs.front());
    }

    if (ms_hDC)
    {
        ReleaseDC(ms_hWnd, ms_hDC);
        ms_hDC = NULL;
    }

    if (ms_lpd3dDevice)
    {
        if (ms_lpd3dDevice->Release() > 0)
            OutputDebugString("D3D reference counter is not zero. Some resources have not yet been released\n");
        ms_lpd3dDevice = NULL;
    }
    M2_SAFE_RELEASE_CHECK(ms_lpd3d);

    if (m_pStateManager)
    {
        delete m_pStateManager;
        m_pStateManager = NULL;
    }

    __Initialize();
}
