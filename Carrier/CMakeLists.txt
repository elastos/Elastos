cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5)
project(elastos-carrier)

# set(CMAKE_VERBOSE_MAKEFILE TRUE)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(CarrierDefaults)
include(CarrierPackages)
include(AddSubModule)

if(IOS)
    set(ENABLE_SHARED_DEFAULT FALSE)
    set(ENABLE_STATIC_DEFAULT TRUE)
else()
    set(ENABLE_SHARED_DEFAULT TRUE)
    set(ENABLE_STATIC_DEFAULT TRUE)
endif()

if(CMAKE_CROSSCOMPILING)
    set(ENABLE_APPS_DEFAULT FALSE)
    set(ENABLE_TESTS_DEFAULT FALSE)
else()
    set(ENABLE_APPS_DEFAULT TRUE)
    set(ENABLE_TESTS_DEFAULT TRUE)
endif()

set(ENABLE_SHARED ${ENABLE_SHARED_DEFAULT} CACHE BOOL "Build shared library")
set(ENABLE_STATIC ${ENABLE_STATIC_DEFAULT} CACHE BOOL "Build static library")
set(ENABLE_APPS ${ENABLE_APPS_DEFAULT} CACHE BOOL "Build demo applications")
set(ENABLE_TESTS ${ENABLE_TESTS_DEFAULT} CACHE BOOL "Build test cases")
set(ENABLE_DOCS FALSE CACHE BOOL "Build APIs documentation")

add_subdirectory(deps)
add_subdirectory(src)

if(ENABLE_APPS OR ENABLE_TESTS)
    add_submodule(checkconfig
        DIRECTORY config
        DEPENDS libcrystal libconfig ela-carrier ela-session)
endif()

if(ENABLE_APPS)
    add_subdirectory(apps)
endif()

if(ENABLE_TESTS)
    add_subdirectory(tests/apitests)
    add_subdirectory(tests/offmsg)
endif()

if(ENABLE_DOCS AND (${CMAKE_SYSTEM_NAME} STREQUAL "Linux"))
    add_subdirectory(docs)
endif()

# Alias target for package. But it fix issue about making package
# inrelevant to udpatation of cmake scripts.
add_custom_target(dist
    COMMAND ${CMAKE_MAKE_PROGRAM} package
    COMMENT "Package Carrier Distrubtion")
