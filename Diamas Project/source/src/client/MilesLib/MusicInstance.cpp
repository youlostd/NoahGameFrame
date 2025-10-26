#include "Stdafx.h"
#include "MusicInstance.hpp"

MusicInstance::MusicInstance(HSTREAM stream, std::string_view filename,
                             float fVolume, float fVolumeSpeed)
    : MusicState(MUSIC_STATE_FADE_IN)
      , fVolume(fVolume)
      , fLimitVolume(0.0f)
      , fVolumeSpeed(fVolumeSpeed)
      , filename(filename)
      , stream(stream)
{
    this->stream.SetVolume(fVolume);
    this->stream.Play(0);
}

bool MusicInstance::Update(float musicVolume)
{
    if (MUSIC_STATE_OFF == MusicState)
        return false;

    switch (MusicState)
    {
    case MUSIC_STATE_FADE_IN:
        fVolume += fVolumeSpeed;

        if (fVolume >= musicVolume)
        {
            fVolume = musicVolume;
            fVolumeSpeed = 0.0f;
            MusicState = MUSIC_STATE_PLAY;
        }

        stream.SetVolume(fVolume);
        break;

    case MUSIC_STATE_FADE_LIMIT_OUT:
        fVolume -= fVolumeSpeed;

        if (fVolume <= fLimitVolume)
        {
            fVolume = fLimitVolume;
            fVolumeSpeed = 0.0f;
            MusicState = MUSIC_STATE_OFF;
        }

        stream.SetVolume(fVolume);
        break;

    case MUSIC_STATE_FADE_OUT:
        fVolume -= fVolumeSpeed;
        return fVolume > 0.0f;
    }

    return true;
}
