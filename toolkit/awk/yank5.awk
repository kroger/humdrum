####################################################################
##					YANK5.AWK
# Programmed by: Tim Racinsky        Date: May, 1994
# Copyright (c) 1994 David Huron
#
# Modifications:
#   Date:      Programmer:       	Description:
# June 7/94	Tim Racinsky	Modified parse_command to operate with getopts
#
#
#	This program implements the 's' option or, the 'section label' option,
# of the yank program. It is called from the program yank.ksh.
# As with all yank programs, comments and interpretations within the yanked 
# material are output as well as all interpretations before the yanked material.
#
# Main Functions used:
#	parse_command()		determine_range()		parse_dollar_range()
#	separate_number_range()	process_interpretations()
#
# The program number.awk is also used to assist in finding all occurrences
# of the specified regular expression before any records are processed.
#
#	VARIABLES:
#
#  -NUMBER:  Regular expression to match a number (non-zero)
#  -section_label:  Regular expression to match section label interp. record
#  -exclusive_record:  Regular expression to match exclusive interp. record
#  -spine_terminate:  Regular expression to match termination record
#  -RANGE:  Holds range of labels to extract as specified by the user
#  -LABEL:  Holds the section label to extract as specified by the user
#  -COMMENTS:  Flag to determine whether extra comments should be printed
#  -occurrences_count:  Keeps track of how many section labels corresponding
#   to the users selection have been seen.
#  -last_occurrence:  Holds number (<= max_occurrence) of last label that the
#   user wants to print
#  -max_occurrence:  Holds maximum number of specified section labels in file
#  -number_of_fields:  Holds last number of fields printed
#
#	ARRAYS:
#
#  -which_to_print:  Each index of this array is a number corresponding to one
#   of the specified section labels.  These are the ones that will be output.
#
BEGIN {
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	#
	# Define regular expressions
	#
	NUMBER = "^[1-9][0-9]*$"
	s_label = "[[:alnum:] _+.#@~-]"
	section_label = "^\\*>[^\\]\\[,\t]+(\t\\*>" s_label "+)*$"
	exclusive_record = "^\\*\\*[^\t]+(\t\\*\\*[^\t]+)*$"
	spine_terminate = "^\\*-(\t\\*-)*$"
	#
	# Set other global variables and parse the command line
	#
	RANGE = ""
	LABEL = ""
	COMMENTS = FALSE
	number_of_fields = 0
	parse_command()
	}
{
#
# For each new file, reset the flags and determine the occurrences of the 
# user defined section label
#
if (FNR == 1)
	{
	if (number_of_fields > 0)
		{
		last_line = "*-"
		for (i = 1; i <= number_of_fields - 1; i += 1)
			last_line = last_line "\t*-"
		print last_line
		}
	FIRST_PRINT = FALSE
	PRINTING = FALSE
	MORE_TO_PRINT = TRUE
	last_occurrence = 0
	occurrence_count = 0
	# system("$AWK_VER -f ${HUMDRUM}/bin/number.awk '" LABEL "' " FILENAME " > $TMPDIR/yank5$")
	# "cat $TMPDIR/yank5$" | getline max_occurrences
	# close("cat $TMPDIR/yank5$")
	#max_occurrences = max_occurrences + 0
    ("$AWK_VER -f ${HUMDRUM}/bin/number.awk '" LABEL "' " FILENAME ) | getline max_occurrences
	determine_range(RANGE,max_occurrences)
	}
#
# Print comments only if the first section has been output and there are more
# labels to print or they specified -c option and there are more labels to print
#
if ($0 ~ /^!/ && (FIRST_PRINT || COMMENTS) && MORE_TO_PRINT)
	print $0
#
# If there is an interpretation, process it
#
else if ($0 ~ /^\*/)
	process_interpretations()
#
# For all other records, if this section is printing, then print it too
#
else if (PRINTING)
	print $0
}
#
# Be sure to remove the temporary file created.
#
END	{
	if (number_of_fields > 0)
		{
		last_line = "*-"
		for (i = 1; i <= number_of_fields - 1; i += 1)
			last_line = last_line "\t*-"
		print last_line
		#system("rm $TMPDIR/yank5$")
		}
}

###########################################################################
#				 FUNCTIONS IN YANK5.AWK
###########################################################################

######################################################################
##				Function Parse_Command
#	This function simply determines which options the user has specified.
#
function parse_command()
	{
	if (ARGV[2] ~ /c/) COMMENTS = TRUE
	LABEL = ARGV[3]
	RANGE = ARGV[4]
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ""
	#
	# Prepend a '*>' to LABEL if the user did not do so on command line
	#
	if (LABEL !~ /^\*>/) LABEL = "\\*>" LABEL
	else sub(/^\*/,"\\*",LABEL)
	}

###########################################################################
## 				function process_interpretations
#  This function will process the interpretation records depending on the 
# choices made by the user.
#
function process_interpretations()
	{
	#
	# If this record is a section label record
	#
	if ($0 ~ section_label)
		{
		#
		# If currently printing a section, stop
		#
		if (PRINTING) PRINTING = FALSE
		#
		# If there are no more sections to print, set the flag to show this
		#
		if (occurrence_count == last_occurrence)
			{
			MORE_TO_PRINT = FALSE
			number_of_fields = NF
			}
		#
		# If this record matches the one specified by the user
		#
		if ($0 ~ LABEL && MORE_TO_PRINT)
			{
			#
			# Increase the current count of occurrences and determine if
			# this is a section that should be printed
			#
			occurrence_count += 1
			if (occurrence_count in which_to_print)
				{
				PRINTING = TRUE
				if (!FIRST_PRINT) FIRST_PRINT = TRUE
				print $0
				}
			}
		}
	#
	# If it's not a section label, then stop printing a currently printing
	# section if it is an exclusive record or spine-terminating record
	#
	else if ($0 ~ exclusive_record || $0 ~ spine_terminate)
		{
		if (PRINTING) PRINTING = FALSE
		#
		# If there are no more sections to print, set the flag to show this
		#
		if (occurrence_count == last_occurrence)
			{
			MORE_TO_PRINT = FALSE
			number_of_fields = NF
			}
		if (MORE_TO_PRINT) print $0
		}
	else if (MORE_TO_PRINT) print $0
	}

#################################################################
##				Function Determine_range
#	This function determines which section labels will be printed out.  It
# needs to know the maximum number of section labels corresponding to the 
# users specifications.
#
function determine_range(range_string,max_range,    arraya,j)
	{
	#
	# Delete the old array subscripts
	#
	for (j in which_to_print) delete which_to_print[j]
	#
	# Place each separate range in an array element and loop through them
	#
	split(range_string,arraya,",")
	for (j in arraya)
		{
		#
		# If the current range contains a dollar sign, parse it
		#
		if (arraya[j] ~ /\$/) 
			parse_dollar_range(arraya[j],max_range)
		#
		# Otherwise, if the current range is a range of numbers, parse it
		#
		else if (arraya[j] ~ /[-]/)
			separate_number_range(arraya[j],max_range)
		#
		# Otherwise it must be a single number
		#
		else if (arraya[j] !~ NUMBER)
			{
			print "yank: ERROR: Invalid number given: " arraya[j] \
				 > stderr
			exit
			}
		else if (arraya[j]+0 <= max_range)
			which_to_print[arraya[j]] = ""
		}
	#
	# Determine which occurrence of the section label will be the last to
	# be printed out (as specified by the user in RANGE)
	#
	last_occurrence = 0
	for (j in which_to_print)
		{
		if (j > last_occurrence) last_occurrence = j
		}
	}

##################################################################
##			Function Parse_dollar_range
#	This function accepts a range containing dollar signs and the
# number of records it has to work with and it parses the range and
# creates the appropriate array subscripts in which_to_print
#
function parse_dollar_range(current_string,max_range,  arrayb,j,num1,num2)
	{
	#
	# Split the passed string according to the "-" and parse the array
	#
	split(current_string,arrayb,"-")
	#
	# There should not be more than 4 elements in the array
	#
	if ("5" in arrayb)
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	#
	# If there are four elements in the array
	#
	if ("4" in arrayb)
		{
		#
		# The 1st and 3rd elements must be dollars. The 2nd and 4th must
		# be numbers with the 2nd less than the 4th
		#
		if ((arrayb[1] != "$" || arrayb[3] != "$") \
			|| (arrayb[2] !~ NUMBER || arrayb[4] !~ NUMBER) \
			|| (arrayb[2] < arrayb[4]))
			{
			print "yank: ERROR: Invalid range given: " current_string  \
				 > stderr
			exit
			}
		else
			{
			#
			# Determine the proper range (must be valid or default to 0)
			# and add the appropriate subscripts.
			#
			if (max_range-arrayb[2] < 0) num1 = 0
			else num1 = max_range-arrayb[2]
			if (max_range-arrayb[4] < 0) num2 = 0
			else num2 = max_range-arrayb[4]
			for (j = num1; j <= num2; j++) which_to_print[j] = ""
			}
		}
	#
	# If there are only 3 elements in the array, there are 2 possible cases
	#
	else if ("3" in arrayb)
		{
		# 
		# First case, 1st and 3rd elements are a "$" and the 2nd must
		# be a number
		#
		if (arrayb[1] == "$")
			{
			if (arrayb[3] != "$" || arrayb[2] !~ NUMBER)
				{
				print "yank: ERROR: Invalid range given: " \
                               current_string > stderr
				exit
				}
			else
				{
				#
				# Determine range and add the subscripts
				#
				if (max_range-arrayb[2] < 0) num1 = 0
				else num1 = max_range-arrayb[2]
				for (j = num1; j <= max_range+0; j++)
					which_to_print[j] = ""
				}
			}
		#
		# Otherwise they must be inverted
		#
		else
			{
			if ((arrayb[2] != "$") \
				|| (arrayb[1] !~ NUMBER || arrayb[3] !~ NUMBER))
				{
				print "yank: ERROR: Invalid range given: " \
                          current_string > stderr
				exit
				}
			else
				{
				#
				# Only add the subscripts if the first number comes
				# before the second number
				#
				if (arrayb[1]+0 <= max_range-arrayb[3])
					{
					for (j=arrayb[1]+0;j<=max_range-arrayb[3];j++)
						which_to_print[j] = ""
					}
				}
			}
		}
	#
	# If there are 2 elements, again 2 cases
	#
	else if ("2" in arrayb)
		{
		#
		# If the first element is a dollar, then a single record
		# is being referenced and the 2nd element must be a number.
		#
		if (arrayb[1] == "$")
			{
			if (arrayb[2] !~ NUMBER)
				{
				print "yank: ERROR: Invalid range given: " \
					 current_string > stderr
				exit
				}
			#
			# If it is a positive reference, add the subscript
			#
			else if (max_range - arrayb[2]+0 >= 0)
				which_to_print[max_range-arrayb[2]+0] = ""
			}
		#
		# Otherwise, the order must be switched
		#
		else
			{
			if (arrayb[1] !~ NUMBER || arrayb[2] != "$")
				{
				print "yank: ERROR: Invalid range given: " \
					  current_string > stderr
				exit
				}
			#
			# The number must be less than the number of lines
			#
			else	if (arrayb[1]+0 <= max_range)
				{
				for (j=arrayb[1]+0; j <= max_range+0; j++)
					which_to_print[j] = ""
				}
			}
		}
	#
	# Otherwise, must be a single dollar sign
	#
	else if (arrayb[1] != "$") 
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	else which_to_print[max_range] = ""
	}

##################################################################
##		 	Function Separate_number_range
#	This function separates a range of numbers passed to it using the
# variable max_range and creates the appropriate array subscripts
#
function separate_number_range(current_string,max_range,    j,arrayc,num2)
	{
	#
	# Place each number in a separate element and then parse
	#
	split(current_string,arrayc,"-")
	#
	# If there are 3 elements, an error has occurred
	#
	if ("3" in arrayc)
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	#
	# Both elements must be numbers with the 1st not greater than the 2nd
	#
	else if ((arrayc[1] !~ NUMBER || arrayc[2] !~ NUMBER) \
			|| arrayc[1]+0 > arrayc[2]+0)
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	else
		{
		#
		# Find the appropriate range
		#
		if (arrayc[2] > max_range) num2 = max_range
		else num2 = arrayc[2]+0
		if (arrayc[1]+0 <= max_range)
			{
			for (j = arrayc[1]+0; j <= num2; j++)
				which_to_print[j] = ""
			}
		}
	}
