project(libsqlite)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSQLITE_TEMP_STORE=3")

include(ProjectDefaults)
include(ExternalProject)
include(ExternalConfigureArgs)

if (WIN32)
	set(CONFIGURE_CMD "echo")
	set(BUILD_CMD nmake /f Makefile.msc)
	set(INSTALL_CMD
			copy ".\\sqlite3.h"    "${PROJECT_INT_DIST_DIR}\\include" /y &&
			copy ".\\sqlite3ext.h" "${PROJECT_INT_DIST_DIR}\\include" /y &&
			copy ".\\sqlite3.lib"  "${PROJECT_INT_DIST_DIR}\\lib" /y &&
			copy ".\\sqlite3.dll"  "${PROJECT_INT_DIST_DIR}\\lib" /y &&
			copy ".\\sqlite3.def"  "${PROJECT_INT_DIST_DIR}\\lib" /y)
else()
	set(CONFIGURE_CMD
			./configure
			--disable-shared
			--enable-tempstore=always
			--prefix=${PROJECT_INT_DIST_DIR}
			${CONFIGURE_ARGS_INIT})
	set(BUILD_CMD make)
	set(INSTALL_CMD make install)
endif()

ExternalProject_Add(
	libsqlite

	PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
	URL "https://www.sqlite.org/2019/sqlite-autoconf-3300100.tar.gz"
	URL_HASH SHA1=8383f29d53fa1d4383e4c8eb3e087f2ed940a9e0
	DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
	DOWNLOAD_NO_PROGRESS 1

	BUILD_IN_SOURCE 1

	PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/sqlite.patch

	CONFIGURE_COMMAND ${CONFIGURE_CMD}
	BUILD_COMMAND ${BUILD_CMD}
	INSTALL_COMMAND ${INSTALL_CMD}
)

if(SPV_ENABLE_STATIC)
	export_static_library(sqlite3)
endif()