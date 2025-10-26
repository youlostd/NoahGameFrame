#include "StdAfx.h"
#include "EffectElementBase.h"
#include "../EterLib/TextFileLoader.h"
#include "Util.hpp"

#include <storm/StringUtil.hpp>

void CEffectElementBase::GetPosition(double fTime, Vector3 &rPosition)
{
    rPosition = GetTimeEventBlendValue(fTime, m_TimeEventTablePosition);
}

/*
bool CEffectElementBase::isVisible(float fTime)
{
	for (uint32_t i = 0; i < m_TimeEventTableVisible.size(); ++i)
	{
		float fPointTime = m_TimeEventTableVisible[i];

		if (fTime < fPointTime)
		{
			if (1 == i % 2)
				return true;
			else
				return false;
		}
	}

	return 1 == (m_TimeEventTableVisible.size() % 2);
}

void CEffectElementBase::GetAlpha(float fTime, float * pAlpha)
{
	GetTimeEventBlendValue<TTimeEventTableFloat, float>(fTime, m_TimeEventAlpha, pAlpha);
}

void CEffectElementBase::GetScale(float fTime, float * pScale)
{
	GetTimeEventBlendValue<TTimeEventTableFloat, float>(fTime, m_TimeEventScale, pScale);
}
*/

bool CEffectElementBase::isData()
{
    return OnIsData();
}

void CEffectElementBase::Clear()
{
    m_fStartTime = 0.0f;

    OnClear();
}

bool CEffectElementBase::LoadScript(CTextFileLoader &rTextFileLoader)
{
    CTokenVector *pTokenVector;
    if (!rTextFileLoader.GetTokenFloat("starttime", &m_fStartTime))
    {
        m_fStartTime = 0.0f;
    }
    if (rTextFileLoader.GetTokenVector("timeeventposition", &pTokenVector))
    {
        m_TimeEventTablePosition.clear();

        auto dwIndex = 0;
        for (auto i = 0; i < pTokenVector->size(); ++dwIndex)
        {
            TEffectPosition EffectPosition;

            storm::ParseNumber(pTokenVector->at(i++), EffectPosition.m_fTime);

            if (pTokenVector->at(i) == "MOVING_TYPE_BEZIER_CURVE")
            {
                i++;

                EffectPosition.m_iMovingType = MOVING_TYPE_BEZIER_CURVE;

                storm::ParseNumber(pTokenVector->at(i++), EffectPosition.m_Value.x);
                storm::ParseNumber(pTokenVector->at(i++), EffectPosition.m_Value.y);
                storm::ParseNumber(pTokenVector->at(i++), EffectPosition.m_Value.z);

                storm::ParseNumber(pTokenVector->at(i++), EffectPosition.m_vecControlPoint.x);
                storm::ParseNumber(pTokenVector->at(i++), EffectPosition.m_vecControlPoint.y);
                storm::ParseNumber(pTokenVector->at(i++), EffectPosition.m_vecControlPoint.z);
            }
            else if (pTokenVector->at(i) == "MOVING_TYPE_DIRECT")
            {
                i++;

                EffectPosition.m_iMovingType = MOVING_TYPE_DIRECT;

                storm::ParseNumber(pTokenVector->at(i++), EffectPosition.m_Value.x);
                storm::ParseNumber(pTokenVector->at(i++), EffectPosition.m_Value.y);
                storm::ParseNumber(pTokenVector->at(i++), EffectPosition.m_Value.z);

                EffectPosition.m_vecControlPoint = Vector3(0.0f, 0.0f, 0.0f);
            }
            else
            {
                return FALSE;
            }

            m_TimeEventTablePosition.push_back(EffectPosition);
        }
    }

    return OnLoadScript(rTextFileLoader);
}

float CEffectElementBase::GetStartTime()
{
    return m_fStartTime;
}

CEffectElementBase::CEffectElementBase()
{
    m_fStartTime = 0.0f;
}

CEffectElementBase::~CEffectElementBase()
{
}
