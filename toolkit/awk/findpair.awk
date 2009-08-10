#########################################################################
##					FINDPAIR.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
#
#
#	This program is used by yank4.awk to printout the line numbers
# that desired measures start on.
#
# Uses function Check_terminate()
#
#	VARIABLES:
#
#  -FOUND:  flag to indicate if regular1 has been found when regular1 
#   is not equal to regular2.
#  -FIRST_MATCH:  flag to indicate if the first match has been found for
#   regualar1 when regular1 = regular2.
#  -regular1:  holds the first regular expression of a pair to search for.
#  -regular2:  holds the secod regular expression to search for (this may
#   be the same as regular1.
#  -record_number:  holds the line number of the record that matches regular1.
#   It is only printed if it begins a valid segment.
#
# FALSETE: A 'valid' segment is a segment that has a proper beginning record
#	(one that matches regular1) and ending record (one that matches regular2).
#
BEGIN { 
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	FOUND = FALSE
	FIRST_MATCH = TRUE
	regular1 = ARGV[1] 
	regular2 = ARGV[2]
	ARGV[1] = ARGV[2] = ""
	record_number = 0
	}
{
#
# If we are looking for pairs of different regular expressions
#
if (regular1 != regular2)
	{
	#
	# If the record matches the first regular expression and a match is 
	# currently not in progress, store this number
	#
	if ($0 ~ regular1 && !FOUND)
		{
		FOUND = TRUE
		record_number = FNR
		check_terminate()
		}
	#
	# If the record matches the second regular expression and a match is
	# currently in progress reset the variable and print the starting number
	#
	else if ($0 ~ regular2 && FOUND)
		{
		print record_number
		FOUND = FALSE
		}
	#
	# Check to see if the current section terminates
	#
	else if ($0 ~ /^\*/) check_terminate()
	}
#
# If searching for pairs of the same regular expression, print out all
# line numbers that begin a valid segment.
#
else
	{
	#
	# If the current record matches the regular expression
	#
	if ($0 ~ regular1)
		{
		#
		# If this is a first match after the beginning of a file or after
		# all spines have been terminated and then restarted.
		#
		if (FIRST_MATCH)
			{
			FIRST_MATCH = FALSE
			record_number = FNR
			check_terminate()
			}
		#
		# Otherwise print the number and store the current one as well
		#
		else
			{
			print record_number
			record_number = FNR
			check_terminate()
			}
		}
	else if ($0 ~ /^\*/) check_terminate()
	}
}

########################################################################
##				Function Check_terminate
#	This function determines if all spines terminate.  If so, then the
# current FOUND and FIRST_MATCH variables are reset
#
function check_terminate(   j,do_terminate)
	{
	do_terminate = TRUE
	for (j = 1; j <= NF; j++)
		{	
		if ($j != "*-")
			{
			do_terminate = FALSE
			break
			}
		}
	if (do_terminate)
		{
		FOUND = FALSE
		FIRST_MATCH = TRUE
		}
	}
