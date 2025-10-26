#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include "DibBar.h"



class CTextBar : public CDibBar
{
	public:
		CTextBar (int fontSize, bool isBold);
		virtual ~CTextBar();

		void TextOut (int ix, int iy, const char* c_szText);
		void SetTextColor (int r, int g, int b);
		void GetTextExtent (const char* c_szText, SIZE* p_size);
		void DrawText (RECT& c_rRect, const char* c_szText);
		void SetTextAlign (float fMode);

	protected:
		void __SetFont (int fontSize, bool isBold);
		void OnCreate();

	protected:
		HFONT m_hFont;
		HFONT m_hOldFont;

		int		m_fontSize;
		bool	m_isBold;
};
