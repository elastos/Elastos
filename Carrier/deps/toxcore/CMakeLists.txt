project(toxcore)

include(CarrierDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    toxcore0

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/TokTok/c-toxcore/archive/v0.2.7.tar.gz"
    URL_HASH SHA256=bef6dbb06d921af58c6a2d7629e6f8eeafa1e541d379e4ff2ccb4ff5d3499cd1
    DOWNLOAD_NAME "c-toxcore-0.2.7.tar.gz"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/toxcore.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CARRIER_INT_DIST_DIR}
        -DCMAKE_USE_SLIM_PTHREAD_INIT=TRUE
        -DENABLE_SHARED=OFF
        -DENABLE_STATIC=ON
        -DBUILD_TOXAV=OFF
        -DBUILD_AV_TEST=OFF
        -DBUILD_NTOX=OFF
        -DDHT_BOOTSTRAP=OFF
        -DBUILD_CARRIER=ON
        ${CMAKE_ARGS_INIT}
)
