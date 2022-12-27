if(__project_defaults_included)
    return()
endif()
set(__project_defaults_included TRUE)

# Global default variables defintions
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(NOT CMAKE_CROSSCOMPILING)
    string(TOUPPER ${CMAKE_SYSTEM_NAME} CANONICAL_SYSTEM_NAME)
    set(${CANONICAL_SYSTEM_NAME} TRUE)
endif()

if(WIN32)
    set(CMAKE_CPP_FLAGS
        "${CMAKE_CPP_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -fPIC -fvisibility=hidden")

    if(NOT (DARWIN OR IOS))
        set(CMAKE_SHARED_LINKER_FLAGS
            "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--exclude-libs,ALL")
    endif()
endif()

set(CMAKE_CXX_FLAGS
    "${CMAKE_C_FLAGS} -fexceptions")

if(CMAKE_CROSSCOMPILING)
    if("${CMAKE_INSTALL_PREFIX}" STREQUAL "")
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/outputs")
    endif()

    if(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local")
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/outputs")
    endif()
endif()

# Third-party dependency tarballs directory
set(PROJECT_DEPS_TARBALL_DIR "${CMAKE_SOURCE_DIR}/build/.tarballs")
set(PROJECT_DEPS_BUILD_PREFIX "external")

# Intermediate distribution directory
set(PROJECT_INT_DIST_DIR "${CMAKE_BINARY_DIR}/intermediates")
if(WIN32)
    file(TO_NATIVE_PATH
        "${PROJECT_INT_DIST_DIR}" PROJECT_INT_DIST_DIR)
endif()

# Rpath setup
set(CMAKE_MACOSX_RPATH TRUE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_BUILD_RPATH "${PROJECT_INT_DIST_DIR}/lib;${CMAKE_BINARY_DIR}/src")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# Host tools directory
set(PROJECT_HOST_TOOLS_DIR "${CMAKE_BINARY_DIR}/host")
if(WIN32)
    file(TO_NATIVE_PATH
         "${PROJECT_HOST_TOOLS_DIR}" PROJECT_HOST_TOOLS_DIR)
endif()

if(WIN32)
    set(PATCH_EXE "${PROJECT_HOST_TOOLS_DIR}/usr/bin/patch.exe")
else()
    set(PATCH_EXE "patch")
endif()

##Only suport for windows.
if(WIN32)
function(set_win_build_options build_options suffix)
    # check vs platform.
    # Notice: use CMAKE_SIZEOF_VOID_P to check whether target is
    # 64bit or 32bit instead of using CMAKE_SYSTEM_PROCESSOR.
    if(${CMAKE_SIZEOF_VOID_P} STREQUAL "8")
        set(_PLATFORM "x64")
    else()
        set(_PLATFORM "Win32")
    endif()

    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(_CONFIGURATION "Debug${suffix}")
    else()
        set(_CONFIGURATION "Release${suffix}")
    endif()

    string(CONCAT _BUILD_OPTIONS
        "/p:"
        "Configuration=${_CONFIGURATION},"
        "Platform=${_PLATFORM},"
        "InstallDir=${PROJECT_INT_DIST_DIR}")

    # update parent scope variable.
    set(${build_options} "${_BUILD_OPTIONS}" PARENT_SCOPE)
endfunction()
endif()
