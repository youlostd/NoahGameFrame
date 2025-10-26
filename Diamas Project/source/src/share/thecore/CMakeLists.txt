include_directories(
	${EIGEN_ROOT}
)

set(SOURCE_FILES
	buffer.cpp
	fdwatch.cpp
	heart.cpp
	log.cpp
	main.cpp
	md5.cpp
	sha1.cpp
	sha256.cpp
	poly.cpp
	semaphore.cpp
	signal.cpp
	socket.cpp
	utils.cpp

	# win32 compat. stuff
	xgetopt.cpp
	xdirent.cpp
	xmd5.cpp
)

metin2_add_static_library(metin2_thecore ${SOURCE_FILES})
