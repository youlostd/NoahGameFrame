#include "StdAfx.h"
#include "RenderTargetManager.h"
#include "../EterBase/Stl.h"
#include "GrpBase.h"

CRenderTargetManager::CRenderTargetManager()
    : m_widht(800),
      m_height(600), m_isShow{false},
      m_currentRenderTarget(nullptr)
{
}

CRenderTargetManager::~CRenderTargetManager()
{
    Destroy();
}

void CRenderTargetManager::Destroy()
{
    stl_wipe_second(m_renderTargets);
    stl_wipe_second(m_wikiRenderTargets);

    m_widht = 800;
    m_height = 600;
    m_currentRenderTarget = nullptr;
}

void CRenderTargetManager::CreateRenderTargetTextures()
{
    for (auto & [fst, snd] : m_renderTargets)
        snd->OnRestoreDevice();

    for (auto & [fst, snd] : m_wikiRenderTargets)
        snd->OnRestoreDevice();
}

void CRenderTargetManager::ReleaseRenderTargetTextures()
{
    for (auto & [fst, snd] : m_renderTargets)
        snd->OnLostDevice();

    for (auto & [fst, snd] : m_wikiRenderTargets)
        snd->OnLostDevice();
}

bool CRenderTargetManager::CreateRenderTarget(int width, int height)
{
    m_widht = width;
    m_height = height;
    return CreateGraphicTexture(0, width, height, D3DFMT_X8R8G8B8, D3DFMT_D16);
}

bool CRenderTargetManager::CreateRenderTargetWithIndex(int width, int height, uint32_t index)
{
    m_widht = width;
    m_height = height;

    return CreateGraphicTexture(index, width, height, D3DFMT_X8R8G8B8, D3DFMT_D16);
}

bool CRenderTargetManager::GetRenderTargetRect(uint32_t index, RECT &rect)
{
    auto* pTarget = GetRenderTarget(index);
    if (!pTarget)
        return false;

    rect = *pTarget->GetRenderingRect();
    return true;
}

bool CRenderTargetManager::GetWikiRenderTargetRect(DWORD index, RECT &rect)
{
    CGraphicRenderTargetTexture *pTarget = GetWikiRenderTarget(index);
    if (!pTarget)
        return false;

    rect = *pTarget->GetRenderingRect();
    return true;
}

bool CRenderTargetManager::ChangeWikiRenderTarget(DWORD index)
{
    m_currentRenderTarget = GetWikiRenderTarget(index);
    if (!m_currentRenderTarget)
        return false;

    return m_currentRenderTarget->SetRenderTarget();
}

bool CRenderTargetManager::ChangeRenderTarget(uint32_t index)
{
    m_currentRenderTarget = GetRenderTarget(index);

    if (!m_currentRenderTarget)
        return false;

    return m_currentRenderTarget->SetRenderTarget();
}

void CRenderTargetManager::ResetRenderTarget()
{
    if (m_currentRenderTarget)
    {
        m_currentRenderTarget->ResetRenderTarget();
        m_currentRenderTarget = nullptr;
    }
}

void CRenderTargetManager::ClearRenderTarget(uint32_t color) const
{
    if (m_currentRenderTarget)
        ms_lpd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
}

CGraphicRenderTargetTexture *CRenderTargetManager::GetWikiRenderTarget(DWORD index)
{
    auto it = m_wikiRenderTargets.find(index);
    if (it != m_wikiRenderTargets.end())
        return it->second;

    return nullptr;
}

CGraphicRenderTargetTexture *CRenderTargetManager::GetRenderTarget(uint32_t index)
{
    auto it = m_renderTargets.find(index);
    if (it != m_renderTargets.end())
        return it->second;

    return nullptr;
}

CGraphicRenderTargetTexture *CRenderTargetManager::CreateWikiRenderTarget(DWORD index, DWORD width, DWORD height)
{
   if (auto* p = GetWikiRenderTarget(index); p)
		return p;

    OutputDebugString(fmt::format("CreateWikiRenderTarget {}", index).c_str());

	auto* pTex = new CGraphicRenderTargetTexture;
	if (!pTex->Create(width, height, D3DFMT_A8R8G8B8)) {
		delete pTex;
		return nullptr;
	}

	m_wikiRenderTargets.emplace(index, pTex);
	return pTex;
}

bool CRenderTargetManager::CreateGraphicTexture(uint32_t index, uint32_t width, uint32_t height, D3DFORMAT texFormat,
                                                D3DFORMAT dephtFormat)
{
	if (index > 3 || GetRenderTarget(index))
		return false;

	auto* pTex = new CGraphicRenderTargetTexture;
	if (!pTex->Create(width, height, texFormat)) {
		delete pTex;
		return false;
	}

	m_renderTargets.emplace(index, pTex);
	return true;
}
