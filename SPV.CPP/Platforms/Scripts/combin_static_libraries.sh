#!/bin/bash
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
		echo "$*"
		$@ 2>&1
	else
		$@ > /dev/null 2>&1
	fi
}

log() {
	if [ "$VERBOSE" = "YES" ] || [ "$VERBOSE" = "yes" ] || [ "$VERBOSE" = "ON" ]; then
		echo "$*"
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

LIBS=
register_option "--libs=<libs>" do_option_libs "Libraries need to be merged."
do_option_libs() {
	LIBS=$(echo $1 | tr ',' '\n')
}

ARCHS=
register_option "--archs=<archs>" do_option_archs "Architectures."
do_option_archs() {
	ARCHS=$(echo $1 | tr ',' '\n')
}

OUTPUT_DIR="Output"
register_option "--output-dir=<dir>" do_option_output_dir "Spacify output directory."
do_option_output_dir() {
	OUTPUT_DIR=$1
}

OUTPUT_FILENAME=
register_option "--output-filename=<name>" do_option_output_filename "Output file name."
do_option_output_filename() {
	OUTPUT_FILENAME=$1
}

LIBTOOL=
register_option "--libtool=<tool>" do_option_libtool "libtool."
do_option_libtool() {
	LIBTOOL=$1
}

SYSTEM_NAME=
register_option "--system-name=<name>" do_option_system_name "system name."
do_option_system_name() {
	SYSTEM_NAME=$1
}

AR_PATH=
register_option "--ar-path=<path>" do_option_ar_path "ar path."
do_option_ar_path() {
	AR_PATH=$1
}

RANLIB_PATH=
register_option "--ranlib-path=<path>" do_option_ranlib_path "ranlib path."
do_option_ranlib_path() {
	RANLIB_PATH=$1
}

extract_parameters $@

if [ -z "$LIBS" ]; then
	echo "error: LIBS should not be empty"
	exit 1
fi

if [ -z "$OUTPUT_FILENAME" ]; then
	echo "error: OUTPUT_FILENAME should not be empty"
	exit 1
fi

if [ -z "$AR_PATH" ]; then
	echo "error: AR_PATH should not be empty"
	exit 1
fi

apple_combin() {
	log "Splitting and decomposing all existing fat binaries..."
	if [ -z "$ARCHS" ]; then
		echo "error: ARCH should not be empty"
		exit 1
	fi

	if [ -z "$LIBTOOL" ]; then
		echo "error: LIBTOOL should not be empty"
		exit 1
	fi

	for LIB in $LIBS; do
		for ARCH in $ARCHS; do
			LIBNAME=`basename $LIB`
			OBJDIR=`echo $LIBNAME | cut -d . -f1`
			mkdir -p $OUTPUT_DIR/$ARCH/thin/$OBJDIR

			FILE_ARCHS=`lipo -archs $LIB`
			if ! pattern_match "$ARCH" "$FILE_ARCHS"; then
				echo "error: $LIBNAME do not contain $ARCH"
				exit 1
			fi

			FILE_INFO=`lipo -info $LIB`
			if pattern_match "^Non-fat file:" "$FILE_INFO"; then
				run cp -f $LIB $OUTPUT_DIR/$ARCH/thin/$LIBNAME
			else
				run lipo "$LIB" -thin $ARCH -o $OUTPUT_DIR/$ARCH/thin/$LIBNAME
			fi

			(
				cd $OUTPUT_DIR/$ARCH/thin/$OBJDIR; run $AR_PATH -x $OUTPUT_DIR/$ARCH/thin/$LIBNAME
				for FILE in *.o; do
					NEW_FILE="${OBJDIR}_${FILE}"
					mv "$FILE" "$NEW_FILE"
				done
			)
		done
	done

	ALL_ARCH_LIBRARIES=
	for ARCH in $ARCHS; do
		log "Linking $ARCH architecture into an uber thin libspvsdk.a ..."
		ALL_OBJS=`find $OUTPUT_DIR/$ARCH/thin -name "*.o"`
		rm -fr $OUTPUT_DIR/$ARCH/$OUTPUT_FILENAME
		run $LIBTOOL -static -o $OUTPUT_DIR/$ARCH/$OUTPUT_FILENAME $ALL_OBJS
		rm -fr $OUTPUT_DIR/$ARCH/thin
		ALL_ARCH_LIBRARIES=$ALL_ARCH_LIBRARIES" $OUTPUT_DIR/$ARCH/$OUTPUT_FILENAME"
	done

	log "Generate fat uber libspvsdk.a"
	rm -fr $OUTPUT_DIR/$OUTPUT_FILENAME
	run xcrun lipo -create $ALL_ARCH_LIBRARIES -o $OUTPUT_DIR/$OUTPUT_FILENAME
}

unix_combin() {
	if [ -z "$RANLIB_PATH" ]; then
		echo "error: RANLIB_PATH should not be empty"
		exit 1
	fi

	for LIB in $LIBS; do
		LIBNAME=`basename $LIB`
		OBJDIR=`echo $LIBNAME | cut -d . -f1`
		mkdir -p $OUTPUT_DIR/obj/$OBJDIR

		(
			run cd $OUTPUT_DIR/obj/$OBJDIR; run $AR_PATH -x "$LIB"
			for FILE in *.o; do
				NEW_FILE="${OBJDIR}_${FILE}"
				mv "$FILE" "$NEW_FILE"
			done
		)
	done

	ALL_OBJS=`find $OUTPUT_DIR/obj -name "*.o"`
	rm -fr $OUTPUT_DIR/$OUTPUT_FILENAME
	run $AR_PATH qc $OUTPUT_DIR/$OUTPUT_FILENAME $ALL_OBJS
	run $RANLIB_PATH $OUTPUT_DIR/$OUTPUT_FILENAME
	rm -fr $OUTPUT_DIR/obj
}

if [ "$SYSTEM_NAME" = "Darwin" ]; then
	apple_combin
else
	unix_combin
fi

