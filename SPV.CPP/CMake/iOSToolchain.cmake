# Configurable variables.

# IOS_PLATFORM [iphoneos | iphonesimulator]
# IOS_ARCH optional(default arm64 for iphoneos, x86_64 for iphonesimulator)

# Targeting system
set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_C_COMPILER_ID Clang)
set(IOS_DEPLOYMENT_TARGET "10.0")
set(IOS TRUE)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CheckBuildOptions)

# Check iOS Platform target variable invalid.
check_build_option(IOS_PLATFORM)
if(${IOS_PLATFORM} STREQUAL "NOTFOUND")
    message(FATAL_ERROR "Variable IOS_PLATFORM not set")
endif()

if(${IOS_PLATFORM} STREQUAL "iphoneos")
    set(IOS_ARCH "arm64")
else()
    set(IOS_ARCH "x86_64")
endif()

# Update CMAKE_TOOLCHAIN_FILE to absolute path
get_filename_component(CMAKE_TOOLCHAIN_FILE
        "${CMAKE_TOOLCHAIN_FILE}"
        ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# CMAKE_SYSTEM_PROCESSOR
set(CMAKE_SYSTEM_PROCESSOR  ${IOS_ARCH})
# CMAKE_OSX_ARCHITECTURES
set(CMAKE_OSX_ARCHITECTURES ${IOS_ARCH})

# CMAKE_OSX_SYSROOT
execute_process(
    COMMAND xcodebuild -version -sdk ${IOS_PLATFORM} Path
    OUTPUT_VARIABLE IOS_SYSROOT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_OSX_SYSROOT ${IOS_SYSROOT})

# CMAKE_SYSTEM_VERSION
execute_process(
    COMMAND xcodebuild -sdk ${CMAKE_OSX_SYSROOT} -version SDKVersion
    OUTPUT_VARIABLE IOS_SDK_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_SYSTEM_VERSION ${IOS_SDK_VERSION})

# Find the C & C++ compilers for the specified SDK.
if (NOT CMAKE_C_COMPILER)
    execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find clang
        OUTPUT_VARIABLE CMAKE_C_COMPILER
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
if (NOT CMAKE_CXX_COMPILER)
    execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find clang++
        OUTPUT_VARIABLE CMAKE_CXX_COMPILER
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

# Find (Apple's) libtool.
execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find libtool
    OUTPUT_VARIABLE IOS_LIBTOOL
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Find (Apple's) ranlib.
execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find ranlib
    OUTPUT_VARIABLE IOS_RANLIB
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Find (Apple's) ar.
execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find ar
    OUTPUT_VARIABLE IOS_AR
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Configure libtool to be used instead of ar + ranlib to build static libraries.
# This is required on Xcode 7+, but should also work on previous versions of
# Xcode.
set(CMAKE_C_CREATE_STATIC_LIBRARY
    "${IOS_LIBTOOL} -static -o <TARGET> <LINK_FLAGS> <OBJECTS> ")
set(CMAKE_CXX_CREATE_STATIC_LIBRARY
    "${IOS_LIBTOOL} -static -o <TARGET> <LINK_FLAGS> <OBJECTS> ")

# Get the version of Darwin (OS X) of the host.
execute_process(COMMAND uname -r
    OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# CMAKE_<lang>_FLAGS
set(IOS_BITCODE_FLAG "-fembed-bitcode")
unset(IOS_BITCODE_FLAG)

if(${IOS_ARCH} STREQUAL "arm64")
    set(IOS_DEPLOYMENT_TARGET_FLAG "-miphoneos-version-min=")
    set(IOS_ARCH_FLAGS "-arch arm64 -D__arm64__ -DTARGET_PLATFORM_IOS")
elseif(${IOS_ARCH} STREQUAL "x86_64")
    set(IOS_DEPLOYMENT_TARGET_FLAG "-mios-simulator-version-min=")
    set(IOS_ARCH_FLAGS "-arch x86_64 -D__x86_64__ -DTARGET_PLATFORM_IOS")
endif()

string(CONCAT _CMAKE_C_FLAGS
    "${CAKE_C_FLAGS_INIT}"
    "${IOS_DEPLOYMENT_TARGET_FLAG}${IOS_DEPLOYMENT_TARGET} "
    "${IOS_ARCH_FLAGS} "
    "${IOS_BITCODE_FLAG} "
    "-fobjc-abi-version=2 "
    "-isysroot ${CMAKE_OSX_SYSROOT}")

set(CMAKE_C_FLAGS ${_CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS ${_CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -fomit-frame-pointer -ffast-math ${CMAKE_CXX_FLAGS_RELEASE}")
