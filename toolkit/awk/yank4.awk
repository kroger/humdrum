####################################################################
##					YANK4.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       	Description:
# May 16/94	Tim Racinsky		Removed Error when no regular expression is
#							found to retain 'Silence is Golden'
# May 16/94	Tim Racinsky		Fixed error when printing only anacrusis 
#							material (i.e. 0 is only number in range)
# June 7/94	Tim Racinsky		Modified parse_command for getopts
#
#
#	This program implements the 'o' option or, the 'ordinal' option,
# of the yank program. It is called from the program yank.ksh.
# It allows the user to specify a regular expression (or matching pairs of
# regular expressions with the -e option) so that marked measures are
# yanked.  'Ordinal' counting begins at the start of the file and carries
# on until the end of the file.  As with all yank programs, comments 
# and interpretations within the yanked material are output as well
# as all interpretations before the yanked material.
#
# Main Functions used:
#	Parse_command()			Determine_range()
#	Separate_number_range()		Parse_dollar_range()
#	Start_printing() 			Stop_printing()
#	Determine_number_of_fields()	Store_indicators()
#	Process_indicators()		Terminate()
#
# Helper Functions used:
#	Print_records_to_cut()		Print_possible_lines()
#
#	The program findpair.awk is used to pre-parse each input file and
# determine the segments of data which can be yanked.
#
#	VARIABLES:
#
#  -RANGE:  holds the range specified by the user from either the command
#   line or a file.
#  -REGEXP1:  holds the starting regular expression to search for.
#  -REGEXP2:  holds the ending regular expression to search for (this may
#   be the same as the starting regular expression.
#  -COMMENTS:  flag to determine if '-c' option has been selected.
#  -PRINTING:  flag to indicate if data records are currently being 
#   printed or not.
#  -FIRST_MATCH_FOUND:  flag to indicate when first segment is yanked.
#  -ZERO_RANGE:  flag to indicate when first regular expression is found.
#   (Note the difference between extracting the first segment and finding
#   the first record with the regular expression).
#  -current_file:  holds the name of the current file being processed.
#  -reg_exp_count:  holds the number of actual segments found so far.
#  -max_number:  holds the maximum number of segments that can be yanked.
#  -last_number:  holds the number of the last segment that the user wishes
#   to yank from the current file.
#  -number_of_fields:  holds the number of currently_active_spines.
#  -spine_path_record: flag to indicate if current interpretation record
#   is a spine-path record or not.
#  -options: holds either just the 'o' or the 'oe' options.
#
#	ARRAYS:
#
#  -possible_lines:  holds the line numbers that each possible segment to
#   yank begins on.
#  -records_to_cut:  holds which segments are to be yanked.  Each subscript
#   represents the n'th segment.
#  -path_indicator:  standard array to process spine-path indicators.
#
BEGIN {
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	NUMBER = "^(0|[1-9][0-9]*)$"
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	RANGE = ""
	REGEXP1 = ""
	REGEXP2 = ""
	current_file = ""
	COMMENTS = FALSE
	options = ""
	parse_command()
	}
{
#
# If the variable current_file has been set to the current filename, then
# skip through the rest of the records of that file.
#
if (current_file == FILENAME) next
#
# If looking at the first record of the current file
#
if (FNR == 1)
	{
	#
	# Reset all variables for the new file
	#
	PRINTING = FIRST_MATCH_FOUND = FALSE
	ZERO_RANGE = TRUE
	reg_exp_count = max_number = line_number = last_number = 0
	number_of_fields = 0
	for (j in possible_lines) delete possible_lines[j] 
	for (j in records_to_cut) delete records_to_cut[j] 
	#
	# Use the program 'findpair.awk' to find all pairs of regular
	# expressions in the current input file
	#
	("echo $TMPDIR/temp-$$-" ++i) | getline tmpname
	system("$AWK_VER -f ${HUMDRUM}/bin/findpair.awk '" REGEXP1 "' '" REGEXP2 "' " FILENAME " > " tmpname)
	#
	# Store the numbers in the array possible_lines
	#
	while (("cat " tmpname) | getline line_number > 0)
		{
		possible_lines[line_number] = ""
		max_number++
		}
	system("rm " tmpname)
	#
	# If there are no occurrences of REGEXP in the current file, print error
	#
	if (max_number == 0)
		{
		#print "yank: ERROR: Regular expression not found "\
		#	 "in " FILENAME > stderr
		exit
		}
	#
	# Otherwise resolve the range passed by the user for this file
	#
	else determine_range(RANGE)
	}
#
# If the current record matches the first regular expression
#
if ($0 ~ REGEXP1)
	{
	if (ZERO_RANGE) ZERO_RANGE = FALSE
	#
	# This special case only occurs when the user wants to print out the
	# anacrusis material only
	#
	if (last_number == 0)
		{
		if ($0 !~ /^!!/) number_of_fields = NF
		if ($0 ~ /^\*/) determine_number_of_fields()
		terminate()
		next
		}
	#
	# If not currently printing a measure
	#
	if (!PRINTING)
		{
		#
		# If the current record is a possible start to a measure
		#
		if (FNR in possible_lines)
			{
			reg_exp_count++
			#
			# If the user has selected to print this measure, start printing
			#
			if (reg_exp_count in records_to_cut)
				{
				start_printing()
				next
				}
			}
		}
	}
#
# If the current record matches the second regular expression and printing
# was taking place, stop the printing of the measure
#
if ($0 ~ REGEXP2 && (PRINTING || (ZERO_RANGE && (0 in records_to_cut))))
	{
	stop_printing()
	next
	}
#
# Otherwise, if there were no matches and this is an interpretation record
# then just print the record.
#
if ($0 ~ /^\*/)
	{
	print $0
	determine_number_of_fields()
	}
#
# If this is a comment record
#
else if ($0 ~ /^!/)
	{
	#
	# If the first match has not been found
	#
	if (!FIRST_MATCH_FOUND)
		{
		#
		# Print if -c options specified or if the user wishes to print
		# the anacrusis section
		#
		if ((COMMENTS) || ((ZERO_RANGE) && (0 in records_to_cut)))
			{ 
			print $0
			if ($0 !~ /^!!/) number_of_fields = NF
			}
		}
	#
	# Otherwise, print or store the record
	#
	else
		{
		print $0
		if ($0 !~ /^!!/) number_of_fields = NF
		}
	}
#
# If this is a data field
#
else
	{
	#
	# Print the record if currently printing
	#
	if (FIRST_MATCH_FOUND)
		{
		if (PRINTING) { print $0 ; number_of_fields = NF }
		}
	#
	# Print if anacrusis section specified by the user
	#
	else if ((ZERO_RANGE) && (0 in records_to_cut))
		{
		print $0
		number_of_fields = NF
		}
	}
}

###########################################################################
#			 	FUNCTIONS IN YANK4.AWK
###########################################################################

######################################################################
##				Function Parse_Command
#	This function determines what options the user wants
# If the 'o' option is specified, only one regular expression will follow.
# If the 'e' option is specified, two distinct regular expressions will follow.
#
function parse_command()
	{
	if (ARGV[2] ~ /c/) COMMENTS = TRUE
	REGEXP1 = ARGV[3]
	RANGE = ARGV[4]
	if (ARGV[2] ~ /e/)
		{
		REGEXP2 = ARGV[5]
		options = "e"
		}
	else REGEXP2 = REGEXP1
	ARGV[1] = ARGV[2] = ARGV[3] = ARGV[4] = ARGV[5] = ""
	}

#################################################################
##				Function Determine_range
#	This function determines the range of measures to yank given
# the RANGE variable and the number of measures in the current file
#
function determine_range(range_string,    arraya,j)
	{
	#
	# Split the RANGE variable according to "," so that each individual
	# range may be parsed correctly
	#
	split(range_string,arraya,",")
	for (j in arraya)
		{
		#
		# If the current range contains a "$" then parse as a dollar range
		#
		if (arraya[j] ~ /\$/) parse_dollar_range(arraya[j])
		#
		# Otherwise, if it contains a "-", then it must be a number range
		#
		else if (arraya[j] ~ /[-]/) separate_number_range(arraya[j])
		#
		# Otherwise, it must be a single number
		#
		else if (arraya[j] !~ NUMBER)
			{
			print "yank: ERROR: Invalid number given: " arraya[j] \
				 > stderr
			exit
			}
		else 
			{
			if (arraya[j] <= max_number)
				{
				if (arraya[j]+0 > last_number) last_number = arraya[j]+0
				records_to_cut[arraya[j]] = ""
				}
			}
		}
	}

##################################################################
##			Function Parse_dollar_range
#	This function takes a 'dollar' range and resolves the actual
# range based on the variable 'max_number' which represents the 
# total number of measures.
#
function parse_dollar_range(current_string,       arrayb,j,num1,num2)
	{
	#
	# Place each number or dollar sign in a separate array element
	#
	split(current_string,arrayb,"-")
	#
	# There should not be more than 4 elements in the array
	#
	if ("5" in arrayb)
		{
		print "yank: ERROR: Invalid range given: " \
			  current_string  > stderr
		exit
		}
	#
	# If there are 4 elements in the array
	#
	if ("4" in arrayb)
		{
		#
		# Must be of form '$-NUMBER1-$-NUMBER2' with NUMBER1 > NUMBER2
		#
		if ((arrayb[1] != "$" || arrayb[3] != "$") \
			|| (arrayb[2] !~ NUMBER || arrayb[4] !~ NUMBER) \
			|| (arrayb[2] < arrayb[4]))
			{
				print "yank: ERROR: Invalid range given: " \
					  current_string  > stderr
				exit
			}
		else
			{ 
			if (max_number-arrayb[2] < 0) num1 = 0
			else num1 = max_number-arrayb[2]
			if (max_number-arrayb[4] < 0) num2 = 0
			else num2 = max_number-arrayb[4]
			for (j = num1; j <= num2; j++)
				{
				if (j > last_number) last_number = j	
				records_to_cut[j] = ""
				}
			}
		}
	#
	# If there are 3 elements, there are 2 cases to deal with
	#
	else if ("3" in arrayb)
		{
		#
		# First case of form '$-NUMBER-$'
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
				if (max_number-arrayb[2] < 0 ) num1 = 0
				else num1 = max_number-arrayb[2]
				for (j = num1; j <= max_number+0; j++)
					{
					if (j > last_number) last_number = j
					records_to_cut[j] = ""
					}
				}
			}
		#
		# Second case of form 'NUMBER-$-NUMBER'
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
			#
			# First number must precede second number
			#
			else if (arrayb[1]+0 <= max_number-arrayb[3])
				{
				for (j=arrayb[1]+0; j<=max_number-arrayb[3];j++)
					{
					if (j > last_number) last_number = j
					records_to_cut[j] = ""
					}
				}
			}
		}
	#
	# If there are 2 elements, also 2 cases
	#
	else if ("2" in arrayb)
		{
		#
		# First case of form '$-NUMBER'
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
			# If it is a non-negative reference, add the subscript
			#
			else if (max_number - arrayb[2]+0 >= 0)
				{
				if (max_number-arrayb[2]+0 > last_number)
					last_number = max_number-arrayb[2]+0
				records_to_cut[max_number-arrayb[2]+0] = ""
				}
			}
		#
		# Second case of form 'NUMBER-$'
		#
		else
			{
			if (arrayb[1] !~ NUMBER || arrayb[2] != "$")
				{
				print "yank: ERROR: Invalid range given: " \
					  current_string > stderr
				exit
				}
			else	if (arrayb[1]+0 <= max_number)
				{
				for (j = arrayb[1]+0; j <= max_number+0; j++)
					{
					if (j > last_number) last_number = j
					records_to_cut[j] = ""
					}
				}
			}
		}
	#
	# If there is only one element, it must be a dollar sign
	#
	else if (arrayb[1] != "$")
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	else
		{
		if (last_number != max_number) 
			last_number = max_number
		records_to_cut[max_number] = ""
		}
	}

##################################################################
##		 	Function Separate_number_range
#	This function separates a range of numbers into individual components
#
function separate_number_range(current_string,    j,arrayc,num2)
	{
	#
	# Place each number into a separate array element
	#
	split(current_string,arrayc,"-")
	#
	# There cannot be more than 2 numbers in a range
	#
	if ("3" in arrayc)
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	#
	# Both must be valid numbers
	#
	else if (arrayc[1] !~ NUMBER || arrayc[2] !~ NUMBER \
			|| arrayc[1] > arrayc[2])
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}	
	#
	# Add the appropriate array subscripts to records_to_cut
	#
	else
		{
		if (arrayc[2]+0 > max_number) num2 = max_number
		else num2 = arrayc[2]+0
		if (arrayc[1]+0 <= max_number)
			{
			for (j = arrayc[1]+0; j <= num2; j++)
				{
				if (j > last_number) last_number = j
				records_to_cut[j] = ""
				}
			}
		}
	}

######################################################################
##				Function Start_printing
#	This function is called when a record is found that begins a 
# measure that the user wishes to yank
#
function start_printing(     i)
	{
	if (!FIRST_MATCH_FOUND) FIRST_MATCH_FOUND = TRUE
	#
	# Begin printing with this record
	#
	PRINTING = TRUE
	print $0
	if ($0 !~ /^!!/) number_of_fields = NF
	if ($0 ~ /^\*/) determine_number_of_fields()
	}

#######################################################################
##				Function Stop_printing
#	This function is called when a 'closing' expression is found so that
# the currently printed measure is stopped from printing
#
function stop_printing()
	{
	#
	# If this is the last segment to print, then terminate
	#
	if (reg_exp_count >= last_number)
		{
		if (options ~ /e/) print $0
		if ($0 !~ /^!!/) number_of_fields = NF
		if ($0 ~ /^\*/) determine_number_of_fields()
		terminate()
		}
	#
	# If the 'closing' expression is not the same as the 'opening' expression
	#
	else if (REGEXP1 != REGEXP2)
		{
		#
		# Print the record and set the variables
		#
		print $0
		if ($0 !~ /^!!/) number_of_fields = NF
		if ($0 ~ /^\*/) determine_number_of_fields()
		PRINTING = FALSE
		}
	#
	# If matching pairs are the same
	#
	else
		{
		#
		# If the current record that closes a measure is also one
		# that opens a measure, start printing again
		#
		reg_exp_count++
		if (reg_exp_count in records_to_cut)
			{
			start_printing()
			}
		#
		# Otherwise, just print the record and reset the variables
		#
		else
			{
			if (options ~ /e/) print $0
			if ($0 !~ /^!!/) number_of_fields = NF
			if ($0 ~ /^\*/) determine_number_of_fields()
			PRINTING = FALSE
			}
		}	
	}

########################################################################
##			Function Determine_number_of_fields
#	This function is used to determine if the current spine-path record
# consists of all spine-path terminators.  If it does, then the variables
# must be reset.  Otherwise, the number of resulting spines is calculated.
#
function determine_number_of_fields(   j,spine_path_record)
	{
	number_of_fields = NF
	spine_path_record = FALSE
	for (i = 1; i <= NF ; i++)
		{			
		if ($i ~ /^\*\+$|^\*\-$|^\*\^$|^\*v$|^\*x$/)
			{
			spine_path_record = TRUE 
			break
			}
		}
	#
	# If it is a spine-path record, determine how many spines 
	# are added (or deleted) from the current number of fields
	#
	if (spine_path_record)
		{
		store_indicators()
		process_indicators()
		}
	if (number_of_fields == 0)
		{
		#
		# Reset all variables
		#
		FIRST_MATCH_FOUND = PRINTING = FALSE
		}	
	}

##############################################################
##			Function Store_indicators
#	This function allows the spine-path indicators for the 
# current record to be stored in the array 'path_indicator' so
# that they may be used later.
#
function store_indicators(   i)
	{
	for (i = 1; i <= NF; i++)
		{
		if ($i == "*") path_indicator[i] = "*"
		else path_indicator[i] = substr($i,2,1)
		}
	}

##################################################################
##			Function Process_indicators
#	This function takes the spine-path indicators that were stored
# in the array 'path_indicator' in the function 'store_indicators'
# and manipulates the variable number_of_fields to reflect the number
# of fields that results from the spine-path indicators.
#
function process_indicators(    i)
	{
	for (i = 1; i <= NF; i++)
		{
		#
		# A field has been added or split, increase number of fields
		#
		if (path_indicator[i] == "^" || path_indicator[i] == "+")
			number_of_fields++
		#
		# A field has been terminated, decrease number of fields
		#
		if (path_indicator[i] == "-")
			number_of_fields--
		#
		# For join-path indicators, decrease number of fields only if
		# next indicator is also for join-path
		#
		if (path_indicator[i] == "v")
			{
			if (path_indicator[i+1] == "v")
				number_of_fields--
			}
		}
	}

#####################################################################
##				Function Terminate
#	This function is called when it is time to terminate processing
# this file and move on to the next file (or quit the program)
#
function terminate(    lastline)
	{
	#
	# Create and print the terminating record
	#
	for (j = 1; j <= number_of_fields; j++) lastline = lastline "*-\t"
	if (lastline != "") print substr(lastline,1,length(lastline)-1)
	current_file = FILENAME
	}
