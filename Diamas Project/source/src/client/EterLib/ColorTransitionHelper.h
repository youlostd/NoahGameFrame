#ifndef METIN2_CLIENT_ETERLIB_COLORTRANSITIONHELPER_HPP
#define METIN2_CLIENT_ETERLIB_COLORTRANSITIONHELPER_HPP

#pragma once

#include <d3d9.h>

class CColorTransitionHelper
{
public:
    CColorTransitionHelper();
    ~CColorTransitionHelper();

    void Clear(const float &c_rfRed,
               const float &c_rfGreen,
               const float &c_rfBlue,
               const float &c_rfAlpha);
    void SetSrcColor(const float &c_rfRed,
                     const float &c_rfGreen,
                     const float &c_rfBlue,
                     const float &c_rfAlpha);
    void SetTransition(const float &c_rfRed,
                       const float &c_rfGreen,
                       const float &c_rfBlue,
                       const float &c_rfAlpha,
                       const uint32_t &dwDuration);
    const D3DCOLOR &GetCurColor(); // { return m_dwCurColor; }

    void StartTransition();
    bool Update();

    bool isTransitionStarted()
    {
        return m_bTransitionStarted;
    }

private:
    D3DCOLOR m_dwCurColor; // 현재 색

    uint32_t m_dwStartTime; // 바뀌기 시작하는 시간
    uint32_t m_dwDuration;  // 얼마 동안에 바뀌는가?

    bool m_bTransitionStarted;

    float m_fSrcRed, m_fSrcGreen, m_fSrcBlue, m_fSrcAlpha;
    float m_fDstRed, m_fDstGreen, m_fDstBlue, m_fDstAlpha;
};

#endif
