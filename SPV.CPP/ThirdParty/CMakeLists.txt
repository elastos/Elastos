set(BUILD_SHARED_LIBS OFF "build static thirdparty")

include(libconfig.cmake)
include(openssl.cmake)
include(boost.cmake)
include(fruit.cmake)
include(sqlite.cmake)
include(catch2.cmake)
include(json.cmake)
include(spdlog.cmake)

set(
	ThirdParty_INC_DIRS
	${ThirdParty_FRUIT_INC_DIR}
	${ThirdParty_SPDLOG_INC_DIR}
	${ThirdParty_JSON_INC_DIR}
	${ThirdParty_CATCH2_INC_DIR}
	${ThirdParty_OPENSSL_INC_DIR}
	${ThirdParty_BOOST_INC_DIR}
	${ThirdParty_SQLITE_INC_DIR}
	${ThirdParty_BIGINT_INC_DIR}
	${ThirdParty_LIBCONFIG_INC_DIR}
	CACHE INTERNAL "ThirdParty include directories" FORCE
)
