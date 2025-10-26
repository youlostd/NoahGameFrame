#ifndef METIN2_CLIENT_MAIN_INSULTCHECKER_H
#define METIN2_CLIENT_MAIN_INSULTCHECKER_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CInsultChecker
{
public:
    CInsultChecker &GetSingleton();

public:
    CInsultChecker();
    virtual ~CInsultChecker();

    void Clear();

    void AppendInsult(const std::string &c_rstInsult);
    bool IsInsultIn(const char *c_szLine, UINT uLineLen);
    void FilterInsult(char *szLine, UINT uLineLen);

private:
    bool __GetInsultLength(const char *c_szWord, UINT *puInsultLen);
    bool __IsInsult(const char *c_szWord);

private:
    std::list<std::string> m_kList_stInsult;
};
#endif /* METIN2_CLIENT_MAIN_INSULTCHECKER_H */
