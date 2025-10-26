#include "SharedVertexBuffer.hpp"

#include "GrpDevice.h"

SharedVertexBuffer::SharedVertexBuffer(CGraphicDevice* dev)
:m_SharedVB(ENGINE_INVALID_HANDLE),m_CurrentUsingVBOffset(0),m_NonusedVBDataOffset(0),m_CurrentVertexWriteCount(0),m_device(dev)
{
	m_SharedVB = m_device->CreateVertexBuffer( m_VBSizeLimit,sizeof(TPDTVertex), NULL, (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY));
}

SharedVertexBuffer::~SharedVertexBuffer( void)
{
	if( isValid(m_SharedVB))
	{
	//	m_device->DeleteVertexBuffer( m_SharedVB);
		m_SharedVB = ENGINE_INVALID_HANDLE;
	}
}
int SharedVertexBuffer::Render( D3DPRIMITIVETYPE primitiveType, unsigned int nPrimitiveCount,void* pVertex, unsigned int VertexFormatSize,int TheNumberOfVertexData )
{
	if( AddVertex((TPDTVertex*)pVertex,TheNumberOfVertexData) )
	{
		m_device->SetVertexBuffer( m_SharedVB, 0, 0);
		return m_device->DrawPrimitive( primitiveType, m_CurrentUsingVBOffset / sizeof(TPDTVertex) , nPrimitiveCount);
	}
	return 0;
}

bool SharedVertexBuffer::AddVertex( TPDTVertex* pVertex,int TheNumberOfVBData )
{
	int TotalSizeOfData = TheNumberOfVBData * sizeof(TPDTVertex);	//데이타 크기
	DWORD dwLockFlags = D3DLOCK_NOOVERWRITE;

	if( m_NonusedVBDataOffset + TotalSizeOfData >= m_VBSizeLimit)
	{
		// No space remains. Start over from the beginning 
		//   of the vertex buffer.
		dwLockFlags = D3DLOCK_DISCARD;
		m_NonusedVBDataOffset = 0;
	}

	TPDTVertex *pVideoVertex = (TPDTVertex*)m_device->LockVertexBuffer( m_SharedVB, dwLockFlags, m_NonusedVBDataOffset / sizeof(TPDTVertex), TheNumberOfVBData);
	if( pVideoVertex)
	{
		memcpy( pVideoVertex, pVertex, TotalSizeOfData );
		m_device->UnlockVertexBuffer(m_SharedVB);
		m_CurrentVertexWriteCount = TheNumberOfVBData;

		m_CurrentUsingVBOffset = m_NonusedVBDataOffset;			
		m_NonusedVBDataOffset+=TotalSizeOfData;					
		return true;
	}

	m_CurrentVertexWriteCount = 0;
	return false;
}