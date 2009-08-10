#####################################################################
##					PATTERN.AWK
# Programmed by: Tim Racinsky         Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#    Date:   	Programmer:		Description:
# May 24/94	Tim Racinsky		Added file name to output
#							Added multiple file capability
# June 7/94	Tim Racinsky		Modified parse_command to work with getopts
#
#
#	This program is called from pattern.ksh and allows the user to search
# for a pattern in the input file specified by a template file.
#
# Options:
#	s regexp - pattern becomes insensitive to data records with the
#		 	 given regular expression 
#	c - makes pattern sensitive to comments
#	i - makes pattern sensitive to interpretations
#	y - outputs 'yank' commands instead of regular output
#
# Main Functions used:
#	Parse_command()	Parse_pattern_array()		Match_record()
#	Check_for_start()	Check_for_finish()			Print_patterns()
#	Find_min()
#
#	VARIABLES:
#  -options:  holds options specified by the user.
#  -reg_exp:  holds regular expression if specified by the user.
#  -smallest_index:  holds smallest possible index of the pattern array that
#   constitutes a matched pattern.
#  -pattern_index:  holds number of patterns in pattern array.
#  -file_name:  holds value of current file name.
#  -first_file:  flat to indicate if this is first file processed or not.
#
#	ARRAYS:
#  -line_repeat:  holds repeat for symbols for each pattern in array pattern.
#  -pattern:  holds the patterns from user defined template.
#  -current_patterns:  holds patterns currently being matched.  This is a 
#   2-dimensional array where the first dimension is a unique pattern index and
#   the second dimension is the line number the pattern started on.  Each
#   array element contains the pattern index of the last pattern matched.
#  -final_array:  2-dimensional array holding the final matched patterns.  The
#   first dimension is the starting line number and the second dimension is the
#   finishing line number.  Each array element holds the number of patterns
#   found between those 2 particular line numbers.
#
BEGIN {
	USAGE="\nUSAGE: pattern -h                 (Help Screen)\n       pattern"\
		 "[-ciy] [-s regexp] -f template [file ...]"
	#
	# Global variables and settings
	#
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	options = ""
	reg_exp = ""
	smallest_index = last_index = 0
	pattern_index = 1
	file_name = ""
	first_file = TRUE
	#
	# Read in the user defined template
	#
	while (getline < ARGV[3] > 0)
		{
		last_index++
		pattern[last_index] = $0
		}
	ARGV[3] = ""
	#
	# Determine the user's options and parse the user's template
	#
	parse_command()
	parse_pattern_array()
	}
{
#
# Check to see if a new file has started and if so, reset variables
#
if (FNR == 1)
	{
	if (first_file) first_file = FALSE
	#
	# Print the patterns from the last file
	#
	else
		{
		print_patterns()
		for (i in current_patterns) delete current_patterns[i]
		for (i in final_array) delete final_array[i]
		}
	if (FILENAME == "-") file_name = "STNDIN"
	else file_name = FILENAME
	}
#
# Skip over any lines the user wishes to ignore.  And skip over comments
# and interpretations unless the user specifies otherwise.
#
if (options ~ /s/ && $0 ~ reg_exp) next
else if ($0 ~ /^\!/ && options !~ /c/) next
else if ($0 ~ /^\*/ && options !~ /i/) next
#
# Otherwise, check the current record for the next patterns.
#
else 
	{
	match_record()
	check_for_start()
	check_for_finish()
	}
}
END	{
	#
	# Print the patterns in the appropriate format
	#
	print_patterns()
	}

############################################################################
#				FUNCTIONS USED IN PATTERN.AWK
############################################################################

############################################################################
##				Function Parse_command
# This function inspects the array ARGV in order to determine the correct
# options that the user has specified.
#
function parse_command()
	{
	#
	# Otherwise, set the 'options' variable
	#
	if (ARGV[2] != "null") options = ARGV[2]
	if (ARGV[4] != "null") reg_exp = ARGV[4]
	ARGV[1] = ARGV[2] = ARGV[4] = ""
	}

#########################################################################
##			Function Parse_pattern_array
#	This function loops through the pattern array and checks for any 
# 'repeat line' symbols.  These symbols should be separated from the rest
# of the pattern by a tab and are either a '*', '?', '+', or nothing.
# The array repeat_array is then created depending on these symbols.
#
function parse_pattern_array(     j,arrayb,count,VALID)
	{
	#
	# Loop through each position in the pattern array
	#
	for (j in pattern)
		{
		#
		# Split the pattern array variable to separate the special symbols
		#
		count = split(pattern[j],arrayb)
		#
		# If there is a special symbol, create the appropriate element 
		# in repeat_array
		#
		if (count == 2)
			{
			if (arrayb[2] ~ /\?/) line_repeat[j] = "?"
			else if (arrayb[2] ~ /\+/) line_repeat[j] = "+"
			else if (arrayb[2] ~ /\*/) line_repeat[j] = "*"
			else
				{
				print "pattern: ERROR: Invalid repetition character in "\
					 "expression " pattern[j] "." > stderr
				exit
				}
			}
		else line_repeat[j] = "NULL"
		#
		# If any "\t" characters occur in the pattern, replace them with
		# tab characters
		#
		gsub(/\\t/,"	",arrayb[1])
		pattern[j] = arrayb[1]
		for (j in arrayb) delete arrayb[j]
		}
	#
	# Check that at least one of the patterns specifes that a match
	# must be found (Otherwise, the 'null' line is a match)
	#
	VALID = FALSE
	for (j = last_index; j >= 1; j -= 1)
		{
		if (line_repeat[j] == "+" || line_repeat[j] == "NULL")
			{
			VALID = TRUE
			smallest_index = j
			break
			}
		}
	if (!VALID)
		{
		print "pattern: ERROR: One pattern must specify that at least one\n"\
			 "                match must be found." > stderr
		exit
		}
	}

#############################################################################
##				Function Match_record
#	This function loops through the patterns that are currently being formed
# to see if the current record continues the pattern.
#
function match_record(   i,pattern_number,subscripts,num_of_patterns\
										,current_index,temp_array)
	{
	#
	# Store the indices of the current patterns in a temporary array
	#
	num_of_patterns = 0
	for (i in current_patterns)
		{
		num_of_patterns += 1
		temp_array[num_of_patterns] = i
		}
	#
	# Loop through each current pattern
	#
	for (i = 1; i <= num_of_patterns; i += 1)
		{
		current_index = temp_array[i]
		#
		# Obtain the subscripts of the index
		#
		split(current_index,subscripts,SUBSEP)
		pattern_number = current_patterns[current_index]
		#
		# If the last pattern was a 'multiple' pattern check to see if the
		# current record continues that pattern
		#
		if (line_repeat[pattern_number] == "*" \
							|| line_repeat[pattern_number] == "+")
			{
			if ($0 !~ pattern[pattern_number])
				delete current_patterns[current_index]
			}
		#
		# Otherwise, delete the pattern since it has already been checked
		# in the function check_for_finish()
		#
		else delete current_patterns[current_index]
		#
		# Move on to the next pattern in the pattern array (if it exists)
		#
		pattern_number += 1
		if (pattern_number <= last_index)
			{
			#
			# If the pattern matches, create a new element in the pattern
			# array so that this pattern may be matched
			#
			if ($0 ~ pattern[pattern_number])
				{
				current_patterns[pattern_index,subscripts[2]] \
												= pattern_number
				pattern_index += 1
				}
			#
			# Continue to search for new patterns if the repeat 
			# symbols permit it
			#
			while (line_repeat[pattern_number] == "*" \
								|| line_repeat[pattern_number] == "?")
				{
				pattern_number += 1
				if (pattern_number <= last_index)
					{
					if ($0 ~ pattern[pattern_number])
						{
						current_patterns[pattern_index,subscripts[2]] \
												= pattern_number
						pattern_index += 1
						}
					}
				else break
				}
			}
		}
	}

#############################################################################
##				Function Check_for_start
#	This function checks to see if any patterns start on the current line.
#
function check_for_start(      i)
	{
	#
	# Loop through the array 'pattern'
	#
	for (i = 1; i <= last_index; i += 1)
		{
		#
		# If there is a match, start a new pattern
		#
		if ($0 ~ pattern[i])
			{
			current_patterns[pattern_index,FNR] = i
			pattern_index += 1
			}
		#
		# Continue until the repeat symbols do not permit it any longer
		#
		if (line_repeat[i] != "*" && line_repeat[i] != "?") break
		}
	}

#############################################################################
##				Function Check_for_finish
#	This function checks to see if a pattern can be counted as finished.
#
function check_for_finish(      i,pattern_number,subscripts)
	{
	#
	# Loop through all current patterns
	#
	for (i in current_patterns)
		{
		split(i,subscripts,SUBSEP)
		pattern_number = current_patterns[i]
		#
		# If the pattern's currently matched index qualifies as a 
		# matched pattern, then store it in 'final_array'
		#
		if (pattern_number >= smallest_index)
			{
			if ((subscripts[2],FNR) in final_array)
				final_array[subscripts[2],FNR] += 1
			else
				final_array[subscripts[2],FNR] = 1
			}
		}
	}

####################################################################
##				Function Print_patterns
#	This function is called from the END block and is responsible for
# printing all of the patterns that were found in sequence.
#
function print_patterns(   i,current_index,subscripts)
	{
	current_index = find_min()
	while (current_index != 0)
		{
		split(current_index,subscripts,SUBSEP)
		if (options ~ /y/)
			print "yank -l -r '" subscripts[1] "-" subscripts[2] "'"\
				 " " file_name
		else
			{
			#
			# If only one pattern was found for this particular set of lines,
			# then print that one pattern was found
			#
			if (final_array[current_index] == 1)
				print "1 pattern found from line " subscripts[1] " to "\
				      "line " subscripts[2] " of file " file_name "."
			#
			# Otherwise, print the number of patterns found
			#
			else
				print final_array[current_index] " patterns found from "\
				      "line " subscripts[1] " to line " subscripts[2] \
					 " of file " file_name "."
			}
		delete final_array[current_index]
		#
		# Find the next smallest index.
		#
		current_index = find_min()
		}
	}

###################################################################
##				Function Find_min
#	This function finds the pattern starting at the earliest line number.
#
function find_min(    minimum,i,min_subscripts,subscripts)
	{
	minimum = 0
	for (i in final_array)
		{
		split(i,subscripts,SUBSEP)
		#
		# If minimum has not been set yet, set it to the first element
		#
		if (minimum == 0)
			{
			minimum = i
			min_subscripts = subscripts[1]
			}
		#
		# Otherwise, check to see if the current element (pattern) begins
		# on an earlier line number
		#
		else if (subscripts[1] < min_subscripts)
				{
				minimum = i
				min_subscripts = subscripts[1]
				}
		}
	return minimum
	}
