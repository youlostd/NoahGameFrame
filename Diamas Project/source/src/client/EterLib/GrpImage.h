#ifndef __INC_GRPIMAGE_H__
#define __INC_GRPIMAGE_H__

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include "Resource.h"
#include "GrpImageTexture.h"

class CGraphicImage : public ConcreteResource<CGraphicImage>
{
public:
	CGraphicImage();
	~CGraphicImage() = default;

	void Clear() override;

	virtual bool CreateDeviceObjects();
	virtual void DestroyDeviceObjects();

	int GetWidth() const;
	int GetHeight() const;

	const RECT & GetRectReference() const;

	const CGraphicTexture & GetTextureReference() const;
	CGraphicTexture * GetTexturePointer();

	bool Load(int iSize, const void * c_pvBuf) override;

private:
	bool LoadRawImage(int size, const void* buf);
	bool LoadSubImage(int iSize, const void* c_pvBuf);

protected:
	CGraphicImageTexture m_imageTexture;
	CGraphicImage::Ptr m_subImage;
	RECT m_rect{};
};

#endif
