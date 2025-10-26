#include "StdAfx.h"
#include "StateManager.h"

#define StateManager_Assert(a) assert(a)

CStateManager::CStateManager(LPDIRECT3DDEVICE9 lpDevice)
    : m_bForce(false)
      , m_bScene(false)
      , m_dwBestMinFilter(D3DTEXF_LINEAR)
      , m_dwBestMagFilter(D3DTEXF_LINEAR)
      , m_lpD3DDev(NULL)
{
    SetDevice(lpDevice);
}

CStateManager::~CStateManager()
{
    if (m_lpD3DDev)
    {
        m_lpD3DDev->Release();
        m_lpD3DDev = NULL;
    }
}

void CStateManager::SetDefaultState()
{
      UpdateAnisotropy();

    m_CurrentState.ResetState();
    m_CopyState.ResetState();

    m_bScene = false;
    m_bForce = true;



    SetTransform(D3DTS_WORLD, &Matrix::Identity);
    SetTransform(D3DTS_VIEW, &Matrix::Identity);
    SetTransform(D3DTS_PROJECTION, &Matrix::Identity);

    D3DMATERIAL9 DefaultMat;
    ZeroMemory(&DefaultMat, sizeof(D3DMATERIAL9));

    DefaultMat.Diffuse.r = 1.0f;
    DefaultMat.Diffuse.g = 1.0f;
    DefaultMat.Diffuse.b = 1.0f;
    DefaultMat.Diffuse.a = 1.0f;
    DefaultMat.Ambient.r = 1.0f;
    DefaultMat.Ambient.g = 1.0f;
    DefaultMat.Ambient.b = 1.0f;
    DefaultMat.Ambient.a = 1.0f;
    DefaultMat.Emissive.r = 0.0f;
    DefaultMat.Emissive.g = 0.0f;
    DefaultMat.Emissive.b = 0.0f;
    DefaultMat.Emissive.a = 0.0f;
    DefaultMat.Specular.r = 0.0f;
    DefaultMat.Specular.g = 0.0f;
    DefaultMat.Specular.b = 0.0f;
    DefaultMat.Specular.a = 0.0f;
    DefaultMat.Power = 0.0f;

    SetMaterial(&DefaultMat);

    SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
    SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
    SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
    SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);

    SetRenderState(D3DRS_LASTPIXEL, false);
    SetRenderState(D3DRS_ALPHAREF, 1);
    SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    SetRenderState(D3DRS_FOGSTART, 0);
    SetRenderState(D3DRS_FOGEND, 0);
    SetRenderState(D3DRS_FOGDENSITY, 0);
    SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
    SetRenderState(D3DRS_AMBIENT, 0x00000000);
    SetRenderState(D3DRS_LOCALVIEWER, false);
    SetRenderState(D3DRS_NORMALIZENORMALS, false);
    SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
    SetRenderState(D3DRS_CLIPPLANEENABLE, 0);

    SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, false);
    SetRenderState(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);
    SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, false);
    SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
    SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    SetRenderState(D3DRS_ALPHABLENDENABLE, false);
    SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    SetRenderState(D3DRS_FOGENABLE, false);
    SetRenderState(D3DRS_FOGCOLOR, 0xFF000000);
    SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
    SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
    SetRenderState(D3DRS_RANGEFOGENABLE, false);
    //SetRenderState(D3DRS_ZENABLE, true);
    //SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    //SetRenderState(D3DRS_ZWRITEENABLE, true);
    SetRenderState(D3DRS_DITHERENABLE, true);
    SetRenderState(D3DRS_STENCILENABLE, false);
    SetRenderState(D3DRS_ALPHATESTENABLE, false);
    SetRenderState(D3DRS_CLIPPING, true);
    SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    SetRenderState(D3DRS_LIGHTING, false);
    SetRenderState(D3DRS_SPECULARENABLE, false);
    SetRenderState(D3DRS_COLORVERTEX, false);
    SetRenderState(D3DRS_WRAP0, 0);
    SetRenderState(D3DRS_WRAP1, 0);
    SetRenderState(D3DRS_WRAP2, 0);
    SetRenderState(D3DRS_WRAP3, 0);
    SetRenderState(D3DRS_WRAP4, 0);
    SetRenderState(D3DRS_WRAP5, 0);
    SetRenderState(D3DRS_WRAP6, 0);
    SetRenderState(D3DRS_WRAP7, 0);

    SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

    for (int i = 1; i < 8; ++i)
    {
        SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
        SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }

    for (int i = 0; i < 8; ++i)
        SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, i);

    for (DWORD i = 0; i < 8; ++i)
    {
        SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

        SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

        SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, 0);

        SetTexture(i, NULL);
    }

    m_bForce = false;

#ifdef _DEBUG
    for (int i = 0; i < STATEMANAGER_MAX_RENDERSTATES; i++)
        m_bRenderStateSavingFlag[i] = false;

    for (int j = 0; j < STATEMANAGER_MAX_TRANSFORMSTATES; j++)
        m_bTransformSavingFlag[j] = false;

    for (int j = 0; j < STATEMANAGER_MAX_STAGES; ++j)
    {
        for (int i = 0; i < STATEMANAGER_MAX_TEXTURESTATES; ++i)
            m_bTextureStageStateSavingFlag[j][i] = false;

        for (int i = 0; i < STATEMANAGER_MAX_SAMPLERSTATES; ++i)
            m_bSamplerStateSavingFlag[j][i] = false;
    }
#endif _DEBUG
}

void CStateManager::Restore()
{
    m_bForce = true;

    for (int i = 0; i < STATEMANAGER_MAX_RENDERSTATES; ++i)
        SetRenderState(D3DRENDERSTATETYPE(i), m_CurrentState.m_RenderStates[i]);

    for (int i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
        for (int j = 0; j < STATEMANAGER_MAX_TEXTURESTATES; ++j)
            SetTextureStageState(i, D3DTEXTURESTAGESTATETYPE(j), m_CurrentState.m_TextureStates[i][j]);

    for (int i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
        SetTexture(i, m_CurrentState.m_Textures[i]);

    m_bForce = false;
}

bool CStateManager::BeginScene()
{
    m_bScene = true;

    Matrix m4Proj;
    Matrix m4View;
    Matrix m4World;
    GetTransform(D3DTS_WORLD, &m4World);
    GetTransform(D3DTS_PROJECTION, &m4Proj);
    GetTransform(D3DTS_VIEW, &m4View);
    SetTransform(D3DTS_WORLD, &m4World);
    SetTransform(D3DTS_PROJECTION, &m4Proj);
    SetTransform(D3DTS_VIEW, &m4View);

    HRESULT hr = m_lpD3DDev->BeginScene();
    if (FAILED(hr))
    {
        SPDLOG_ERROR("BeginScene failed with {0}", hr);
        return false;
    }

    return true;
}

void CStateManager::EndScene()
{
    m_lpD3DDev->EndScene();
    m_bScene = false;
}

// Material
void CStateManager::SaveMaterial()
{
    m_CopyState.m_D3DMaterial = m_CurrentState.m_D3DMaterial;
}

void CStateManager::SaveMaterial(const D3DMATERIAL9 *pMaterial)
{
    // Check that we have set this up before, if not, the default is this.
    m_CopyState.m_D3DMaterial = m_CurrentState.m_D3DMaterial;
    SetMaterial(pMaterial);
}

void CStateManager::RestoreMaterial()
{
    SetMaterial(&m_CopyState.m_D3DMaterial);
}

void CStateManager::SetMaterial(const D3DMATERIAL9 *pMaterial)
{
    m_lpD3DDev->SetMaterial(pMaterial);
    m_CurrentState.m_D3DMaterial = *pMaterial;
}

void CStateManager::GetMaterial(D3DMATERIAL9 *pMaterial)
{
    // Set the renderstate and remember it.
    *pMaterial = m_CurrentState.m_D3DMaterial;
}

void CStateManager::SetLight(DWORD index, CONST D3DLIGHT9 *pLight)
{
    m_lpD3DDev->SetLight(index, pLight);
}

void CStateManager::GetLight(DWORD index, D3DLIGHT9 *pLight)
{
    m_lpD3DDev->GetLight(index, pLight);
}

// Renderstates
DWORD CStateManager::GetRenderState(D3DRENDERSTATETYPE Type)
{
    return m_CurrentState.m_RenderStates[Type];
}

void CStateManager::SaveRenderState(D3DRENDERSTATETYPE Type, DWORD dwValue)
{
#ifdef _DEBUG
    if (m_bRenderStateSavingFlag[Type])
    {
        SPDLOG_TRACE("CStateManager::SaveRenderState - This render state is already saved [{0}, {1}]", Type, dwValue);
        StateManager_Assert(!"This render state is already saved!");
    }
    m_bRenderStateSavingFlag[Type] = true;
#endif

    // Check that we have set this up before, if not, the default is this.
    m_CopyState.m_RenderStates[Type] = m_CurrentState.m_RenderStates[Type];
    SetRenderState(Type, dwValue);
}

void CStateManager::RestoreRenderState(D3DRENDERSTATETYPE Type)
{
#ifdef _DEBUG
    if (!m_bRenderStateSavingFlag[Type])
    {
        SPDLOG_TRACE("CStateManager::SaveRenderState - This render state was not saved [{0}]", Type);
        StateManager_Assert(!" This render state was not saved!");
    }
    m_bRenderStateSavingFlag[Type] = false;
#endif

    SetRenderState(Type, m_CopyState.m_RenderStates[Type]);
}

void CStateManager::SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value)
{
    if (m_CurrentState.m_RenderStates[Type] == Value)
        return;

    m_lpD3DDev->SetRenderState(Type, Value);
    m_CurrentState.m_RenderStates[Type] = Value;
}

void CStateManager::GetRenderState(D3DRENDERSTATETYPE Type, DWORD *pdwValue)
{
    *pdwValue = m_CurrentState.m_RenderStates[Type];
}

// Textures
void CStateManager::SaveTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture)
{
    // Check that we have set this up before, if not, the default is this.
    m_CopyState.m_Textures[dwStage] = m_CurrentState.m_Textures[dwStage];
    SetTexture(dwStage, pTexture);
}

void CStateManager::SetScissorRect(RECT scissorRect)
{
    if (!std::memcmp(&m_CurrentState.m_scissorRect, &scissorRect, sizeof(RECT)))
        return;

    m_CurrentState.m_scissorRect = scissorRect;
    m_lpD3DDev->SetScissorRect(&m_CurrentState.m_scissorRect);
}

void CStateManager::RestoreScissorRect()
{
    SetScissorRect(m_CopyState.m_scissorRect);
}

void CStateManager::SaveScissorRect(RECT scissorRect)
{
    // Check that we have set this up before, if not, the default is this.
    m_CopyState.m_scissorRect = m_CurrentState.m_scissorRect;
    SetScissorRect(scissorRect);
}

void CStateManager::RestoreTexture(DWORD dwStage)
{
    SetTexture(dwStage, m_CopyState.m_Textures[dwStage]);
}

void CStateManager::SetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture)
{
    if (pTexture == m_CurrentState.m_Textures[dwStage])
        return;

    m_lpD3DDev->SetTexture(dwStage, pTexture);
    m_CurrentState.m_Textures[dwStage] = pTexture;
}

void CStateManager::GetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 *ppTexture)
{
    *ppTexture = m_CurrentState.m_Textures[dwStage];
}

// Texture stage states
void CStateManager::SaveTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type, DWORD dwValue)
{
    // Check that we have set this up before, if not, the default is this.
#ifdef _DEBUG
    if (m_bTextureStageStateSavingFlag[dwStage][Type])
    {
        SPDLOG_TRACE(" CStateManager::SaveTextureStageState - This texture stage state is already saved [{0}, {1}]",
                     dwStage, Type);
        StateManager_Assert(!" This texture stage state is already saved!");
    }
    m_bTextureStageStateSavingFlag[dwStage][Type] = true;
#endif

    m_CopyState.m_TextureStates[dwStage][Type] = m_CurrentState.m_TextureStates[dwStage][Type];
    SetTextureStageState(dwStage, Type, dwValue);
}

void CStateManager::RestoreTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type)
{
#ifdef _DEBUG
    if (!m_bTextureStageStateSavingFlag[dwStage][Type])
    {
        SPDLOG_TRACE(" CStateManager::RestoreTextureStageState - This texture stage state was not saved [{0}, {1}]",
                     dwStage, Type);
        assert(!" This texture stage state was not saved!");
    }
    m_bTextureStageStateSavingFlag[dwStage][Type] = false;
#endif

    SetTextureStageState(dwStage, Type, m_CopyState.m_TextureStates[dwStage][Type]);
}

void CStateManager::SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type, DWORD dwValue)
{
    if (m_CurrentState.m_TextureStates[dwStage][Type] == dwValue)
        return;

    m_lpD3DDev->SetTextureStageState(dwStage, Type, dwValue);
    m_CurrentState.m_TextureStates[dwStage][Type] = dwValue;
}

void CStateManager::GetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pdwValue)
{
    *pdwValue = m_CurrentState.m_TextureStates[dwStage][Type];
}

void CStateManager::SaveSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD dwValue)
{
    // Check that we have set this up before, if not, the default is this.
#ifdef _DEBUG
    if (m_bSamplerStateSavingFlag[dwStage][Type])
    {
        SPDLOG_TRACE("CStateManager::SaveSamplerState - This texture stage state is already saved [{0}, {1}]", dwStage,
                     Type);
        StateManager_Assert(!" This texture stage state is already saved!");
    }

    m_bSamplerStateSavingFlag[dwStage][Type] = true;
#endif

    m_CopyState.m_SamplerStates[dwStage][Type] = m_CurrentState.m_SamplerStates[dwStage][Type];
    SetSamplerState(dwStage, Type, dwValue);
}

void CStateManager::RestoreSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type)
{
#ifdef _DEBUG
    if (!m_bSamplerStateSavingFlag[dwStage][Type])
    {
        SPDLOG_TRACE(" CStateManager::RestoreSamplerState - This texture stage state was not saved [{0}, {1}]", dwStage,
                     Type);
        StateManager_Assert(!" This texture stage sampler state was not saved!");
    }

    m_bSamplerStateSavingFlag[dwStage][Type] = false;
#endif

    SetSamplerState(dwStage, Type, m_CopyState.m_SamplerStates[dwStage][Type]);
}

void CStateManager::SetSamplerState(DWORD dwStage, _D3DSAMPLERSTATETYPE Type, DWORD dwValue)
{
    if (m_CurrentState.m_SamplerStates[dwStage][Type] == dwValue)
        return;

    m_lpD3DDev->SetSamplerState(dwStage, Type, dwValue);
    m_CurrentState.m_SamplerStates[dwStage][Type] = dwValue;
}

void CStateManager::GetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD *pdwValue)
{
    *pdwValue = m_CurrentState.m_SamplerStates[dwStage][Type];
}

void CStateManager::SetBestFiltering(DWORD dwStage)
{
    SetSamplerState(dwStage, D3DSAMP_MINFILTER, m_dwBestMinFilter);
    SetSamplerState(dwStage, D3DSAMP_MAGFILTER, m_dwBestMagFilter);
    SetSamplerState(dwStage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

// *** These states are cached, but not protected from multiple sends of the same value.
// Transform
void CStateManager::SaveTransform(D3DTRANSFORMSTATETYPE Type, const Matrix *pMatrix)
{
#if defined(_DEBUG) && defined(StateManager_Debug)
	if (m_bTransformSavingFlag[Type])
	{
		SPDLOG_TRACE( " CStateManager::SaveTransform - This transform is already saved [{0}]", Type);
		StateManager_Assert(!" This trasform is already saved!");
	}
	m_bTransformSavingFlag[Type] = true;
#endif

    m_CopyState.m_Matrices[Type] = m_CurrentState.m_Matrices[Type];
    SetTransform(Type, (Matrix *)pMatrix);
}

void CStateManager::RestoreTransform(D3DTRANSFORMSTATETYPE Type)
{
#if defined(_DEBUG) && defined(StateManager_Debug)
	if (!m_bTransformSavingFlag[Type])
	{
		SPDLOG_TRACE( " CStateManager::RestoreTransform - This transform was not saved [{0}]\n", Type);
		StateManager_Assert(!" This render state was not saved!");
	}
	m_bTransformSavingFlag[Type] = false;
#endif

    SetTransform(Type, &m_CopyState.m_Matrices[Type]);
}

// Don't cache-check the transform. Too much to do
void CStateManager::SetTransform(D3DTRANSFORMSTATETYPE Type, const Matrix *pMatrix)
{
    if (m_bScene)
        m_lpD3DDev->SetTransform(Type, (D3DMATRIX*)pMatrix);
    else
        assert(D3DTS_VIEW==Type || D3DTS_PROJECTION==Type || D3DTS_WORLD==Type);

    m_CurrentState.m_Matrices[Type] = *pMatrix;
}

void CStateManager::GetTransform(D3DTRANSFORMSTATETYPE Type, Matrix *pMatrix)
{
    *pMatrix = m_CurrentState.m_Matrices[Type];
}

void CStateManager::SaveStreamSource(UINT StreamNumber, LPDIRECT3DVERTEXBUFFER9 pStreamData, UINT Stride)
{
    // Check that we have set this up before, if not, the default is this.
    m_CopyState.m_StreamData[StreamNumber] = m_CurrentState.m_StreamData[StreamNumber];
    SetStreamSource(StreamNumber, pStreamData, Stride);
}

void CStateManager::RestoreStreamSource(UINT StreamNumber)
{
    SetStreamSource(StreamNumber,
                    m_CopyState.m_StreamData[StreamNumber].m_lpStreamData,
                    m_CopyState.m_StreamData[StreamNumber].m_Stride);
}

void CStateManager::SetStreamSource(UINT StreamNumber, LPDIRECT3DVERTEXBUFFER9 pStreamData, UINT Stride)
{
    CStreamData kStreamData(pStreamData, Stride);
    if (m_CurrentState.m_StreamData[StreamNumber] == kStreamData)
        return;

    m_lpD3DDev->SetStreamSource(StreamNumber, pStreamData, 0, Stride);
    m_CurrentState.m_StreamData[StreamNumber] = kStreamData;
}

void CStateManager::SaveIndices(LPDIRECT3DINDEXBUFFER9 pIndexData)
{
    m_CopyState.m_indexBuffer = m_CurrentState.m_indexBuffer;
    SetIndices(pIndexData);
}

void CStateManager::RestoreIndices()
{
    SetIndices(m_CopyState.m_indexBuffer);
}

void CStateManager::SetIndices(LPDIRECT3DINDEXBUFFER9 pIndexData)
{
    if (m_CurrentState.m_indexBuffer == pIndexData)
        return;

    m_lpD3DDev->SetIndices(pIndexData);
    m_CurrentState.m_indexBuffer = pIndexData;
}

// Vertex Shader
void CStateManager::SaveVertexShader(LPDIRECT3DVERTEXSHADER9 lpShader)
{
	m_CopyState.m_lpVertexShader = m_CurrentState.m_lpVertexShader;
	SetVertexShader(lpShader);
}

void CStateManager::RestoreVertexShader()
{
	SetVertexShader(m_CopyState.m_lpVertexShader);
}

void CStateManager::SetVertexShader(LPDIRECT3DVERTEXSHADER9 lpShader)
{
	if (m_CurrentState.m_lpVertexShader != lpShader)
	{
		m_CurrentState.m_lpVertexShader = lpShader;
		m_lpD3DDev->SetVertexShader(lpShader);
	}
}

void CStateManager::GetVertexShader(LPDIRECT3DVERTEXSHADER9 *lppShader)
{
	*lppShader = m_CurrentState.m_lpVertexShader;
}

void CStateManager::SaveVertexShaderConstant(UINT uiRegister, const float* pConstantData, UINT uiConstantCount)
{
	for (UINT i = 0; i < uiConstantCount; i++)
	{
		StateManager_Assert((uiRegister + i) < STATEMANAGER_MAX_VCONSTANTS);
		m_CopyState.m_VertexShaderConstants[uiRegister + i] = m_CurrentState.m_VertexShaderConstants[uiRegister + i];
	}

	SetVertexShaderConstant(uiRegister, pConstantData, uiConstantCount);
}

void CStateManager::RestoreVertexShaderConstant(UINT uiRegister, UINT uiConstantCount)
{
	SetVertexShaderConstant(uiRegister, &m_CopyState.m_VertexShaderConstants[uiRegister], uiConstantCount);
}

void CStateManager::SetVertexShaderConstant(UINT uiRegister, const float* pConstantData, UINT uiConstantCount)
{
	m_lpD3DDev->SetVertexShaderConstantF(uiRegister, pConstantData, uiConstantCount);  // Converting2DirectX9

	// Set the renderstate and remember it.
	for (UINT i = 0; i < uiConstantCount; i++)
	{
		StateManager_Assert((uiRegister + i) < STATEMANAGER_MAX_VCONSTANTS);
		m_CurrentState.m_VertexShaderConstants[uiRegister + i] = *(pConstantData + i);
	}
}

void CStateManager::UpdateAnisotropy() {
  D3DCAPS9 d3dCaps;
  m_lpD3DDev->GetDeviceCaps(&d3dCaps);

  if (d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
    m_dwBestMagFilter = D3DTEXF_ANISOTROPIC;
  else
    m_dwBestMagFilter = D3DTEXF_LINEAR;

  if (d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
    m_dwBestMinFilter = D3DTEXF_ANISOTROPIC;
  else
    m_dwBestMinFilter = D3DTEXF_LINEAR;

  DWORD dwMax = d3dCaps.MaxAnisotropy;
  dwMax = dwMax < 4 ? dwMax : 4;

  for (int i = 0; i < 8; ++i)
    m_lpD3DDev->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, dwMax);

  m_lpD3DDev->SetSoftwareVertexProcessing(false);
}

void CStateManager::SetDevice(LPDIRECT3DDEVICE9 lpDevice)
{
    StateManager_Assert(lpDevice);
    lpDevice->AddRef();

    if (m_lpD3DDev)
    {
        m_lpD3DDev->Release();
        m_lpD3DDev = NULL;
    }

    m_lpD3DDev = lpDevice;


    SetDefaultState();
}
