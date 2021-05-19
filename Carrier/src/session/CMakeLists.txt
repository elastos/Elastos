project(ela-session C)
cmake_policy(SET CMP0054 NEW)

include(CarrierDefaults)
include(CheckIncludeFile)

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

check_include_file(netdb.h HAVE_NETDB_H)
if(HAVE_NETDB_H)
    add_definitions(-DHAVE_NETDB_H=1)
endif()

check_include_file(sys/time.h HAVE_SYS_TIME_H)
if(HAVE_SYS_TIME_H)
    add_definitions(-DHAVE_SYS_TIME_H=1)
endif()

check_include_file(sys/eventfd.h HAVE_SYS_EVENTFD_H)
if(HAVE_SYS_EVENTFD_H)
    add_definitions(-DHAVE_SYS_EVENTFD_H=1)
endif()

check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
if(HAVE_SYS_SOCKET_H)
    add_definitions(-DHAVE_SYS_SOCKET_H=1)
endif()

check_include_file(sys/select.h HAVE_SYS_SELECT_H)
if(HAVE_SYS_SELECT_H)
    add_definitions(-DHAVE_SYS_SELECT_H=1)
endif()

check_include_file(arpa/inet.h HAVE_ARPA_INET_H)
if(HAVE_ARPA_INET_H)
    add_definitions(-DHAVE_ARPA_INET_H=1)
endif()

check_include_file(winsock2.h HAVE_WINSOCK2_H)
if(HAVE_WINSOCK2_H)
    add_definitions(-DHAVE_WINSOCK2_H=1)
endif()

set(SRC
    session.c
    ice.c
    reliable_handler.c
    multiplex_handler.c
    udp_eventfd.c
    portforwarding.c
    crypto_handler.c
    fdset.c
    pseudotcp/pseudotcp.c
    pseudotcp/glist.c
    pseudotcp/gqueue.c)

set(HEADERS
    ela_session.h)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread m)
endif()

include_directories(
    .
    ../carrier
    pseudotcp
    ${CARRIER_INT_DIST_DIR}/include)

link_directories(
    ${CARRIER_INT_DIST_DIR}/lib
    ${CMAKE_CURRENT_BINARY_DIR}/../carrier)

if(WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_WARNINGS)

    # Force source code encoding to utf-8
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
    set(LIBS
        crystal
        pjmedia
        pjnath
        pjlib-util
        pjlib
        pthread
        libsrtp.lib)
else()
    add_definitions(-DPJ_AUTOCONF)
    set(LIBS
        crystal
        pjmedia
        pjnath
        pjlib-util
        pj
        srtp)
endif()

add_definitions(-DCARRIER_BUILD)

add_custom_target(ela-session)

if(ENABLE_STATIC)
    add_library(elasession-static STATIC ${SRC})
    target_compile_definitions(elasession-static PRIVATE CARRIER_STATIC CRYSTAL_STATIC)
    set_target_properties(elasession-static PROPERTIES OUTPUT_NAME elasession)

    add_dependencies(ela-session elasession-static)

    install(TARGETS elasession-static
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

if(ENABLE_SHARED)
    add_library(elasession-shared SHARED ${SRC})
    target_compile_definitions(elasession-shared PRIVATE CARRIER_DYNAMIC CRYSTAL_DYNAMIC)
    set_target_properties(elasession-shared PROPERTIES OUTPUT_NAME elasession)
    target_link_libraries(elasession-shared elacarrier-shared ${LIBS} ${SYSTEM_LIBS})

    if(IOS)
        string(CONCAT LINK_FLAGS
            "-framework AudioToolbox "
            "-framework AVFoundation "
            "-framework CFNetwork "
            "-framework Foundation")

        set_target_properties(elasession-shared PROPERTIES LINK_FLAGS ${LINK_FLAGS})
    endif()

    add_dependencies(ela-session elasession-shared)

    install(TARGETS elasession-shared
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

add_subdirectory(pseudotcp)

install(FILES ${HEADERS} DESTINATION "include")
