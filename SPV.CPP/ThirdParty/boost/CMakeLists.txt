project(boost)

include(ProjectDefaults)
include(ExternalProject)

if(CMAKE_VERBOSE_MAKEFILE)
	set(BOOST_BUILD_VERBOSE -d+2)
else()
	set(BOOST_BUILD_VERBOSE -d0)
endif()

set(BUILD_COMMON_ARGS
	-q
	${BOOST_BUILD_VERBOSE}
	-j 4
	variant=release
	link=static
	cflags=-fPIC
	cxxflags=-fPIC
	threading=multi
	strip=on
	--prefix=${PROJECT_INT_DIST_DIR}
	--layout=system
	--with-filesystem
	--with-system
	--with-program_options
	--with-thread)

if(ANDROID)
	set(ANDROID_TOOLCHAIN_RANLIB ${CMAKE_CXX_ANDROID_TOOLCHAIN_PREFIX}ranlib${CMAKE_CXX_ANDROID_TOOLCHAIN_SUFFIX})
	get_filename_component(ANDROID_TOOLCHAIN_BIN_PATH ${CMAKE_CXX_COMPILER} DIRECTORY)
	get_filename_component(GCC_TOOLCHAIN_BIN_PATH ${ANDROID_TOOLCHAIN_RANLIB} DIRECTORY)
	string(REGEX REPLACE "[-_]" "" JAMARCH ${CMAKE_ANDROID_ARCH_ABI})

	set(CONFIGURE_CMD
		./bootstrap.sh &&
		${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/configs/user-config-android.jam tools/build/src/user-config.jam)

	set(BUILD_CMD
		ANDROID_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}
		ANDROID_NDK_HOME=${ANDROID_NDK_HOME}
		ANDROID_TOOLCHAIN_BIN_PATH=${ANDROID_TOOLCHAIN_BIN_PATH}
		GCC_TOOLCHAIN_BIN_PATH=${GCC_TOOLCHAIN_BIN_PATH}
		ANDROID_TOOLCHAIN_RANLIB=${ANDROID_TOOLCHAIN_RANLIB}
		./b2
		target-os=android
		toolset=${CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION}-${JAMARCH}
		${BUILD_COMMON_ARGS}
		install)
elseif(IOS)
	set(CONFIGURE_CMD
		./bootstrap.sh &&
		${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/configs/user-config-ios.jam tools/build/src/user-config.jam)

	set(BUILD_CMD
		CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		CMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
		IOS_AR=${IOS_AR}
		IOS_RANLIB=${IOS_RANLIB}
		./b2
		target-os=iphone
		toolset=clang-${IOS_PLATFORM}
		${BUILD_COMMON_ARGS}
		install)
else()
	set(CONFIGURE_CMD ./bootstrap.sh)
	set(BUILD_CMD
		./b2
		${BUILD_COMMON_ARGS}
		install)
endif()

ExternalProject_Add(
	boost

	PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
	URL "https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.bz2"
	URL_HASH SHA256=59c9b274bc451cf91a9ba1dd2c7fdcaf5d60b1b3aa83f2c9fa143417cc660722
	DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
	DOWNLOAD_NO_PROGRESS 1
	BUILD_IN_SOURCE 1

	CONFIGURE_COMMAND ${CONFIGURE_CMD}

	BUILD_COMMAND ${BUILD_CMD}
	INSTALL_COMMAND ""
)

if(SPV_ENABLE_STATIC)
	export_static_library(boost_filesystem)
	export_static_library(boost_system)
	export_static_library(boost_thread)
endif()