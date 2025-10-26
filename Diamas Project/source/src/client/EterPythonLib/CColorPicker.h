#include <Config.hpp>

#include <d3dx9mesh.h>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


class CColorPicker : public CScreen, public CSingleton<CColorPicker>
{
public:
	~CColorPicker();

	void Destroy();

	HRESULT Init(int32_t Width, int32_t Height);
	LPDIRECT3DTEXTURE9 GetTexture();

	void Draw(float x, float y);
	D3DCOLOR GetPickedColor(int X, int Y);

private:

	LPDIRECT3DTEXTURE9 m_ColorPickerTexture = nullptr;
	LPD3DXSPRITE m_ColorPickerSprite = nullptr;
	int m_ColorPickerWidth = 0;
	int m_ColorPickerHeight = 0;
};

