#include "SoundManager.h"
#include "StdAfx.h"

#include "../EterBase/Timer.h"
#include "../EterBase/Utils.h"
#undef min
#undef max
#include <cmath>
#define IS_STATIC
#include <StepTimer.h>

#include "SoundVfs.hpp"
#include "mss.h"

MilesLibrary::MilesLibrary()
{
    // AIL_configure_logging("miles.log", nullptr, 2);

    AIL_set_redist_directory("miles");
    AIL_startup();

    RegisterMilesVfs();
}

MilesLibrary::~MilesLibrary()
{
    AIL_shutdown();
}

CSoundManager::CSoundManager()
    : m_isSoundDisable(false), m_max2dSoundsPlaying(8), m_max3dSoundsPlaying(64), m_maxMusicPlaying(4),
      m_fxPosition(0.0f), m_fyPosition(0.0f), m_fzPosition(0.0f), m_fSoundScale(200.0f), m_fAmbienceSoundScale(1000.0f),
      m_fSoundVolume(1.0f), m_fMusicVolume(1.0f), m_fBackupMusicVolume(0.0f), m_fBackupSoundVolume(0.0f),
      m_driver(nullptr)
{
    // ctor
}

bool CSoundManager::Create()
{
    m_driver = AIL_open_digital_driver(44100, 16, MSS_MC_USE_SYSTEM_CONFIG , 0);
    if (!m_driver)
    {
        SPDLOG_ERROR("AIL_open_digital_driver(): {1}", AIL_last_error());
        return false;
    }
    AIL_set_3D_distance_factor(m_driver, 0.001f);
    return true;
}

void CSoundManager::SetPosition(float fx, float fy, float fz)
{
    m_fxPosition = fx;
    m_fyPosition = fy;
    m_fzPosition = fz;
}

void CSoundManager::SetDirection(float fxDir, float fyDir, float fzDir, float fxUp, float fyUp, float fzUp)
{
    if (!m_driver)
        return;

    AIL_set_listener_3D_orientation(m_driver, fxDir, fyDir, -fzDir, fxUp, fyUp, -fzUp);
}

void CSoundManager::Update()
{
    for (auto it = m_sounds2d.begin(); it != m_sounds2d.end();)
    {
        if (it->IsDone())
            it = m_sounds2d.erase(it);
        else
            ++it;
    }

    for (auto it = m_sounds3d.begin(); it != m_sounds3d.end();)
    {
        if (it->IsDone())
            it = m_sounds3d.erase(it);
        else
            ++it;
    }

    for (auto it = m_music.begin(); it != m_music.end();)
    {
        // If Update() returns false, the song has finished playing.
        if (it->second.Update(m_fMusicVolume))
            ++it;
        else
            it = m_music.erase(it);
    }
}

float CSoundManager::GetSoundScale()
{
    return m_fSoundScale;
}

void CSoundManager::SetSoundScale(float fScale)
{
    m_fSoundScale = fScale;
}

void CSoundManager::SetAmbienceSoundScale(float fScale)
{
    m_fAmbienceSoundScale = fScale;
}

void CSoundManager::SetSoundVolume(float fVolume)
{
    if (m_isSoundDisable)
    {
        m_fBackupSoundVolume = fVolume;
        return;
    }

    fVolume = std::max(std::min(fVolume, 1.0f), 0.0f);
    m_fSoundVolume = fVolume;
    m_fBackupSoundVolume = fVolume;
}

void CSoundManager::SetMusicVolume(float fVolume)
{
    if (m_isSoundDisable)
    {
        m_fBackupMusicVolume = fVolume;
        return;
    }

    fVolume = std::max(std::min(fVolume, 1.0f), 0.0f);
    m_fMusicVolume = fVolume;
    m_fBackupMusicVolume = fVolume;

    for (auto &p : m_music)
    {
        if (MUSIC_STATE_OFF == p.second.MusicState)
            continue;
        if (MUSIC_STATE_FADE_OUT == p.second.MusicState)
            continue;

        p.second.fVolume = fVolume;
        p.second.stream.SetVolume(fVolume);
    }
}

void CSoundManager::SaveVolume()
{
    // NOTE : 두번 이상 Save를 시도할때는 그냥 Return
    if (m_isSoundDisable)
        return;

    float fBackupMusicVolume = m_fMusicVolume;
    float fBackupSoundVolume = m_fSoundVolume;
    SetMusicVolume(0.0f);
    SetSoundVolume(0.0f);
    m_fBackupMusicVolume = fBackupMusicVolume;
    m_fBackupSoundVolume = fBackupSoundVolume;
    m_isSoundDisable = true;
}

void CSoundManager::RestoreVolume()
{
    m_isSoundDisable = false;
    SetMusicVolume(m_fBackupMusicVolume);
    SetSoundVolume(m_fBackupSoundVolume);
}

float CSoundManager::GetSoundVolume()
{
    return m_fSoundVolume;
}

float CSoundManager::GetMusicVolume()
{
    return m_fMusicVolume;
}

void CSoundManager::PlaySound2D(std::string_view filename)
{
    if (0.0f == GetSoundVolume())
        return;

    if (m_sounds2d.size() >= m_max2dSoundsPlaying)
        return;

    const auto file = m_cache.Get(filename);
    if (!file)
        return;

    if (!m_driver)
        return;

    const auto sample = AIL_allocate_sample_handle(m_driver);
    if (!sample)
    {
        SPDLOG_ERROR("AIL_allocate_sample_handle(): {0} for {1}", AIL_last_error(), filename);
        return;
    }

    m_sounds2d.emplace_back(sample);
    m_sounds2d.back().SetFile(file);
    m_sounds2d.back().SetVolume(GetSoundVolume());
    m_sounds2d.back().Play(1);
}

float CSoundManager::__GetVolumeFromDistance(float fDistance)
{
    return GetSoundVolume() - (fDistance / 2500);
}
 //   return 1.0f / (1.0f + (1.0f * (fDistance - 1.0f)));


void CSoundManager::PlaySound3D(float fx, float fy, float fz, std::string_view filename, int iPlayCount)
{
    if (0.0f == GetSoundVolume())
        return;

    if (m_sounds3d.size() >= m_max3dSoundsPlaying)
    {
        return;
    }
    const auto file = m_cache.Get(filename);
    if (!file)
    {
        SPDLOG_ERROR("Not playing {} file not found", filename);
        return;
    }

    const auto sample = AIL_allocate_sample_handle(m_driver);
    if (!sample)
    {
        SPDLOG_ERROR("AIL_allocate_sample_handle(): {0} for {1}", AIL_last_error(), filename);
        return;
    }

      float fDistance = sqrtf((fx - m_fxPosition) * (fx - m_fxPosition) + (fy - m_fyPosition) * (fy - m_fyPosition) +
                            (fz - m_fzPosition) * (fz - m_fzPosition));
    SPDLOG_INFO("SampSet distance {} {}", fDistance ,__GetVolumeFromDistance(fDistance));


    m_sounds3d.emplace_back(sample);
    m_sounds3d.back().SetFile(file);
    m_sounds3d.back().SetPosition((fx - m_fxPosition) / m_fSoundScale,
                                  (fy - m_fyPosition) / m_fSoundScale,
                                  (fz - m_fzPosition) / m_fSoundScale);
    m_sounds3d.back().SetVolume(__GetVolumeFromDistance(fDistance));
    m_sounds3d.back().Play(iPlayCount);
}

std::unique_ptr<SoundSample> CSoundManager::PlayAmbienceSound3D(float fx, float fy, float fz, std::string_view filename,
                                                                int iPlayCount)
{
    if (0.0f == GetSoundVolume())
        return nullptr;

    if (m_sounds3d.size() >= m_max3dSoundsPlaying)
        return nullptr;

    const auto file = m_cache.Get(filename);
    if (!file)
        return nullptr;

    if (!m_driver)
        return nullptr;

    const auto sample = AIL_allocate_sample_handle(m_driver);
    if (!sample)
    {
        SPDLOG_ERROR("AIL_allocate_sample_handle(): {0} for {1}", AIL_last_error(), filename);
        return nullptr;
    }

    auto sound = std::make_unique<SoundSample>(sample);
    sound->SetFile(file);
    sound->SetPosition((fx - m_fxPosition) / m_fSoundScale,
                       (fy - m_fyPosition) / m_fSoundScale,
                       (fz - m_fzPosition) / m_fSoundScale);
    sound->SetVolume(GetSoundVolume());
    sound->Play(iPlayCount);

    return sound;
}

void CSoundManager::PlayCharacterSound3D(float fx, float fy, float fz, const std::string &filename,
                                         bool bCheckFrequency)
{
    if (0.0f == GetSoundVolume())
        return;

    // 어느 정도의 최적화가 필요할 수도 있다 - [levites]
    if (bCheckFrequency)
    {
        static float s_fLimitDistance = 5000 * 5000;
        float fdx = (fx - m_fxPosition) * (fx - m_fxPosition);
        float fdy = (fy - m_fyPosition) * (fy - m_fyPosition);

        if (fdx + fdy > s_fLimitDistance)
            return;

        const auto itor = m_playSoundHistoryMap.find(filename);
        if (m_playSoundHistoryMap.end() != itor)
        {
            float fTime = itor->second;
            if (DX::StepTimer::instance().GetTotalSeconds() - fTime < 0.3f)
            {
                // Tracef("똑같은 소리가 0.3초 내에 다시 플레이 %s\n", filename);
                return;
            }
            m_playSoundHistoryMap.erase(itor);
        }

        m_playSoundHistoryMap.emplace(filename, DX::StepTimer::instance().GetTotalSeconds());
    }

    SPDLOG_INFO("Playing sound {}", filename);
    PlaySound3D(fx, fy, fz, filename, 1);
}

void CSoundManager::StopAllSound3D()
{
    m_sounds3d.clear();
}

void CSoundManager::FadeInMusic(const std::string &filename, float fVolumeSpeed)
{
    const auto it = m_music.find(filename);
    if (it != m_music.end())
    {
        it->second.MusicState = MUSIC_STATE_FADE_IN;
        it->second.fVolumeSpeed = fVolumeSpeed;
        return;
    }

    if (m_music.size() >= m_maxMusicPlaying)
        return;

    FadeOutAllMusic();

    if (!m_driver)
        return;

    const auto stream = AIL_open_stream(m_driver, filename.c_str(), 0);
    if (!stream)
    {
        SPDLOG_ERROR("AIL_open_stream({0}): {1}", filename, AIL_last_error());
        return;
    }

    m_music.emplace(std::piecewise_construct, std::forward_as_tuple(filename),
                    std::forward_as_tuple(stream, filename, 0.0f, fVolumeSpeed));
}

void CSoundManager::FadeLimitOutMusic(const std::string &filename, float fLimitVolume, float fVolumeSpeed)
{
    const auto it = m_music.find(filename);
    if (it == m_music.end())
    {
        // SPDLOG_ERROR("FadeLimitOutMusic: {0} is not being played", filename);
        return;
    }

    it->second.MusicState = MUSIC_STATE_FADE_LIMIT_OUT;
    it->second.fVolumeSpeed = fVolumeSpeed;
    it->second.fLimitVolume = fLimitVolume;
}

void CSoundManager::FadeOutMusic(const std::string &filename, float fVolumeSpeed)
{
    const auto it = m_music.find(filename);
    if (it == m_music.end())
    {
        // SPDLOG_ERROR("FadeOutMusic: {0} is not being played", filename);
        return;
    }

    it->second.MusicState = MUSIC_STATE_FADE_OUT;
    it->second.fVolumeSpeed = fVolumeSpeed;
}

void CSoundManager::FadeOutAllMusic()
{
    for (auto &p : m_music)
    {
        if (MUSIC_STATE_OFF == p.second.MusicState)
            continue;

        p.second.MusicState = MUSIC_STATE_FADE_OUT;
        p.second.fVolumeSpeed = 0.01f;
    }
}
