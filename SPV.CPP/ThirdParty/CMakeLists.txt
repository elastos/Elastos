
include(openssl.cmake)
include(boost.cmake)
#include(fruit.cmake)
include(sqlite.cmake)
include(catch2.cmake)
include(json.cmake)
include(spdlog.cmake)
include(bigint.cmake)

set(
	ThirdParty_INC_DIRS
	${ThirdParty_FRUIT_INC_DIRS}
	${ThirdParty_SPDLOG_INC_DIR}
	${ThirdParty_JSON_INC_DIR}
	${ThirdParty_CATCH2_INC_DIR}
	${ThirdParty_OPENSSL_INC_DIR}
	${ThirdParty_BOOST_INC_DIR}
	${ThirdParty_SQLITE_INC_DIR}
	${ThirdParty_BIGINT_INC_DIR}
	CACHE INTERNAL "ThirdParty include directories" FORCE
)
