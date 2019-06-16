project(eladid C)

include(ProjectDefaults)
include(CheckIncludeFile)

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H=1)
endif()

check_include_file(stdlib.h HAVE_STDLIB_H)
if(HAVE_STDLIB_H)
    add_definitions(-DHAVE_STDLIB_H=1)
endif()

check_include_file(malloc.h HAVE_MALLOC_H)
if(HAVE_MALLOC_H)
    add_definitions(-DHAVE_MALLOC_H=1)
endif()

check_include_file(io.h HAVE_IO_H)
if(HAVE_IO_H)
    add_definitions(-DHAVE_IO_H=1)
endif()

check_include_file(sys/time.h HAVE_SYS_TIME_H)
if(HAVE_SYS_TIME_H)
    add_definitions(-DHAVE_SYS_TIME_H=1)
endif()

set(SRC
    did.c)

set(HEADERS
    ela_did.h)

if(WIN32)
    set(SYSTEM_LIBS Ws2_32 Iphlpapi)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread m)
endif()

include_directories(
    BEFORE
    .
    ${PROJECT_INT_DIST_DIR}/include)

link_directories(
    ${PROJECT_INT_DIST_DIR}/lib)

if(WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_WARNINGS)

    # Force source code encoding to utf-8
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()

set(LIBS
    crypto
    cjson)

if(WIN32)
set(LIBS
    ${LIBS}
    pthread)
else()
set(LIBS
    ${LIBS})
endif()

add_definitions(-DDID_BUILD)

add_custom_target(eladid)

set(ELADID_DEPENDS libressl cJSON)

if(ENABLE_STATIC)
    add_library(eladid-static STATIC ${SRC})
    add_dependencies(eladid-static ${ELADID_DEPENDS})

    target_compile_definitions(eladid-static PRIVATE DID_STATIC)
    set_target_properties(eladid-static PROPERTIES OUTPUT_NAME eladid)
    if(WIN32)
        set_target_properties(eladid-static PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
    endif()

    add_dependencies(eladid eladid-static)

    install(TARGETS eladid-static
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

if(ENABLE_SHARED)
    add_library(eladid-shared SHARED ${SRC})
    add_dependencies(eladid-shared ${ELADID_DEPENDS})

    target_compile_definitions(eladid-shared PRIVATE DID_DYNAMIC)
    set_target_properties(eladid-shared PROPERTIES
        OUTPUT_NAME eladid
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})
    if(WIN32)
        set_target_properties(eladid-shared PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
    endif()
    if(DARWIN OR IOS)
        set_property(SOURCE ${SRC} APPEND PROPERTY
            OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/export.symbols")
        set_property(TARGET eladid-shared APPEND_STRING PROPERTY
            LINK_FLAGS " -Wl,-exported_symbols_list,\"${CMAKE_CURRENT_SOURCE_DIR}/export.symbols\"")
    endif()
    target_link_libraries(eladid-shared ${LIBS} ${SYSTEM_LIBS})

    add_dependencies(eladid eladid-shared)

    install(TARGETS eladid-shared
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

install(FILES ${HEADERS} DESTINATION "include")