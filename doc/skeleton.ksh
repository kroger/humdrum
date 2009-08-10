###################################################################
# 					PROGRAM.KSH
#
#	This file contains a skeleton of a Humdrum shell program that may
# be of use to you if you wish to create new tools to work along with
# the existing Humdrum tools.  This is NOT a complete program but should
# be used as a guide only.  It would be used in conjunction with 
# skeleton.awk
#
# Programmed by:                      Date:
#
# Modifications:
#   Date:      Programmer:       Description:
#
#
# 	This shell program is used to invoke the awk program program.awk.
#
# These variables will be used in the shell script
FILENAME=""
OPTIONS=""
HELP=""
FLAG=""
STNDIN=""
#
# Fill in the appropriate command usage here
USAGE1=""
USAGE2=""
# Continue to loop through the positional parameters using getopts
while getopts ????? arg
do
	case ${arg} in
	h) 	HELP="ON" ;;
     [options])
		OPTIONS="$OPTIONS${arg}" ;;
	[?]) echo "" 1>&2
		echo "$USAGE1" 1>&2
		echo "$USAGE2" 1>&2
		exit;;
	esac
done
# If no parameters were specified, print out a help screen
if [ -n "$HELP" ]
then
	if [ -f ${HUMDRUM}/bin/helpscrn/????.hlp ]
	then
		cat ${HUMDRUM}/bin/helpscrn/????.hlp
	else
		echo "No help available on this command."
	fi
	exit
fi
# Process any remaining filenames
while [ "$#" -ne 0 ]
do
	# A '-' specifies standard input
	if [ x"$1" = x- ]		# (Kludge:  Prepend the letter "x" to force
						#  string comparison.)
	then
		# Only one '-' may be used
		if [ -z "$STNDIN" ]
		then
			FILENAME="$FILENAME $1"
			STNDIN="ON"
			shift
		else
			shift
		fi
	elif [ -f "$1" ]
	then
		FILENAME="$FILENAME $1"
		shift
	else
		echo "????: ERROR: File not found: $1" 1>&2
		FLAG="ON"
		shift
	fi
done	
# If only invalid files specified, exit
if [ -z "$FILENAME" -a -n "$FLAG" ]
then
	exit
fi	
# Also send a DUMMY string so that any options or filenames starting with
# a dash (-) are evaluated properly when using UNIX
$AWK_VER -f $HUMDRUM/????.awk "DUMMY" "${OPTIONS:-null}" $FILENAME
