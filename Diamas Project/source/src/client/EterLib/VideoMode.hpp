#ifndef METIN2_CLIENT_ETERLIB_VIDEOMODE_HPP
#define METIN2_CLIENT_ETERLIB_VIDEOMODE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace Graphics
{
static unsigned int modeCount = 0;

class VideoMode
{
private:
    D3DDISPLAYMODE mDisplayMode;
    unsigned int modeNumber;

public:
    VideoMode()
    {
        modeNumber = ++modeCount;
        ZeroMemory(&mDisplayMode, sizeof(D3DDISPLAYMODE));
    }

    VideoMode(const VideoMode &ob)
    {
        modeNumber = ++modeCount;
        mDisplayMode = ob.mDisplayMode;
    }

    VideoMode(D3DDISPLAYMODE d3ddm)
    {
        modeNumber = ++modeCount;
        mDisplayMode = d3ddm;
    }

    ~VideoMode()
    {
        modeCount--;
    }

    unsigned int getWidth() const
    {
        return mDisplayMode.Width;
    }

    unsigned int getHeight() const
    {
        return mDisplayMode.Height;
    }

    D3DFORMAT getFormat() const
    {
        return mDisplayMode.Format;
    }

    unsigned int getRefreshRate() const
    {
        return mDisplayMode.RefreshRate;
    }

    unsigned int getColourDepth() const;

    D3DDISPLAYMODE getDisplayMode() const
    {
        return mDisplayMode;
    }

    void increaseRefreshRate(unsigned int rr)
    {
        mDisplayMode.RefreshRate = rr;
    }

    std::string getDescription() const;
};
}

#endif /* METIN2_CLIENT_ETERLIB_VIDEOMODE_HPP */
