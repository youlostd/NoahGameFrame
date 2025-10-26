#ifndef METIN2_CLIENT_ETERLIB_VIDEOMODELIST_HPP
#define METIN2_CLIENT_ETERLIB_VIDEOMODELIST_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "VideoMode.hpp"

namespace Graphics
{
class VideoModeList
{
private:
    std::vector<VideoMode> mModeList;

public:
    VideoModeList();
    ~VideoModeList();

    BOOL enumerate();

    VideoMode *item(size_t index);
    size_t count();

    VideoMode *item(const std::string &name);
};
}

#endif /* METIN2_CLIENT_ETERLIB_VIDEOMODELIST_HPP */
