
file(GLOB_RECURSE SPVJNI_SOURCE_FILES "*.cpp")

set(
	SPVJNI_SOURCE_FILES
	${SPVJNI_SOURCE_FILES}
	CACHE INTERNAL "spv jni source files" FORCE
)

set(
	SPVJNI_INC_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_ANDROID_NDK}/sysroot/usr/include
	CACHE INTERNAL "spv jni include directories" FORCE
)
