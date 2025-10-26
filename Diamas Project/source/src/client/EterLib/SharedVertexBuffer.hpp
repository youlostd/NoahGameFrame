#ifndef METIN2_CLIENT_ETERLIB_SHAREDVERTEXBUFFER_HPP
#define METIN2_CLIENT_ETERLIB_SHAREDVERTEXBUFFER_HPP

#include <Config.hpp>


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "GrpBase.h"

class CGraphicDevice;

class SharedVertexBuffer
{
public:
	SharedVertexBuffer(CGraphicDevice * dev);
	~SharedVertexBuffer(void);

public:
	int		Render(D3DPRIMITIVETYPE primitiveType, unsigned int nPrimitiveCount,void* pVertex, unsigned int VertexFormatSize,int TheNumberOfVertexData);

private:
	bool	AddVertex(TPDTVertex* pVertex,int TheNumberOfVBData);

private:
	VertexBufferHandle		m_SharedVB;
	static const int		m_VBSizeLimit = 2048;

	int						m_NonusedVBDataOffset;		//D3DLOCK_NOOVERWRITE 사용시에 gpu가 사용하고 있지 않는 버퍼의 메모리영역 시작주소
	int						m_CurrentUsingVBOffset;	//현재 락을 걸어서 버텍스데이타를 카피한 주소. 렌더링시에 사용됨
	int						m_CurrentVertexWriteCount;
	CGraphicDevice*			m_device;
};

#endif