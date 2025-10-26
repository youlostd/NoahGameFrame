#ifndef METIN2_CLIENT_GAMELIB_PROPERTYMANAGER_H
#define METIN2_CLIENT_GAMELIB_PROPERTYMANAGER_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CProperty;

class CPropertyManager : public CSingleton<CPropertyManager>
{
  public:
    CPropertyManager();
    virtual ~CPropertyManager();

    void Clear();

    bool BuildPack();

    bool LoadReservedCRC(const char *c_pszFileName);
    void ReserveCRC(uint32_t dwCRC);
    uint32_t GetUniqueCRC(const char *c_szSeed);
    template <class FuncType> void ForEachTree(FuncType func);

    bool Initialize(const char *c_pszPackFileName = NULL);
    bool Put(CProperty *property);
    bool Erase(const storm::String &filename);
    bool Register(std::string_view filename, CProperty **ppProperty = NULL);

    bool Get(uint32_t dwCRC, CProperty **ppProperty);
    bool Get(const std::string &c_pszFileName, CProperty **ppProperty);

  protected:
    typedef std::map<uint32_t, std::unique_ptr<CProperty>> TPropertyCRCMap;
    typedef std::set<uint32_t> TCRCSet;

    bool m_isFileMode;
    TPropertyCRCMap m_PropertyByCRCMap;
    TCRCSet m_ReservedCRCSet;
};

template <typename FuncType>
void CPropertyManager::ForEachTree(FuncType func)
{
    for(auto & [crc, uProperty] : m_PropertyByCRCMap)
    {
        const char* c_szPropertyType;
        if (!uProperty->GetString("PropertyType", &c_szPropertyType))
            return;

        switch (GetPropertyType(c_szPropertyType))
        {
        case PROPERTY_TYPE_TREE:
            func(uProperty.get());
            break;
        }
    }


}
#endif /* METIN2_CLIENT_GAMELIB_PROPERTYMANAGER_H */
