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

check_include_file(glob.h HAVE_GLOB_H)
if(HAVE_GLOB_H)
    add_definitions(-DHAVE_GLOB_H=1)
endif()

check_include_file(dirent.h HAVE_DIRENT_H)
if(HAVE_DIRENT_H)
    add_definitions(-DHAVE_DIRENT_H=1)
endif()

check_include_file(sys/time.h HAVE_SYS_TIME_H)
if(HAVE_SYS_TIME_H)
    add_definitions(-DHAVE_SYS_TIME_H=1)
endif()

file(GLOB utility-SOURCES "utility/*.c")
file(GLOB backend-SOURCES "backend/*.c")
file(GLOB jwt-SOURCES "jwt/*.c")

set(SRC
    diderror.c
    did.c
    diddocument.c
    credential.c
    didstore.c
    didbackend.c
    mnemonic.c
    presentation.c
    issuer.c
    credmeta.c
    didmeta.c
    ${utility-SOURCES}
    ${backend-SOURCES}
    ${jwt-SOURCES})

set(HEADERS
    ela_did.h
    jwt/ela_jwt.h)

if(ENABLE_TESTS)
    set(HEADERS
        ${HEADERS}
        adapter/dummyadapter.h)

    set(SRC
        ${SRC}
        adapter/dummyadapter.c)

    set(DUMMYADAPTER_CREATE
        "_DummyAdapter_Create")
    set(DUMMYADAPTER_DESTROY
        "_DummyAdapter_Destroy")
endif()

configure_file(
    export.symbols.in
    export.symbols
    "@ONLY")

if(WIN32)
    set(SYSTEM_LIBS Ws2_32 Iphlpapi)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread m)
endif()

include_directories(
    BEFORE
    utility
    backend
    adapter
    jwt
    .
    ../hdkey
    ${PROJECT_INT_DIST_DIR}/include)

link_directories(
    ../hdkey
    ${PROJECT_INT_DIST_DIR}/lib)

#add_definitions(
#    -DHAVE___INT128=1
#    -DUSE_NUM_NONE=1
#    -DUSE_FIELD_5X52=1
#    -DUSE_FIELD_INV_BUILTIN=1
#    -DUSE_SCALAR_4X64=1
#    -DUSE_SCALAR_INV_BUILTIN=1)

if(WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_WARNINGS)

    # Force source code encoding to utf-8
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()

set(LIBS
    cjson
    curl
    ssl
    crypto
    z
    jansson
    cjose)

if(WIN32)
    set(LIBS
        ${LIBS}
        pthread)
endif()

add_definitions(-DDID_BUILD)

add_custom_target(ela-did)

set(ELADID_DEPENDS libressl libcurl cJSON hdkey libjansson libcjose)

if(WIN32)
    set(ELADID_DEPENDS
        #libcrystal
        ${ELADID_DEPENDS})
endif()

if(ENABLE_STATIC)
    add_library(eladid-static STATIC ${SRC})
    add_dependencies(eladid-static ${ELADID_DEPENDS})

    #target_compile_definitions(eladid-static PRIVATE DID_STATIC CRYSTAL_STATIC)
    target_compile_definitions(eladid-static PRIVATE DID_STATIC)
    set_target_properties(eladid-static PROPERTIES OUTPUT_NAME eladid)
    if(WIN32)
        set_target_properties(eladid-static PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
    endif()

    add_dependencies(ela-did eladid-static)

    install(TARGETS eladid-static
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

if(ENABLE_SHARED)
    add_library(eladid-shared SHARED ${SRC})
    add_dependencies(eladid-shared ${ELADID_DEPENDS})

    #target_compile_definitions(eladid-shared PRIVATE DID_DYNAMIC CRYSTAL_DYNAMIC)
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
            OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/export.symbols")
        set_property(TARGET eladid-shared APPEND_STRING PROPERTY
            LINK_FLAGS " -Wl,-exported_symbols_list,\"${CMAKE_CURRENT_BINARY_DIR}/export.symbols\"")
    endif()
    target_include_directories(eladid-shared
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(eladid-shared PRIVATE hdkey-static ${LIBS} ${SYSTEM_LIBS})

    add_dependencies(ela-did eladid-shared)

    install(TARGETS eladid-shared
        EXPORT ElastosDIDConfig
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")

    install(EXPORT
            ElastosDIDConfig
        NAMESPACE
            "ElastosDID::"
        DESTINATION
            share/ElastosDID/cmake
    )

    export(
        TARGETS
            eladid-shared
        FILE
            ElastosDIDConfig.cmake
    )
endif()

install(FILES ${HEADERS} DESTINATION "include")


