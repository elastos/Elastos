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


run() {
	if [ "$VERBOSE" = "YES" ] || [ "$VERBOSE" = "yes" ] || [ "$VERBOSE" = "ON" ]; then
		$@ 2>&1
	else
		$@ > /dev/null 2>&1
	fi
}

pattern_match() {
	echo "$2" | grep -q -E -e "$1"
}

find_program() {
	local PROG
	PROG=`which $2 2>/dev/null`
	if [ -n "$PROG" ]; then
		if pattern_match "^no " "$PROG"; then
			PROG=
		fi
	fi
	eval $1="$PROG"
}

# $1 url
# $2 target file
download_file() {
	find_program CMD_WGET wget
	find_program CMD_CURL curl

	if pattern_match "^(http|https|ssh):.*" "$1"; then
		if [ -n "$CMD_WGET" ]; then
			run $CMD_WGET -O $2 $1
		elif [ -n "$CMD_CURL" ]; then
			run $CMD_CURL -L -o $2 $1
		else
			echo "Please install wget or curl"
			exit 1
		fi
	fi
}

# return the value of a given named variable
# $1: variable name
#
# example:
#    FOO=BAR
#    BAR=ZOO
#    echo `var_value $FOO`
#    will print 'ZOO'
#
var_value() {
	# find a better way to do that ?
	eval echo "$`echo $1`"
}

# Set a given option attribute
# $1: option name
# $2: option attribute
# $3: attribute value
#
option_set_attr() {
	eval OPTIONS_$1_$2=\"$3\"
}

# Get a given option attribute
# $1: option name
# $2: option attribute
#
option_get_attr() {
	echo `var_value OPTIONS_$1_$2`
}

# Translate dashes to underscores
# Usage:  str=`dashes_to_underscores <values>`
dashes_to_underscores() {
	echo $@ | tr '-' '_'
}

# Translate underscores to dashes
# Usage: str=`underscores_to_dashes <values>`
underscores_to_dashes() {
	echo $@ | tr '_' '-'
}

# Register a new option
# $1: option
# $2: name of function that will be called when the option is parsed
# $3: small abstract for the option
# $4: optional. default value
register_option() {
	local optname optvalue opttype optlabel
	optlabel=
	optname=
	optvalue=
	opttype=

	while true
	do
		# Check for something like --setting=<value>
		echo "$1" | grep -q -E -e '^--[^=]+=<.+>$'
		if [ $? = 0 ]; then
			optlabel=`expr -- "$1" : '\(--[^=]*\)=.*'`
			optvalue=`expr -- "$1" : '--[^=]*=\(<.*>\)'`
			opttype="long_setting"
			break
		fi

		# Check for something like --flag
		echo "$1" | grep -q -E -e '^--[^=]+$'
		if [ $? = 0 ]; then
			optlabel="$1"
			opttype="long_flag"
			break
		fi

		# Check for something like -f<value>
		echo "$1" | grep -q -E -e '^-[A-Za-z0-9]<.+>$'
		if [ $? = 0 ]; then
			optlabel=`expr -- "$1" : '\(-.\).*'`
			optvalue=`expr -- "$1" : '-.\(<.+>\)'`
			opttype="short_setting"
			break
		fi

		# Check for something like -f
		echo "$1" | grep -q -E -e '^-.$'
		if [ $? = 0 ]; then
			optlabel="$1"
			opttype="short_flag"
			break
		fi

		echo "ERROR: Invalid option format: $1"
		echo "       Check register_option call"
		exit 1

	done

	#echo "optlabel=$optlabel optvalue=$optvalue opttype=$opttype"

	optname=`dashes_to_underscores $optlabel`
	OPTIONS="$OPTIONS $optname"
	OPTIONS_TEXT="$OPTIONS_TEXT $1"
	option_set_attr $optname label "$optlabel"
	option_set_attr $optname otype "$opttype"
	option_set_attr $optname value "$optvalue"
	option_set_attr $optname text "$1"
	option_set_attr $optname funcname "$2"
	option_set_attr $optname abstract "$3"
	option_set_attr $optname default "$4"
}

# Return the maximum length of a series of strings
#
# Usage:  len=`max_length <string1> <string2> ...`
#
max_length() {
	echo "$@" | tr ' ' '\n' | awk 'BEGIN {max=0} {len=length($1); if (len > max) max=len} END {print max}'
}

# Print the help, including a list of registered options for this program
# Note: Assumes PROGRAM_PARAMETERS and PROGRAM_DESCRIPTION exist and
#       correspond to the parameters list and the program description
#
print_help() {
	local opt text abstract default

	echo "Usage: $PROGNAME [options] $PROGRAM_PARAMETERS"
	echo ""
	if [ -n "$PROGRAM_DESCRIPTION" ]; then
		echo "$PROGRAM_DESCRIPTION"
		echo ""
	fi
	echo "Valid options (defaults are in brackets):"
	echo ""

	maxw=`max_length "$OPTIONS_TEXT"`
	AWK_SCRIPT=`echo "{ printf \"%-${maxw}s\", \\$1 }"`
	for opt in $OPTIONS; do
		text=`option_get_attr $opt text | awk "$AWK_SCRIPT"`
		abstract=`option_get_attr $opt abstract`
		default=`option_get_attr $opt default`
		if [ -n "$default" ]; then
			echo "  $text     $abstract [$default]"
		else
			echo "  $text     $abstract"
		fi
	done
	echo ""
}

option_panic_no_args() {
	echo "ERROR: Option '$1' does not take arguments. See --help for usage."
	exit 1
}

option_panic_missing_arg() {
	echo "ERROR: Option '$1' requires an argument. See --help for usage."
	exit 1
}

extract_parameters() {
	local opt optname otype value name fin funcname
	PARAMETERS=""
	while [ -n "$1" ]; do
		# If the parameter does not begin with a dash
		# it is not an option.
		param=`expr -- "$1" : '^\([^\-].*\)$'`
		if [ -n "$param" ]; then
			if [ -z "$PARAMETERS" ]; then
				PARAMETERS="$1"
			else
				PARAMETERS="$PARAMETERS $1"
			fi
			shift
			continue
		fi

		while true
		do
			# Try to match a long setting, i.e. --option=value
			opt=`expr -- "$1" : '^\(--[^=]*\)=.*$'`
			if [ -n "$opt" ]; then
				otype="long_setting"
				value=`expr -- "$1" : '^--[^=]*=\(.*\)$'`
				break
			fi

			# Try to match a long flag, i.e. --option
			opt=`expr -- "$1" : '^\(--.*\)$'`
			if [ -n "$opt" ]; then
				otype="long_flag"
				value=
				break
			fi

			# Try to match a short setting, i.e. -o<value>
			opt=`expr -- "$1" : '^\(-[A-Za-z0-9]\).*$'`
			if [ -n "$opt" ]; then
				otype="short_setting"
				value=`expr -- "$1" : '^-.\(.*\)$'`
				break
			fi

			# Try to match a short flag, i.e. -o
			opt=`expr -- "$1" : '^\(-.\)$'`
			if [ -n "$opt" ]; then
				otype="short_flag"
				value=
				break
			fi

			echo "ERROR: Unknown option '$1'. Use --help for list of valid values."
			exit 1
		done

		#echo "Found opt='$opt' otype='$otype' value='$value'"

		name=`dashes_to_underscores $opt`
		found=0
		for xopt in $OPTIONS; do
			if [ "$name" != "$xopt" ]; then
				continue
			fi
			# Check that the type is correct here
			#
			# This also allows us to handle -o <value> as -o<value>
			#
			xotype=`option_get_attr $name $otype`
			if [ "$otype" != "$xotype" ]; then
				case "$xotype" in
					"short_flag")
						option_panic_no_args $opt
						;;
					"short_setting")
						if [ -z "$2" ]; then
							option_panic_missing_arg $opt
						fi
						value="$2"
						shift
						;;
					"long_flag")
						option_panic_no_args $opt
						;;
					"long_setting")
						option_panic_missing_arg $opt
						;;
				esac
			fi
			found=1
			break
			break
		done
		if [ "$found" = "0" ]; then
			echo "ERROR: Unknown option '$opt'. See --help for usage."
			exit 1
		fi
		# Launch option-specific function, value, if any as argument
		eval `option_get_attr $name funcname` \"$value\"
		shift
	done
}

register_option "--help"          do_option_help     "Print this help."
do_option_help() {
	print_help
	exit 0
}

VERBOSE=no
register_option "--verbose"       do_option_verbose  "Enable verbose mode."
do_option_verbose() {
	VERBOSE=yes
}

LIBRARIES=
register_option "--with-libraries=<list>" do_with_libraries "Comma separated list of libraries to build."
do_with_libraries() {
	for lib in $(echo $1 | tr ',' '\n'); do LIBRARIES="--with-$lib ${LIBRARIES}"; done
}

register_option "--without-libraries=<list>" do_without_libraries "Comma separated list of libraries to exclude from the build."
do_without_libraries() {
	for lib in $(echo $1 | tr ',' '\n') ; do LIBRARIES="--without-$lib ${LIBRARIES}"; done
}

register_option "--toolchain=<toolchain>" select_toolchain "Select a toolchain. To see available execute ls -l ANDROID_NDK/toolchains."
select_toolchain() {
	TOOLCHAIN=$1

	if [ ! -d $ANDROID_NDK/toolchains/$TOOLCHAIN ]; then
	    echo "ERROR: invalid toolchain $TOOLCHAIN"
	    exit 1
	fi
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

EXTRACT_DIR=`pwd`
register_option "--extract-dir=<path>" do_extract_dir "Select <path> to extract the source code, default is here."
do_extract_dir() {
	EXTRACT_DIR=$1
	if [ ! -d "$EXTRACT_DIR" ]; then
		mkdir -p $EXTRACT_DIR
	fi
}

BOOST_VER1=1
BOOST_VER2=67
BOOST_VER3=0
BOOST_SUPPORT_LIST="1.67.0, 1.66.0, 1.65.1, 1.55.0, 1.54.0, 1.53.0, 1.49.0, 1.48.0, 1.45.0"
register_option "--boost=<version>" boost_version "Boost version to be used, one of {$BOOST_SUPPORT_LIST}, default is 1.67.0."
boost_version() {
	BOOST_VER1=`expr -- "$1" : '\([0-9]*\)\.[0-9]*\.[0-9]*'`
	BOOST_VER2=`expr -- "$1" : '[0-9]*\.\([0-9]*\)\.[0-9]*'`
	BOOST_VER3=`expr -- "$1" : '[0-9]*\.[0-9]*\.\([0-9]*\)'`
	support=0
	for ver in $(echo $BOOST_SUPPORT_LIST | tr ',' ' ');
	do
		if [ "$BOOST_VER1.$BOOST_VER2.$BOOST_VER3" = "$ver" ]; then
			support=1
		fi
	done

	if [ "$support" = "0" ]; then
		echo "Unsupported boost version '$1'."
		exit 1
	fi
}

extract_parameters $@

echo "Building boost version: $BOOST_VER1.$BOOST_VER2.$BOOST_VER3"

BOOST_DOWNLOAD_LINK="http://downloads.sourceforge.net/project/boost/boost/$BOOST_VER1.$BOOST_VER2.$BOOST_VER3/boost_${BOOST_VER1}_${BOOST_VER2}_${BOOST_VER3}.tar.bz2?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fboost%2Ffiles%2Fboost%2F${BOOST_VER1}.${BOOST_VER2}.${BOOST_VER3}%2F&ts=1291326673&use_mirror=garr"
BOOST_TAR="boost_${BOOST_VER1}_${BOOST_VER2}_${BOOST_VER3}.tar.bz2"
BOOST_DIR="$EXTRACT_DIR/boost_${BOOST_VER1}_${BOOST_VER2}_${BOOST_VER3}"
PROGDIR=`pwd`

if [ ! -f $BOOST_TAR ]; then
	echo "Downloading $BOOST_TAR please wait..."
	download_file $BOOST_DOWNLOAD_LINK $BOOST_TAR
fi

if [ ! -d $BOOST_DIR ]; then
	echo "Unpacking boost to $EXTRACT_DIR"
	if ! tar xvf $BOOST_TAR -C $EXTRACT_DIR > /dev/null 2>&1; then
		rm -fr $BOOST_DIR $BOOST_TAR
		echo "ERROR: Unpack boost fail because of bad tarball."
		echo "Remove the bad $BOOST_TAR file, rebuild will download again."
		exit 1
	fi
	echo "Unpack boost done."
fi

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

bootstrap_and_apply_patch() {
	if [ ! -f $BOOST_DIR/bjam ]; then
		cd $BOOST_DIR
		case "$PlatformOS" in
			windows)
				cmd //c "boostrap.bat"
				;;
			*)
				./bootstrap.sh
		esac
		if [ $? != 0 ]; then
			echo "ERROR: Could not perform boostrap!"
			rm -fr $BOOST_DIR
			exit 1
		fi

		cd $PROGDIR
		BOOST_VER=${BOOST_VER1}_${BOOST_VER2}_${BOOST_VER3}
		if [ "$TOOLSET" = "clang" ]; then
			cp -fv configs/user-config-boost-${BOOST_VER}.jam $BOOST_DIR/tools/build/src/user-config.jam || exit 1
			for FILE in configs/user-config-boost-${BOOST_VER}-*.jam; do
				ARCH="`echo $FILE | sed s%configs/user-config-boost-${BOOST_VER}-%% | sed s/[.]jam//`"
				if [ "$ARCH" = "common" ]; then
					continue
				fi
				JAMARCH="`echo ${ARCH} | tr -d '_-'`" # Remove all dashes, bjam does not like them
				sed "s/%ARCH%/${JAMARCH}/g" configs/user-config-boost-${BOOST_VER}-common.jam >> $BOOST_DIR/tools/build/src/user-config.jam || exit 1
				cat configs/user-config-boost-${BOOST_VER}-$ARCH.jam >> $BOOST_DIR/tools/build/src/user-config.jam || exit 1
				echo ';' >> $BOOST_DIR/tools/build/src/user-config.jam || exit 1
			done
		else
			cp -fv configs/user-config-boost-${BOOST_VER}.jam $BOOST_DIR/tools/build/v2/user-config.jam || exit 1
		fi

		PATCH_BOOST_DIR=$PROGDIR/patches/boost-$BOOST_VER
		if [ ! -d $PATCH_BOOST_DIR ]; then
			echo "ERROR: Could not find directory '$PATCH_BOOST_DIR' while looking for patches"
			exit 1
		fi

		PATCHES=`(cd $PATCH_BOOST_DIR && ls *.patch | sort) 2> /dev/null`
		if [ -z "$PATCHES" ]; then
			echo "ERROR: No Patches found in directory '$PATCH_BOOST_DIR'"
			exit 1
		fi

		cd $BOOST_DIR
		for PATCH in $PATCHES; do
			PATCH=`echo $PATCH | sed -e 's#^\./##g'`
			patch -p 1 < $PATCH_BOOST_DIR/$PATCH
			if [ $? != 0 ]; then
				echo "ERROR: Patch failure !! Please check your patches directory!"
				echo "       Problem patch: $PATCH_BOOST_DIR/$PATCH"
				exit 1
			fi
		done
	fi
}

android_boost_build() {
	android_build_setup
	bootstrap_and_apply_patch

	if [ -z "$ARCHLIST" ]; then
		echo "WARNING: Build arch list is empty"
		exit 1
	fi

	cd $BOOST_DIR
	for ARCH in $ARCHLIST; do
		export AndroidBinariesPath=`dirname $CXXPATH`
		export PATH=$AndroidBinariesPath:$PATH
		export AndroidNDKRoot=$NDK_ROOT
		export NO_BZIP2=1
		export PlatformOS
		export NDK_API_LEVEL

		LIBRARIES_BROKEN=""
		if [ "$TOOLSET" = "clang" ]; then
			JAMARCH="`echo $ARCH | tr -d '_-'`" # Remove all dashes, bjam does not like them
			TOOLSET_ARCH=$TOOLSET-$JAMARCH
			TARGET_OS=android
			if [ "$ARCH" = "armeabi" ]; then
				if [ -z "$LIBRARIES" ]; then
					echo "Disabling boost_math library on armeabi architecture, because of broken toolchain"
					LIBRARIES_BROKEN="--without-math"
				elif echo $LIBRARIES | grep math; then
					echo "ERROR: Cannot build boost_math library for armeabi architecture because of broken toolchain"
					echo "       However, it is explicitly included"
					exit 1
				fi
			fi
		else
			TOOLSET_ARCH=$TOOLSET
			TARGET_OS=linux
		fi

		WITHOUT_LIBRARIES=--without-python
		if [ -n "$LIBRARIES" ]; then
			unset WITHOUT_LIBRARIES
		fi

		./bjam -q \
			-j$NCPU \
			-d0 \
			target-os=$TARGET_OS \
			toolset=$TOOLSET_ARCH \
			link=static \
			threading=multi \
			--layout=system \
			--build-dir=$BUILD_DIR/$ARCH \
			--prefix=$PREFIX/$ARCH \
			$LIBRARIES \
			$WITHOUT_LIBRARIES \
			$LIBRARIES_BROKEN \
			install 2>&1 \
			|| { echo "ERROR: Failed to build boost for android for $ARCH!"; rm -fr $PREFIX/$ARCH $BUILD_DIR/$ARCH; exit 1; }
	done
}

host_boost_build() {
	echo "Boost building..."

	cd $BOOST_DIR
	if ! ./bootstrap.sh; then
		echo "ERROR: bootstrap failed"
		rm -fr $BOOST_DIR
		exit 1
	fi

	./bjam -q \
		-j$NCPU \
		-d0 \
		link=static \
		cflags=-fPIC \
		cxxflags=-fPIC \
		threading=multi \
		--layout=system \
		--build-dir=$BUILD_DIR \
		--prefix=$PREFIX \
		$LIBRARIES \
		$WITHOUT_LIBRARIES \
		install 2>&1 \
		|| { echo "ERROR: Failed to build boost!"; rm -fr $PREFIX $BUILD_DIR; exit 1; }
}

if [ -z "$NDK_ROOT" ]; then
	host_boost_build
else
	android_boost_build
fi

echo "Done!"

