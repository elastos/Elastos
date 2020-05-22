include(AddSubModule)

add_submodule(platform-specific)

## Dependencis of src: carrier and session.
add_submodule(flatcc
    DEPENDS platform-specific)
add_submodule(libsodium
    DEPENDS platform-specific)
add_submodule(libconfig
    DEPENDS platform-specific)
add_submodule(pjsip
    DEPENDS platform-specific)
add_submodule(CUnit
    DEPENDS platform-specific)
add_submodule(zlib
    DEPENDS platform-specific)
add_submodule(libressl
    DEPENDS platform-specific)
add_submodule(libcurl
    DEPENDS platform-specific libressl zlib)
add_submodule(cJSON
    DEPENDS platform-specific)

if(WIN32)
    add_submodule(PDCurses
        DEPENDS platform-specific)
#    add_submodule(slim-pthread
    add_submodule(pthreads-w32
        DEPENDS platform-specific)
endif()

add_submodule(libcrystal
    DEPENDS platform-specific libsodium)
add_submodule(toxcore0
    DIRECTORY toxcore
    DEPENDS platform-specific libsodium libconfig)
if(WIN32)
#    add_dependencies(libcrystal slim-pthread)
    add_dependencies(libcrystal pthreads-w32)
#    add_dependencies(toxcore0 slim-pthread)
    add_dependencies(toxcore0 pthreads-w32)
endif()
