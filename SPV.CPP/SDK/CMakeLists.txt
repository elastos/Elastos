
add_subdirectory(WalletCore)

file(GLOB_RECURSE SPVSDK_SOURCE_FILES
	"Account/*.cpp"
	"Common/*.cpp"
	"Database/*.cpp"
	"IDAgent/*.cpp"
	"Implement/*.cpp"
	"P2P/*.cpp"
	"Plugin/*.cpp"
	"SpvService/*.cpp"
	"Wallet/*.cpp")

set(
	SPVSDK_SOURCE_FILES
	${SPVSDK_SOURCE_FILES}
	CACHE INTERNAL "spv sdk source files" FORCE
)
