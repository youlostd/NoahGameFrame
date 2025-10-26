#ifndef METIN2_CLIENT_ETERLIB_GRPDIB_HPP
#define METIN2_CLIENT_ETERLIB_GRPDIB_HPP
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CGraphicDib
{
public:
    CGraphicDib();
    virtual ~CGraphicDib();

    void Destroy();
    bool Create(HDC hDC, int width, int height);

    void SetBkMode(int iBkMode);
    void TextOut(int ix, int iy, const char *c_szText);
    void Put(HDC hDC, int x, int y);

    int GetWidth();
    int GetHeight();

    void *GetPointer();

    HDC GetDCHandle();

protected:
    void Initialize();

protected:
    HDC m_hDC;
    HBITMAP m_hBmp;
    BITMAPINFO m_bmi;

    int m_width;
    int m_height;

    void *m_pvBuf;
};

#endif
