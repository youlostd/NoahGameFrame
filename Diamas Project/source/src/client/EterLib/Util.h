#pragma once

#include "../eterBase/FileLoader.h"

#include <Direct3D.hpp>
#include <map>
#include <vector>
#include <utility>

namespace storm
{
class File;
}

template <typename T>
class CTransitor
{
public:
    CTransitor()
        : m_dwID(0), m_bActivated(0), m_fStartTime(0), m_fEndTime(0)
    {
    }

    ~CTransitor()
    {
    }

    void SetActive(bool bActive = TRUE)
    {
        m_bActivated = bActive;
    }

    bool isActive()
    {
        return m_bActivated;
    }

    bool isActiveTime(float fcurTime)
    {
        if (fcurTime >= m_fEndTime)
            return FALSE;

        return TRUE;
    }

    uint32_t GetID()
    {
        return m_dwID;
    }

    void SetID(uint32_t dwID)
    {
        m_dwID = dwID;
    }

    void SetSourceValue(const T &c_rSourceValue)
    {
        m_SourceValue = c_rSourceValue;
    }

    void SetTransition(const T &c_rSourceValue, const T &c_rTargetValue, float fStartTime, float fBlendTime)
    {
        m_SourceValue = c_rSourceValue;
        m_TargetValue = c_rTargetValue;
        m_fStartTime = fStartTime;
        m_fEndTime = fStartTime + fBlendTime;
    }

    bool GetValue(float fcurTime, T *pValue)
    {
        if (fcurTime <= m_fStartTime)
            return FALSE;

        float fPercentage = (fcurTime - m_fStartTime) / (m_fEndTime - m_fStartTime);
        *pValue = m_SourceValue + (m_TargetValue - m_SourceValue) * fPercentage;
        return TRUE;
    }

protected:
    uint32_t m_dwID; // Public Transitor ID

    bool m_bActivated; // Have been started to blend?
    float m_fStartTime;
    float m_fEndTime;

    T m_SourceValue;
    T m_TargetValue;
};

typedef CTransitor<float> TTransitorFloat;
typedef CTransitor<Vector3> TTransitorVector3;
typedef CTransitor<DirectX::SimpleMath::Color> TTransitorColor;

///////////////////////////////////////////////////////////////////////////////////////////////////

void PrintfTabs(FILE *File, int iTabCount, const char *c_szString, ...);
void PrintfTabs(storm::File &File, int iTabCount, const char *c_szString, ...);

//typedef CTokenVector TTokenVector;

extern bool LoadTextData(const char *c_szFileName, CTokenMap &rstTokenMap);
extern bool LoadMultipleTextData(const char *c_szFileName, CTokenVectorMap &rstTokenVectorMap);

extern Vector3 TokenToVector(CTokenVector &rVector);
extern DirectX::SimpleMath::Color TokenToColor(CTokenVector &rVector);

#define GOTO_CHILD_NODE(TextFileLoader, Index) CTextFileLoader::CGotoChild Child(TextFileLoader, Index);

///////////////////////////////////////////////////////////////////////////////////////////////////

extern uint32_t GetDefaultCodePage();
extern void base64_decode(const char *str, char *resultStr);

extern DWORD GetMaxTextureWidth();
extern DWORD GetMaxTextureHeight();

std::pair<float, float> GetKeyMovementDirection(bool isLeft, bool isRight,
                                                bool isUp, bool isDown);
float GetKeyMovementRotation(float x, float y, float w, float h);
