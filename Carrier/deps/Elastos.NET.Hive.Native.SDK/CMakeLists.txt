project(Elastos.NET.Hive.Native.SDK)

include(CarrierDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

if(WIN32)
    set(CURL_LIB_FILE "${CARRIER_INT_DIST_DIR}/lib/libcurl_a.lib")
else()
    set(CURL_LIB_FILE "${CARRIER_INT_DIST_DIR}/lib/libcurl.a")
endif()
file(TO_NATIVE_PATH ${CURL_LIB_FILE} CURL_LIB)
file(TO_NATIVE_PATH "${CARRIER_INT_DIST_DIR}/include" CURL_INC_DIR)

ExternalProject_Add(
    Elastos.NET.Hive.Native.SDK

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/elastos/Elastos.NET.Hive.Native.SDK/archive/feat_migrate_extenal_json.zip"
    URL_HASH SHA256=814982e26d8c6b67e7516e28a0354685d3f88afa63862cb22c6a314dab6c09b7
    DOWNLOAD_NAME "Elastos.NET.Hive.Native.SDK-feat_migrate_extenal_json.zip"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/hive.patch

    CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=${CARRIER_INT_DIST_DIR}"
        "-DCURL_LIBRARY=${CURL_LIB}"
        "-DCURL_INCLUDE_DIR=${CURL_INC_DIR}"
        -DBUILD_TESTING=false
        -DBUILD_SAMPLE=false
        ${CMAKE_ARGS_INIT})

set(HIVE_DEPS
    hive-api
    hive-api++
    curl)
if(WIN32)
    set(HIVE_DEPS ${HIVE_DEPS} zlib1 ws2_32)
else()
    set(HIVE_DEPS ${HIVE_DEPS} z stdc++)
endif()

add_library(hive INTERFACE)
target_link_libraries(hive INTERFACE ${HIVE_DEPS})
