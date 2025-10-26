#include "StdAfx.h"
#include "PythonGraphic.h"

#include <d3dx9tex.h>


#include "../EterLib/GrpDevice.h"
#include "base/Remotery.h"

void CPythonGraphic::Destroy()
{
}

LPDIRECT3D9 CPythonGraphic::GetD3D()
{
    return ms_lpd3d;
}

float CPythonGraphic::GetOrthoDepth()
{
    return m_fOrthoDepth;
}

void CPythonGraphic::SetInterfaceRenderState()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CPythonGraphic::SetInterfaceRenderState **");
        rmt_ScopedCPUSample(SetInterfaceRenderState, 0);

    STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &ms_matIdentity);
    STATEMANAGER.SaveTransform(D3DTS_VIEW, &ms_matIdentity);
    STATEMANAGER.SaveTransform(D3DTS_WORLD, &ms_matIdentity);

    STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

    STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

    STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    CPythonGraphic::Instance().SetBlendOperation();
    CPythonGraphic::Instance().SetOrtho2D(ms_iWidth, ms_iHeight, GetOrthoDepth());

    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
    STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    D3DPERF_EndEvent();
}

void CPythonGraphic::SetGameRenderState()
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CPythonGraphic::SetGameRenderState **");
    STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);
    STATEMANAGER.RestoreTransform(D3DTS_VIEW);
    STATEMANAGER.RestoreTransform(D3DTS_WORLD);

    STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    STATEMANAGER.SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

    CPythonGraphic::Instance().SetDiffuseOperation();

    D3DPERF_EndEvent();
}

void CPythonGraphic::SetCursorPosition(int x, int y)
{
    CScreen::SetCursorPosition(x, y, ms_iWidth, ms_iHeight);
}

void CPythonGraphic::SetOmniLight()
{
    // Set up a material
    D3DMATERIAL9 Material;
    Material.Ambient = {1.0f, 1.0f, 1.0f, 1.0f};
    Material.Diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    Material.Emissive = {0.1f, 0.1f, 0.1f, 1.0f};
    STATEMANAGER.SetMaterial(&Material);

    D3DLIGHT9 Light;
    Light.Type = D3DLIGHT_SPOT;
    Light.Position = {50.0f, 150.0f, 350.0f};
    Light.Direction = {-0.15f, -0.3f, -0.9f};
    Light.Theta = DirectX::XMConvertToRadians(30.0f);
    Light.Phi = DirectX::XMConvertToRadians(45.0f);
    Light.Falloff = 1.0f;
    Light.Attenuation0 = 0.0f;
    Light.Attenuation1 = 0.005f;
    Light.Attenuation2 = 0.0f;
    Light.Diffuse.r = 1.0f;
    Light.Diffuse.g = 1.0f;
    Light.Diffuse.b = 1.0f;
    Light.Diffuse.a = 1.0f;
    Light.Ambient.r = 1.0f;
    Light.Ambient.g = 1.0f;
    Light.Ambient.b = 1.0f;
    Light.Ambient.a = 1.0f;
    Light.Range = 500.0f;
    ms_lpd3dDevice->SetLight(0, &Light);
    ms_lpd3dDevice->LightEnable(0, true);

    Light.Type = D3DLIGHT_POINT;
    Light.Position = {0.0f, 200.0f, 200.0f};
    Light.Attenuation0 = 0.1f;
    Light.Attenuation1 = 0.01f;
    Light.Attenuation2 = 0.0f;
    ms_lpd3dDevice->SetLight(1, &Light);
    ms_lpd3dDevice->LightEnable(1, true);
 
}

void CPythonGraphic::SetViewport(DWORD fx, DWORD fy, float fWidth, float fHeight)
{
    ms_lpd3dDevice->GetViewport(&m_backupViewport);

    D3DVIEWPORT9 ViewPort;
    ViewPort.X = fx;
    ViewPort.Y = fy;
    ViewPort.Width = fWidth;
    ViewPort.Height = fHeight;
    ViewPort.MinZ = 0.0f;
    ViewPort.MaxZ = 1.0f;
    if (FAILED(
        ms_lpd3dDevice->SetViewport (&ViewPort)
        ))
    {
        SPDLOG_DEBUG("CPythonGraphic::SetViewport({0}, {1}, {2}, {3}) - Error",
                      ViewPort.X, ViewPort.Y,
                      ViewPort.Width, ViewPort.Height
            );
    }
}

void CPythonGraphic::RestoreViewport()
{
    ms_lpd3dDevice->SetViewport(&m_backupViewport);
}

void CPythonGraphic::SetGamma(float fGammaFactor)
{
    D3DCAPS9 d3dCaps;
    D3DGAMMARAMP NewRamp;
    int ui, val;

    ms_lpd3dDevice->GetDeviceCaps(&d3dCaps);

    if (D3DCAPS2_FULLSCREENGAMMA != (d3dCaps.Caps2 & D3DCAPS2_FULLSCREENGAMMA))
        return;

    for (int i = 0; i < 256; ++i)
    {
        val = (int)(i * fGammaFactor * 255.0f);
        ui = 0;

        if (val > 32767)
        {
            val = val - 32767;
            ui = 1;
        }

        if (val > 32767)
            val = 32767;

        NewRamp.red[i] = (WORD)(val | (32768 * ui));
        NewRamp.green[i] = (WORD)(val | (32768 * ui));
        NewRamp.blue[i] = (WORD)(val | (32768 * ui));
    }

    ms_lpd3dDevice->SetGammaRamp(0, D3DSGR_NO_CALIBRATION, &NewRamp);
}

void GenScreenShotTag(const char *src, DWORD crc32, char *leaf, size_t leafLen)
{
    const char *p = src;
    const char *n = p;
    while (n = strchr(p, '\\'))
    {
        p = n + 1;
    }

    _snprintf(leaf, leafLen, "NA2:%s:0x%.8x", p, crc32);
}

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define HRCHECK(__expr) {hr=(__expr);if(FAILED(hr)){wprintf(L"FAILURE 0x%08X (%i)\n\tline: %u file: '%s'\n\texpr: '" WIDEN(#__expr) L"'\n",hr, hr, __LINE__,__WFILE__);goto cleanup;}}
#define RELEASE(__p) {if(__p!=nullptr){__p->Release();__p=nullptr;}}

HRESULT SavePixelsToFile32bppPBGRA(UINT width, UINT height, UINT stride, LPBYTE pixels, LPWSTR filePath,
                                   const GUID &format)
{
    if (!filePath || !pixels)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    IWICImagingFactory *factory = nullptr;
    IWICBitmapEncoder *encoder = nullptr;
    IWICBitmapFrameEncode *frame = nullptr;
    IWICStream *stream = nullptr;
    GUID pf = GUID_WICPixelFormat32bppPBGRA;
    BOOL coInit = CoInitialize(nullptr);

    HRCHECK(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)));
    HRCHECK(factory->CreateStream(&stream));
    HRCHECK(stream->InitializeFromFilename(filePath, GENERIC_WRITE));
    HRCHECK(factory->CreateEncoder(format, nullptr, &encoder));
    HRCHECK(encoder->Initialize(stream, WICBitmapEncoderNoCache));
    HRCHECK(encoder->CreateNewFrame(&frame, nullptr)); // we don't use options here
    HRCHECK(frame->Initialize(nullptr));               // we dont' use any options here
    HRCHECK(frame->SetSize(width, height));
    HRCHECK(frame->SetPixelFormat(&pf));
    HRCHECK(frame->WritePixels(height, stride, stride * height, pixels));
    HRCHECK(frame->Commit());
    HRCHECK(encoder->Commit());

cleanup:
    RELEASE(stream);
    RELEASE(frame);
    RELEASE(encoder);
    RELEASE(factory);
    if (coInit)
        CoUninitialize();
    return hr;
}
#include <fmt/xchar.h>
HRESULT Direct3D9TakeScreenshots(UINT adapter, UINT count, const std::string &name)
{
    std::wstring filename;
    filename.assign(name.begin(), name.end());
    HRESULT hr = S_OK;
    IDirect3D9 *d3d = nullptr;
    IDirect3DDevice9 *device = nullptr;
    IDirect3DSurface9 *surface = nullptr;
    D3DPRESENT_PARAMETERS parameters = {0};
    D3DDISPLAYMODE mode;
    D3DLOCKED_RECT rc;
    UINT pitch;
    SYSTEMTIME st;
    LPBYTE *shots = nullptr;

    // init D3D and get screen size
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    HRCHECK(d3d->GetAdapterDisplayMode(adapter, &mode));

    parameters.Windowed = TRUE;
    parameters.BackBufferCount = 1;
    parameters.BackBufferHeight = mode.Height;
    parameters.BackBufferWidth = mode.Width;
    parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    parameters.hDeviceWindow = NULL;

    // create device & capture surface
    HRCHECK(
        d3d->CreateDevice(adapter, D3DDEVTYPE_HAL, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &parameters, &device));
    HRCHECK(
        device->CreateOffscreenPlainSurface(mode.Width, mode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &surface,
            nullptr));
    // compute the required buffer size
    HRCHECK(surface->LockRect(&rc, NULL, 0));
    pitch = rc.Pitch;
    HRCHECK(surface->UnlockRect());

    // allocate screenshots buffers
    shots = new LPBYTE[count];
    for (UINT i = 0; i < count; i++)
    {
        shots[i] = new BYTE[pitch * mode.Height];
    }

    GetSystemTime(&st); // measure the time we spend doing <count> captures
    wprintf(L"%i:%i:%i.%i\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    for (UINT i = 0; i < count; i++)
    {
        // get the data
        HRCHECK(device->GetFrontBufferData(0, surface));

        // copy it into our buffers
        HRCHECK(surface->LockRect(&rc, NULL, 0));
        CopyMemory(shots[i], rc.pBits, rc.Pitch * mode.Height);
        HRCHECK(surface->UnlockRect());
    }
    GetSystemTime(&st);
    wprintf(L"%i:%i:%i.%i\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    // save all screenshots
    for (UINT i = 0; i < count; i++)
    {
        auto file = fmt::format(L"{}{}.png", filename, i);
        HRCHECK(
            SavePixelsToFile32bppPBGRA(mode.Width, mode.Height, pitch, shots[i], (LPWSTR)file.c_str(),
                GUID_ContainerFormatPng));
    }

cleanup:
    if (shots != nullptr)
    {
        for (UINT i = 0; i < count; i++)
        {
            delete shots[i];
        }
        delete[] shots;
    }
    RELEASE(surface);
    RELEASE(device);
    RELEASE(d3d);

    return hr;
}

bool CPythonGraphic::SaveScreenShot(const char *c_pszFileName)
{
    LPDIRECT3DSURFACE9 surface;
    HRESULT hr = ms_lpd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO,
                                               &surface);

    if (FAILED(hr))
    {
        SPDLOG_ERROR("Failed to get back buffer with {0}", hr);
        return false;
    }

    hr = D3DXSaveSurfaceToFile(c_pszFileName, D3DXIFF_PNG, surface, NULL, NULL);

    surface->Release();

    if (FAILED(hr))
    {
        SPDLOG_ERROR("Failed to save screenshot to {0} with {1}",
                      c_pszFileName, hr);
        return false;
    }

    return true;
}

bool CPythonGraphic::SaveScreenShotFull(const char *c_pszFileName)
{
    Direct3D9TakeScreenshots(0, 1, c_pszFileName);

    return true;
}

void CPythonGraphic::PushState()
{
    TState curState;

    curState.matProj = ms_matProj;
    curState.matView = ms_matView;
    //STATEMANAGER.SaveTransform(D3DTS_WORLD, &m_SaveWorldMatrix);

    m_stateStack.push(curState);
    //CCamera::Instance().PushParams();
}

void CPythonGraphic::PopState()
{
    if (m_stateStack.empty())
    {
        assert(!"PythonGraphic::PopState StateStack is EMPTY");
        return;
    }

    TState &rState = m_stateStack.top();

    //STATEMANAGER.RestoreTransform(D3DTS_WORLD);
    ms_matProj = rState.matProj;
    ms_matView = rState.matView;

    UpdatePipeLineMatrix();

    m_stateStack.pop();
    //CCamera::Instance().PopParams();
}

void CPythonGraphic::RenderImage(CGraphicImageInstance *pImageInstance, float x, float y)
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 0, 0, 0), L"** CPythonGraphic::RenderImage **");

    assert(pImageInstance != NULL);

    //SetColorRenderState();
    const CGraphicTexture *c_pTexture = pImageInstance->GetTexturePointer();

    float width = (float)pImageInstance->GetWidth();
    float height = (float)pImageInstance->GetHeight();

    c_pTexture->SetTextureStage(0);

    RenderTextureBox(x,
                     y,
                     x + width,
                     y + height,
                     0.0f,
                     0.5f / width,
                     0.5f / height,
                     (width + 0.5f) / width,
                     (height + 0.5f) / height);

    D3DPERF_EndEvent();
}

void CPythonGraphic::RenderAlphaImage(CGraphicImageInstance *pImageInstance, float x, float y, float aLeft,
                                      float aRight)
{
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 0, 0, 0), L"** RenderAlphaImage **");

    assert(pImageInstance != NULL);

    DirectX::SimpleMath::Color DiffuseColor1 = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, aLeft);
    DirectX::SimpleMath::Color DiffuseColor2 = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, aRight);

    const CGraphicTexture *c_pTexture = pImageInstance->GetTexturePointer();

    float width = (float)pImageInstance->GetWidth();
    float height = (float)pImageInstance->GetHeight();

    c_pTexture->SetTextureStage(0);

    float sx = x;
    float sy = y;
    float ex = x + width;
    float ey = y + height;
    float z = 0.0f;

    float su = 0.0f;
    float sv = 0.0f;
    float eu = 1.0f;
    float ev = 1.0f;

    TPDTVertex vertices[4];
    vertices[0].position = TPosition(sx, sy, z);
    vertices[0].diffuse = DiffuseColor1.BGRA().c;
    vertices[0].texCoord = TTextureCoordinate(su, sv);

    vertices[1].position = TPosition(ex, sy, z);
    vertices[1].diffuse = DiffuseColor2.BGRA().c;
    vertices[1].texCoord = TTextureCoordinate(eu, sv);

    vertices[2].position = TPosition(sx, ey, z);
    vertices[2].diffuse = DiffuseColor1.BGRA().c;
    vertices[2].texCoord = TTextureCoordinate(su, ev);

    vertices[3].position = TPosition(ex, ey, z);
    vertices[3].diffuse = DiffuseColor2.BGRA().c;
    vertices[3].texCoord = TTextureCoordinate(eu, ev);

    Engine::GetDevice().SetFvF(TPDTVertex::kFVF);
    // 2004.11.18.myevan.DrawIndexPrimitiveUP -> DynamicVertexBuffer
    Engine::GetDevice().SetDefaultIndexBuffer(DEFAULT_IB_FILL_RECT);
    if (Engine::GetDevice().SetPDTStream(vertices, 4))
    {
        Engine::GetDevice().DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
    }
    D3DPERF_EndEvent();
}

void CPythonGraphic::RenderCoolTimeBoxColor(float fxCenter, float fyCenter, float fRadius, float fTime, float fColorR,
                                            float fColorG, float fColorB, float fColorA)
{
    if (fTime >= 1.0f)
    {
        return;
    }

    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CPythonGraphic::RenderCoolTimeBoxColor **");
    fTime = std::max(0.0f, fTime);

    static DirectX::SimpleMath::Color color = DirectX::SimpleMath::Color(fColorR, fColorG, fColorB, fColorA);
    static WORD s_wBoxIndicies[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    static Vector2 s_v2BoxPos[8] =
    {
        Vector2(-1.0f, -1.0f),
        Vector2(-1.0f, 0.0f),
        Vector2(-1.0f, +1.0f),
        Vector2(0.0f, +1.0f),
        Vector2(+1.0f, +1.0f),
        Vector2(+1.0f, 0.0f),
        Vector2(+1.0f, -1.0f),
        Vector2(0.0f, -1.0f),
    };

    int iTriCount = int(8 - 8.0f * fTime);
    float fLastPercentage = (8 - 8.0f * fTime) - iTriCount;

    std::vector<TPDTVertex> vertices;
    TPDTVertex vertex;
    vertex.position.x = fxCenter;
    vertex.position.y = fyCenter;
    vertex.position.z = 0.0f;
    vertex.diffuse = color.BGRA().c;
    vertex.texCoord.x = 0.0f;
    vertex.texCoord.x = 0.0f;
    vertices.push_back(vertex);
    vertex.position.x = fxCenter;
    vertex.position.y = fyCenter - fRadius;
    vertex.position.z = 0.0f;
    vertex.diffuse = color.BGRA().c;
    vertex.texCoord.x = 0.0f;
    vertex.texCoord.x = 0.0f;
    vertices.push_back(vertex);

    for (int j = 0; j < iTriCount; ++j)
    {
        vertex.position.x = fxCenter + s_v2BoxPos[j].x * fRadius;
        vertex.position.y = fyCenter + s_v2BoxPos[j].y * fRadius;
        vertices.push_back(vertex);
    }

    if (fLastPercentage > 0.0f)
    {
        Vector2 *pv2Pos;
        Vector2 *pv2LastPos;

        assert((iTriCount - 1 + 8) % 8 >= 0 && (iTriCount - 1 + 8) % 8 < 8);
        assert((iTriCount + 8) % 8 >= 0 && (iTriCount + 8) % 8 < 8);
        pv2LastPos = &s_v2BoxPos[(iTriCount - 1 + 8) % 8];
        pv2Pos = &s_v2BoxPos[(iTriCount + 8) % 8];

        vertex.position.x = fxCenter + ((pv2Pos->x - pv2LastPos->x) * fLastPercentage + pv2LastPos->x) * fRadius;
        vertex.position.y = fyCenter + ((pv2Pos->y - pv2LastPos->y) * fLastPercentage + pv2LastPos->y) * fRadius;
        vertices.push_back(vertex);
        ++iTriCount;
    }

    if (vertices.empty())
    {
        return;
    }

    if (Engine::GetDevice().SetPDTStream(&vertices[0], vertices.size()))
    {
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        STATEMANAGER.SetTexture(0, NULL);
        STATEMANAGER.SetTexture(1, NULL);
        Engine::GetDevice().SetFvF(TPDTVertex::kFVF);
        Engine::GetDevice().DrawPrimitive(D3DPT_TRIANGLEFAN, 0, iTriCount);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
    }

    D3DPERF_EndEvent();
}

void CPythonGraphic::RenderCoolTimeImageBox(CGraphicImageInstance *image, float fTime) const
{
    image->OnRenderWithCoolTimeBox(fTime);
}

void CPythonGraphic::RenderCoolTimeBoxInverse(float fxCenter, float fyCenter, float fRadius, float fTime)
{
    if (fTime >= 1.0f)
    {
        return;
    }
    D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CPythonGraphic::RenderCoolTimeBoxInverse **");

    fTime = std::max(0.0f, fTime);

    static DirectX::SimpleMath::Color color = DirectX::SimpleMath::Color(0.0f, 0.0f, 0.0f, 0.5f);

    static Vector2 s_v2BoxPos[8] =
    {
        Vector2(+1.0f, -1.0f),
        Vector2(+1.0f, 0.0f),
        Vector2(+1.0f, +1.0f),
        Vector2(0.0f, +1.0f),
        Vector2(-1.0f, +1.0f),
        Vector2(-1.0f, 0.0f),
        Vector2(-1.0f, -1.0f),
        Vector2(0.0f, -1.0f),
    };

    int iTriCount = int(fTime * 8.0f);
    float fLastPercentage = ((fTime * 8.0f + 1.0f) - (fTime * 8.0f));

    std::vector<TPDTVertex> vertices;
    TPDTVertex vertex;
    vertex.position.x = fxCenter;
    vertex.position.y = fyCenter;
    vertex.position.z = 0.0f;
    vertex.diffuse = color.BGRA().c;
    vertex.texCoord.x = 0.0f;
    vertex.texCoord.x = 0.0f;
    vertices.push_back(vertex);

    vertex.position.x = fxCenter;
    vertex.position.y = fyCenter - fRadius;
    vertex.position.z = 0.0f;
    vertex.diffuse = color.BGRA().c;
    vertex.texCoord.x = 0.0f;
    vertex.texCoord.x = 0.0f;
    vertices.push_back(vertex);

    for (int j = 0; j < iTriCount; ++j)
    {
        vertex.position.x = s_v2BoxPos[j].x * fRadius + fxCenter;
        vertex.position.y = s_v2BoxPos[j].y * fRadius + fyCenter;
        vertices.push_back(vertex);
    }

    if (fLastPercentage > 0.0f)
    {
        Vector2 *pv2Pos;
        Vector2 *pv2LastPos;

        pv2LastPos = &s_v2BoxPos[(iTriCount + 8) % 8];
        pv2Pos = &s_v2BoxPos[((iTriCount + 7) % 8)];
        vertex.position.x = ((pv2Pos->x - pv2LastPos->x) * fLastPercentage + pv2LastPos->x) * fRadius + fxCenter;
        vertex.position.y = ((pv2Pos->y - pv2LastPos->y) * fLastPercentage + pv2LastPos->y) * fRadius + fyCenter;

        vertices.push_back(vertex);
        ++iTriCount;
    }

    if (vertices.empty())
    {
        return;
    }

    if (Engine::GetDevice().SetPDTStream(vertices.data(), vertices.size()))
    {
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        STATEMANAGER.SetTexture(0, NULL);
        STATEMANAGER.SetTexture(1, NULL);
        Engine::GetDevice().SetFvF(TPDTVertex::kFVF);
        DWORD cullMode;
        STATEMANAGER.GetRenderState(D3DRS_CULLMODE, &cullMode);
        STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        Engine::GetDevice().DrawPrimitive(D3DPT_TRIANGLEFAN, 0, iTriCount);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
        STATEMANAGER.SetRenderState(D3DRS_CULLMODE, cullMode);
    }
    D3DPERF_EndEvent();
}

long CPythonGraphic::GenerateColor(float r, float g, float b, float a)
{
    return GetColor(r, g, b, a);
}

void CPythonGraphic::RenderDownButton(float sx, float sy, float ex, float ey)
{
    RenderBox2d(sx, sy, ex, ey);

    SetDiffuseColor(m_darkColor);
    RenderLine2d(sx, sy, ex, sy);
    RenderLine2d(sx, sy, sx, ey);

    SetDiffuseColor(m_lightColor);
    RenderLine2d(sx, ey, ex, ey);
    RenderLine2d(ex, sy, ex, ey);
}

void CPythonGraphic::RenderUpButton(float sx, float sy, float ex, float ey)
{
    RenderBox2d(sx, sy, ex, ey);

    SetDiffuseColor(m_lightColor);
    RenderLine2d(sx, sy, ex, sy);
    RenderLine2d(sx, sy, sx, ey);

    SetDiffuseColor(m_darkColor);
    RenderLine2d(sx, ey, ex, ey);
    RenderLine2d(ex, sy, ex, ey);
}

DWORD CPythonGraphic::GetAvailableMemory()
{
    return ms_lpd3dDevice->GetAvailableTextureMem();
}

CPythonGraphic::CPythonGraphic()
{
    m_lightColor = GetColor(1.0f, 1.0f, 1.0f);
    m_darkColor = GetColor(0.0f, 0.0f, 0.0f);

    memset(&m_backupViewport, 0, sizeof(D3DVIEWPORT9));

    m_fOrthoDepth = 1000.0f;
}

CPythonGraphic::~CPythonGraphic()
{
}
