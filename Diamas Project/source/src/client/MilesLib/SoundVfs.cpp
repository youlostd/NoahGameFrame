#include "Stdafx.h"
#include "SoundVfs.hpp"

#include "../EterBase/Timer.h"
#include "../EterBase/MappedFile.h"

#include <mss.h>

#include <unordered_map>
#include <mutex>
#include <memory>
#include <pak/Vfs.hpp>
#include <algorithm>

namespace
{
struct SoundFile
{
    std::unique_ptr<VfsFile> fp;
    uint32_t pos;
};

std::unordered_map<U32, SoundFile> soundFiles;
std::vector<U32> freeIndices;
U32 soundFileIndex = 0;
std::mutex soundFileMutex;

U32 AILCALLBACK open_callback(char const *filename, UINTa *file_handle)
{
	U32 index = 0;

	{
		std::lock_guard<std::mutex> l(soundFileMutex);

		if (!freeIndices.empty()) {
			index = freeIndices.back();
			freeIndices.pop_back();
		} else {
			index = ++soundFileIndex;
		}
	}

	auto& file = soundFiles[index];
	file.fp = GetVfs().Open(filename, 0);
	file.pos = 0;

	if (!file.fp)
		return 0;

	*file_handle = index;
	return 1;
}

void AILCALLBACK close_callback(UINTa file_handle)
{
    std::lock_guard<std::mutex> l(soundFileMutex);

    soundFiles.erase(file_handle);
    freeIndices.push_back(file_handle);
}

S32 AILCALLBACK seek_callback(UINTa file_handle, S32 offset, U32 type)
{
	const auto it = soundFiles.find(file_handle);
	if (it == soundFiles.end())
		return 0;

	auto& file = it->second;

	if (type == AIL_FILE_SEEK_END)
		file.pos = file.fp->GetSize() + offset;
	else if (type == AIL_FILE_SEEK_CURRENT)
		file.pos += offset;
	else
		file.pos = offset;

	file.pos = std::min<uint32_t>(file.fp->GetSize(), file.pos);
	return file.pos;
}

U32 AILCALLBACK read_callback(UINTa file_handle, void *buffer, U32 bytes)
{
	const auto it = soundFiles.find(file_handle);
	if (it == soundFiles.end())
		return 0;

	auto& file = it->second;

	bytes = std::min<U32>(file.fp->GetSize() - file.pos, bytes);
	if (!file.fp->Read(file.pos, buffer, bytes))
		return 0;

	file.pos += bytes;
	return bytes;
}
}

void RegisterMilesVfs()
{
    AIL_set_file_callbacks(open_callback, close_callback,
                           seek_callback, read_callback);
}
