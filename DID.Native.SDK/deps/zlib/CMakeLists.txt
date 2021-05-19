project(zlib)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    zlib

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/madler/zlib/archive/v1.2.8.tar.gz"
    URL_HASH SHA256=e380bd1bdb6447508beaa50efc653fe45f4edc1dafe11a251ae093e0ee97db9a
    DOWNLOAD_NAME "zlib-1.2.8.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1
    LOG_DOWNLOAD FALSE

    PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/zlib.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        ${CMAKE_ARGS_INIT}
)

if(WIN32)
    ExternalProject_Add_Step(
        zlib post-install
        DEPENDEES install
        WORKING_DIRECTORY "${PROJECT_INT_DIST_DIR}/lib"
        COMMAND ${CMAKE_COMMAND} -E rename zlib1.lib z.lib
    )
endif()
