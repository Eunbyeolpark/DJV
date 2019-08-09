include(ExternalProject)

ExternalProject_Add(
    JPEG_EXTERNAL
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/JPEG
    DEPENDS ZLIB
    URL "http://sourceforge.net/projects/libjpeg-turbo/files/2.0.2/libjpeg-turbo-2.0.2.tar.gz?download"
    CMAKE_ARGS
        -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
		-DENABLE_SHARED=${JPEG_SHARED_LIBS}
        -DBUILD_SHARED_LIBS=${JPEG_SHARED_LIBS}
        -DZLIB_SHARED_LIBS=${ZLIB_SHARED_LIBS})

set(JPEG_FOUND TRUE)
set(JPEG_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    if(WIN32)
    elseif(APPLE)
    else()
        set(JPEG_LIBRARY ${CMAKE_INSTALL_PREFIX}/lib/libjpeg.a)
    endif()
else()
    if(WIN32)
    elseif(APPLE)
    else()
        set(JPEG_LIBRARY ${CMAKE_INSTALL_PREFIX}/lib/libjpeg.a)
    endif()
endif()
set(JPEG_LIBRARIES ${JPEG_LIBRARY} ${ZLIB_LIBRARIES})

if(JPEG_FOUND AND NOT TARGET JPEG::JPEG)
    add_library(JPEG::JPEG UNKNOWN IMPORTED)
    add_dependencies(JPEG::JPEG JPEG_EXTERNAL)
    set_target_properties(JPEG::JPEG PROPERTIES
        IMPORTED_LOCATION "${JPEG_LIBRARY}"
        INTERFACE_LINK_LIBRARIES ZLIB
        INTERFACE_INCLUDE_DIRECTORIES "${JPEG_INCLUDE_DIRS}"
        INTERFACE_COMPILE_DEFINITIONS JPEG_FOUND)
endif()
if(JPEG_FOUND AND NOT TARGET JPEG)
    add_library(JPEG INTERFACE)
    target_link_libraries(JPEG INTERFACE JPEG::JPEG)
endif()
