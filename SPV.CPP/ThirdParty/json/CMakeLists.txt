project(json)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalCMakeArgs)

ExternalProject_Add(
	json

	PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
#	GIT_REPOSITORY "https://github.com/nlohmann/json.git"
#	GIT_TAG v3.7.0
#	GIT_PROGRESS TRUE
	URL "https://github.com/nlohmann/json/archive/v3.7.0.tar.gz"
	URL_HASH SHA256=d51a3a8d3efbb1139d7608e28782ea9efea7e7933157e8ff8184901efd8ee760
	DOWNLOAD_NAME "json-3.7.0.tar.gz"
	DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
	DOWNLOAD_NO_PROGRESS 1

	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
		${CMAKE_ARGS_INIT}
		-DCMAKE_BUILD_TYPE=Release
		-DJSON_BuildTests=OFF
		-DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
)