project(libconfig)

include(CarrierDefaults)
include(ExternalProject)

if(WIN32)
    set_win_build_options(WIN_BUILD_OPTIONS "Static")

    set(CONFIGURE_CMD "echo")
    set(CONFIGURE_ARGS "Done")

    set(BUILD_CMD "msbuild")
    set(BUILD_ARGS ${WIN_BUILD_OPTIONS}
        "libconfig_vs2017.sln")

    set(INSTALL_CMD "echo")
    set(INSTALL_ARGS "Done")
else()
    include(ExternalConfigureArgs)

    set(CONFIGURE_CMD "./configure")
    set(CONFIGURE_ARGS
        "--prefix=${CARRIER_INT_DIST_DIR}"
        "--enable-shared=no"
        "--disable-shared"
        "--enable-static=yes"
        "--disable-cxx"
        ${CONFIGURE_ARGS_INIT})

    set(BUILD_CMD "make")

    set(INSTALL_CMD "make")
    set(INSTALL_ARGS "install")
endif()

ExternalProject_Add(
    libconfig

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://hyperrealm.github.io/libconfig/dist/libconfig-1.7.2.tar.gz"
    URL_HASH SHA256=7c3c7a9c73ff3302084386e96f903eb62ce06953bb1666235fac74363a16fad9
    DOWNLOAD_NAME "libconfig-1.7.2.tar.gz"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    BUILD_IN_SOURCE 1

    PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/libconfig.patch

    CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
    INSTALL_COMMAND ${INSTALL_CMD} ${INSTALL_ARGS}
)
