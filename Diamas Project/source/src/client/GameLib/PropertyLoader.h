#ifndef METIN2_CLIENT_GAMELIB_PROPERTYLOADER_H
#define METIN2_CLIENT_GAMELIB_PROPERTYLOADER_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../eterbase/FileDir.h"

class CPropertyManager;

class CPropertyLoader : public CDir
{
  public:
    CPropertyLoader();
    virtual ~CPropertyLoader();

    void SetPropertyManager(CPropertyManager *pPropertyManager);
    uint32_t RegisterFile(const char *c_szPathName, const char *c_szFileName);

    virtual bool OnFolder(const char *c_szFilter, const char *c_szPathName, const char *c_szFileName);
    virtual bool OnFile(const char *c_szPathName, const char *c_szFileName);

  protected:
    CPropertyManager *m_pPropertyManager;
};
#endif /* METIN2_CLIENT_GAMELIB_PROPERTYLOADER_H */
