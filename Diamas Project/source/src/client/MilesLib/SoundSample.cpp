#include "SoundSample.hpp"
#include "Stdafx.h"

#include "../EterBase/Timer.h"
#include "../EterBase/Utils.h"

SoundSample::SoundSample(HSAMPLE sample) : m_sample(sample)
{
}

SoundSample::SoundSample(SoundSample &&other) : m_sample(other.m_sample), m_sampleFile(std::move(other.m_sampleFile))
{
    other.m_sample = nullptr;
    other.m_sampleFile.reset();
}

SoundSample::~SoundSample()
{
    if (m_sample)
    {
        AIL_release_sample_handle(m_sample);
        m_sample = nullptr;
    }
}

SoundSample &SoundSample::operator=(SoundSample &&other)
{
    if (this == &other)
        return *this;

    if (m_sample)
        AIL_release_sample_handle(m_sample);

    m_sample = other.m_sample;
    other.m_sample = nullptr;

    m_sampleFile = std::move(other.m_sampleFile);
    return *this;
}

bool SoundSample::SetFile(SampleFilePtr sample)
{
    if (!AIL_set_named_sample_file(m_sample, sample->GetFilename().c_str(), sample->GetData(), sample->GetSize(), 0))
    {
        SPDLOG_ERROR("{0}: {1}", sample->GetFilename(), AIL_last_error());
        return false;
    }

    m_sampleFile = std::move(sample);
    return true;
}

bool SoundSample::IsDone() const
{
    return AIL_sample_status(m_sample) == SMP_DONE;
}

void SoundSample::Play(int loopCount) const
{
    //AIL_set_sample_3D_distances(m_sample, 5000.0f, -1.0f, 0); 
    AIL_set_sample_loop_count(m_sample, loopCount);
    AIL_start_sample(m_sample);
}

void SoundSample::Pause() const
{
    AIL_stop_sample(m_sample);
}

void SoundSample::Resume() const
{
    AIL_resume_sample(m_sample);
}

void SoundSample::Stop() const
{
    AIL_end_sample(m_sample);
}

float SoundSample::GetVolume() const
{
    float volume;
    AIL_sample_volume_pan(m_sample, &volume, nullptr);
    return volume;
}

void SoundSample::SetVolume(float volume) const
{
    volume = std::max<float>(0.0f, std::min<float>(1.0f, volume));
    AIL_set_sample_volume_pan(m_sample, volume, 0.5f);
}

void SoundSample::SetPosition(float x, float y, float z) const
{
    //AIL_set_sample_3D_position(m_sample, x, y, -z);
    // AIL_set_sample_is_3D(m_sample, FALSE);
}

void SoundSample::SetVelocity(float fDistanceX, float fDistanceY, float fDistanceZ, float fNagnitude) const
{
    AIL_set_sample_3D_velocity(m_sample, fDistanceX, fDistanceY, fDistanceZ, fNagnitude);
}

void SoundSample::UpdatePosition(float fElapsedTime)
{
    AIL_update_sample_3D_position(m_sample, fElapsedTime);
}
