#include "StdAfx.h"
#include <pak/Vfs.hpp>
#include "../EterBase/CRC32.h"
#include "../EterBase/Timer.h"
#include "../EterBase/MappedFile.h"

#include "Resource.h"
#include "ResourceManager.h"

#include <storm/io/View.hpp>

AbstractResource::AbstractResource()
    : m_flags(0)
      , m_dataSize(0)
{
}

AbstractResource::~AbstractResource()
{
    // dtor
}

bool AbstractResource::Reload()
{
    Clear();

    spdlog::trace("AbstractResource::Reload {0}", GetFileName());
    const auto fp = GetVfs().Open(GetFileName(), kVfsOpenFullyBuffered);

	if (!fp)
		return false;

	const auto size = fp->GetSize();

	storm::View data(storm::GetDefaultAllocator());
	fp->GetView(0, data, size);

	return Load(size, data.GetData());
}

bool AbstractResource::Create(std::string_view filename,
                              const void *data,
                              uint32_t dataSize)
{
    spdlog::trace("AbstractResource::Create({0})", filename);

    m_filename = filename;
    m_dataSize = dataSize;

    if (!Load(dataSize, data))
        return false;

    m_flags |= kResourceFlagAlive;
    return true;
}

void DestroyReferenceCounted(AbstractResource *resource)
{
    //DSPDLOG_DEBUG("AbstractResource::Destroy(%s)",
    //	resource->GetFileName());

    auto resMgr = CResourceManager::InstancePtr();
    if (resMgr)
        resMgr->DestroyResourceLazy(resource);
}
