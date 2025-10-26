#ifndef __INC_MAPPEDFILE_H__
#define __INC_MAPPEDFILE_H__

#include "FileBase.h"

#include <memory>

class CMappedFile : public CFileBase
{
public:
	enum ESeekType
	{
		SEEK_TYPE_BEGIN,
		SEEK_TYPE_CURRENT,
		SEEK_TYPE_END
	};

	CMappedFile();
	~CMappedFile() override;

	bool Create(const std::string& filename);
	void Create(size_t size);

	void Destroy();

	int Map(int offset=0, int size=0);
	int Seek(uint32_t offset, int iSeekType = SEEK_TYPE_BEGIN);

	uint32_t Size() const;
	const void* Get() const;

	bool Read(void* dest, uint32_t bytes);
	bool Read(size_t seekPos, void* dest, uint32_t bytes);

	uint32_t GetSeekPosition();
	uint8_t* GetCurrentSeekPoint();

private:
	void Unmap(LPCVOID data);

	// non-owning ptr to currently bound data
	uint8_t* m_pbBufLinkData;
	uint32_t m_dwBufLinkSize;

	uint32_t m_seekPosition;

	// Real file-mapping code
	HANDLE m_hFM;
	uint32_t m_dataOffset;
	uint32_t m_mapSize;
	LPVOID m_lpMapData;

	// A simple buffer owned by the CMappedFile object.
	// Used for files loaded from the VFS.
	std::unique_ptr<uint8_t[]> m_ownedBuffer;
};

#endif
