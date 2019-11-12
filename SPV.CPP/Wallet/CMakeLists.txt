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

set(SRC ELAWallet.cpp)

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

set(LIBS spvsdk sqlite3 crypto ssl boost_system boost_filesystem boost_thread fruit)

add_executable(elawallet ${SRC})
add_dependencies(elawallet libspvsdk libfruit boost libressl json libsqlite)

target_link_libraries(elawallet ${LIBS} ${SYSTEM_LIBS})

install(TARGETS elawallet
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")
