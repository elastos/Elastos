project(secp256k1)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalConfigureArgs)

ExternalProject_Add(
	secp256k1

	PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
	GIT_REPOSITORY "https://github.com/bitcoin-core/secp256k1.git"
	GIT_TAG e34ceb333b1c0e6f4115ecbb80c632ac1042fa49
	GIT_PROGRESS TRUE
	DOWNLOAD_NO_PROGRESS 1

	BUILD_IN_SOURCE 1

	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)