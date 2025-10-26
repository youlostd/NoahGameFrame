#ifndef METIN2_CLIENT_ETERLIB_BLOCKTEXTURES_HPP
#define METIN2_CLIENT_ETERLIB_BLOCKTEXTURES_HPP


#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif



#include "GrpBase.h"


class CGraphicDib;

class CBlockTexture : public CGraphicBase
{
	public:
		CBlockTexture();
		virtual ~CBlockTexture();

		bool Create(CGraphicDib * pDIB, const RECT & c_rRect, uint32_t dwWidth, uint32_t dwHeight);
		void SetClipRect(const RECT & c_rRect);
		void Render(int ix, int iy);
		void InvalidateRect(const RECT & c_rsrcRect);

	protected:
		CGraphicDib * m_pDIB;
		RECT m_rect;
		RECT m_clipRect;
		bool m_bClipEnable;
		uint32_t m_dwWidth;
		uint32_t m_dwHeight;
		LPDIRECT3DTEXTURE9 m_lpd3dTexture;
};

#endif