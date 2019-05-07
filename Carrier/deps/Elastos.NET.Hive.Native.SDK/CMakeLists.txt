project(Elastos.NET.Hive.Native.SDK)

include(CarrierDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

if(WIN32)
    set(CURL_LIB_FILE "${CARRIER_INT_DIST_DIR}/lib/libcurl_a.lib")
else()
    set(CURL_LIB_FILE "${CARRIER_INT_DIST_DIR}/lib/libcurl.a")
endif()

ExternalProject_Add(
    Elastos.NET.Hive.Native.SDK

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/elastos/Elastos.NET.Hive.Native.SDK/archive/release-v0.1.1.tar.gz"
    URL_HASH SHA256=3a203a5746479421d434cbdc91e434b9ae078231b2645c55f3c58a161b403240
    DOWNLOAD_NAME "Elastos.NET.Hive.Native.SDK-release-v0.1.1.tar.gz"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=${CARRIER_INT_DIST_DIR}"
        "-DCURL_LIBRARY=${CURL_LIB_FILE}"
        "-DCURL_INCLUDE_DIR=${CARRIER_INT_DIST_DIR}/include"
        -DBUILD_TESTING=FALSE
        ${CMAKE_ARGS_INIT})

set(HIVE_DEPS
    hive-api
    hive-api++
    curl)

set(LIBC++_STATIC
    ${CMAKE_ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libs/${CMAKE_ANDROID_ARCH_ABI}/libc++_static.a)

if(WIN32)
    set(HIVE_DEPS ${HIVE_DEPS} zlib1 ws2_32)
elseif(ANDROID)
    set(HIVE_DEPS ${HIVE_DEPS} z ${LIBC++_STATIC})
else()
    set(HIVE_DEPS ${HIVE_DEPS} z stdc++)
endif()

add_library(hive INTERFACE)
target_link_libraries(hive INTERFACE ${HIVE_DEPS})

if(ENABLE_STATIC)
    export_static_library(hive-api++)
    export_static_library(hive-api)
endif()

if(ENABLE_STATIC AND ANDROID)
    install(FILES ${LIBC++_STATIC} DESTINATION lib)
endif()
