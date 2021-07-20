project(platform-specific)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalConfigureArgs)

if(ANDROID)
    set(MAKE_STANDALONE_TOOLCHAIN
        "${CMAKE_ANDROID_NDK}/build/tools/make_standalone_toolchain.py")

    if(NOT EXISTS ${MAKE_STANDALONE_TOOLCHAIN})
        message(FATAL_ERROR
            "Android NDK: make_standalone_toolchain.py is missing.")
    endif()

    if(${CMAKE_ANDROID_STL_TYPE} MATCHES "^c\\+\\+_")
        set(STL_LIBRARY "libc++")
    elseif(${CMAKE_ANDROID_STL_TYPE} MATCHES "^stlport_")
        set(STL_LIBRARY "stlport")
    else()
        set(STL_LIBRARY "gnustl")
    endif()

    ExternalProject_Add(
        android-toolchain

        SOURCE_DIR ${CMAKE_ANDROID_NDK}
        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND ${MAKE_STANDALONE_TOOLCHAIN}
            --force
            --verbose
            --arch ${CMAKE_ANDROID_ARCH}
            --api ${CMAKE_SYSTEM_VERSION}
            --stl ${STL_LIBRARY}
            --install-dir ${XDK_TOOLCHAIN}
    )

    add_custom_target(platform-specific
        COMMENT "Build platform-specific: Android Standalone Toolchain"
        COMMAND echo Done)
    add_dependencies(platform-specific
        android-toolchain
        flatcc-parser)
elseif(WIN32)
    if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
        set(CYGWIN_ARCH "x86_64")
    else()
        set(CYGWIN_ARCH "x86")
    endif()

    if(${CYGWIN_ARCH} STREQUAL "x86_64")
        set(CYGWIN_RUNTIME_HASH "a1c9b3761e3e9c8e863de0415ed71722")
        set(CYGWIN_LIBATTR_HASH "7423e99e8de393819ba982239061e7a2")
        set(CYGWIN_LIBICONV_HASH "9fd5294883bacfadb3d50a28abc2ccfd")
        set(CYGWIN_LIBSIGSEGV_HASH "3C74C7599DCEAAF18EE182F54F66BBCC")
        set(CYGWIN_LIBINTL8_HASH "c3453c7896bce5419afefacae6ed5ec9")
        set(CYGWIN_PATCH_HASH "e2a7c92fa552903f3710bb0277e2a461")
        set(CYGWIN_DIFF_HASH "4B254F727F2A2165CB75943960D27370")
    else()
        set(CYGWIN_RUNTIME_HASH "4ecea5ee9e023badd8201ff3c5c4ae44")
        set(CYGWIN_LIBATTR_HASH "2c60a11de348a3a694492cd482adfb7e")
        set(CYGWIN_LIBICONV_HASH "d9df418d5cc50dfcbfad971282d6fe1f")
        set(CYGWIN_LIBSIGSEGV_HASH "1E4B9346418D13208899C4EB68B8FA00")
        set(CYGWIN_LIBINTL8_HASH "4225b6d4d30b421341d761a6532156fe")
        set(CYGWIN_PATCH_HASH "b652c0c0815e74b195cd29892515ffa0")
        set(CYGWIN_DIFF_HASH "64A2192FE0810A4C26AD7966B5F0ABC8")
    endif()

    set(URL_PREFIX "http://mirrors.kernel.org/sourceware/cygwin/${CYGWIN_ARCH}/release")

    ExternalProject_Add(
        cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/cygwin/cygwin-3.1.6-1.tar.xz"
        URL_HASH MD5=${CYGWIN_RUNTIME_HASH}
        DOWNLOAD_NAME "cygwin-3.1.6-1_${CYGWIN_ARCH}.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND if not exist ${PROJECT_HOST_TOOLS_DIR} mkdir ${PROJECT_HOST_TOOLS_DIR}
                COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}
    )

    ExternalProject_Add(
        cygwin-libattr

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/attr/libattr1/libattr1-2.4.46-1.tar.bz2"
        URL_HASH MD5=${CYGWIN_LIBATTR_HASH}
        DOWNLOAD_NAME "libattr1-2.4.46-1_${CYGWIN_ARCH}.tar.bz2"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-libiconv

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/libiconv/libiconv2/libiconv2-1.14-3.tar.xz"
        URL_HASH MD5=${CYGWIN_LIBICONV_HASH}
        DOWNLOAD_NAME "libiconv2-1.14-3_${CYGWIN_ARCH}.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-libsigsegv

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/libsigsegv/libsigsegv2/libsigsegv2-2.10-2.tar.xz"
        URL_HASH MD5=${CYGWIN_LIBSIGSEGV_HASH}
        DOWNLOAD_NAME "libsigsegv2-2.10-2_${CYGWIN_ARCH}.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-libintl8

        DEPENDS cygwin-runtime

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/gettext/libintl8/libintl8-0.19.8.1-2.tar.xz"
        URL_HASH MD5=${CYGWIN_LIBINTL8_HASH}
        DOWNLOAD_NAME "libintl8-0.19.8.1-2_${CYGWIN_ARCH}.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-patch

        DEPENDS cygwin-runtime cygwin-libattr cygwin-libintl8 cygwin-libiconv

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/patch/patch-2.7.4-1.tar.xz"
        URL_HASH MD5=${CYGWIN_PATCH_HASH}
        DOWNLOAD_NAME "patch-2.7.4-1_${CYGWIN_ARCH}.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    ExternalProject_Add(
        cygwin-diff

        DEPENDS cygwin-runtime cygwin-libsigsegv

        PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
        URL "${URL_PREFIX}/diffutils/diffutils-3.5-2.tar.xz"
        URL_HASH MD5=${CYGWIN_DIFF_HASH}
        DOWNLOAD_NAME "diffutils-3.5-2_${CYGWIN_ARCH}.tar.xz"
        DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
        DOWNLOAD_NO_PROGRESS 1

        BUILD_IN_SOURCE 1

        CONFIGURE_COMMAND echo Done
        BUILD_COMMAND echo Done
        INSTALL_COMMAND xcopy /s /e /q /y . ${PROJECT_HOST_TOOLS_DIR}\\usr
    )

    add_custom_target(platform-specific
        COMMENT "Build platform-specific: Cygwin Runtime"
        COMMAND echo Done
    )
    add_dependencies(platform-specific
        cygwin-patch)
else()
    add_custom_target(platform-specific
        COMMENT "Build platform-specific: None"
        COMMAND echo Done
    )
endif()
