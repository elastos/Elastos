project(Elastos-wallet)

set(CMAKE_CXX_STANDARD 11)

include(ProjectDefaults)
include(CheckIncludeFile)
include(CheckFunctionExists)

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H=1)
endif()

check_include_file(getopt.h HAVE_GETOPT_H)
if(HAVE_GETOPT_H)
    add_definitions(-DHAVE_GETOPT_H=1)
endif()

check_include_file(sys/time.h HAVE_SYS_TIME_H)
if(HAVE_SYS_TIME_H)
    add_definitions(-DHAVE_SYS_TIME_H=1)
endif()

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

file(GLOB_RECURSE WALLET_SOURCE_FILES "*.cpp")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread dl)
endif()

include_directories(
    ../Interface
    ${PROJECT_INT_DIST_DIR}/include
)

link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/../SDK
    ${PROJECT_INT_DIST_DIR}/lib
)

if (SPV_ENABLE_SHARED)
    set(LIBS spvsdk resolv)
	set(ELAWALLET_DEPENDS libspvsdk)
elseif(SPV_ENABLE_STATIC)
    set(LIBS spvsdk-static sqlite3 crypto ssl boost_system boost_filesystem boost_thread fruit resolv)
    set(ELAWALLET_DEPENDS libspvsdk libfruit boost libressl json libsqlite)
else()
	message(FATAL_ERROR "libspvsdk should enable static or shared")
endif()

add_executable(wallet ${WALLET_SOURCE_FILES})
add_dependencies(wallet ${ELAWALLET_DEPENDS})

target_link_libraries(wallet ${LIBS} ${SYSTEM_LIBS})

install(TARGETS wallet
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")
