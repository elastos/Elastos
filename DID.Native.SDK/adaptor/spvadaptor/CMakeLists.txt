project(spvadaptor VERSION 1.0.0 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 11)

include(ProjectDefaults)

set(SRC
    spvadaptor.cpp)

set(HEADERS
    spvadaptor.h)

set(SYSTEM_LIBS
    z)

set(LIBS
    curl
    spvsdk)

include_directories(
    BEFORE
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

set(SPVADAPTOR_DEPENDS libressl libcurl)

add_custom_target(spvadaptor)

if(ENABLE_STATIC)
    add_library(spvadaptor-static STATIC ${SRC})
    add_dependencies(spvadaptor-static ${SPVADAPTOR_DEPENDS})

    set_target_properties(spvadaptor-static PROPERTIES OUTPUT_NAME spvadaptor)
    if(WIN32)
        set_target_properties(spvadaptor-static PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
    endif()

    add_dependencies(spvadaptor spvadaptor-static)

    install(TARGETS spvadaptor-static
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

if(ENABLE_SHARED)
    add_library(spvadaptor-shared SHARED ${SRC})
    add_dependencies(spvadaptor-shared ${SPVADAPTOR_DEPENDS})

    set_target_properties(spvadaptor-shared PROPERTIES
        OUTPUT_NAME spvadaptor
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})
    if(WIN32)
        set_target_properties(spvadaptor-shared PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
    endif()

    target_link_libraries(spvadaptor-shared ${LIBS} ${SYSTEM_LIBS})

    add_dependencies(spvadaptor spvadaptor-shared)

    install(TARGETS spvadaptor-shared
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

install(FILES ${HEADERS} DESTINATION "include")