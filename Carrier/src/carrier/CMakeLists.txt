project(elastos-carrier C)
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

configure_file(
    version.h.in
    version.h
    "@ONLY")

set(SRC
    dht/dht.c
    elacp.c
    ela_carrier.c
    ela_error.c)

set(HEADERS
    ela_carrier.h)

add_custom_command(
    OUTPUT elacp_generated.h
    COMMAND ${CARRIER_HOST_TOOLS_DIR}/bin/flatcc${CMAKE_EXECUTABLE_SUFFIX}
        --outfile elacp_generated.h
        -a "${CMAKE_CURRENT_LIST_DIR}/elacp.fbs"
    DEPENDS "${CMAKE_CURRENT_LIST_DIR}/elacp.fbs" flatcc-parser)

add_custom_target(elacp_generated_h
    DEPENDS elacp_generated.h)

if(WIN32)
    set(SYSTEM_LIBS Ws2_32 Iphlpapi)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread m)
endif()

include_directories(
    .
    dht
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CARRIER_INT_DIST_DIR}/include)

link_directories(
    ${CARRIER_INT_DIST_DIR}/lib)

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
    toxcore
    flatccrt)

if(WIN32)
    set(LIBS
        ${LIBS}
        pthread
        libsodium.lib)
else()
    set(LIBS
        ${LIBS}
        sodium)
endif()

add_definitions(-DCARRIER_BUILD)

add_custom_target(ela-carrier)

if(ENABLE_STATIC)
    add_library(elacarrier-static STATIC ${SRC})

    target_compile_definitions(elacarrier-static PRIVATE CARRIER_STATIC CRYSTAL_STATIC)
    set_target_properties(elacarrier-static PROPERTIES OUTPUT_NAME elacarrier)
    add_dependencies(elacarrier-static elacp_generated_h)

    add_dependencies(ela-carrier elacarrier-static)

    install(TARGETS elacarrier-static
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

if(ENABLE_SHARED)
    add_library(elacarrier-shared SHARED ${SRC})

    target_compile_definitions(elacarrier-shared PRIVATE CARRIER_DYNAMIC CRYSTAL_DYNAMIC)
    set_target_properties(elacarrier-shared PROPERTIES OUTPUT_NAME elacarrier)
    target_link_libraries(elacarrier-shared ${LIBS} ${SYSTEM_LIBS})
    add_dependencies(elacarrier-shared elacp_generated_h)

    add_dependencies(ela-carrier elacarrier-shared)

    install(TARGETS elacarrier-shared
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

install(FILES ${HEADERS} DESTINATION "include")
