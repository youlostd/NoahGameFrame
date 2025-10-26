#ifndef METIN2_CLIENT_ETERGRNLIB_MOTION_HPP
#define METIN2_CLIENT_ETERGRNLIB_MOTION_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CGrannyMotion
{
public:
    CGrannyMotion();
    virtual ~CGrannyMotion();

    bool IsEmpty();

    void Destroy();
    bool BindGrannyAnimation(granny_animation *pgrnAni);

    granny_animation *GetGrannyAnimationPointer() const;

    const char *GetName() const;
    float GetDuration() const;
    void GetTextTrack(const char *c_szTextTrackName, int *pCount, float *pArray) const;

    void SetFromFilename(const std::string &fromFilename)
    {
        m_fromFilename = fromFilename;
    }

    const std::string &GetFromFilename() const
    {
        return m_fromFilename;
    }

protected:
    void Initialize();

    std::string m_fromFilename;
    granny_animation *m_pgrnAni;
};

#endif
