project(libcrystal)

include(CarrierDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    libcrystal

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/iwhisperio/libcrystal/archive/release-v1.0.5.tar.gz"
    URL_HASH SHA256=6256713f34bb341d718a38b5f3a108c6d9c1db93e6391f670108de1cd09b3cee
    DOWNLOAD_NAME "libcrystal-v1.0.5.tar.gz"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CARRIER_INT_DIST_DIR}
        -DENABLE_STATIC=${ENABLE_STATIC}
        -DENABLE_SHARED=${ENABLE_SHARED}
        -DLIBSODIUM_STATIC=TRUE
        -DENABLE_CRYPTO=TRUE
        -DENABLE_TESTS=FALSE
        ${CMAKE_ARGS_INIT}
)

if(ENABLE_STATIC)
    export_static_library(crystal)
endif()

if(ENABLE_SHARED)
    export_shared_library(crystal)
endif()
