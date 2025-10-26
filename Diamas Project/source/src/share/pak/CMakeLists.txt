set(SOURCE_FILES
	ClientDb.hpp
	DiskVfsFile.cpp
	DiskVfsFile.hpp
	DiskVfsProvider.cpp
	DiskVfsProvider.hpp
	pak/PakArchive.cpp
	pak/PakArchive.hpp
	pak/PakFile.cpp
	pak/PakFile.hpp
	pak/PakFilename.cpp
	pak/PakFilename.hpp
	pak/PakFormat.hpp
	pak/PakUtil.hpp
	pak/PakWriter.cpp
	pak/PakWriter.hpp
	pak/PakVfsProvider.hpp
	pak/PakVfsProvider.cpp
	Util.cpp
	Util.hpp
	Vfs.cpp
	Vfs.hpp
	VfsFile.hpp
	VfsProvider.hpp
)

metin2_add_static_library(metin2_pak ${SOURCE_FILES})
target_link_libraries(metin2_pak PUBLIC metin2client_base lz4::lz4 xxhash)
