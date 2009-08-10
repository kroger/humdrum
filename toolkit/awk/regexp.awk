#####################################################################
##					REGEXP.AWK
# Programmed by: David Huron         Date: April, 1994
# Copyright (c) 1994 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 17/94	Tim Racinsky	Modified parse_command to work with getopts
#
#
#	This program is called from regexp.ksh and is used to test
# user-defined regular expressions using a test text file.
#
# Main Functions used:
#	get_textfile()
#
#	VARIABLES:
#
#  -pattern : holds the current user-specified regular expression
#  -leading_space : string variable containing only leading spaces
#  -trailing_space : string variable containing only trailing spaces
#  -spaces : a temporary variable holding the number of spaces for padding
#  -lines : stores the number of lines in the input text file
#
#	ARRAYS:
#
#  -line[] stores successive lines of text in the input text file
#
BEGIN {
	get_textfile()
}
{
pattern = $0
# if (NR == 1) get_textfile()
#
# If no input is given, print all of the test text lines.
if (pattern == "")
	{
	for (i=1; i<=lines; i++) print "	" line[i]
	next
	}
for (i=1; i<=lines; i++)
	{
	if (line[i] ~ pattern)
		{
		print "	" line[i]
		match(line[i], pattern)	# The match function sets RSTART & RLENGTH.
		spaces = RSTART
		while (spaces > 1)
			{
			leading_space = leading_space " "
			spaces--
			}
		if (RLENGTH == 1 || RLENGTH == 0) print "	" leading_space "^"
		else
			{
			spaces = RLENGTH
			while (spaces > 2)
				{
				trailing_space = trailing_space " "
				spaces--
				}
			print "	" leading_space "^" trailing_space "^"
			}
		leading_space = trailing_space = ""
		}
	}
print ""
}
function get_textfile()
	{
	while (getline < ARGV[1] > 0) # (Awk automatically generates
		{			# an error message if the
		lines++	 	# test file is not found.)
		line[lines] = $0
		}
	ARGV[1] = ""
	return
	}
