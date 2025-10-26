#include "StdAfx.h"
#include "Type.h"
#include <storm/StringUtil.hpp>

bool GetTokenTimeEventFloat(CTextFileLoader &loader,
                            const char *key,
                            TTimeEventTableFloat *table)
{
    CTokenVector *pTokenVector;
    if (!loader.GetTokenVector(key, &pTokenVector))
        return false;

    table->clear();
    table->resize(pTokenVector->size() / 2);

    uint32_t dwIndex = 0;
    for (uint32_t i = 0; i < pTokenVector->size(); i += 2, ++dwIndex)
    {
        storm::ParseNumber(pTokenVector->at(i), table->at(dwIndex).m_fTime);
        storm::ParseNumber(pTokenVector->at(i + 1), table->at(dwIndex).m_Value);
    }

    return true;
}

bool GetTokenTimeEventDouble(CTextFileLoader &loader,
                             const char *key,
                             TTimeEventTableDouble *table)
{
    CTokenVector *pTokenVector;
    if (!loader.GetTokenVector(key, &pTokenVector))
        return false;

    table->clear();
    table->resize(pTokenVector->size() / 2);

    uint32_t dwIndex = 0;
    for (uint32_t i = 0; i < pTokenVector->size(); i += 2, ++dwIndex)
    {
        storm::ParseNumber(pTokenVector->at(i), table->at(dwIndex).m_fTime);
        storm::ParseNumber(pTokenVector->at(i + 1), table->at(dwIndex).m_Value);
    }

    return true;
}
