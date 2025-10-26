#include "StdAfx.h"
#include "GrpTechnique.h"


LPDIRECT3DDEVICE9 GrpTechnique::m_pDevice = NULL;
bool GrpTechnique::m_bUsing = false;
TECHNIQUE_PRESERVE_STATE GrpTechnique::m_ePreserveStates = TECH_PRESERVE_NOT;
GrpTechnique* GrpTechnique::m_pUsingTechnique = NULL;
UINT GrpTechnique::m_nUsingPass = 0;

///////////////////////////////////////////////////////////////////////  
//  GrpTechnique::GrpTechnique

GrpTechnique::GrpTechnique( ) 
: m_pEffect(NULL) 
, m_hTechnique(NULL)
{
}


///////////////////////////////////////////////////////////////////////  
//  GrpTechnique::~GrpTechnique

GrpTechnique::~GrpTechnique( )
{
}


///////////////////////////////////////////////////////////////////////  
//  GrpTechnique::Begin
// bPreserveStates : device 의 state 를 보존해 줄것인지 여부
bool GrpTechnique::Begin(UINT* pPasses, enum TECHNIQUE_PRESERVE_STATE ePreserveStates )
{
	if(!IsValid())
		return false;

	bool bSuccess = false;
	DWORD dwFlags = 0;
	switch( ePreserveStates)
	{
	case TECH_PRESERVE_NOT: dwFlags = D3DXFX_DONOTSAVESTATE; break;
	case TECH_PRESERVE_ALL: dwFlags = 0; break;
	case TECH_PRESERVE_EXCEPT_SAMPLER: dwFlags = D3DXFX_DONOTSAVESAMPLERSTATE; break;
	case TECH_PRESERVE_EXCEPT_SHADER: dwFlags = D3DXFX_DONOTSAVESHADERSTATE; break;
	}

	m_ePreserveStates = ePreserveStates;

	_ASSERT(m_bUsing == false);

	if (SUCCEEDED(m_pEffect->SetTechnique(m_hTechnique)))
	{
		if (SUCCEEDED(m_pEffect->Begin(pPasses, dwFlags)))
		{
			m_bUsing = true;
			m_pUsingTechnique = this;
			bSuccess = true;
		}
	}

	_ASSERT(bSuccess);
	return bSuccess;
}
 
bool GrpTechnique::BeginPass(UINT nPass)
{
	if(!IsValid())
		return false;
	m_nUsingPass = nPass;

	bool bSuccess = false;

	if (SUCCEEDED(m_pEffect->BeginPass(nPass)))
	{
		bSuccess = true;
	}

	_ASSERT(bSuccess);
	return bSuccess;
}

bool GrpTechnique::EndPass()
{
	if(!IsValid())
		return false;

	bool bSuccess = false;

	if (SUCCEEDED(m_pEffect->EndPass()))
	{
		bSuccess = true;
	}

	_ASSERT(bSuccess);
	return bSuccess;
}

///////////////////////////////////////////////////////////////////////  
//  GrpTechnique::Begin

bool GrpTechnique::End(void)
{
	if(!IsValid())
		return false;

	bool bSuccess = false;

	if (SUCCEEDED(m_pEffect->End( )))
	{
		m_bUsing = false;
		m_pUsingTechnique = NULL;
		bSuccess = true;
	}

	_ASSERT(bSuccess);
	return bSuccess;
}

bool GrpTechnique::IsValid(void) const
{
    return m_hTechnique != NULL && m_pEffect != NULL;
}

void GrpTechnique::CommitChanges()
{
	if (m_pEffect)
	{
		m_pEffect->CommitChanges();
	}
}

// Gets the handle of a pass.
D3DXHANDLE GrpTechnique::GetPass(UINT nPass)
{
	return m_pEffect->GetPass(m_hTechnique, nPass);
}

// Gets a pass description.
// description 정보에는 PassName과 주석의 개수(?) VS, PS의 FucntionPointer 등의 정보가 있다
bool GrpTechnique::GetPassDesc(UINT nPass, D3DXPASS_DESC* pDesc)
{
	if(!IsValid())
		return false;

    bool bSuccess = false;

	if (SUCCEEDED(m_pEffect->GetPassDesc( GetPass(nPass), pDesc )))
	{
		bSuccess = true;
	}

	_ASSERT(bSuccess);
	return bSuccess;
}

// D3DXPASS_DESC 의 PassName을 얻는다.
bool GrpTechnique::GetPassDescName(UINT nPass, char *pPassName)
{
	if(!IsValid())
		return false;

	D3DXPASS_DESC description;
	
	bool bSuccess = GetPassDesc(nPass, &description);

	strcpy(pPassName, description.Name);

	return bSuccess;
}


/// sampler index 를 얻어온다, 없으면 -1 리턴
unsigned int GrpTechnique::GetSamplerIndex( int nPass , const char* szConstant)
{
	D3DXPASS_DESC descPass;

	HRESULT hr;
	D3DXHANDLE hPass = m_pEffect->GetPass( m_hTechnique, nPass );
	hr = m_pEffect->GetPassDesc( hPass, &descPass);
	LPD3DXCONSTANTTABLE pConstTable;
	D3DXGetShaderConstantTable(descPass.pPixelShaderFunction, &pConstTable);

	/* debug 용 const enum 방법
	D3DXCONSTANTTABLE_DESC descConstTable;
	pConstTable->GetDesc(&descConstTable);
	for(unsigned int i=0;i<descConstTable.Constants;i++)
	{
		D3DXHANDLE hConst = pConstTable->GetConstant(NULL,i);
		UINT nCount = 10;
		D3DXCONSTANT_DESC descConst[10];
		pConstTable->GetConstantDesc( hConst , descConst, &nCount);
	}
	//*/


	D3DXHANDLE hConst = pConstTable->GetConstantByName( NULL, szConstant );
	if(!hConst)
		return -1;
	unsigned int nIndex = pConstTable->GetSamplerIndex(hConst);
	return nIndex;
}

bool GrpTechnique::IsEqual( const GrpTechnique& technique ) const
{
	return ( m_hTechnique == technique.m_hTechnique );
}

