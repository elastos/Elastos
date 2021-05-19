project(toxcore)

include(CarrierDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    toxcore0

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/TokTok/c-toxcore/archive/v0.2.10.tar.gz"
    URL_HASH SHA256=5ccc4ecd79b95c367efe8341b06bf2a81f853032e6fcd7cb2bc0ad0ef7419d40
    DOWNLOAD_NAME "c-toxcore-0.2.10.tar.gz"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/toxcore.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CARRIER_INT_DIST_DIR}
        -DCMAKE_USE_SLIM_PTHREAD_INIT=TRUE
        -DENABLE_SHARED=OFF
        -DENABLE_STATIC=ON
        -DBUILD_TOXAV=OFF
        -DBUILD_AV_TEST=OFF
        -DDHT_BOOTSTRAP=OFF
        -DBUILD_CARRIER=ON
        ${CMAKE_ARGS_INIT}
)

if(ENABLE_STATIC)
    export_static_library(toxcore)
endif()
