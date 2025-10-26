#include "Stdafx.h"
#include "SoundStream.hpp"

#include <algorithm>

SoundStream::SoundStream(HSTREAM stream)
    : m_stream(stream)
{
}

SoundStream::~SoundStream()
{
    if (m_stream)
    {
        AIL_close_stream(m_stream);
        m_stream = nullptr;
    }
}

bool SoundStream::IsDone() const
{
    return AIL_stream_status(m_stream) == -1;
}

void SoundStream::Play(int loopCount) const
{
    AIL_set_stream_loop_count(m_stream, loopCount);
    AIL_start_stream(m_stream);
}

void SoundStream::Pause() const
{
    AIL_pause_stream(m_stream, 1);
}

void SoundStream::Resume() const
{
    AIL_pause_stream(m_stream, 0);
}

void SoundStream::Stop()
{
    AIL_close_stream(m_stream);
    m_stream = NULL;
}

float SoundStream::GetVolume() const
{
    const auto sample = AIL_stream_sample_handle(m_stream);
    if (!sample)
        return 0.0f;

    float volume;
    AIL_sample_volume_pan(sample, &volume, nullptr);
    return volume;
}

void SoundStream::SetVolume(float volume) const
{
    const auto sample = AIL_stream_sample_handle(m_stream);
    if (!sample)
        return;

    volume = std::max<float>(0.0f, std::min<float>(1.0f, volume));
    AIL_set_sample_volume_pan(sample, volume, 0.5f);
}
