project(slim-pthread)

include(CarrierDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    slim-pthread

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/iwhisperio/slim-pthread/archive/master.zip"
    URL_HASH SHA256=503e968a1e57764e031401689f58765d70a70b7d4a04952990c3a04a9e96ccf6
    DOWNLOAD_NAME "slim-pthread-master.zip"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CARRIER_INT_DIST_DIR}
        -DENABLE_STATIC=${ENABLE_STATIC}
        -DENABLE_SHARED=${ENABLE_SHARED}
        -DENABLE_TESTS=FALSE
        ${CMAKE_ARGS_INIT}
)

if(ENABLE_SHARED)
    set(SHARED_LIBRARY_NAME
        "${CARRIER_INT_DIST_DIR}/bin/pthread.dll")

    file(RELATIVE_PATH SHARED_LIBRARY_NAME ${CMAKE_CURRENT_LIST_DIR}
        ${SHARED_LIBRARY_NAME})

    install(PROGRAMS ${SHARED_LIBRARY_NAME}
        DESTINATION bin)
endif()
