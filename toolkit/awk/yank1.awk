####################################################################
##					YANK1.AWK
# Programmed by: Tim Racinsky        Date: July, 1993
# Copyright (c) 1993 David Huron
#
# Modifications:
#   Date:      Programmer:       Description:
# June 7/94	Tim Racinsky	Modified parse_command to work with getopts
#
#
#	This program implements the 'l' option or, the 'line number' option,
# of the yank program. It is called from the program yank.ksh.
# It allows the user to specify which line numbers from the input he 
# wants to see on the output.  As with all yank programs, comments
# and interpretations within the yanked material are output as well as
# interpretations before the yanked material.
#
# Main Functions used:
#	Parse_command() 		Determine_range()
#	Parse_dollar_range()	Separate_number_range()
#	Store_indicators()		Process_indicators()
#
# Helper Functions used:
#	Print_records_to_cut()
#
#	VARIABLES:
#
#  -RANGE:  holds the range that the user specified (whether from the command
#   line or from a specified file).
#  -COMMENTS:  holds whether or not the user specified the '-c' option.
#  -last_number:  used to determine the last line number to be output according
#   to the user's specification (may need to evaluate '$').
#  -no_of_lines:  holds the total number of lines in the input file.
#  -number_of_fields:  holds the current number of active spines (of the last
#   line printed).  If the last line printed was a spine-path record,
#   then the number of fields must be calculated.
#  -FIRST_NUMBER_PRINTED:  a flag indicating when the first line number
#   specified by the user has been yanked.
#  -spine_path_record:  a flag indicating if the current interpretation
#   record is a spine-path record.
#  -new_path: a flag to indicate if a new path indicator is on the line
#   selected as the last line to be yanked.
#
#	ARRAYS:
#
#  -records_to_cut:  holds the line numbers that are to be yanked from the
#   current input file.  Each line number is a subscript in the array.
#
BEGIN {
	OPTIONS = "c"
	OPTIONS_REGEXP = "[" OPTIONS "]"
	OPTIONS_NEG_REGEXP = "[^" OPTIONS "]"
	USAGE = "\nUSAGE: yank -h|-?                    (Help Screen)\n       "\
		   "yank -l range [-c] [file ...]\n       yank -m regexp range [-c]"\
		   " [file ...]\n       yank -n regexp range [-c] [file ...]\n     "\
		   "  yank -o regexp [-e regexp] range [-c] [file ...]"
	FS = OFS = "\t"
	TRUE = 1; FALSE = 0
	#
	# Set standard error to the appropriate 'file'
	#
	#if (ARGV[0] == "nawk") stderr = "/dev/tty"
	#else stderr = "con"
	"echo $CON" | getline stderr
	close("echo $CON")
	NUMBER = "^[1-9][0-9]*$"
	RANGE = ""
	COMMENTS = FALSE
	#
	# Parse the command line options passed from yank.ksh
	#
	parse_command()
	}
{
#
# If looking at the first record of the current file
#
if (FNR == 1)
	{
	#
	# Reinitialize variables for each new file
	#
	FIRST_NUMBER_PRINTED = FALSE
	spine_path_record = FALSE
	no_of_lines = 0
	number_of_fields = 0
	last_number = 0
	#
	# Determine the number of lines of the current file and store it
	# Note that a different temporary file must be used for every
	# different input file (otherwise EOF is always true)
	#
	#system("$AWK_VER 'END{print NR}' " FILENAME " > $TMPDIR/linenum" ++i)
	#"cat $TMPDIR/linenum" i | getline no_of_lines
	#no_of_lines = no_of_lines + 0
	#system("rm $TMPDIR/linenum" i)
	  cmd = "$AWK_VER 'END{print NR}' " FILENAME
	  cmd | getline no_of_lines
	  close(cmd)
	#
	# Determine the lines of the current file to print
	#
	determine_range(RANGE)
	}
#
# If the current record is a record to be cut, then print it
#
if (FNR in records_to_cut) 
	{
	if (!FIRST_NUMBER_PRINTED) FIRST_NUMBER_PRINTED = TRUE
	print $0
	#
	# If the current record is an interpretation record, then determine
	# if it is a spine-path record or not
	#
	if ($0 ~ /^\*/)
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
		# If it is a spine-path record, determine how many spines are added
		# (or deleted) from the current number of fields
		#
		if (spine_path_record)
			{
			store_indicators()
			process_indicators()
			}
		}
	#
	# If this is the last record to be printed for this file
	#
	if (FNR == last_number)
		{
		#
		# Store the number of fields
		#
		if ($0 !~ /^!!/ && $0 !~ /^\*/) number_of_fields = NF
		#
		# Make sure the record following a new path indicator is printed
		#
		if ($0 ~ /^\*/)
			{
			new_path = FALSE
			for (j = 1; j <= number_of_fields; j++)
				{
				if ($j ~ /^\*\+$/)
					{
					new_path = TRUE
					break
					}
				}
			if (new_path)
				{
				getline
				print $0
				}
			}
		#
		# Create and print the terminating record
		#
		terminate = ""
		for (j = 1; j <= number_of_fields; j++) terminate = terminate "*-\t"
		if (terminate != "") print substr(terminate,1,length(terminate)-1)
		#
		# Loop through the rest of the records for this particular file
		#
		while (FNR < no_of_lines)
			{ if (getline != 1) break }
		}
	}
#
# Otherwise, if the current record is a comment, process it
#
else if ($0 ~ /^!/)
	{
	#
	# If the first line has not been printed yet
	#
	if (!FIRST_NUMBER_PRINTED)
		{
		#
		# Print only if the comments option is specified
		#
		if (COMMENTS) print $0
		}
	#
	# Otherwise print all comments
	#
	else print $0
	}
#
# Print all interpretations
#
else if ($0 ~ /^\*/)
	{
	print $0
	#
	# If the current record is an interpretation record, then determine
	# if it is a spine-path record or not
	#
	if ($0 ~ /^\*/)
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
		# If it is a spine-path record, determine how many spines are added
		# (or deleted) from the current number of fields
		#
		if (spine_path_record)
			{
			store_indicators()
			process_indicators()
			}
		}
	}
}

###########################################################################
#				FUNCTIONS IN YANK1.AWK
###########################################################################

######################################################################
##				Function Parse_Command
#	This function is used to determine which options the user selected from
# the list sent from yank.ksh
#
function parse_command()
	{
	if (ARGV[2] ~ /c/) COMMENTS = TRUE
	RANGE = ARGV[3]
	ARGV[1] = ARGV[2] = ARGV[3] = ""
	}

#################################################################
##			Function Determine_range
#	This function determines the exact line numbers that are to be
# yanked and creates the corresponding subscripts in records_to_cut.
#
function determine_range(range_string,    arraya,j)
	{
	#
	# Delete the old array subscripts
	#
	for (j in records_to_cut) delete records_to_cut[j]
	#
	# Each range should be separated by a comma
	#
	split(range_string,arraya,",")
	#
	# Loop through each range separately
	#
	for (j in arraya)
		{
		#
		# If there is a dollar in the current range parse it with
		# parse_dollar_range
		#
		if (arraya[j] ~ /\$/) parse_dollar_range(arraya[j])
		#
		# Otherwise, if there is a "-", then it must be a number range
		#
		else if (arraya[j] ~ /[-]/) separate_number_range(arraya[j])
		#
		# Otherwise, it must be a single number
		#
		else if (arraya[j] !~ NUMBER)
			{
			print "yank: ERROR: Invalid number given: " \
				  arraya[j] > stderr
			exit
			}
		else 
			{
			#
			# Create the array subscript and update the variable
			# last_number if necessary
			#
			if (arraya[j]+0 <= no_of_lines)
				{
				records_to_cut[arraya[j]] = ""
				if (arraya[j]+0 > last_number) last_number = arraya[j]+0
				}
			}
		}
	}

##################################################################
##			Function Parse_dollar_range
#	This function takes a range containing a dollar sign and parses
# it according to what a valid 'dollar' range is.
#
function parse_dollar_range(current_string,       arrayb,j,num1,num2)
	{
	#
	# Place each character or set of characters in a different array element
	#
	split(current_string,arrayb,"-")
	#
	# If there are 5 elements an invalid range has been specified
	#
	if ("5" in arrayb)
		{
		print "yank: ERROR: Invalid range given: " \
			 current_string > stderr
		exit
		}
	#
	# If there are four elements then the first and third must be dollar
	# signs and the second and fourth numbers with the 2nd > 4th
	#
	if ("4" in arrayb)
		{
		if ((arrayb[1] != "$" || arrayb[3] != "$") \
			|| (arrayb[2] !~ NUMBER || arrayb[4] !~ NUMBER) \
			|| (arrayb[2] < arrayb[4]))
			{
				print "yank: ERROR: Invalid range given: " \
					  current_string > stderr
				exit
			}
		else
			{
			#
			# Create the appropriate subscripts and update
			# last_number if necessary
			#
			if (no_of_lines-arrayb[2] < 1) num1 = 1
			else num1 = no_of_lines-arrayb[2]
			if (no_of_lines-arrayb[4] < 1) num2 = 1
			else num2 = no_of_lines-arrayb[4]
			for (j = num1; j <= num2; j++)
				{
				if (j > last_number) last_number = j	
				records_to_cut[j] = ""
				}
			}
		}
	#
	# If there are 3 elements, then there are 2 different cases
	#
	else if ("3" in arrayb)
		{
		#
		# If the first element is a dollar sign, then the third must
		# be too and the second must be a number
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
				# Create array subscripts and update last_number
				#
				if (no_of_lines-arrayb[2] < 1 ) num1 = 1
				else num1 = no_of_lines-arrayb[2]
				for (j = num1; j <= no_of_lines+0; j++)
					{
					if (j > last_number) last_number = j
					records_to_cut[j] = ""
					}
				}
			}
		#
		# Otherwise, the second element must be a dollar sign and
		# the 1st and 3rd must be numbers
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
			# Create array subscripts only if first number appears
			# before second number
			#
			else if (arrayb[1]+0 <= no_of_lines-arrayb[3])
				{
				for (j=arrayb[1]+0; j<=no_of_lines-arrayb[3];j++)
					{
					if (j > last_number) last_number = j
					records_to_cut[j] = ""
					}
				}
			}
		}
	#
	# If there are only 2 elements then there are 2 cases
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
			else if (no_of_lines - arrayb[2]+0 > 0)
				{
				if (no_of_lines - arrayb[2]+0 > last_number)
					last_number = no_of_lines - arrayb[2]+0
				records_to_cut[no_of_lines - arrayb[2]] = ""
				}
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
			else	if (arrayb[1]+0 <= no_of_lines)
				{
				for (j = arrayb[1]+0; j <= no_of_lines+0; j++)
					{
					if (j > last_number) last_number = j
					records_to_cut[j] = ""
					}
				}
			}
		}
	#
	# If there is only one array element, it must be a dollar sign
	#
	else if (arrayb[1] != "$")
		{
		print "yank: ERROR: Invalid range given: " current_string \
			 > stderr
		exit
		}
	else
		{
		if (last_number != no_of_lines) 
			last_number = no_of_lines
		records_to_cut[no_of_lines] = ""
		}
	}

##################################################################
##		 	Function Separate_number_range
#	This function separates a number range that is passed to it
#
function separate_number_range(current_string,    j,arrayc,num2)
	{
	#
	# Place each number in a separate array element
	#
	split(current_string,arrayc,"-")
	#
	# There cannot be more than two array elements!
	#
	if ("3" in arrayc)
		{
		print "yank: ERROR: Invalid range given: " \
			  current_string > stderr
		exit
		}
	#
	# Both of the elements must be valid numbers with 1st less than 2nd
	#
	else if (arrayc[1] !~ NUMBER || arrayc[2] !~ NUMBER \
				|| arrayc[1]+0 > arrayc[2]+0)
		{
		print "yank: ERROR: Invalid range given: " \
			  current_string > stderr
		exit
		}
	#
	# Otherwise create the appropriate array subsripts and update the
	# variable last_number if necessary
	#
	else
		{
		if (arrayc[2]+0 > no_of_lines) num2 = no_of_lines
		else num2 = arrayc[2]+0
		if (arrayc[1]+0 <= no_of_lines)
			{
			for (j = arrayc[1]+0; j <= num2; j++)
				{
				if (j > last_number) last_number = j
				records_to_cut[j] = ""
				}
			}
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
##				Function Process_indicators
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

########################################################################
##			Helper Function Print_records_to_cut
#
function print_records_to_cut(   j)
	{
	print "Array Records to cut"
	for (j in records_to_cut) print j
	print "End array records to cut"
	}
