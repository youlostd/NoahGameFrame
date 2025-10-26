#ifndef METIN2_CLIENT_MILESLIB_SOUNDMANAGER_HPP
#define METIN2_CLIENT_MILESLIB_SOUNDMANAGER_HPP

#pragma once

#include "SampleFileCache.hpp"
#include "MusicInstance.hpp"
#include "SoundSample.hpp"

#include <base/Singleton.hpp>

#include <boost/unordered_map.hpp>

struct MilesLibrary
{
    MilesLibrary();
    ~MilesLibrary();
};

class CSoundManager : public CSingleton<CSoundManager>
{
public:
    CSoundManager();

    bool Create();

    void SetPosition(float fx, float fy, float fz);
    void SetDirection(float fxDir, float fyDir, float fzDir, float fxUp, float fyUp, float fzUp);
    void Update();

    float GetSoundScale();
    void SetSoundScale(float fScale);
    void SetAmbienceSoundScale(float fScale);

    void SetSoundVolume(float fVolume);
    void SetMusicVolume(float fVolume);

    void SaveVolume();
    void RestoreVolume();

    float GetSoundVolume();
    float GetMusicVolume();

    // Sound
    void PlaySound2D(std::string_view filename);
    float __GetVolumeFromDistance(float fDistance);
    void PlaySound3D(float fx, float fy, float fz,
                     std::string_view filename,
                     int iPlayCount = 1);
    std::unique_ptr<SoundSample> PlayAmbienceSound3D(float fx, float fy, float fz,
                                                     std::string_view filename,
                                                     int iPlayCount = 1);
    void PlayCharacterSound3D(float fx, float fy, float fz,
                              const std::string &filename,
                              bool bCheckFrequency = false);
    void StopAllSound3D();

    // Music
    void FadeInMusic(const std::string &filename,
                     float fVolumeSpeed = 0.016f);
    void FadeOutMusic(const std::string &filename,
                      float fVolumeSpeed = 0.016f);
    void FadeLimitOutMusic(const std::string &filename,
                           float fLimitVolume, float fVolumeSpeed = 0.016f);
    void FadeOutAllMusic();

protected:
    MilesLibrary m_miles;
    bool m_isSoundDisable;

    uint32_t m_max2dSoundsPlaying;
    uint32_t m_max3dSoundsPlaying;
    uint32_t m_maxMusicPlaying;

    float m_fxPosition;
    float m_fyPosition;
    float m_fzPosition;

    float m_fSoundScale;
    float m_fAmbienceSoundScale;
    float m_fSoundVolume;
    float m_fMusicVolume;

    float m_fBackupMusicVolume;
    float m_fBackupSoundVolume;

    HDIGDRIVER m_driver;

    SampleFileCache m_cache;
    boost::unordered_map<std::string, MusicInstance> m_music;
    std::vector<SoundSample> m_sounds2d;
    std::vector<SoundSample> m_sounds3d;
    std::unordered_map<std::string, float> m_playSoundHistoryMap;
};

#endif
