// Decal.h: interface for the CDecal class.
//
//////////////////////////////////////////////////////////////////////
#ifndef METIN2_CLIENT_ETERLIB_DECAL_HPP
#define METIN2_CLIENT_ETERLIB_DECAL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include "GrpBase.h"

METIN2_BEGIN_NS

class CDecal
{
public:

	enum
	{
		MAX_DECAL_VERTICES	= 256,
	};

	CDecal();
	virtual ~CDecal();

	void Clear();

	virtual void Make(D3DXVECTOR3 v3Center, D3DXVECTOR3 v3Normal, D3DXVECTOR3 v3Tangent, float fWidth, float fHeight, float fDepth) = 0;
	virtual void Render();

protected:
	//
	D3DXVECTOR3		m_v3Center;
	D3DXVECTOR3		m_v3Normal;

	// Clip Plane
	D3DXPLANE		m_v4LeftPlane;
	D3DXPLANE		m_v4RightPlane;
	D3DXPLANE		m_v4BottomPlane;
	D3DXPLANE		m_v4TopPlane;
	D3DXPLANE		m_v4FrontPlane;
	D3DXPLANE		m_v4BackPlane;

	// 개수
	DWORD			m_dwVertexCount;
	DWORD			m_dwPrimitiveCount;

	// 버택스 버퍼와 인댁스 버퍼 대신에 배열 만들고 DrawIndexedPrimitiveUP로 그리자.
	typedef struct
	{
		WORD			m_wMinIndex;
		DWORD			m_dwVertexCount;
		DWORD			m_dwPrimitiveCount;
		DWORD			m_dwVBOffset;
	} TTRIANGLEFANSTRUCT;

	std::vector<TTRIANGLEFANSTRUCT> m_TriangleFanStructVector;

	TPDTVertex		m_Vertices[MAX_DECAL_VERTICES];
	WORD			m_Indices[MAX_DECAL_VERTICES];

	const float m_cfDecalEpsilon;

protected:
	bool AddPolygon(DWORD dwAddCount, const D3DXVECTOR3 *c_pv3Vertex, const D3DXVECTOR3 *c_pv3Normal);
	void ClipMesh(DWORD dwPrimitiveCount, const D3DXVECTOR3 *c_pv3Vertex, const D3DXVECTOR3 *c_pv3Normal);

	DWORD ClipPolygon(DWORD dwVertexCount,
		const D3DXVECTOR3 *c_pv3Vertex,
		const D3DXVECTOR3 *c_pv3Normal,
		D3DXVECTOR3 *c_pv3NewVertex,
		D3DXVECTOR3 *c_pv3NewNormal) const;

	static DWORD ClipPolygonAgainstPlane(const D3DXPLANE& v4Plane,
		DWORD dwVertexCount,
		const D3DXVECTOR3 *c_pv3Vertex,
		const D3DXVECTOR3 *c_pv3Normal,
		D3DXVECTOR3 *c_pv3NewVertex,
		D3DXVECTOR3 *c_pv3NewNormal);
};

METIN2_END_NS

#endif
