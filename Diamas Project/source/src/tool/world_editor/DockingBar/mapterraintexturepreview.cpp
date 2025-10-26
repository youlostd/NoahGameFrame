#include "stdafx.h"
#include "..\WorldEditor.h"
#include "mapterraintexturepreview.h"
#include "../DataCtrl/MapAccessorTerrain.h"

#include <il/ilu.h>

METIN2_BEGIN_NS

CMapTerrainTexturePreview::CMapTerrainTexturePreview()
{
	m_ilImage = 0;
}

CMapTerrainTexturePreview::~CMapTerrainTexturePreview()
{
	if (m_ilImage)
		ilDeleteImages(1, &m_ilImage);

	m_ilImage = 0;
}

BEGIN_MESSAGE_MAP(CMapTerrainTexturePreview, CWnd)
	//{{AFX_MSG_MAP(CMapTerrainTexturePreview)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapTerrainTexturePreview message handlers

BOOL CMapTerrainTexturePreview::Create()
{
	CRect rect;
	GetClientRect(&rect);
	if (!m_DIB.Create(NULL, rect.Width(), rect.Height()))
		return FALSE;

	ilGenImages(1, &m_ilImage);
	ilBindImage(m_ilImage);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMapTerrainTexturePreview message handlers

void CMapTerrainTexturePreview::UpdatePreview(int nTexNum)
{
	CWorldEditorApp * pApplication = (CWorldEditorApp *)AfxGetApp();
	CMapManagerAccessor * pMapManagerAccessor = pApplication->GetMapManagerAccessor();
	CTextureSet * pTextureSet = CTerrain::GetTextureSet();

	if (nTexNum == 0)
	{
		ilClearImage();
	}
	else
	{
		CGraphicImageInstance & rImageInstance = pTextureSet->GetTexture(nTexNum).ImageInstance;
		auto pImage = rImageInstance.GetGraphicImagePointer();
		if (!pImage)
			return;

		ilLoadImage((const ILstring)pImage->GetFileName());
		iluScale(m_DIB.GetWidth(), m_DIB.GetHeight(), 1);
		ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);

		Invalidate(TRUE);
	}
}

void CMapTerrainTexturePreview::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (ilIsImage(m_ilImage)) {
		ilBindImage(m_ilImage);
		std::memcpy(m_DIB.GetPointer(), ilGetData(),
		            ilGetInteger(IL_IMAGE_WIDTH) *
		            ilGetInteger(IL_IMAGE_HEIGHT) *
		            ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL));
	} else {
		std::memset(m_DIB.GetPointer(), 0,
		            m_DIB.GetWidth() * m_DIB.GetHeight() * 4);
	}

	m_DIB.Put(dc.GetSafeHdc(), 0, 0);
}

METIN2_END_NS
