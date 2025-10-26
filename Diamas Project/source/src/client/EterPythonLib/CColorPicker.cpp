#include "stdafx.h"
#include "CColorPicker.h"

#include <d3dx9core.h>

D3DCOLOR HSLToRGB(float H, float S, float L)
{
    float Q;

    if (L < 0.5f)
        Q = L * (S + 1.0f);
    else
        Q = L + S - (L * S);

    float P = 2 * L - Q;

    float RGBs[3];

    RGBs[0] = H + (1.0f / 3.0f);
    RGBs[1] = H;
    RGBs[2] = H - (1.0f / 3.0f);

    for (int i = 0; i < 3; ++i)
    {
        if (RGBs[i] < 0)
            RGBs[i] += 1.0f;

        if (RGBs[i] > 1)
            RGBs[i] -= 1.0f;

        if (RGBs[i] < (1.0f / 6.0f))
            RGBs[i] = P + ((Q - P) * 6 * RGBs[i]);
        else if (RGBs[i] < 0.5f)
            RGBs[i] = Q;
        else if (RGBs[i] < (2.0f / 3.0f))
            RGBs[i] = P + ((Q - P) * 6 * ((2.0f / 3.0f) - RGBs[i]));
        else
            RGBs[i] = P;
    }

    return D3DCOLOR_XRGB(int(RGBs[0] * 255.0f), int(RGBs[1] * 255.0f), int(RGBs[2] * 255.0f));
}

CColorPicker::~CColorPicker()
{
    Destroy();
}

void CColorPicker::Destroy()
{
    M2_SAFE_RELEASE_CHECK(m_ColorPickerSprite);
    M2_SAFE_RELEASE_CHECK(m_ColorPickerTexture);
}

HRESULT CColorPicker::Init(int32_t Width, int32_t Height)
{
    m_ColorPickerWidth = Width;
    m_ColorPickerHeight = Height;

    bool Bits32 = true;
    Destroy();

    HRESULT CreateTextureReturn;

    CreateTextureReturn = ms_lpd3dDevice->CreateTexture(Width, Height, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED,
                                                        &m_ColorPickerTexture, 0);

    if (FAILED(CreateTextureReturn))
    {
        Bits32 = false;

        CreateTextureReturn = ms_lpd3dDevice->CreateTexture(Width, Height, 1, 0, D3DFMT_X4R4G4B4, D3DPOOL_MANAGED,
                                                            &m_ColorPickerTexture, 0);

        if (FAILED(CreateTextureReturn))
            return CreateTextureReturn;
    }

    D3DLOCKED_RECT Palette;

    CreateTextureReturn = m_ColorPickerTexture->LockRect(0, &Palette, 0, 0);

    if (FAILED(CreateTextureReturn))
    {
        Destroy();
        return CreateTextureReturn;
    }

    float H = 0;
    float S = 0.99f;
    float L = 1.0f;

    D3DCOLOR Color;

    BYTE R;
    BYTE G;
    BYTE B;

    DWORD *Colors32 = ((DWORD *)Palette.pBits) - 1;
    WORD *Colors = ((WORD *)Palette.pBits) - 1;

    for (int i = 0; i < Width; ++i)
    {
        for (int j = 0; j < Height; ++j)
        {
            Color = HSLToRGB(H, S, L);

            if (Bits32)
            {
                Colors32++;
                *Colors32 = Color;
            }
            else
            {
                R = ((Color >> 16) & 0xFF) / 0x10;
                G = ((Color >> 8) & 0xFF) / 0x10;
                B = ((Color >> 0) & 0xFF) / 0x10;

                Colors++;

                *Colors = (0xFF << 12) | (R << 8) | (G << 4) | (B << 0);
            }

            H += (1.0f / Width);
        }

        L -= (1.0f / Height);
        H = 0.0f;
    }

    m_ColorPickerTexture->UnlockRect(0);

    if (m_ColorPickerSprite == NULL)
        D3DXCreateSprite(ms_lpd3dDevice, &m_ColorPickerSprite);

    return S_OK;
}

LPDIRECT3DTEXTURE9 CColorPicker::GetTexture()
{
    return m_ColorPickerTexture;
}

void CColorPicker::Draw(float x, float y)
{
    if (m_ColorPickerSprite != NULL)
    {
        m_ColorPickerSprite->Begin(D3DXSPRITE_ALPHABLEND);
        m_ColorPickerSprite->Draw(GetTexture(), NULL, NULL, &D3DXVECTOR3(x, y, 0.0f), 0xFFFFFFFF);
        m_ColorPickerSprite->End();
    }
}

D3DCOLOR CColorPicker::GetPickedColor(int X, int Y)
{
    float H = X * (1.0f / m_ColorPickerWidth);
    float S = 0.99f;
    float L = 1.0f - Y * (1.0f / m_ColorPickerHeight);

    return HSLToRGB(H, S, L);
}
