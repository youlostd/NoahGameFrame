set(SOURCE_FILES
    Document.cpp
    Document.hpp
    Types.hpp
    ErrorHandling.cpp
    ErrorHandling.hpp
    Util.cpp
    Util.hpp
)

metin2_add_static_library(metin2_xml ${SOURCE_FILES})
target_link_libraries(metin2_xml PUBLIC rapidxml)
