
add_subdirectory(WalletCore)
add_subdirectory(Common)

file(GLOB_RECURSE SPVSDK_SOURCE_FILES
	"Account/*.cpp"
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
