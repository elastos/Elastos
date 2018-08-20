project(toxcore)

include(CarrierDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

if(NOT WIN32 AND NOT CMAKE_CROSSCOMPILING)
    set(TOX_BOOTSTRAP_OPT -DBOOTSTRAP_DAEMON=ON)
endif()

ExternalProject_Add(
    toxcore0

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/TokTok/c-toxcore/releases/download/v0.1.10/c-toxcore-0.1.10.tar.gz"
    URL_HASH SHA256=4e9a2881dd0ea8e65a35fc9621644ccf500c1797a2d37983b0057ed3be971299
    DOWNLOAD_NAME "c-toxcore-0.1.10.tar.gz"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/toxcore.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CARRIER_INT_DIST_DIR}
        -DENABLE_SHARED=OFF
        -DENABLE_STATIC=ON
        -DBUILD_TOXAV=OFF
        -DBUILD_AV_TEST=OFF
        -DBUILD_NTOX=OFF
        -DDHT_BOOTSTRAP=ON
        ${TOX_BOOTSTRAP_OPT}
        ${CMAKE_ARGS_INIT}
)
