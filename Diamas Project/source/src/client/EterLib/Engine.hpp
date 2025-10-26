#ifndef METIN2_CLIENT_ETERLIB_ENGINE_HPP
#define METIN2_CLIENT_ETERLIB_ENGINE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "StdAfx.h"

#include "ClientConfig.h"
#include "FontManager.hpp"
#include "GrpDevice.h"

template <class T> struct EngineObject
{
  public:
    template <typename... Args> EngineObject(T *&ptr, Args &&... args) : m_ptr(ptr), m_obj(std::forward<Args>(args)...)
    {
        m_ptr.p = &m_obj;
    }

  private:
    struct Ptr
    {
        Ptr(T *&ptr, T *obj = nullptr) : p(ptr)
        {
            p = obj;
        }

        ~Ptr()
        {
            p = nullptr;
        }

        T *&p;
    };

    Ptr m_ptr;
    T m_obj;
};

class Engine
{
  public:
    inline static CClientConfig &GetSettings()
    {
        return m_config;
    }
    inline static FontManager &GetFontManager()
    {
        return m_fontManager;
    }
    inline static CGraphicDevice &GetDevice()
    {
        _ASSERT(m_device != NULL);
        return *m_device;
    }

    inline static CGraphicDevice *GetDevicePtr()
    {
        return m_device;
    }

    static void RegisterDevice(CGraphicDevice *pDevice)
    {
        m_device = pDevice;
    }

  private:
    static CClientConfig &m_config;
    static FontManager &m_fontManager;
    static CGraphicDevice *m_device;
};

class KeyboardInput;

KeyboardInput &GetKeyboardInput();
void SetKeyboardInput(KeyboardInput *keyboard);

#endif
