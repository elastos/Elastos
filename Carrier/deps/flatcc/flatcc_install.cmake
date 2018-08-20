if(WIN32)
    #TODO
else()
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        execute_process(
            COMMAND ln -sf libflatccrt_d.a libflatccrt.a)
    endif()
endif()
