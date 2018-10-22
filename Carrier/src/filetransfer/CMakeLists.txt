project(elastos-filetransfer C)
cmake_policy(SET CMP0054 NEW)

include(CarrierDefaults)
include(CheckIncludeFile)

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H=1)
endif()

check_include_file(alloca.h HAVE_ALLOCA_H)
if(HAVE_ALLOCA_H)
    add_definitions(-DHAVE_ALLOCA_H=1)
endif()

check_include_file(malloc.h HAVE_MALLOC_H)
if(HAVE_MALLOC_H)
    add_definitions(-DHAVE_MALLOC_H=1)
endif()

check_include_file(io.h HAVE_IO_H)
if(HAVE_IO_H)
    add_definitions(-DHAVE_IO_H=1)
endif()

check_include_file(direct.h HAVE_DIRECT_H)
if(HAVE_DIRECT_H)
    add_definitions(-DHAVE_DIRECT_H=1)
endif()

check_include_file(arpa/inet.h HAVE_ARPA_INET_H)
if(HAVE_ARPA_INET_H)
    add_definitions(-DHAVE_ARPA_INET_H=1)
endif()

check_include_file(winsock2.h HAVE_WINSOCK2_H)
if(HAVE_WINSOCK2_H)
    add_definitions(-DHAVE_WINSOCK2_H=1)
endif()

check_include_file(ws2tcpip.h HAVE_WS2TCPIP_H)
if(HAVE_WS2TCPIP_H)
    add_definitions(-DHAVE_WS2TCPIP_H=1)
endif()

check_include_file(sys/time.h HAVE_SYS_TIME_H)
if(HAVE_SYS_TIME_H)
    add_definitions(-DHAVE_SYS_TIME_H=1)
endif()

set(SRC
    filetransfer.c)

set(HEADERS
    ela_filetransfer.h)

if(WIN32)
    set(SYSTEM_LIBS Ws2_32 Iphlpapi)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread m)
endif()

include_directories(
    .
    ../carrier
    ../session
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CARRIER_INT_DIST_DIR}/include)

link_directories(
    ${CARRIER_INT_DIST_DIR}/lib
    ${CMAKE_CURRENT_BINARY_DIR}/../carrier
    ${CMAKE_CURRENT_BINARY_DIR}/../session)

if(WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_WARNINGS)

    # Force source code encoding to utf-8
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()

set(LIBS
    crystal
    elacarrier
    elasession)

if(WIN32)
    set(LIBS
        ${LIBS}
        pthread)
endif()

add_definitions(-DCARRIER_BUILD)

add_custom_target(ela-filetransfer)

if(ENABLE_STATIC)
    add_library(elafiletransfer-static STATIC ${SRC})

    target_compile_definitions(elafiletransfer-static PRIVATE CARRIER_STATIC CRYSTAL_STATIC)
    set_target_properties(elafiletransfer-static PROPERTIES OUTPUT_NAME elafiletransfer)
    add_dependencies(ela-filetransfer elafiletransfer-static)

    install(TARGETS elafiletransfer-static
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

if(ENABLE_SHARED)
    add_library(elafiletransfer-shared SHARED ${SRC})

    target_compile_definitions(elafiletransfer-shared PRIVATE CARRIER_DYNAMIC CRYSTAL_DYNAMIC)
    set_target_properties(elafiletransfer-shared PROPERTIES OUTPUT_NAME elafiletransfer)
    target_link_libraries(elafiletransfer-shared ${LIBS} ${SYSTEM_LIBS})
    add_dependencies(ela-filetransfer elafiletransfer-shared)

    install(TARGETS elafiletransfer-shared
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

install(FILES ${HEADERS} DESTINATION "include")
