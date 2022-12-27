project(cJSON)

include(CarrierDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    cJSON

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/DaveGamble/cJSON/archive/v1.7.10.tar.gz"
    URL_HASH SHA256=cc544fdd065f3dd19113f1d5ba5f61d696e0f810f291f4b585d1dec361b0188e
    DOWNLOAD_NAME "cJSON-1.7.10.tar.gz"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CARRIER_INT_DIST_DIR}
        -DBUILD_SHARED_LIBS=Off
        -DENABLE_HIDDEN_SYMBOLS=On
        ${CMAKE_ARGS_INIT}
)

if(ENABLE_STATIC)
    export_static_library(cjson)
endif()
