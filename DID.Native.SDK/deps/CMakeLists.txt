include(AddSubModule)

add_submodule(platform-specific)
add_submodule(cJSON
    DEPENDS platform-specific)
add_submodule(libressl
    DEPENDS platform-specific)

if(WIN32)
    add_submodule(pthreads-w32
        DEPENDS platform-specific)
endif()
