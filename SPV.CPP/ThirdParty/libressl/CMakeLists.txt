project(libressl)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
	libressl

	PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
	URL "https://github.com/elastos/libressl-portable/archive/v2.9.2.tar.gz"
	URL_HASH SHA256=27888e600cf61395d4129692afe2403854a2324d3c26c609d93490fde40ea253
	DOWNLOAD_NAME "libressl-2.9.2.tar.gz"
	DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
	DOWNLOAD_NO_PROGRESS 1

	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
		${CMAKE_ARGS_INIT}
		-DLIBRESSL_SKIP_INSTALL=OFF
		-DLIBRESSL_APPS=OFF
		-DLIBRESSL_TESTS=OFF
		-DBUILD_SHARED_LIBS=OFF
		-DENABLE_ASM=OFF
		-DENABLE_EXTRATESTS=OFF
		-DENABLE_NC=OFF
		-DCMAKE_BUILD_TYPE=Release
		-DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
)

if(SPV_ENABLE_STATIC)
	export_static_library(ssl)
	export_static_library(crypto)
endif()
