include(ProjectDefaults)
#include(CheckIncludeFile)

file( GLOB BR-SOURCES "BR/*.c" )

set(SRC
    HDkey.c
    crypto.c
    ${BR-SOURCES})

set(HEADERS
    crypto.h
    HDkey.h)

set(LIBS
    crypto)

include_directories(
    BEFORE
    .
    BR
    secp256k1
    ${PROJECT_INT_DIST_DIR}/include)

if(WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_WARNINGS)

    # Force source code encoding to utf-8
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()

link_directories(
    ${PROJECT_INT_DIST_DIR}/lib)

set(HDKEY_DEPENDS libressl)

add_custom_target(hdkey)

if(ENABLE_STATIC)
    add_library(hdkey-static STATIC ${SRC})
    add_dependencies(hdkey-static ${HDKEY_DEPENDS})

    set_target_properties(hdkey-static PROPERTIES OUTPUT_NAME hdkey)
    if(WIN32)
        set_target_properties(hdkey-static PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
    endif()

    add_dependencies(hdkey hdkey-static)

    install(TARGETS hdkey-static
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

if(ENABLE_SHARED)
    add_library(hdkey-shared SHARED ${SRC})
    add_dependencies(hdkey-shared ${HDKEY_DEPENDS})

    set_target_properties(hdkey-shared PROPERTIES
        OUTPUT_NAME hdkey
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})
    if(WIN32)
        set_target_properties(hdkey-shared PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
    endif()

    target_link_libraries(hdkey-shared ${LIBS} ${SYSTEM_LIBS})

    add_dependencies(hdkey hdkey-shared)

    install(TARGETS hdkey-shared
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

install(FILES ${HEADERS} DESTINATION "include")