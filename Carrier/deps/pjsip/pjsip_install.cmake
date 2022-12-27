if(WIN32)
    set(PJ_MODULES
        "pjlib"
        "pjlib-util"
        "pjnath"
        "pjmedia"
        "pjsip")

    foreach(_MODULE ${PJ_MODULES})
        execute_process(COMMAND
            xcopy
            "${_MODULE}\\include"
            "${INT_DIST_DIR}\\include"
            /s
            /e
            /y)
    endforeach()

    list(APPEND PJ_MODULES "third_party")

    set(pjlib-MODULES "pjlib")
    set(pjlib-util-MODULES "pjlib-util")
    set(pjmedia-MODULES
        "pjmedia-audiodev"
        "pjmedia-videodev"
        "pjmedia-codec"
        "pjmedia")
    set(pjnath-MODULES "pjnath")
    set(pjsip-MODULES
        "pjsip-core"
        "pjsip-simple"
        "pjsip-ua"
        "pjsua-lib"
        "pjsua2-lib")
    set(third_party-MODULES "libsrtp")

    foreach(_MODULE  ${PJ_MODULES})
        foreach(_SUB_MODULE ${${_MODULE}-MODULES})
            execute_process(
                COMMAND cmd
                    /c
                    copy
                    "${_MODULE}\\lib\\${_SUB_MODULE}-${PJLIB_SUFFIX}.lib"
                    "${INT_DIST_DIR}\\lib"
                    /y
                COMMAND cmd
                    /c
                    copy
                    "${_MODULE}\\lib\\${_SUB_MODULE}-${PJLIB_SUFFIX}.lib"
                    "${INT_DIST_DIR}\\lib\\${_SUB_MODULE}.lib"
                    /y
                )
        endforeach()
    endforeach()
else()
    set(PJ_LIBS
        "pj"
        "pjlib-util"
        "pjmedia"
        "pjmedia-audiodev"
        "pjmedia-codec"
        "pjmedia-videodev"
        "pjnath"
        "pjsip"
        "pjsip-ua"
        "pjsip-simple"
        "pjsua"
        "pjsua2"
        "srtp")

    foreach(_LIB ${PJ_LIBS})
        execute_process(COMMAND cp
            lib${_LIB}-${PJLIB_SUFFIX}.a
            lib${_LIB}.a)
    endforeach()
endif()
