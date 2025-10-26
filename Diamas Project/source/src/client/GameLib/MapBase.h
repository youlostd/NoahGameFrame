#ifndef METIN2_CLIENT_GAMELIB_MAPBASE_H
#define METIN2_CLIENT_GAMELIB_MAPBASE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "MapType.h"

#include "../EterLib/GrpScreen.h"

class CMapBase : public CScreen
{
  public:
    enum EMAPTYPE
    {
        MAPTYPE_INVALID,
        MAPTYPE_INDOOR,
        MAPTYPE_OUTDOOR,
    };

  public:
    CMapBase();
    virtual ~CMapBase();

    virtual void Clear();
    virtual bool Initialize() = 0;
    virtual bool Destroy() = 0;
    virtual bool Load(float x, float y, float z) = 0;

    virtual bool Update(float fX, float fY, float fZ) = 0;
    virtual void UpdateAroundAmbience(float fX, float fY, float fZ) = 0;
    virtual float GetHeight(float fx, float fy) = 0;

  protected:
    virtual void OnRender() = 0;

  public:
    void Render();
    bool Enter();
    bool Leave();
    bool IsReady()
    {
        return m_bReady;
    }

    bool LoadProperty();

    //////////////////////////////////////////////////////////////////////////
    // Setting
    //////////////////////////////////////////////////////////////////////////
    EMAPTYPE GetType() const
    {
        return m_eType;
    }
    void SetType(EMAPTYPE eType)
    {
        m_eType = eType;
    }

    const std::string &GetName() const
    {
        return m_strName;
    }
    void SetName(std::string name)
    {
        m_strName = std::move(name);
    }

    int GetMapIndex() const
    {
        return m_mapIndex;
    };
    void SetMapIndex(uint32_t mapIndex)
    {
        m_mapIndex = mapIndex;
    }

    bool IsCopiedMap() const
    {
        return !m_strParentMapName.empty();
    } // "원본 데이터는 따로 있는" 복사 맵인가?
    const std::string &GetParentMapName() const
    {
        return m_strParentMapName;
    } // 원본 데이터를 갖고있는 맵의 이름을 리턴 (복사맵의 경우 필수로 세팅되어야 함)
    const std::string &GetMapDataDirectory() const
    {
        return IsCopiedMap() ? m_strParentMapName : m_strName;
    }

  protected:
    std::string m_strName; // 맵 이름. 맵 이름이 맵 식별자가 될 수 있을까?
    std::string m_strParentMapName; // 원본 맵의 이름. 이 값이 세팅되어 있다면 실제 모든 맵 데이터는 ParentMap/*
                                    // 경로에서 읽어온다. 맵의 일부만 읽어오는 기능은 필요없대서 패스.
    EMAPTYPE m_eType; // 맵 종류... 현재는 Indoor와 Ourdoor가 있다.
    uint32_t m_mapIndex;
    bool m_bReady;
};
#endif /* METIN2_CLIENT_GAMELIB_MAPBASE_H */
