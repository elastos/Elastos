project(libzip)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
    libzip

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/nih-at/libzip/releases/download/rel-1-6-1/libzip-1.6.1.tar.gz"
    URL_HASH SHA256=06eb8e9141fd19e2788cabaea9c9c2fd4d488d9e1484eb474bbfcac78e7b1d88
    DOWNLOAD_NAME "libzip-1.6.1.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        -DBUILD_SHARED_LIBS=OFF
        -DENABLE_GNUTLS=OFF
        -DENABLE_MBEDTLS=OFF
        -DENABLE_WINDOWS_CRYPTO=OFF
        -DENABLE_BZIP2=OFF
        -DENABLE_LZMA=OFF
        -DBUILD_TOOLS=OFF
        -DBUILD_EXAMPLES=OFF
        -DBUILD_DOC=OFF
        -DBUILD_REGRESS=OFF
        -DCMAKE_FIND_ROOT_PATH=${PROJECT_INT_DIST_DIR}
        ${CMAKE_ARGS_INIT}
)
