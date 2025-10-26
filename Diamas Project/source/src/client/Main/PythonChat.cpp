#include "StdAfx.h"
#include "PythonChat.h"

#include "../EterLib/ResourceManager.h"

void CPythonChat::IgnoreCharacter(const char *c_szName)
{
    const auto itor = m_IgnoreCharacterSet.find(c_szName);
    if (m_IgnoreCharacterSet.end() != itor)
    {
        m_IgnoreCharacterSet.erase(itor);
    }
    else
    {
        m_IgnoreCharacterSet.insert(c_szName);
    }
}

bool CPythonChat::IsIgnoreCharacter(const char *c_szName)
{
    return m_IgnoreCharacterSet.end() != m_IgnoreCharacterSet.find(c_szName);
}
