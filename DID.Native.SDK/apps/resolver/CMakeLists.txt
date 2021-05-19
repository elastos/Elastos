project(resolver C)

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

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

set(SRC
    main.c)

include_directories(
    ../../src
    ${PROJECT_INT_DIST_DIR}/include)

link_directories(
    ${PROJECT_INT_DIST_DIR}/lib
    ${CMAKE_CURRENT_BINARY_DIR}/../../src)

set(LIBS
    eladid)

add_executable(resolver ${SRC})
target_link_libraries(resolver ${LIBS})

install(TARGETS resolver
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")
