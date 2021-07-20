include(AddSubModule)

add_submodule(platform-specific)
add_submodule(boost DEPENDS platform-specific)
add_submodule(libressl DEPENDS platform-specific)
add_submodule(json DEPENDS platform-specific)
add_submodule(catch2 DEPENDS platform-specific)
add_submodule(libsqlite DEPENDS platform-specific)
add_submodule(libspdlog DEPENDS platform-specific)
add_submodule(secp256k1 DEPENDS platform-specific)

if(SPV_BUILD_APPS)
	add_submodule(zlib DEPENDS platform-specific)
	add_submodule(libcurl DEPENDS libressl zlib platform-specific)
endif()

if(WIN32)
	add_submodule(pthreads-w32 DEPENDS platform-specific)
endif()