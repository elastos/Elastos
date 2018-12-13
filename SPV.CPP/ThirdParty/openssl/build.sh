#!/bin/sh
#
# Copyright (C) 2010 Mystic Tree Games
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

. `dirname $0`/utils.sh

PROGDIR=`pwd`/openssl

register_option "--toolchain=<toolchain>" select_toolchain "Select a toolchain. To see available execute ls -l ANDROID_NDK/toolchains."
select_toolchain() {
	TOOLCHAIN=$1
}

PREFIX=/usr/local
register_option "--prefix=<path>" do_prefix "Prefix to be used when installing libraries and includes."
do_prefix() {
	PREFIX=$1;
}

BUILD_DIR="build"
register_option "--build-dir=<DIR>" do_build_dir "Build in this location instead of building within the distribution tree."
do_build_dir() {
	BUILD_DIR=$1
}

SUPPORT_ARCHLIST="arm64-v8a,armeabi,armeabi-v7a,mips,mips64,x86,x86_64"
ARCHLIST=
register_option "--arch=<list>" do_arch "Comma separated list of architectures to build: $SUPPORT_ARCHLIST"
do_arch() {
	for ARCH in $(echo $1 | tr ',' ' '); do ARCHLIST="$ARCH ${ARCHLIST}"; done

	for ARCH in $ARCHLIST; do
		support=0
		for SUPPORT_ARCH in $(echo $SUPPORT_ARCHLIST | tr ',' ' '); do
			if [ "$ARCH" = "$SUPPORT_ARCH" ]; then
				support=1
			fi
		done
		if [ "$support" = "0" ]; then
			echo "Unsupported arch: $ARCH."
			exit 1
		fi
	done
}

NDK_ROOT=
register_option "--ndk-root=<path>" do_ndk_root "Select NDK to build for android."
do_ndk_root() {
	NDK_ROOT=$1
	if [ ! -f "$NDK_ROOT/ndk-build" ]; then
		echo "ERROR: $NDK_ROOT is not a valid NDK root"
		exit 1
	fi
}

NDK_API_LEVEL=
register_option "--ndk-api-level=<num>" do_ndk_api_level "Select api level. To see available execute ls ANDROID_NDK/platforms/android-<num>."
do_ndk_api_level() {
    NDK_API_LEVEL=$1
    if [ ! -d $ANDROID_NDK/platforms/android-$NDK_API_LEVEL ]; then
        echo "ERROR: invalid api level $NDK_API_LEVEL"
        exit 1
    fi
}

extract_parameters $@

if [ -z "$NCPU" ]; then
	if uname -s | grep -i "Darwin" > /dev/null; then
		NCPU=`sysctl -a | grep -i "ncpu" | awk '{ print $2 }'`
	elif uname -s | grep -i "linux" > /dev/null; then
		NCPU=`cat /proc/cpuinfo | grep -c -i processor`
	else
		NCPU=4
	fi
fi


android_build_setup() {
	PlatformOS=
	HOST_OS=`uname -s`
	case "$HOST_OS" in
		Darwin | FreeBsd)
			PlatformOS=darwin
			;;
		Linux)
			PlatformOS=linux
			;;
		CYGWIN* | *_NT-*)
			PlatformOS=windows
			;;
		*)
			echo "Unknow platform os"
			exit 1
	esac

	NDK_RELEASE_FILE="$NDK_ROOT/RELEASE.TXT"
	if [ -f "$NDK_RELEASE_FILE" ]; then
		NDK_RN=`cat $NDK_RELEASE_FILE | sed 's/^r\(.*\)$/\1/g'`
	else
		NDK_RELEASE_FILE="$NDK_ROOT/source.properties"
		if [ -f "$NDK_RELEASE_FILE" ]; then
			NDK_RN=`cat $NDK_RELEASE_FILE | grep 'Pkg.Revision' | sed -E 's/^.*[=] *([0-9]+[.][0-9]+)[.].*/\1/g'`
		else
			echo "ERROR: can not find ndk version"
			exit 1
		fi
	fi

	echo "Detected Android NDK version $NDK_RN"
	case "$NDK_RN" in
		4*)
			TOOLCHAIN=${TOOLCHAIN:-arm-eabi-4.4.0}
			CXXPATH=$NDK_ROOT/build/prebuilt/$PlatformOS-x86/${TOOLCHAIN}/bin/arm-eabi-g++
			TOOLSET=gcc-androidR4
			;;
		5*)
			TOOLCHAIN=${TOOLCHAIN:-arm-linux-androideabi-4.4.3}
			CXXPATH=$NDK_ROOT/toolchains/${TOOLCHAIN}/prebuilt/$PlatformOS-x86/bin/arm-linux-androideabi-g++
			TOOLSET=gcc-androidR5
			;;
		7-crystax-5.beta3)
			TOOLCHAIN=${TOOLCHAIN:-arm-linux-androideabi-4.6.3}
			CXXPATH=$NDK_ROOT/toolchains/${TOOLCHAIN}/prebuilt/$PlatformOS-x86/bin/arm-linux-androideabi-g++
			TOOLSET=gcc-androidR7crystax5beta3
			;;
		8)
			TOOLCHAIN=${TOOLCHAIN:-arm-linux-androideabi-4.4.3}
			CXXPATH=$NDK_ROOT/toolchains/${TOOLCHAIN}/prebuilt/$PlatformOS-x86/bin/arm-linux-androideabi-g++
			TOOLSET=gcc-androidR8
			;;
		8b|8c|8d)
			TOOLCHAIN=${TOOLCHAIN:-arm-linux-androideabi-4.6}
			CXXPATH=$NDK_ROOT/toolchains/${TOOLCHAIN}/prebuilt/$PlatformOS-x86/bin/arm-linux-androideabi-g++
			TOOLSET=gcc-androidR8b
			;;
		8e|9|9b|9c|9d)
			TOOLCHAIN=${TOOLCHAIN:-arm-linux-androideabi-4.6}
			CXXPATH=$NDK_ROOT/toolchains/${TOOLCHAIN}/prebuilt/$PlatformOS-x86/bin/arm-linux-androideabi-g++
			TOOLSET=gcc-androidR8e
			;;
		"8e (64-bit)")
			TOOLCHAIN=${TOOLCHAIN:-arm-linux-androideabi-4.6}
			CXXPATH=$NDK_ROOT/toolchains/${TOOLCHAIN}/prebuilt/${PlatformOS}-x86_64/bin/arm-linux-androideabi-g++
			TOOLSET=gcc-androidR8e
			;;
		"9 (64-bit)"|"9b (64-bit)"|"9c (64-bit)"|"9d (64-bit)")
			TOOLCHAIN=${TOOLCHAIN:-arm-linux-androideabi-4.6}
			CXXPATH=$NDK_ROOT/toolchains/${TOOLCHAIN}/prebuilt/${PlatformOS}-x86_64/bin/arm-linux-androideabi-g++
			TOOLSET=gcc-androidR8e
			;;
		"10 (64-bit)"|"10b (64-bit)"|"10c (64-bit)"|"10d (64-bit)")
			TOOLCHAIN=${TOOLCHAIN:-arm-linux-androideabi-4.6}
			CXXPATH=$NDK_ROOT/toolchains/${TOOLCHAIN}/prebuilt/${PlatformOS}-x86_64/bin/arm-linux-androideabi-g++
			TOOLSET=gcc-androidR8e
			;;
		16.*|17.*|18.*)
			TOOLCHAIN=${TOOLCHAIN:-llvm}
			CXXPATH=$NDK_ROOT/toolchains/${TOOLCHAIN}/prebuilt/${PlatformOS}-x86_64/bin/clang++
			TOOLSET=clang
			;;
		*)
			echo "Undefined or not supported Android NDK version: $NDK_RN"
			exit 1
	esac

	# Check if the ndk is valid or not
	if [ ! -f "$CXXPATH" ]; then
		echo "Can not find C++ compiler at: $CXXPATH"
		exit 1
	fi
}

android_openssl_build() {
	if [ -z "$ARCHLIST" ]; then
		echo "WARNING: Build arch list is empty"
		exit 1
	fi

	android_build_setup

	for ARCH in $ARCHLIST; do

		if [ "$ARCH" = "armeabi" ] || [ "$ARCH" = "armeabi-v7a" ]; then
			TARGET=android-arm
		elif [ "$ARCH" = "arm64-v8a" ]; then
			TARGET=android-arm64
		elif [ "$ARCH" = "mips" ]; then
			TARGET=android-mips
		elif [ "$ARCH" = "mips64" ]; then
			TARGET=android-mips64
		elif [ "$ARCH" = "x86" ]; then
			TARGET=android-x86
		elif [ "$ARCH" = "x86_64" ]; then
			TARGET=android-x86_64
		fi

		export ANDROID_NDK=$NDK_ROOT
		export PATH=`dirname $CXXPATH`:$PATH

		ARCH_BUILD_DIR=$BUILD_DIR/$ARCH
		if [ ! -d $ARCH_BUILD_DIR ]; then
			if ! mkdir -p $ARCH_BUILD_DIR; then
				echo "ERROR: mkdir build dir: '$ARCH_BUILD_DIR'"
				exit 1
			fi
		fi

		cd $ARCH_BUILD_DIR && $PROGDIR/Configure $TARGET shared -D__ANDROID_API__=$NDK_API_LEVEL -fPIC -latomic --prefix=$PREFIX/$ARCH && make all && make install_sw
	done
}

openssl_build() {
	if [ ! -d $BUILD_DIR ]; then
		if ! mkdir -p $BUILD_DIR; then
			echo "ERROR: mkdir build dir: '$BUILD_DIR'"
			exit 1
		fi
	fi

	cd $BUILD_DIR && $PROGDIR/config shared --prefix=$PREFIX && make all && make install_sw
}

if [ -z "$NDK_ROOT" ]; then
	openssl_build
else
	android_openssl_build
fi

