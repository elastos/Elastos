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
pattern_match() {
	echo "$2" | grep -q -E -e "$1"
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

register_option "--help" do_option_help "Print this help."
do_option_help() {
	print_help
	exit 0
}

output_dir="`pwd`/doc"
register_option "--output=<path>" do_option_output "Select <path> to generate the documents, default is $output_dir"
do_option_output() {
	output_dir=$1
}

extract_parameters $@

input_dir="`pwd`/Interface"
project_name="Elastos.ELA.SPV.Cpp"
version=`git describe --dirty --always --tags`
if [ $? != 0 ]; then
    exit
fi

startDoxygen() {
   rm doxygen.cfg
   doxygen -g doxygen.cfg
   sed -i '' -e "s#\(PROJECT_NAME[[:space:]]*=\).*#\1 $project_name#g"  ./doxygen.cfg
   sed -i '' -e "s#\(OUTPUT_DIRECTORY[[:space:]]*=\).*#\1 $output_dir#g" ./doxygen.cfg
   sed -i '' -e "s#\(PROJECT_NUMBER[[:space:]]*=\).*#\1 $version#g" ./doxygen.cfg
   sed -i '' -e "s#\(INPUT[[:space:]]*=\).*#\1 $input_dir#g" ./doxygen.cfg

   doxygen doxygen.cfg
}

res=`which doxygen`
if [ $? != 0 ]; then
    echo "command not found: doxygen"
    exit
fi

startDoxygen
