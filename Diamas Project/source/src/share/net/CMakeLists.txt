
SET(SOURCE_FILES 
    Acceptor.cpp
    Acceptor.hpp
    ClientSocketAdapter-impl.hpp
    ClientSocketAdapter.hpp
    AsyncClientSocketAdapter-impl.hpp
    AsyncClientSocketAdapter.hpp
    Socket.cpp
    Socket.hpp
    SocketBase-impl.hpp
	SocketBase.cpp
	SocketBase.hpp
    Type.hpp
    Util.hpp
)


metin2_add_static_library(metin2_net ${SOURCE_FILES})

if(NOT WIN32)
	target_link_libraries(metin2_net PUBLIC Microsoft.GSL::GSL pthread)
else()
	target_link_libraries(metin2_net PUBLIC Microsoft.GSL::GSL)
endif()
target_include_directories(metin2_net PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>")