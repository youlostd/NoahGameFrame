#include "StdAfx.h"
#include "../eterBase/Stl.h"
#include "GrpMeshVertexBuffer.h"
#include "StateManager.h"
#include "Util.h"

CGraphicMeshVertexBuffer::CGraphicMeshVertexBuffer()
	: m_vb(nullptr)
	, m_usage(0)
	, m_pool(D3DPOOL_MANAGED)
	, m_vertexCount(0)
	, m_lockFlags(0)
{
	// ctor
}

CGraphicMeshVertexBuffer::~CGraphicMeshVertexBuffer()
{
	DestroyDeviceObjects();
}

bool CGraphicMeshVertexBuffer::Create(uint32_t vertexCount, uint32_t vertexSize, uint32_t usage, D3DPOOL pool)
{
	assert(ms_lpd3dDevice != NULL);
	assert(vertexCount > 0);

	DestroyDeviceObjects();

	m_vertexCount = vertexCount;
	m_pool = pool;
	m_usage = usage;
	m_vertexSize = vertexSize;

	if ((usage & D3DUSAGE_WRITEONLY) || (usage & D3DUSAGE_DYNAMIC))
		m_lockFlags = 0;
	else
		m_lockFlags = D3DLOCK_READONLY;

	return CreateDeviceObjects();
}

void CGraphicMeshVertexBuffer::Destroy()
{
	DestroyDeviceObjects();
}

bool CGraphicMeshVertexBuffer::CreateDeviceObjects()
{
	assert(ms_lpd3dDevice != NULL);
	assert(m_vb == NULL);

	const auto size = m_vertexSize * m_vertexCount;
	const auto hr = ms_lpd3dDevice->CreateVertexBuffer(size,
		m_usage,
		0,
		m_pool,
		&m_vb,
		NULL);
	if (FAILED(hr)) {
		SPDLOG_ERROR("CreateVertexBuffer() failed for "
			"size {0} usage {1} vertexSize {2} pool {3} with {4}",
			size, m_usage, m_vertexSize, m_pool, hr);
		return false;
	}

	return true;
}

void CGraphicMeshVertexBuffer::DestroyDeviceObjects()
{
	safe_release(m_vb);
}

bool CGraphicMeshVertexBuffer::IsEmpty() const
{
	return m_vb == nullptr;
}

bool CGraphicMeshVertexBuffer::Copy(int bufSize, const void* srcVertices)
{
	void* dstVertices;
	if (!Lock(&dstVertices))
		return false;

	memcpy(dstVertices, srcVertices, bufSize);

	Unlock();
	return true;
}

bool CGraphicMeshVertexBuffer::LockRange(uint32_t count, void** vertices) const
{
	if (!m_vb)
		return false;

	const auto hr = m_vb->Lock(0,
		m_vertexSize * count,
		vertices,
		m_lockFlags);

	if (FAILED(hr)) {
		spdlog::trace("VertexBuffer::Lock() failed for count {0} with {1}",
			count, hr);
		return false;
	}

	return true;
}

bool CGraphicMeshVertexBuffer::Lock(void** vertices) const
{
	if (!m_vb)
		return false;

	const auto hr = m_vb->Lock(0, 0, vertices, m_lockFlags);
	if (FAILED(hr)) {
		spdlog::trace("VertexBuffer::Lock() failed with {0}",
			hr);
		return false;
	}

	return true;
}

bool CGraphicMeshVertexBuffer::Unlock() const
{
	if (!m_vb)
		return false;

	const auto hr = m_vb->Unlock();
	if (FAILED(hr)) {
		spdlog::trace("VertexBuffer::Unlock() with {0}",
			hr);
		return false;
	}

	return true;
}

void CGraphicMeshVertexBuffer::Bind(uint32_t index, IDirect3DVertexDeclaration9* vertexDecl) const
{
	assert(ms_lpd3dDevice != NULL);
	STATEMANAGER.SetStreamSource(index, m_vb, m_vertexSize);
	D3DPERF_SetMarker(D3DCOLOR_ARGB(251, 50, 50, 0), L" CGraphicMeshVertexBuffer Bind");

}

uint32_t CGraphicMeshVertexBuffer::GetVertexSize() const
{
	return m_vertexSize;
}

uint32_t CGraphicMeshVertexBuffer::GetVertexCount() const
{
	return m_vertexCount;
}

IDirect3DVertexBuffer9* CGraphicMeshVertexBuffer::GetD3DVertexBuffer() const
{
	return m_vb;
}
