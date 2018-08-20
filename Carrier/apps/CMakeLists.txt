include(AddSubModule)

set(DEPEND_MODULES
    libcrystal
    libconfig
    ela-carrier
    ela-session)

add_submodule(elashell
    DIRECTORY shell
    DEPENDS ${DEPEND_MODULES})

add_submodule(elapfd
    DIRECTORY pfd
    DEPENDS ${DEPEND_MODULES})

if (NOT WIN32)
    add_submodule(elaspeedtest
        DIRECTORY speedtest
        DEPENDS ${DEPEND_MODULES})
endif()
