project(json)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalConfigureArgs)

set(JSON_INSTALL_DIR ${PROJECT_INT_DIST_DIR}/include/nlohmann)

ExternalProject_Add(
	json

	PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
	URL "https://github.com/nlohmann/json/releases/download/v3.7.3/json.hpp"
	URL_HASH SHA256=3b5d2b8f8282b80557091514d8ab97e27f9574336c804ee666fda673a9b59926
	DOWNLOAD_NAME json.hpp
	DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
	DOWNLOAD_NO_EXTRACT 1
	DOWNLOAD_NO_PROGRESS 1

	CONFIGURE_COMMAND ${CMAKE_COMMAND} -E make_directory ${JSON_INSTALL_DIR}
	BUILD_COMMAND ""
	INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_DEPS_TARBALL_DIR}/json.hpp ${JSON_INSTALL_DIR}/json.hpp
)