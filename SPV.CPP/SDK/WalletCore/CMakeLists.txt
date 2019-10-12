
file(GLOB_RECURSE WALLETCORE_SOURCE_FILES "*.cpp")

add_library(walletcore STATIC ${WALLETCORE_SOURCE_FILES})
target_include_directories(walletcore PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../
	${ThirdParty_JSON_INC_DIR}
	${ThirdParty_SPDLOG_INC_DIR}
	${ThirdParty_BOOST_INC_DIR}
	${ThirdParty_OPENSSL_INC_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/../../)
set_target_properties(walletcore PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_dependencies(walletcore ${OpenSSL_LIBRARIES} ${SPV_Boost_LIBRARIES})