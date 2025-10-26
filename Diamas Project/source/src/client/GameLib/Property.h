#ifndef METIN2_CLIENT_GAMELIB_PROPERTY_H
#define METIN2_CLIENT_GAMELIB_PROPERTY_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <string>

class CProperty
{
  public:
    CProperty(const std::string &c_pszFileName);
    ~CProperty();

    void Clear();
    bool Save();
    bool ReadFromMemory(const void *c_pvData, int iLen, const char *c_pszFileName);
    bool ReadFromXML(const char *c_pszCRC);

    const char *GetFileName();

    bool GetVector(const char *c_pszKey, CTokenVector &rTokenVector);
    bool GetString(const char *c_pszKey, const char **c_ppString);

    void PutVector(const char *c_pszKey, const CTokenVector &c_rTokenVector);
    void PutString(const char *c_pszKey, const char *c_pszString);

    bool Save(const char *c_pszFileName);

    uint32_t GetSize();
    uint32_t GetCRC();

  protected:
    std::string m_stFileName;
    std::string m_stCRC;
    const char *mc_pFileName;
    uint32_t m_dwCRC;

    CTokenVectorMap m_stTokenMap;
};
#endif /* METIN2_CLIENT_GAMELIB_PROPERTY_H */
