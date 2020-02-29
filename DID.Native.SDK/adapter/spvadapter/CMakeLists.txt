project(spvadapter VERSION 1.0.0 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 11)

include(ProjectDefaults)

set(ENABLE_SHARED TRUE)

set(SRC
    spvadapter.cpp)

set(HEADERS
    spvadapter.h)

set(SYSTEM_LIBS
    z)

if(LINUX)
set(SYSTEM_LIBS
    ${SYSTEM_LIBS}
    dl)
endif()

set(LIBS
    spvsdk
    sqlite3
    fruit
    spdlog
    boost_system
    boost_thread
    boost_filesystem
    ssl
    crypto)

include_directories(
    BEFORE
    ${PROJECT_INT_DIST_DIR}/include)

add_definitions(-DDID_BUILD)

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

set(SPVADAPTER_DEPENDS libspvsdk)

add_custom_target(spvadapter)

if(ENABLE_STATIC)
    add_library(spvadapter-static STATIC ${SRC})
    add_dependencies(spvadapter-static ${SPVADAPTER_DEPENDS})

    set_target_properties(spvadapter-static PROPERTIES OUTPUT_NAME spvadapter)
    if(WIN32)
        set_target_properties(spvadapter-static PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
    endif()

    add_dependencies(spvadapter spvadapter-static)

    install(TARGETS spvadapter-static
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

if(ENABLE_SHARED)
    add_library(spvadapter-shared SHARED ${SRC})
    add_dependencies(spvadapter-shared ${SPVADAPTER_DEPENDS})
    target_compile_definitions(spvadapter-shared PRIVATE DID_DYNAMIC)

    set_target_properties(spvadapter-shared PROPERTIES
        OUTPUT_NAME spvadapter
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})
    if(WIN32)
        set_target_properties(spvadapter-shared PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
    endif()

    target_link_libraries(spvadapter-shared ${LIBS} ${SYSTEM_LIBS})

    add_dependencies(spvadapter spvadapter-shared)

    install(TARGETS spvadapter-shared
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
endif()

install(FILES ${HEADERS} DESTINATION "include")