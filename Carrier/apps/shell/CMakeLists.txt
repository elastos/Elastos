project(elashell C)

include(CarrierDefaults)
include(CheckIncludeFile)
include(CheckFunctionExists)

check_include_file(alloca.h HAVE_ALLOCA_H)
if(HAVE_ALLOCA_H)
    add_definitions(-DHAVE_ALLOCA_H=1)
endif()

check_include_file(malloc.h HAVE_MALLOC_H)
if(HAVE_MALLOC_H)
    add_definitions(-DHAVE_MALLOC_H=1)
endif()

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H=1)
endif()

check_include_file(process.h HAVE_PROCESS_H)
if(HAVE_PROCESS_H)
    add_definitions(-DHAVE_PROCESS_H=1)
endif()

check_include_file(sys/time.h HAVE_SYS_TIME_H)
if(HAVE_SYS_TIME_H)
    add_definitions(-DHAVE_SYS_TIME_H=1)
endif()

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

check_include_file(sys/select.h HAVE_SYS_SELECT_H)
if(HAVE_SYS_SELECT_H)
    add_definitions(-DHAVE_SYS_SELECT_H)
endif()

check_include_file(winsock.h HAVE_WINSOCK_H)
if(HAVE_WINSOCK_H)
    add_definitions(-DHAVE_WINSOCK_H)
endif()

check_function_exists(sigaction HAVE_SIGACTION)
if(HAVE_SIGACTION)
    add_definitions(-DHAVE_SIGACTION=1)
endif()

set(SRC shell.c config.c)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread)
endif()

include_directories(
    ../../src/carrier
    ../../src/session
    ${CARRIER_INT_DIST_DIR}/include)

link_directories(
    ${CARRIER_INT_DIST_DIR}/lib
    ${CMAKE_CURRENT_BINARY_DIR}/../../src/carrier
    ${CMAKE_CURRENT_BINARY_DIR}/../../src/session)

if(ENABLE_SHARED)
    add_definitions(-DCRYSTAL_DYNAMIC)
else()
    add_definitions(-DCRYSTAL_STATIC)
endif()

if(WIN32)
  add_definitions(
    -DWIN32_LEAN_AND_MEAN
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_NONSTDC_NO_WARNINGS)

    # Force source code encoding to utf-8
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
    set(LIBS
        elacarrier
        elasession
        crystal
        pdcurses
        pthread
	    config)
else()
    set(LIBS
        elacarrier
        elasession
        crystal
        ncurses
	    config)
endif()

add_executable(elashell ${SRC})
target_link_libraries(elashell ${LIBS} ${SYSTEM_LIBS})

install(TARGETS elashell
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")

install(FILES elashell.conf
        DESTINATION "etc/carrier"
        RENAME "elashell.conf")

if(NOT WIN32)
    install(PROGRAMS elashell.sh DESTINATION "bin")
endif()
