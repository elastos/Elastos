# Configurable variables.
# Download raspberrypi toolchain from github: https://github.com/raspberrypi/tools
#
# RPI_TOOLCHAIN_HOME RaspberryPi toolchain directory

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(RASPBERRYPI TRUE)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CheckBuildOptions)

# Check raspberrypi toolchain directory valid or exist.
check_build_option_path(RPI_TOOLCHAIN_HOME)
if(${RPI_TOOLCHAIN_HOME} STREQUAL "NOTFOUND")
    message(FATAL_ERROR "Variable RPI_TOOLCHAIN_HOME not set or path is invalid")
endif()

# Update CMAKE_TOOLCHAIN_FILE to absolute path
get_filename_component(CMAKE_TOOLCHAIN_FILE
    "${CMAKE_TOOLCHAIN_FILE}"
    ABSOLUTE
    BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# The top direcotry of raspberrypi toolchain
set(RPI_TOOLCHAIN_TOPDIR
    "${RPI_TOOLCHAIN_HOME}/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64")

set(RPI_TOOLCHAIN_REFIX
    "${RPI_TOOLCHAIN_TOPDIR}/bin/arm-linux-gnueabihf-")

# The cross compiler
set(CMAKE_C_COMPILER "${RPI_TOOLCHAIN_REFIX}gcc")
set(CMAKE_CXX_COMPILER "${RPI_TOOLCHAIN_REFIX}g++")
set(CMAKE_CPP_COMPILER "${RPI_TOOLCHAIN_REFIX}cpp -E")
set(CMAKE_AR "${RPI_TOOLCHAIN_REFIX}ar" CACHE PATH "")
set(CMAKE_RANLIB "${RPI_TOOLCHAIN_REFIX}ranlib" CACHE PATH "")

# CMAKE_SYSROOT
set(CMAKE_SYSROOT "${RPI_TOOLCHAIN_TOPDIR}/arm-linux-gnueabihf/libc")

# CMAKE_<lang>_FLAGS
string(CONCAT _CMAKE_C_FLAGS
    "-Ofast "
    "-mthumb "
    "-marm "
    "-march=armv7-a "
    "-mfloat-abi=hard "
    "-mfpu=vfpv3-d16 "
    "-D__arm__ "
    "-Wall "
    "--sysroot=${CMAKE_SYSROOT} ")

set(CMAKE_C_FLAGS ${_CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
