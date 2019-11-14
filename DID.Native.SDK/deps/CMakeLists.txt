include(AddSubModule)

add_submodule(platform-specific)
add_submodule(cJSON
    DEPENDS platform-specific)
add_submodule(libressl
    DEPENDS platform-specific)
add_submodule(libcrystal
    DEPENDS platform-specific)
add_submodule(CUnit
    DEPENDS platform-specific)

if (ENABLE_SPVADAPTER)
    add_submodule(libspvsdk
        DEPENDS platform-specific)
    # libcurl will use openssl built by libspvsdk
    add_submodule(libcurl
        DEPENDS libspvsdk platform-specific)
endif()

if(WIN32)
    add_submodule(pthreads-w32
        DEPENDS platform-specific)
    add_dependencies(libcrystal pthreads-w32)
endif()
