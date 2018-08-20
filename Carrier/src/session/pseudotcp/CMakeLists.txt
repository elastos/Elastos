project(ela-pseudotcp-tests C)

include(CarrierDefaults)

set(SRC
    pseudotcp.c
    glist.c
    gqueue.c)

include_directories(
    ${CARRIER_INT_DIST_DIR}/include)

if(WIN32)
    set(SYSTEM_LIBS Ws2_32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4244 /wd4267")
endif()

if(ENABLE_SHARED)
    add_definitions(-DCRYSTAL_DYNAMIC)
endif()

link_directories(${CARRIER_INT_DIST_DIR}/lib)

add_executable(test-pseudotcp-fin ${SRC} test-pseudotcp-fin.c)
add_executable(test-pseudotcp ${SRC} test-pseudotcp.c)
add_dependencies(test-pseudotcp-fin libcrystal)
add_dependencies(test-pseudotcp libcrystal)

target_link_libraries(test-pseudotcp-fin crystal ${SYSTEM_LIBS})
target_link_libraries(test-pseudotcp crystal ${SYSTEM_LIBS})

install(TARGETS test-pseudotcp-fin test-pseudotcp
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")
