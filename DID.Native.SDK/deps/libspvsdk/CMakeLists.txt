project(libspvsdk)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

if(IOS)
execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find ar
    OUTPUT_VARIABLE IOS_AR
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find ranlib
    OUTPUT_VARIABLE IOS_RANLIB
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_ARGS_INIT "${CMAKE_ARGS_INIT}"
    "-DIOS_AR=${IOS_AR}"
    "-DIOS_RANLIB=${IOS_RANLIB}")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(DEBUG_OPTION "ON")
else()
    set(DEBUG_OPTION "OFF")
endif()

ExternalProject_Add(
    libspvsdk

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/elastos/Elastos.ELA.SPV.Cpp/archive/v0.5.0-pre.tar.gz"
    URL_HASH SHA256=144202b8f5d46dac9425d9f76fd26ef2fad26c78d6c2b569760635a876aac4b8
    DOWNLOAD_NAME "libspv-0.5.0-pre.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        -DARGUMENT_LOG_ENABLE=ON
        -DSPDSDK_DEBUG_ON=${DEBUG_OPTION}
        -DSPV_CONSOLE_LOG=OFF
        -DSPV_BUILD_TEST_CASES=OFF
        -DSPV_BUILD_APPS=OFF
        -DSPV_ENABLE_SHARED=OFF
        -DSPV_ENABLE_STATIC=ON
        ${CMAKE_ARGS_INIT}
)
