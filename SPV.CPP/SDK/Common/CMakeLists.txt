file(GLOB_RECURSE COMMON_SOURCE_FILES "*.cpp")

include_directories(
	${ThirdParty_OPENSSL_INC_DIR}
	${ThirdParty_BOOST_INC_DIR}
	${ThirdParty_JSON_INC_DIR}
	${ThirdParty_SPDLOG_INC_DIR}
)

add_library(common STATIC ${COMMON_SOURCE_FILES})
set_target_properties(common PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_dependencies(common ${Boost_LIBRARIES})