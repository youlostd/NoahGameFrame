#ifndef METIN2_CLIENT_MILESLIB_SOUNDSTREAM_HPP
#define METIN2_CLIENT_MILESLIB_SOUNDSTREAM_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <boost/noncopyable.hpp>

#include <mss.h>

class SoundStream : boost::noncopyable
{
public:
    SoundStream(HSTREAM stream);
    ~SoundStream();

    void Play(int loopCount = 1) const;
    void Pause() const;
    void Resume() const;
    void Stop();
    float GetVolume() const;
    void SetVolume(float volume) const;
    bool IsDone() const;

private:
    HSTREAM m_stream;
};

#endif
