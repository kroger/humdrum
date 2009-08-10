################################################################
##					HUMSED.AWK
# Programmed by: Tim Racinsky          Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:     Programmer:         Description:
#
#
#	This program is called from humsed.ksh and is used by the shell
# script file to determine at which line numbers the comments
# and interpretations occurr.
#
#	VARIABLES:
#
#  -first_line:  holds the first line of a range of comments or
#   interpretations or both.
#  -counter:  counts the number of comments and/or interpretations in 
#   the current range of comments and/or interpretations.
#  -RANGE:  flag that signifies if currently looking at a range of
#   comments and/or interpretations.
#
#	Note that each range is followed by a "b".  This tells sed to skip the
# lines appearing in the range so that only data records will be processed.
#
BEGIN {
	#
	# Delete the DUMMY variable sent from the shell script
	#
	ARGV[1] = ""
	FS = OFS = "\t"
	TRUE = 1 ; FALSE = 0
	counter = first_line = 0
	RANGE = FALSE
	}
{
#
# If the current line is a comment or interpretation, then begin counting
# the numbers so that a range may be specified.
#
if ($0 ~ /^!/ || $0 ~ /^\*/)
	{
	#
	# If a range is already in progress, increment the counter
	#
	if (RANGE) counter++
	#
	# Otherwise, begin a new range.
	#
	else
		{
		counter = 1
		first_line = NR
		RANGE = TRUE
		}
	}
#
# For any other record, if this record finishes a range, print the range
#
else
	{
	if (RANGE)
		{
		if (counter == 1) print first_line "b"
		else print first_line "," first_line + counter - 1 "b"
		RANGE = FALSE
		counter = 0
		}
	}
}
END	{
	#
	# Print the range if one was in progress at the end of the file
	#
	if (RANGE)
		{
		if (counter == 1) print first_line "b"
		else print first_line "," first_line + counter - 1 "b"
		}
	}
