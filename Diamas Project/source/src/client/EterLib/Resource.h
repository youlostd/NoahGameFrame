#ifndef METIN2_CLIENT_ETERLIB_RESOURCE_HPP
#define METIN2_CLIENT_ETERLIB_RESOURCE_HPP

#pragma once

#include <base/RefCounted.hpp>

class CResourceManager;

enum ResourceFlags
{
    // AbstractResource is added to the deletion queue.
    /// This flag is cleared, should the element be removed from the deletion queue.
    kResourceFlagDeletePending = (1 << 0),

    /// The resource is in use.
    kResourceFlagAlive = (1 << 1),
};

class AbstractResource : public RefCounted<AbstractResource>
{
    friend class CResourceManager;
    friend void DestroyReferenceCounted(AbstractResource *resource);

public:
    AbstractResource();
    virtual ~AbstractResource();

    virtual void Clear() = 0;

    bool Reload();
    virtual bool Load(int iSize, const void *c_pvBuf) = 0;

    uint32_t GetRawSize() const;

    const char *GetFileName() const;
    const std::string &GetFileNameString() const;

protected:
    bool HasFlags(uint32_t flags) const;
    void AddFlags(uint32_t flags);
    void RemoveFlags(uint32_t flags);

    std::string m_filename;
    uint32_t m_flags;

    uint32_t m_dataSize;
private:
    bool Create(std::string_view filename,
                const void *data,
                uint32_t dataSize);
};

BOOST_FORCEINLINE uint32_t AbstractResource::GetRawSize() const
{
    return m_dataSize;
}

BOOST_FORCEINLINE const std::string &AbstractResource::GetFileNameString() const
{
    return m_filename;
}

BOOST_FORCEINLINE const char *AbstractResource::GetFileName() const
{
    return m_filename.c_str();
}

BOOST_FORCEINLINE bool AbstractResource::HasFlags(uint32_t flags) const
{
    return !!(m_flags & flags);
}

BOOST_FORCEINLINE void AbstractResource::AddFlags(uint32_t flags)
{
    m_flags |= flags;
}

BOOST_FORCEINLINE void AbstractResource::RemoveFlags(uint32_t flags)
{
    m_flags &= ~flags;
}

template <class T>
class ConcreteResource : public AbstractResource
{
public:
    typedef T RealType;
    typedef boost::intrusive_ptr<T> Ptr;
};

typedef AbstractResource CResource;

#endif
