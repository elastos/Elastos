project(pjsip)

include(CarrierDefaults)
include(ExternalProject)

if(WIN32)
    set_win_build_options(WIN_BUILD_OPTIONS "")

    set(CONFIGURE_CMD "echo")
    set(CONFIGURE_ARGS "Done")

    set(BUILD_CMD "msbuild")
    set(BUILD_ARGS ${WIN_BUILD_OPTIONS}
        "pjproject-vs14.sln")

    set(INSTALL_CMD "echo")
    set(INSTALL_ARGS "Done")
else()
    # Hack to remove linkage errors for iOS platform.
    if(IOS)
        string(APPEND CMAKE_C_LINK_FLAGS_INIT
            "-framework AudioToolbox "
            "-framework AVFoundation "
            "-framework CFNetwork "
            "-framework Foundation")
    endif()

    if(ANDROID)
        string(APPEND CMAKE_C_FLAGS_INIT
            " "
            "-DPJ_AUTOCONF "
            "-DJNI_OnLoad=PJ_JNI_OnLoad ")
    endif()

    include(ExternalConfigureArgs)

    if(IOS)
      set(ARGS_HOST_REDEF
          "--host=${CMAKE_SYSTEM_PROCESSOR}-apple-darwin_ios")
    endif()

    set(CONFIGURE_CMD "./configure")
    set(CONFIGURE_ARGS
        "--prefix=${CARRIER_INT_DIST_DIR}"
        "--disable-ssl"
        "--disable-shared"
        "--disable-resample"
        "--disable-sound"
        "--disable-oss"
        "--disable-video"
        "--disable-ext-sound"
        "--disable-speex-aec"
        "--disable-g711-codec"
        "--disable-l16-codec"
        "--disable-gsm-codec"
        "--disable-g722-codec"
        "--disable-g7221-codec"
        "--disable-speex-codec"
        "--disable-ilbc-codec"
        "--disable-libsamplerate"
        "--disable-resample-dll"
        "--disable-sdl"
        "--disable-ffmpeg"
        "--disable-v4l2"
        "--disable-openh264"
        "--disable-webrtc"
        "--disable-ipp"
        "--disable-opencore-amr"
        "--disable-silk"
        "--disable-opus"
        "--disable-libyuv"
        "ac_cv_lib_uuid_uuid_generate=no"
        ${CONFIGURE_ARGS_INIT}
        ${ARGS_HOST_REDEF})

    set(BUILD_CMD "make")

    set(INSTALL_CMD "make")
    set(INSTALL_ARGS "install")
endif()

ExternalProject_Add(
    pjsip

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "http://www.pjsip.org/release/2.5.5/pjproject-2.5.5.tar.bz2"
    URL_HASH SHA256=ab39207b761d3485199cd881410afeb2d171dff7c2bf75e8caae91c6dca508f3
    DOWNLOAD_NAME "pjproject-2.5.5.tar.bz2"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    BUILD_IN_SOURCE 1

    PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/pjsip.patch

    CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
    INSTALL_COMMAND ${INSTALL_CMD} ${INSTALL_ARGS}
)

# Issue: Pjsip static library distributions have ugly-and-annoying
# library suffix name, and it varies every time along with updatation
# of host platform. The suffix name should be removed before linkage
# from our project.
#
# Workaround: to create a symbol linkage to each static librariy
# without suffix name on it. Therefore, we have to acquire suffix
# name for each build target.

if(UNIX)
    if(APPLE)
        if(IOS)
            # set library suffix name for iOS target and
            # only support for arm64 and x86_64.
            set(PJLIB_SUFFIX "${CMAKE_SYSTEM_PROCESSOR}-apple-darwin_ios")
        else()
            # set library suffix name for MacOSX
            execute_process(
                COMMAND gcc -dumpmachine
                OUTPUT_VARIABLE PJLIB_SUFFIX
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        endif()
    else()
        if(ANDROID)
            # set suffix name for Android target.
            # support for arch[ arm | arm64 | x86_64 | x86 ]
            if(${CMAKE_ANDROID_ARCH} STREQUAL "arm")
                set(PJLIB_SUFFIX "arm-unknown-linux-androideabi")
            elseif(${CMAKE_ANDROID_ARCH} STREQUAL "arm64")
                set(PJLIB_SUFFIX "aarch64-unknown-linux-android")
            elseif(${CMAKE_ANDROID_ARCH} STREQUAL "x86_64")
                set(PJLIB_SUFFIX "x86_64-pc-linux-android")
            else()
                set(PJLIB_SUFFIX "i686-pc-linux-android")
           endif()
        elseif(RASPBERRYPI)
            # set library suffix name for raspberry PI target.
            # only support for armv7l. (TODO: update)
            # only support with Linux host.
            set(PJLIB_SUFFIX "arm-unknown-linux-gnueabihf")
        elseif(BUILD_ON_RPI)
            # set library suffix name for raspberry PI target
            # only support for compilation on raspberry PI platform.
            set(PJLIB_SUFFIX "armv7l-unknown-linux-gnueabihf")
        else()
            # set library name for Linux target itself.
            set(PJLIB_SUFFIX "x86_64-unknown-linux-gnu")
        endif()
    endif()
endif()

if(WIN32)
    # set library suffix name for windows target.
    # Notice: use CMAKE_SIZEOF_VOID_P to check whether target is 64bit
    # or 32bit instead of using ${CMAKE_SYSTEM_PROCESSOR}
    if(${CMAKE_SIZEOF_VOID_P} STREQUAL "8")
        set(PJ_ARCH "x86_64-x64")
    else()
        set(PJ_ARCH "i386-Win32")
    endif()

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(PJLIB_SUFFIX "${PJ_ARCH}-vc14-Debug")
    else()
        set(PJLIB_SUFFIX "${PJ_ARCH}-vc14")
    endif()
endif()

if(NOT PJLIB_SUFFIX)
    message(FATAL_ERROR
        "Can not acquire PJLIB_SUFFIX, impossible")
endif()

if(WIN32)
    set(INSTALL_WORKING_DIR "${CARRIER_DEPS_BUILD_PREFIX}/src/pjsip")
else()
    set(INSTALL_WORKING_DIR "${CARRIER_INT_DIST_DIR}/lib")
endif()

ExternalProject_Add_Step(
    pjsip
    postinstall

    COMMENT "Performing post install step for 'pjsip'"
    DEPENDEES install
    WORKING_DIRECTORY ${INSTALL_WORKING_DIR}

    COMMAND cmake
        -DPJLIB_SUFFIX=${PJLIB_SUFFIX}
        -DINT_DIST_DIR=${CARRIER_INT_DIST_DIR}
        -P "${CMAKE_CURRENT_LIST_DIR}/pjsip_install.cmake"
)

if(ENABLE_STATIC)
    if(WIN32)
        set(_PJLIB_NAME pjlib)
        set(_SRTPLIB_NAME libsrtp)
    else()
        set(_PJLIB_NAME pj)
        set(_SRTPLIB_NAME srtp)
    endif()

    export_static_library(${_PJLIB_NAME})
    export_static_library(${_SRTPLIB_NAME})
    export_static_library(pjlib-util)
    export_static_library(pjnath)
    export_static_library(pjmedia)
endif()
