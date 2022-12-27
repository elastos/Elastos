project(flatcc)

include(CarrierDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    flatcc

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/dvidelabs/flatcc/archive/v0.6.0.tar.gz"
    URL_HASH SHA256=a92da3566d11e19bb807a83554b1a2c644a5bd91c9d9b088514456bb56e1c666
    DOWNLOAD_NAME "flatcc-0.6.0.tar.gz"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CARRIER_INT_DIST_DIR}
        -DFLATCC_INSTALL=ON
        -DFLATCC_RTONLY=ON
        ${CMAKE_ARGS_INIT}
)

if(WIN32)
    #TODO:
else()
    set(INSTALL_WORKING_DIR "${CARRIER_INT_DIST_DIR}/lib")
endif()

ExternalProject_Add_Step(
    flatcc
    postinstall

    COMMENT "Performing post install step for 'flatcc'"
    DEPENDEES install
    WORKING_DIRECTORY ${INSTALL_WORKING_DIR}

    COMMAND cmake
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DINT_DIST_DIR=${CARRIER_INT_DIST_DIR}
        -P "${CMAKE_CURRENT_LIST_DIR}/flatcc_install.cmake"
)

if(ENABLE_STATIC)
    export_static_library(flatccrt)
endif()
