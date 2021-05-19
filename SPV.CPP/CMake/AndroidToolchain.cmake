# Configurable variables.

# ANDROID_NDK_HOME Android NDK home directory
# ANDROID_ABI [armeabi-v7a | arm64-v8a | x86 | x86_64]

# Targeting system
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_C_COMPILER_ID Clang)
set(CMAKE_SYSTEM_VERSION 21)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CheckBuildOptions)

# Check android ABI variable exist.
check_build_option(ANDROID_ABI)
if(${ANDROID_ABI} STREQUAL "NOTFOUND")
    message(FATAL_ERROR "Variable ANDROID_ABI not set")
endif()

# Check android ndk toolchain variable
check_build_option_path(ANDROID_NDK_HOME)
if(${ANDROID_NDK_HOME} STREQUAL "NOTFOUND")
    message(FATAL_ERROR "Variable ANDROID_NDK_HOME not set or path is invalid")
endif()

# Update CMAKE_TOOLCHAIN_FILE to absolute path
get_filename_component(CMAKE_TOOLCHAIN_FILE
        "${CMAKE_TOOLCHAIN_FILE}"
        ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")

#[[
message("======Android Cross-compilation options=====")
message("ANDROID_ABI=${ANDROID_ABI}")
message("ANDROID_NDK_HOME=${ANDROID_NDK_HOME}")
message("CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
message("============================================")
]]

add_definitions(-D__ANDROID__)

# Android ABI
set(CMAKE_ANDROID_ARCH_ABI ${ANDROID_ABI})

# Android NDK location
set(CMAKE_ANDROID_NDK ${ANDROID_NDK_HOME})

# Compiler
set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang)

# Other options
set(CMAKE_ANDROID_STL_TYPE c++_static)

# Notice: cmake(<3.16.0) has isssue of cross-build for android target (NDK 21.0.6113669).
cmake_minimum_required(VERSION 3.16.0)
