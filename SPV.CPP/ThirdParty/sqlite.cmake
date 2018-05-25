
include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/sqlite"
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/sqlite SQLITE_SOURCE_FILES)

add_library(sqlite STATIC ${SQLITE_SOURCE_FILES})

set_target_properties(sqlite PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/libsqlite.a)
set_target_properties(sqlite PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(
	ThirdParty_SQLITE_INC_DIR
	${CMAKE_CURRENT_SOURCE_DIR}/sqlite
	CACHE INTERNAL "ThirdParty sqlite include directory" FORCE
)
