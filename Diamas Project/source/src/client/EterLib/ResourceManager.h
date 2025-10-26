#ifndef METIN2_CLIENT_ETERLIB_RESOURCEMANAGER_HPP
#define METIN2_CLIENT_ETERLIB_RESOURCEMANAGER_HPP

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/Singleton.hpp>
#include <pak/Vfs.hpp>
#include <boost/unordered_map.hpp>
#include "Resource.h"
#include <SpdLog.hpp>
#include <storm/io/View.hpp>
#include <chrono>
#include <list>
#include <string>

class AbstractResource;

class CResourceManager : public CSingleton<CResourceManager>
{
    friend void DestroyReferenceCounted(AbstractResource *resource);

public:
    CResourceManager();
    ~CResourceManager();

    template <class T>

    typename T::Ptr LoadResource(std::string_view filename);

    void ReloadAll();

    void Update();

    void DumpStatistics();

protected:

    typedef boost::unordered_map<std::string, AbstractResource *> ResourceMap;
    typedef std::list<AbstractResource *> ResourceQueue;

    struct DeletedResource
    {
        AbstractResource *ptr{};
        std::chrono::steady_clock::time_point deleteTime;
    };

    AbstractResource *GetResource(std::string_view filename);

    void DestroyResourceLazy(AbstractResource *r);
    uint32_t PruneDeletionQueue(bool ignoreTime = false);

    ResourceMap m_resources;
    std::vector<DeletedResource> m_deletionQueue;
};

template <class T>

typename T::Ptr CResourceManager::LoadResource(std::string_view filename)
{
    auto cached = GetResource(filename);
    if (cached)
        return static_cast<T *>(cached);

    const auto& fp = GetVfs().Open(filename, kVfsOpenFullyBuffered);
    if (!fp) {
        return nullptr;
    }

    const auto size = fp->GetSize();

    storm::View data(storm::GetDefaultAllocator());
    fp->GetView(0, data, size);

    typename T::Ptr r(new T());
    if (!r->Create(filename, data.GetData(), size)) {
        return nullptr;
    }

    m_resources.emplace(filename, r.get());
    return r;
}

#endif
