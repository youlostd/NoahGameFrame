#include "VideoMode.hpp"

namespace Graphics
{
std::string VideoMode::getDescription() const
{
    unsigned int colourDepth = 16;
    if (mDisplayMode.Format == D3DFMT_X8R8G8B8 ||
        mDisplayMode.Format == D3DFMT_A8R8G8B8 ||
        mDisplayMode.Format == D3DFMT_R8G8B8)
        colourDepth = 32;

    return fmt::format("{}x{} {}bpp", mDisplayMode.Width, mDisplayMode.Height, colourDepth);
}

unsigned int VideoMode::getColourDepth() const
{
    unsigned int colourDepth = 16;
    if (mDisplayMode.Format == D3DFMT_X8R8G8B8 ||
        mDisplayMode.Format == D3DFMT_A8R8G8B8 ||
        mDisplayMode.Format == D3DFMT_R8G8B8)
        colourDepth = 32;

    return colourDepth;
}
}
