
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    if(WIN32)
        set(INSTALL_CMD
            cmd
            /c
            copy
            "${INT_DIST_DIR}\\lib\\flatccrt_d.lib"
            "${INT_DIST_DIR}\\lib\\flatccrt.lib"
            /y)
    else()
        set(INSTALL_CMD
            cp
            "libflatccrt_d.a"
            "libflatccrt.a")
    endif()

    execute_process(COMMAND ${INSTALL_CMD})
endif()
