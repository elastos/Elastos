project(libressl)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    libressl

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-2.7.4.tar.gz"
    URL_HASH SHA256=1e3a9fada06c1c060011470ad0ff960de28f9a0515277d7336f7e09362517da6
    DOWNLOAD_NAME "libressl-2.7.4.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        -DLIBRESSL_SKIP_INSTALL=OFF
        -DLIBRESSL_APPS=OFF
        -DLIBRESSL_TESTS=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DENABLE_ASM=OFF
        -DENABLE_EXTRATESTS=OFF
        -DENABLE_NC=OFF
        ${CMAKE_ARGS_INIT}
)
