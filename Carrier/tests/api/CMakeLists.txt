project(carrier-api-test C)

include(CarrierDefaults)

aux_source_directory(carrier CARRIER_TESTS)
aux_source_directory(session SESSION_TESTS)

set(SRC
    config.c
    launcher.c
    test_helper.c
    tests.c
    robot/cmd.c
    robot/robot.c)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread)
endif()

include_directories(
    .
    ../../src/carrier
    ../../src/session
    robot
    ${CARRIER_INT_DIST_DIR}/include)

link_directories(
    ${CARRIER_INT_DIST_DIR}/lib
    ${CMAKE_CURRENT_BINARY_DIR}/../../src/carrier
    ${CMAKE_CURRENT_BINARY_DIR}/../../src/session)

add_executable(elatests
    ${SRC}
    ${CARRIER_TESTS}
    ${SESSION_TESTS})

target_link_libraries(elatests
    elacarrier
    elasession
    crystal
    config
    cunit
    ncurses
    ${SYSTEM_LIBS})

install(TARGETS elatests
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")

install(FILES tests.conf
    DESTINATION "etc/carrier")

if(NOT WIN32)
    install(PROGRAMS elatests.sh
        DESTINATION "bin")
endif()
