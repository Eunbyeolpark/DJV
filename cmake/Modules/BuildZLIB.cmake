include(ExternalProject)

ExternalProject_Add(
    ZLIB_EXTERNAL
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ZLIB
    URL http://www.zlib.net/zlib-1.2.11.tar.gz
	PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/third-party/zlib-patch/CMakeLists.txt ${CMAKE_CURRENT_BINARY_DIR}/ZLIB/src/ZLIB_EXTERNAL/CMakeLists.txt
    CMAKE_ARGS
        -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DBUILD_SHARED_LIBS=${ZLIB_SHARED_LIBS})

set(ZLIB_FOUND TRUE)
set(ZLIB_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    if(WIN32)
    elseif(APPLE)
    else()
        set(ZLIB_LIBRARY ${CMAKE_INSTALL_PREFIX}/lib/libz.a)
    endif()
else()
    if(WIN32)
    elseif(APPLE)
    else()
        set(ZLIB_LIBRARY ${CMAKE_INSTALL_PREFIX}/lib/libz.a)
    endif()
endif()
set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})

if(ZLIB_FOUND AND NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
    add_dependencies(ZLIB::ZLIB ZLIB_EXTERNAL)
    set_target_properties(ZLIB::ZLIB PROPERTIES
        IMPORTED_LOCATION "${ZLIB_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIR}")
endif()
if(ZLIB_FOUND AND NOT TARGET ZLIB)
    add_library(ZLIB INTERFACE)
    target_link_libraries(ZLIB INTERFACE ZLIB::ZLIB)
endif()
