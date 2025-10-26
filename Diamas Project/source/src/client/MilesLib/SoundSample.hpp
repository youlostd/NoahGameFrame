#ifndef METIN2_CLIENT_MILESLIB_SOUNDSAMPLE_HPP
#define METIN2_CLIENT_MILESLIB_SOUNDSAMPLE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "SampleFile.hpp"

#include <boost/noncopyable.hpp>

#include <mss.h>

class SoundSample : boost::noncopyable
{
public:
    SoundSample(HSAMPLE sample);
    SoundSample(SoundSample &&other);
    ~SoundSample();

    SoundSample &operator=(SoundSample &&other);

    bool SetFile(SampleFilePtr sample);

    void Play(int loopCount = 1) const;
    void Pause() const;
    void Resume() const;
    void Stop() const;
    float GetVolume() const;
    void SetVolume(float volume) const;
    bool IsDone() const;

    void SetPosition(float x, float y, float z) const;
    void SetVelocity(float fx, float fy, float fz, float fMagnitude) const;

    void UpdatePosition(float fElapsedTime);

private:
    HSAMPLE m_sample;
    SampleFilePtr m_sampleFile;
};

#endif
